#include "CollisionManager.h"

#include <algorithm>
#include <variant>

#include "System/Scene/Collider/Collider.h"
#include "System/Scene/GameObject/GameObject.h"

void CollisionManager::Register(CollisionBody* body) {
	bodies_.push_back(body);
}

void CollisionManager::Unregister(CollisionBody* body) {
	std::erase(bodies_, body);

	// 接触中だった相手へ Exit を積む。配送前に呼べば、宙に浮いたポインタを触らずに済む.
	// まだ Enter を配っていない接触は、相手から見れば始まってもいないので Exit も送らない.
	for (const BodyPair& pair : previousPairs_) {
		if (pair.first != body && pair.second != body) {
			continue;
		}
		if (HasUndeliveredEnter(pair)) {
			continue;
		}
		CollisionBody* partner = (pair.first == body) ? pair.second : pair.first;
		PendingContact contact{};
		contact.type = ContactType::Exit;
		contact.a = partner;
		contact.ownerA = partner->GetOwner();
		pendingContacts_.push_back(contact);
	}

	// 破棄されたボディを含むペアを消しておかないと、次フレームに宙に浮いたポインタを触る.
	std::erase_if(previousPairs_, [body](const BodyPair& pair) {
		return pair.first == body || pair.second == body;
	});
	std::erase_if(currentPairs_, [body](const BodyPair& pair) {
		return pair.first == body || pair.second == body;
	});

	// 配送待ちの通知からも外す。添字がずれないよう、消さずに nullptr にする.
	for (PendingContact& contact : pendingContacts_) {
		if (contact.a == body) {
			contact.a = nullptr;
		}
		if (contact.b == body) {
			contact.b = nullptr;
		}
	}
}

CollisionManager::BodyPair CollisionManager::MakePair(CollisionBody* a, CollisionBody* b) {
	// 生ポインタの大小比較は std::less を通す.
	if (std::less<CollisionBody*>{}(b, a)) {
		return {b, a};
	}
	return {a, b};
}

void CollisionManager::Update() {
	currentPairs_.clear();

	// ワールド形状はフレームに1回だけ作り直す.
	for (CollisionBody* body : bodies_) {
		body->UpdateWorldShapes(body->GetOwner()->GetWorldMatrix());
	}

	// 広域判定は総当たり。ペアは一度だけ評価する.
	// ここではコールバックを呼ばないので、bodies_ が途中で増減することはない.
	for (size_t i = 0; i < bodies_.size(); ++i) {
		for (size_t j = i + 1; j < bodies_.size(); ++j) {
			TestPair(bodies_[i], bodies_[j]);
		}
	}

	// 今フレーム離れたペアに Exit を積んでから、差分の基準を入れ替える.
	QueueSeparatedExits();
	previousPairs_.swap(currentPairs_);

	// 判定がすべて終わってから通知を配る.
	DispatchContacts();
}

RaycastResult CollisionManager::Raycast(
	const Cake::Vector3& origin, const Cake::Vector3& direction, float maxDistance,
	uint32_t layerMask, const GameObject* ignoreOwner
) {

	RaycastResult result;
	const Cake::Vector3 rayDirection = Cake::Vector3::Normalize(direction);
	if (rayDirection == Cake::Vector3::Zero || maxDistance <= 0.0f || layerMask == 0) {
		return result;
	}

	// 見つかるたびに探索距離を詰めていくので、最も手前の1つだけが残る.
	// ワールド形状は直近の Update() 時点のもの。Update() の中から撃つと1フレーム前の位置を見る.
	float nearestDistance = maxDistance;
	for (CollisionBody* body : bodies_) {
		if (!body->IsEnabled() || body->GetOwner()->IsDeleted()) {
			continue;
		}
		if ((ToMask(body->GetLayer()) & layerMask) == 0) {
			continue;
		}
		if (ignoreOwner != nullptr && body->GetOwner() == ignoreOwner) {
			continue;
		}

		for (const Collider& collider : body->GetColliders()) {
			const Cake::RaycastHit3D hit = std::visit(
				[&origin, &rayDirection, nearestDistance](const auto& shape) {
					return Cake::Raycast(origin, rayDirection, nearestDistance, shape);
				},
				collider.GetWorldShape()
			);
			if (!hit) {
				continue;
			}

			nearestDistance = hit.distance;
			result.isHit = true;
			result.distance = hit.distance;
			result.point = hit.point;
			result.normal = hit.normal;
			result.body = body;
		}
	}
	return result;
}

void CollisionManager::TestPair(CollisionBody* a, CollisionBody* b) {
	// Delete() されたオブジェクトは、消える前のフレームでも当たらない.
	// 「当たって消えた弾が、同じフレームでもう1体に当たる」のを防ぐ.
	if (a->GetOwner()->IsDeleted() || b->GetOwner()->IsDeleted()) {
		return;
	}
	// 属性による絞り込み。形状には一切触れないので非常に安い.
	if (!a->CanCollideWith(*b)) {
		return;
	}
	// 包む矩形が離れていれば、狭域判定に入る必要はない.
	if (!Cake::Collide(a->GetBounds(), b->GetBounds())) {
		return;
	}

	// 複合形状のうち、最も深くめり込んでいるものを代表として扱う.
	Cake::CollisionInfo3D deepest;
	for (const Collider& colliderA : a->GetColliders()) {
		for (const Collider& colliderB : b->GetColliders()) {
			Cake::CollisionInfo3D info = std::visit(
				[](const auto& shapeA, const auto& shapeB) { return Cake::Collide(shapeA, shapeB); },
				colliderA.GetWorldShape(), colliderB.GetWorldShape()
			);
			if (info && info.depth > deepest.depth) {
				deepest = info;
			}
		}
	}
	if (!deepest) {
		return;
	}

	// どちらかがトリガーなら押し出さず、通知だけ行う.
	if (!a->IsTrigger() && !b->IsTrigger()) {
		Resolve(a, b, deepest);
	}

	BodyPair pair = MakePair(a, b);
	currentPairs_.insert(pair);

	// ここでは呼ばずに積むだけ。配るのは判定がすべて終わってから.
	PendingContact contact{};
	contact.type = previousPairs_.contains(pair) ? ContactType::Stay : ContactType::Enter;
	contact.a = a;
	contact.b = b;
	contact.ownerA = a->GetOwner();
	contact.ownerB = b->GetOwner();
	contact.info = deepest;
	pendingContacts_.push_back(contact);
}

void CollisionManager::Resolve(CollisionBody* a, CollisionBody* b, const Cake::CollisionInfo3D& info) {
	const Cake::Vector3 push = info.normal * info.depth;

	if (a->IsStatic()) {
		// 動かない相手にぶつかったので、b がすべて負担する.
		b->GetOwner()->AddWorldTranslate(-push);
	} else if (b->IsStatic()) {
		a->GetOwner()->AddWorldTranslate(push);
	} else {
		// どちらも動くなら半分ずつ引き離す.
		a->GetOwner()->AddWorldTranslate(push / 2.0f);
		b->GetOwner()->AddWorldTranslate(-push / 2.0f);
	}

	// 動かした2つだけ形状を作り直し、後続のペアに補正後の位置を見せる.
	a->UpdateWorldShapes(a->GetOwner()->GetWorldMatrix());
	b->UpdateWorldShapes(b->GetOwner()->GetWorldMatrix());
}

void CollisionManager::QueueSeparatedExits() {
	for (const BodyPair& pair : previousPairs_) {
		if (currentPairs_.contains(pair)) {
			continue;
		}
		// 離れた通知に押し出し情報は無いので、info は既定値のまま渡す.
		PendingContact contact{};
		contact.type = ContactType::Exit;
		contact.a = pair.first;
		contact.b = pair.second;
		contact.ownerA = pair.first->GetOwner();
		contact.ownerB = pair.second->GetOwner();
		pendingContacts_.push_back(contact);
	}
}

bool CollisionManager::QueueExitsForDeletedOwners() {
	bool queued = false;
	for (auto it = previousPairs_.begin(); it != previousPairs_.end();) {
		GameObject* ownerA = it->first->GetOwner();
		GameObject* ownerB = it->second->GetOwner();
		if (!ownerA->IsDeleted() && !ownerB->IsDeleted()) {
			++it;
			continue;
		}
		PendingContact contact{};
		contact.type = ContactType::Exit;
		contact.a = it->first;
		contact.b = it->second;
		contact.ownerA = ownerA;
		contact.ownerB = ownerB;
		pendingContacts_.push_back(contact);
		it = previousPairs_.erase(it);
		queued = true;
	}
	return queued;
}

bool CollisionManager::HasUndeliveredEnter(const BodyPair& pair) const {
	// 配送中でなければ、Enter は積まれていない.
	if (!isDispatching_) {
		return false;
	}
	for (size_t i = dispatchIndex_ + 1; i < pendingContacts_.size(); ++i) {
		const PendingContact& contact = pendingContacts_[i];
		if (contact.type != ContactType::Enter || contact.a == nullptr || contact.b == nullptr) {
			continue;
		}
		if (MakePair(contact.a, contact.b) == pair) {
			return true;
		}
	}
	return false;
}

void CollisionManager::DispatchContacts() {
	isDispatching_ = true;
	dispatchIndex_ = 0;
	for (;;) {
		// コールバックの中で Unregister() が Exit を積むことがあるので、要素数は毎回読み直す.
		for (; dispatchIndex_ < pendingContacts_.size(); ++dispatchIndex_) {
			DispatchContact(dispatchIndex_);
		}
		// コールバックの中で Delete() されたオブジェクトの接触を、まだ生きているうちに閉じる.
		if (!QueueExitsForDeletedOwners()) {
			break;
		}
	}
	pendingContacts_.clear();
	isDispatching_ = false;
	dispatchIndex_ = 0;
}

void CollisionManager::DispatchContact(size_t index) {
	// コールバックの中で push_back されると参照が無効になるので、値で受け取る.
	const PendingContact contact = pendingContacts_[index];

	if (contact.type != ContactType::Exit) {
		// 配送を待つ間に、どちらかが破棄・削除された接触は成立させない.
		const bool isBroken = contact.a == nullptr || contact.b == nullptr || contact.ownerA->IsDeleted() || contact.ownerB->IsDeleted();
		if (isBroken) {
			if (contact.type == ContactType::Enter && contact.a != nullptr && contact.b != nullptr) {
				// Enter を配っていないので、後で Exit が出ないよう接触の記録も消す.
				previousPairs_.erase(MakePair(contact.a, contact.b));
			}
			// Stay の場合は記録を残し、QueueExitsForDeletedOwners() か Unregister() に Exit を任せる.
			return;
		}
	}

	auto notify = [type = contact.type](GameObject* self, const GameObject::CollisionEvent3D& event) {
		switch (type) {
			case ContactType::Enter:
				self->OnCollisionEnter(event);
				break;
			case ContactType::Stay:
				self->OnCollisionStay(event);
				break;
			case ContactType::Exit:
				self->OnCollisionExit(event);
				break;
		}
	};

	if (contact.a != nullptr) {
		notify(contact.ownerA, {contact.ownerB, contact.a, contact.b, contact.info});
	}

	// a のコールバックの中でどちらかが破棄されたかもしれないので、配列から読み直す.
	CollisionBody* const bodyA = pendingContacts_[index].a;
	CollisionBody* const bodyB = pendingContacts_[index].b;
	if (bodyB != nullptr) {
		// 相手から見た法線は向きが逆になる.
		Cake::CollisionInfo3D reversed = contact.info;
		reversed.normal = -reversed.normal;
		notify(contact.ownerB, {contact.ownerA, bodyB, bodyA, reversed});
	}
}



#ifdef USE_IMGUI
#include <imgui.h>
#include <unordered_set>

#include "System/Render/Camera/Camera.h"
#include "System/Render/Debug/DebugDraw.h"

namespace {

uint32_t ToLayerColor(CollisionLayer layer) {
	switch (layer) {
		case CollisionLayer::Terrain:
			return Cake::DebugDraw::kColorWhite;
		case CollisionLayer::Player:
			return Cake::DebugDraw::kColorCyan;
		case CollisionLayer::Enemy:
			return Cake::DebugDraw::kColorMagenta;
		case CollisionLayer::PlayerAttack:
			return Cake::DebugDraw::kColorYellow;
		case CollisionLayer::EnemyAttack:
			return Cake::DebugDraw::kColorOrange;
		default:
			return Cake::DebugDraw::kColorGreen;
	}
}

const char* ToLayerName(CollisionLayer layer) {
	switch (layer) {
		case CollisionLayer::Terrain:
			return "Terrain";
		case CollisionLayer::Player:
			return "Player";
		case CollisionLayer::Enemy:
			return "Enemy";
		case CollisionLayer::PlayerAttack:
			return "PlayerAttack";
		case CollisionLayer::EnemyAttack:
			return "EnemyAttack";
		default:
			return "Default";
	}
}

} // namespace

void CollisionManager::DrawDebugShapes(const Camera& camera) {
	if (!debugDrawShapes_) {
		return;
	}

	// 注意: Update() の末尾で previousPairs_.swap(currentPairs_) を済ませているため、
	// 「今フレーム接触しているペア」が入っているのは previousPairs_ のほう.
	std::unordered_set<const CollisionBody*> hitBodies;
	for (const BodyPair& pair : previousPairs_) {
		hitBodies.insert(pair.first);
		hitBodies.insert(pair.second);
	}

	for (const CollisionBody* body : bodies_) {
		if ((ToMask(body->GetLayer()) & debugLayerMask_) == 0) {
			continue;
		}

		const bool isHit = hitBodies.contains(body);
		// 接触中は赤、それ以外は属性ごとの色にする.
		uint32_t color = isHit ? Cake::DebugDraw::kColorRed : ToLayerColor(body->GetLayer());
		if (!body->IsEnabled()) {
			// 判定から外れているものは薄く出す。消すと「消えたのか元から無いのか」がわからない.
			color = Cake::DebugDraw::WithAlpha(color, 0.3f);
		} else if (body->IsTrigger()) {
			// トリガーは半透明にして、押し出す判定と見分けられるようにする.
			color = Cake::DebugDraw::WithAlpha(color, 0.6f);
		}

		if (debugDrawBounds_) {
			Cake::DebugDraw::Draw(camera, body->GetBounds(), Cake::DebugDraw::WithAlpha(Cake::DebugDraw::kColorWhite, 0.3f));
		}

		for (const Collider& collider : body->GetColliders()) {
			std::visit([&](const auto& shape) {
				Cake::DebugDraw::Draw(camera, shape, color, isHit ? 2.0f : 1.0f);
			},
			           collider.GetWorldShape());
		}

		if (debugDrawLabels_) {
			Cake::DebugDraw::Text(camera, body->GetBounds().max, color, ToLayerName(body->GetLayer()));
		}
	}
}

void CollisionManager::DrawDebugUI(const char* windowName) {
	if (!ImGui::Begin(windowName)) {
		ImGui::End();
		return;
	}

	if (ImGui::CollapsingHeader("Debug Draw", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Checkbox("Draw Shapes", &debugDrawShapes_);
		ImGui::Checkbox("Draw Broad Phase Bounds", &debugDrawBounds_);
		ImGui::Checkbox("Draw Labels", &debugDrawLabels_);

		ImGui::Separator();
		// 見たい属性だけに絞れると、判定が密集していても追える.
		constexpr CollisionLayer kLayers[] = {
			CollisionLayer::Default, CollisionLayer::Terrain, CollisionLayer::Player,
			CollisionLayer::Enemy, CollisionLayer::PlayerAttack, CollisionLayer::EnemyAttack
		};
		for (CollisionLayer layer : kLayers) {
			bool visible = (debugLayerMask_ & ToMask(layer)) != 0;
			if (ImGui::Checkbox(ToLayerName(layer), &visible)) {
				debugLayerMask_ = visible ? (debugLayerMask_ | ToMask(layer)) : (debugLayerMask_ & ~ToMask(layer));
			}
		}
	}

	if (ImGui::CollapsingHeader("Status", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("Bodies: %d", static_cast<int>(bodies_.size()));
		ImGui::Text("Contacts: %d", static_cast<int>(previousPairs_.size()));
		for (const BodyPair& pair : previousPairs_) {
			ImGui::BulletText("%s <-> %s", ToLayerName(pair.first->GetLayer()), ToLayerName(pair.second->GetLayer()));
		}
	}

	ImGui::End();
}
#endif // USE_IMGUI

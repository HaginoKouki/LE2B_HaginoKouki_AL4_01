#include "SceneBase.h"

#include "System/Platform/Time/Time.h"

SceneBase::~SceneBase() {
	// CollisionBody の登録解除より先に CollisionManager が消えないよう、
	// マネージャとカメラが生きているうちにオブジェクトを破棄しておく.
	// 注意: 派生シーンのメンバはここに来る前に破棄済み。GameObject のデストラクタから触らないこと.
	gameObjects_.clear();
	pendingAdd_.clear();
}

GameObject* SceneBase::AddGameObject(std::unique_ptr<GameObject> object) {
	GameObject* raw = object.get();
	raw->SetScene(this);
	raw->Initialize();
	pendingAdd_.push_back(std::move(object));
	needsSort_ = true;
	return raw;
}

void SceneBase::UpdateGameObjects(Cake::Time* time) {
	// ループを回す前に、追加分を反映する.
	if (!pendingAdd_.empty()) {
		// 有効になる前に Delete() されたもの（親ごと消えた子など）は、1度も Update() せずに捨てる.
		// 移さずに残ったものは activated.clear() で破棄され、デストラクタが親子関係と当たり判定の登録を外す.
		std::vector<std::unique_ptr<GameObject>> activated;
		activated.swap(pendingAdd_);
		for (auto& object : activated) {
			if (!object->IsDeleted()) {
				gameObjects_.push_back(std::move(object));
			}
		}
		activated.clear();
		needsSort_ = true;
	}
	SortIfNeeded();

	// 親を持たないオブジェクト(ルート)だけを updateQueue_ 順に処理し、
	// その中で子へ降りていく.
	for (const auto& object : gameObjects_) {
		if (object->GetParent() == nullptr) {
			UpdateRecursive(object.get(), time);
		}
	}

	// 全 Update() の後、削除掃除の前に衝突を解決する.
	collisionManager_.Update();

	// ループを抜けてから、削除フラグの立ったオブジェクトを消す.
	// 削除フラグを子孫へ伝播させる.
	for (const std::unique_ptr<GameObject>& object : gameObjects_) {
		if (object->IsDeleted()) {
			PropagateDelete(object.get(), false);
		}
	}
	std::erase_if(gameObjects_, [](const std::unique_ptr<GameObject>& object) {
		return object->IsDeleted();
	});
}
void SceneBase::UpdateRecursive(GameObject* object, Cake::Time* time) {
	// Update() の中で生成・親子付けされた子は、まだ保留中。次のフレームから更新する.
	// その子にぶら下がる有効なオブジェクトがあり得るので、降りていくのは止めない.
	if (!IsPending(object)) {
		object->Update(time);
	}

	auto children = object->GetChildren(); // コピーして並べ替える.
	std::stable_sort(children.begin(), children.end(), [](const GameObject* a, const GameObject* b) {
		return a->GetUpdateQueue() < b->GetUpdateQueue();
	});

	for (GameObject* child : children) {
		UpdateRecursive(child, time);
	}
}
bool SceneBase::IsPending(const GameObject* object) const {
	return std::any_of(pendingAdd_.begin(), pendingAdd_.end(), [object](const std::unique_ptr<GameObject>& pending) {
		return pending.get() == object;
	});
}

void SceneBase::PropagateDelete(GameObject* object, bool parentDeleted) {
	if (parentDeleted) {
		object->Delete();
	}

	// 自分が消えるなら、子も消える.
	const bool deleted = object->IsDeleted();
	for (GameObject* child : object->GetChildren()) {
		PropagateDelete(child, deleted);
	}
}

void SceneBase::DrawGameObjects() {
	// カメラとの距離はオブジェクトが動くたびに変わるので、毎フレーム作り直してソートする.
	// 比較のたびに GetWorldPosition() を呼ぶと親を何度も辿るので、先に1回だけ求めておく.
	// デバッグカメラ中は、そちらの視点から見た遠近で並べる.
	const Cake::Vector3 eyePosition = camera_.GetRenderEyePosition();
	drawList_.clear();
	for (const std::unique_ptr<GameObject>& object : gameObjects_) {
		const Cake::Vector3 toObject = object->GetWorldPosition() - eyePosition;
		drawList_.push_back({object.get(), GetDrawPriority(object->GetLayer()), Cake::Vector3::DotProduct(toObject, toObject)});
	}

	std::stable_sort(drawList_.begin(), drawList_.end(), [](const DrawEntry& a, const DrawEntry& b) {
		if (a.priority != b.priority) {
			return a.priority < b.priority;
		}
		// 遠いものから描く.
		return a.sortKey > b.sortKey;
	});

	for (const DrawEntry& entry : drawList_) {
		entry.object->Draw();
	}
}

void SceneBase::BuildSpriteDrawList() {
	// spriteOrder_ は Update() の中で変わり得るので、毎フレーム作り直してソートする.
	drawList_.clear();
	for (const std::unique_ptr<GameObject>& object : gameObjects_) {
		drawList_.push_back({object.get(), GetDrawPriority(object->GetLayer()), object->GetSpriteOrder()});
	}

	std::stable_sort(drawList_.begin(), drawList_.end(), [](const DrawEntry& a, const DrawEntry& b) {
		if (a.priority != b.priority) {
			return a.priority < b.priority;
		}
		return a.sortKey < b.sortKey;
	});
}

void SceneBase::DrawGameObjectsBackground() {
	BuildSpriteDrawList();
	for (const DrawEntry& entry : drawList_) {
		entry.object->DrawBackground();
	}
}

void SceneBase::DrawGameObjectsUI() {
	BuildSpriteDrawList();
	for (const DrawEntry& entry : drawList_) {
		entry.object->DrawUI();
	}
}

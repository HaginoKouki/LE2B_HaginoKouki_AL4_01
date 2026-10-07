#include "GameObject.h"

#include "System/Scene/Collider/CollisionManager.h"
#include "System/Scene/SceneBase.h"

GameObject::~GameObject() {
	// カメラが追従対象として握っていると、破棄後に宙に浮いたポインタを触る.
	// ~SceneBase() はカメラより先にオブジェクトを破棄するので、ここではまだカメラが生きている.
	if (scene_) {
		scene_->GetCamera().GetCameraFollow().Forget(this);
	}

	if (parent_) {
		std::erase(parent_->children_, this);
	}
	for (GameObject* child : children_) {
		child->parent_ = nullptr;
	}
}

CollisionManager* GameObject::GetCollisionManager() const {
	return scene_ ? &scene_->GetCollisionManager() : nullptr;
}
Camera* GameObject::GetCamera() const {
	return scene_ ? &scene_->GetCamera() : nullptr;
}

Cake::Matrix4x4 GameObject::GetWorldMatrix() const {
	const Cake::Matrix4x4 local = Cake::Math::MakeAffineMatrix(localTransform_);
	if (parent_ == nullptr) {
		return local; // 親がいなければローカル = ワールド.
	}
	// 行ベクトル形式なので、自分の変換を先に、親の変換を後に掛ける.
	return local * parent_->GetWorldMatrix();
}

Cake::Vector3 GameObject::GetWorldPosition() const {
	return Cake::Math::GetTranslate(GetWorldMatrix());
}

void GameObject::AddWorldTranslate(const Cake::Vector3& worldDelta) {
	if (parent_ == nullptr) {
		localTransform_.translate += worldDelta;
		return;
	}
	// 親のワールド行列の回転と拡縮を打ち消して、ローカル空間の移動量へ直す.
	// 拡縮が 0 の親では逆行列が作れず（Inverse() は Zero を返す）、移動量も 0 になる.
	const Cake::Matrix4x4 inverseParent = Cake::Matrix4x4::Inverse(parent_->GetWorldMatrix());
	localTransform_.translate += Cake::Vector3::TransformNormal(worldDelta, inverseParent);
}

void GameObject::SetParent(GameObject* parent) {
	for (GameObject* p = parent; p != nullptr; p = p->parent_) {
		if (p == this) {
			assert(false && "親子間で循環参照が発生した");
			return; // Release でも安全に無視する.
		}
	}
	// 既存の親から外す.
	if (parent_) {
		std::erase(parent_->children_, this);
	}
	parent_ = parent;
	if (parent_) {
		parent_->children_.push_back(this);
	}
}

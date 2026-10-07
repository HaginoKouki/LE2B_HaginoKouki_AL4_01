#pragma once
/*====================================
 *
 * GameObject に取り付ける衝突判定コンポーネント。
 * 図形そのものと判定式は Foundation/Math 側（Collision3D）に置き、ここでは
 * オーナーのワールド行列を使ってワールド空間の形状を組み立てる役に徹する。
 *
 * ====================================*/
#include <vector>
#include <variant>
#include <cstdint>

#include "System/Foundation/Math/Geometry.h"
#include "System/Foundation/Math/Transform.h"
#include "System/Scene/Collider/CollisionLayer.h"

class GameObject;
class CollisionManager;

// 対応する形状。追加したら Cake::Collide() と Cake::GetBounds() のオーバーロードも足す.
using Shape3D = std::variant<Cake::Sphere, Cake::AABB, Cake::OBB>;

// 単一の形状。オーナーの原点を基準としたローカル座標で定義する.
// 位置をずらしたい場合は形状自身の中心を動かす（例: Cake::Sphere{{0.0f, 1.0f, 0.0f}, 0.5f}）.
class Collider {
private:
	Shape3D localShape_;
	// 直前に計算したワールド空間の形状。UpdateWorldShape() で作り直す.
	Shape3D worldShape_;

public:
	explicit Collider(const Shape3D& localShape) : localShape_(localShape), worldShape_(localShape) {}

	// オーナーの姿勢を反映してワールド形状を作り直す。フレームに1回だけ呼ぶ.
	// AABB は回転が入ると軸平行で表現できないため、OBB へ格上げされる.
	// 球は非一様スケールを表現できないので、最も大きい軸の倍率で膨らませる.
	void UpdateWorldShape(const Cake::Matrix4x4& worldMatrix);

	const Shape3D& GetWorldShape() const { return worldShape_; }

	// 広域判定用に、ワールド形状を包む軸平行箱を返す.
	Cake::AABB GetWorldBounds() const;
};

class CollisionBody {
private:
	GameObject* owner_ = nullptr;
	CollisionManager* manager_ = nullptr;
	std::vector<Collider> colliders_;
	// オーナー基準で判定だけをずらすためのローカル変換。
	// 手や武器など、本体とは別の位置にある判定に使用する。
	Cake::Transform3 localTransform_{};

	// 自分が何であるか.
	CollisionLayer layer_ = CollisionLayer::Default;
	// どの属性とぶつかるか.
	uint32_t mask_ = kCollisionMaskAll;

	// true なら押し出さず通知のみ行う.
	bool isTrigger_ = false;
	// true なら押し出されない。壁や地形に使う.
	bool isStatic_ = false;
	// false の間は判定から完全に外れる。無敵時間などに使う.
	bool isEnabled_ = true;

	// colliders_ 全体を包む箱。狭域判定の前段で使う.
	Cake::AABB bounds_{};

public:
	// 生成しただけでは判定に参加しない。AttachCollisionManager() で登録する.
	// 破棄と同時に登録は解除される.
	explicit CollisionBody(GameObject* owner) : owner_(owner) {}
	~CollisionBody() { DetachManager(); }

	// CollisionManager が生ポインタで保持するため、コピーも移動も禁止する.
	CollisionBody(const CollisionBody&) = delete;
	CollisionBody& operator=(const CollisionBody&) = delete;
	CollisionBody(CollisionBody&&) = delete;
	CollisionBody& operator=(CollisionBody&&) = delete;

	void AttachCollisionManager(CollisionManager* manager);
	void DetachManager();

	void AddCollider(const Shape3D& localShape) { colliders_.emplace_back(localShape); }
	void ClearColliders() { colliders_.clear(); }

	void SetLayer(CollisionLayer layer) { layer_ = layer; }
	void SetMask(uint32_t mask) { mask_ = mask; }
	void SetTrigger(bool isTrigger) { isTrigger_ = isTrigger; }
	void SetStatic(bool isStatic) { isStatic_ = isStatic; }
	void SetEnabled(bool isEnabled) { isEnabled_ = isEnabled; }
	void SetLocalTransform(const Cake::Transform3& transform) { localTransform_ = transform; }

	GameObject* GetOwner() const { return owner_; }
	CollisionLayer GetLayer() const { return layer_; }
	uint32_t GetMask() const { return mask_; }
	bool IsTrigger() const { return isTrigger_; }
	bool IsStatic() const { return isStatic_; }
	bool IsEnabled() const { return isEnabled_; }

	const std::vector<Collider>& GetColliders() const { return colliders_; }
	const Cake::AABB& GetBounds() const { return bounds_; }

	// フレーム先頭で、オーナーと判定固有の変換からワールド形状と bounds_ を作り直す.
	void UpdateWorldShapes(const Cake::Matrix4x4& ownerWorldMatrix);

	// 狭域判定に入る価値があるかを調べる。形状には触れず、属性だけで絞り込む.
	bool CanCollideWith(const CollisionBody& other) const;
};

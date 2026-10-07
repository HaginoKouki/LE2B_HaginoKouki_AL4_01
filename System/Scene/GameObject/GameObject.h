#pragma once
/*====================================
 *
 * ゲームシーン内のオブジェクトの基底クラス。
 *
 * 【姿勢と親子】
 * 自分の姿勢は localTransform_（Transform3）で持ち、ワールドでの姿勢は
 * GetWorldMatrix() が「自分のローカル行列 × 親のワールド行列」で求める.
 *
 * 【描画は3段階】KamataEngine の「背景スプライト → 3D → 前景スプライト」の順に合わせてある.
 * DrawBackground() : 2D。3D より奥に敷くスプライト。描き終えた後に深度バッファを消すので、
 *                    3D のモデルは必ずこの上に描かれる。不要なら書かなくてよい.
 * Draw()           : 3D。Model::PreDraw()～PostDraw() の内側で呼ばれる。ModelRenderer で描く.
 * DrawUI()         : 2D。3D の上に重ねるスプライト。HP バーなどを SpriteRenderer で描く。不要なら書かなくてよい.
 *
 * ====================================*/
#include <cstdint>
#include <vector>
#include <cassert>
#include "System/Foundation/Math/Transform.h"
#include "System/Foundation/Math/Collision3D.h"

class SceneBase;
class CollisionBody;
class CollisionManager;
class Camera;
namespace Cake {
class Time;
}

inline constexpr int kUpdateQueueEarly = 1000;
inline constexpr int kUpdateQueueBase = 3000;
inline constexpr int kUpdateQueueLate = 5000;

inline constexpr float kSpriteOrderDefault = 0.0f;

// ゲームオブジェクトの属性を表すフラグ。描画順の決定(GetDrawPriority)に使う.
// 更新順は属性からは決めず、updateQueue_ に kUpdateQueue〇〇 を直接入れて指定する.
enum class GameObjectLayer : uint32_t {
	Default = 1 << 0,
	Background = 1 << 1,
	UI = 1 << 2,
	Player = 1 << 3,
	Enemy = 1 << 4,
};
// enum class はビット演算子を持たないので定義する.
inline constexpr GameObjectLayer operator|(GameObjectLayer a, GameObjectLayer b) {
	return static_cast<GameObjectLayer>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline constexpr GameObjectLayer operator&(GameObjectLayer a, GameObjectLayer b) {
	return static_cast<GameObjectLayer>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
inline constexpr GameObjectLayer& operator|=(GameObjectLayer& a, GameObjectLayer b) {
	a = a | b;
	return a;
}

// フラグが立っているか調べる.
inline constexpr bool HasLayer(GameObjectLayer value, GameObjectLayer flag) {
	return (value & flag) != static_cast<GameObjectLayer>(0);
}

// 描画優先度。値が大きいほど後に描かれる。3段階それぞれの中で使う.
// 3D の Draw() では、同じ優先度の中はカメラから遠い順（半透明が正しく重なる順）に並ぶ.
// 2D の DrawBackground() / DrawUI() では、同じ優先度の中は spriteOrder_ の小さい順に並ぶ.
// フラグを複数持つ場合は、上に書いた判定が優先される.
inline int GetDrawPriority(GameObjectLayer layer) {
	if (HasLayer(layer, GameObjectLayer::UI)) {
		return 500;
	}
	if (HasLayer(layer, GameObjectLayer::Background)) {
		return 0;
	}
	return 250;
}

class GameObject {
protected:
	SceneBase* scene_ = nullptr;

	Cake::Transform3 localTransform_;
	GameObjectLayer layer_ = GameObjectLayer::Default;

	// 更新順序。値が小さいほど先に更新される.
	int updateQueue_ = kUpdateQueueBase;
	// DrawBackground() / DrawUI() の順序。値が小さいほど先に（奥に）描画される.
	// 3D の Draw() は深度バッファで前後が決まるので、これは使わない.
	float spriteOrder_ = kSpriteOrderDefault;

	// 削除フラグ。true の場合、更新処理の最後にシーンから削除される.
	bool isDeleted_ = false;

	GameObject* parent_ = nullptr;
	std::vector<GameObject*> children_;

public:
	GameObject() = default;
	virtual ~GameObject();

	// parent_ / children_ を生ポインタで持つので、複製すると親子関係が壊れる.
	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;
	GameObject(GameObject&&) = delete;
	GameObject& operator=(GameObject&&) = delete;

	virtual void Initialize() = 0;
	virtual void Update(Cake::Time*) = 0;
	// 2D 描画（3D より奥）。Sprite::PreDraw()～PostDraw() の内側で、3D より先に呼ばれる.
	virtual void DrawBackground() {}
	// 3D 描画。Model::PreDraw()～PostDraw() の内側で呼ばれる.
	virtual void Draw() = 0;
	// 2D 描画（3D より手前）。Sprite::PreDraw()～PostDraw() の内側で、3D の後に呼ばれる.
	virtual void DrawUI() {}

	void SetScene(SceneBase* scene) { scene_ = scene; }
	CollisionManager* GetCollisionManager() const;
	Camera* GetCamera() const;

	/// <summary>
	/// 親の変換を適用したワールド行列を取得する.
	/// </summary>
	Cake::Matrix4x4 GetWorldMatrix() const;
	/// <summary>
	/// ワールド座標での位置。GetWorldMatrix() の平行移動成分.
	/// </summary>
	Cake::Vector3 GetWorldPosition() const;

	const Cake::Transform3& GetLocalTransform() const { return localTransform_; }

	// 押し出しでマネージャが位置を補正するために使う.
	void AddWorldTranslate(const Cake::Vector3& worldDelta);

	GameObjectLayer GetLayer() const { return layer_; }
	int GetUpdateQueue() const { return updateQueue_; }
	float GetSpriteOrder() const { return spriteOrder_; }

	bool IsDeleted() const { return isDeleted_; }
	void Delete() { isDeleted_ = true; }

	GameObject* GetParent() const { return parent_; }
	const std::vector<GameObject*>& GetChildren() const { return children_; }
	void SetParent(GameObject* parent);

	// 衝突通知に渡す情報。どのボディ同士が当たったかを含む.
	// GameObject 単位の通知だけでは、本体と攻撃判定のように
	// 同じオーナーが複数のボディを持つ場合に見分けがつかないため.
	//
	// 【nullptr になる場合がある】
	// 相手の CollisionBody が先に破棄された場合の Exit では other / otherBody が nullptr になる.
	// 自分のコールバックの中で相手のボディが破棄された場合も、otherBody は nullptr になる.
	struct CollisionEvent3D {
		GameObject* other = nullptr;        //!< 相手のオブジェクト。nullptr があり得る.
		CollisionBody* selfBody = nullptr;  //!< 自分のどのボディが当たったか.
		CollisionBody* otherBody = nullptr; //!< 相手のどのボディに当たったか。nullptr があり得る.
		Cake::CollisionInfo3D info;         //!< normal は自分を相手から引き離す向き.
	};

	// 衝突時に呼ばれる。info.normal は自分を相手から引き離す向き.
	// 判定がすべて終わってから呼ばれるので、中で Delete() や CollisionBody の破棄・生成をしてよい.
	virtual void OnCollisionEnter(const CollisionEvent3D&) {}
	virtual void OnCollisionStay(const CollisionEvent3D&) {}
	virtual void OnCollisionExit(const CollisionEvent3D&) {}

};

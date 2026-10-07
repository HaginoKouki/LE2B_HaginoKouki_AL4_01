#pragma once
/*====================================
 *
 * シーン内の CollisionBody を集めて、1フレームぶんの衝突処理を回すクラス。
 * ペアの生成、レイヤーによる絞り込み、押し出し、
 * Enter / Stay / Exit の配送までを担当する。
 *
 * 【通知は判定がすべて終わってから配る】
 * 判定ループの途中でコールバックを呼ぶと、コールバック内でボディを破棄
 * （攻撃判定の reset() など）した瞬間に、ループが握っているポインタが宙に浮く。
 * 判定中は通知を pendingContacts_ に積むだけにして、最後にまとめて配る。
 * 配送中に破棄されたボディは Unregister() が配送待ちの列から外すので、
 * コールバックの中で CollisionBody を破棄・生成してよい。
 *
 * 【Delete() 済みのオブジェクトは当たらない】
 * Delete() されたオブジェクトは、そのフレームの判定から外れる。
 * 接触中だった相手には、オブジェクトがまだ生きているうちに Exit が届く。
 *
 * 【相手のボディが先に破棄された場合の Exit】
 * CollisionBody を直接破棄した場合、接触中だった相手へは Exit が届くが、
 * other / otherBody は nullptr になる（破棄済みのものを指させないため）。
 *
 * ====================================*/
#include <cstdint>
#include <vector>
#include <unordered_set>
#include <utility>
#include <functional>

#include "System/Foundation/Math/Collision3D.h"
#include "System/Scene/Collider/CollisionLayer.h"

class Camera;
class CollisionBody;
class GameObject;

// Raycast() の結果。body 以降は isHit が true のときだけ意味を持つ.
struct RaycastResult {
	bool isHit = false;
	float distance = 0.0f;
	Cake::Vector3 point = Cake::Vector3::Zero;
	Cake::Vector3 normal = Cake::Vector3::Zero;
	CollisionBody* body = nullptr; //!< 当たったボディ。所有権は持たない.

	explicit operator bool() const { return isHit; }
};

class CollisionManager {
private:
	using BodyPair = std::pair<CollisionBody*, CollisionBody*>;

	struct BodyPairHash {
		size_t operator()(const BodyPair& pair) const {
			size_t first = std::hash<const void*>{}(pair.first);
			size_t second = std::hash<const void*>{}(pair.second);
			return first ^ (second + 0x9e3779b9 + (first << 6) + (first >> 2));
		}
	};

	enum class ContactType : uint8_t {
		Enter,
		Stay,
		Exit,
	};

	// 配送待ちの通知1件。a と b の両方へ配る.
	// 片側だけに配りたい場合（相手が破棄された Exit）は b を nullptr にする.
	struct PendingContact {
		ContactType type = ContactType::Enter;
		CollisionBody* a = nullptr;      //!< 配送待ちの間に破棄されたら nullptr になる.
		CollisionBody* b = nullptr;      //!< 同上.
		GameObject* ownerA = nullptr;    //!< Update() の間は生存が保証される.
		GameObject* ownerB = nullptr;    //!< 同上.
		Cake::CollisionInfo3D info{};    //!< normal は a を b から引き離す向き.
	};

	std::vector<CollisionBody*> bodies_;

	// 前フレームに接触していたペア。Enter / Exit の差分を取るために保持する.
	std::unordered_set<BodyPair, BodyPairHash> previousPairs_;
	std::unordered_set<BodyPair, BodyPairHash> currentPairs_;

	std::vector<PendingContact> pendingContacts_;
	// 配送中なら true。dispatchIndex_ は今配っている pendingContacts_ の添字.
	bool isDispatching_ = false;
	size_t dispatchIndex_ = 0;

public:
	CollisionManager() = default;
	// ボディが生ポインタで自分を指しているので、コピーも移動も禁止する.
	CollisionManager(const CollisionManager&) = delete;
	CollisionManager& operator=(const CollisionManager&) = delete;

	// CollisionBody の生成・破棄から呼ばれる.
	void Register(CollisionBody* body);
	void Unregister(CollisionBody* body);

	// すべての Update() の後、削除掃除の前に1回だけ呼ぶ.
	void Update();

	/// <summary>
	/// レイを飛ばし、最も手前で当たったボディを返す。
	/// layerMask に含まれる属性のボディだけを見る。
	/// マウスで指した物を調べる場合は Camera::ScreenPointToRay() の結果を渡す.
	/// </summary>
	/// <param name="direction">正規化されていなくてもよい。内部で正規化する</param>
	/// <param name="ignoreOwner">自分自身の判定を拾わないよう、撃った側を渡す</param>
	RaycastResult Raycast(
		const Cake::Vector3& origin, const Cake::Vector3& direction, float maxDistance,
		uint32_t layerMask = kCollisionMaskAll, const GameObject* ignoreOwner = nullptr
	);

private:
	// ポインタの順序を揃えて、A-B と B-A を同じキーにする.
	static BodyPair MakePair(CollisionBody* a, CollisionBody* b);

	void TestPair(CollisionBody* a, CollisionBody* b);
	void Resolve(CollisionBody* a, CollisionBody* b, const Cake::CollisionInfo3D& info);

	// 前フレームに接触していて今フレーム離れたペアの Exit を積む.
	void QueueSeparatedExits();
	// Delete() されたオブジェクトを含む接触を閉じ、Exit を積む。積んだら true.
	bool QueueExitsForDeletedOwners();
	// 積まれた通知をすべて配る.
	void DispatchContacts();
	void DispatchContact(size_t index);
	// まだ配っていない Enter のうち、pair に当たるものがあるか.
	bool HasUndeliveredEnter(const BodyPair& pair) const;

#ifdef USE_IMGUI
public:
	/// <summary>
	/// 全ボディのワールド形状を、ワイヤーフレームで画面に重ねて描く。
	/// Update() とカメラの Update() の後、つまりワールド形状とカメラが確定した後に呼ぶこと.
	/// </summary>
	void DrawDebugShapes(const Camera& camera);

	/// <summary>表示切り替え用のデバッグウィンドウ.</summary>
	void DrawDebugUI(const char* windowName = "Collision");
private:
	bool debugDrawShapes_ = true;
	bool debugDrawBounds_ = false; // 広域判定用の AABB も描くか.
	bool debugDrawLabels_ = false;
	uint32_t debugLayerMask_ = kCollisionMaskAll;
#endif // USE_IMGUI
};

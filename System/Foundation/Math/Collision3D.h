#pragma once
/*====================================
 *
 * 3Dの衝突判定。分離軸定理（SAT）にもとづき、衝突の有無に加えて
 * めり込みを解消する最小の押し出しベクトル（MTV）を返す。
 * IsCollision.h が「当たったか」だけを返すのに対し、こちらは押し出す向きと量まで求める。
 * ゲームオブジェクトには一切依存しない純粋な図形計算のみを行う。
 *
 * 【対応する図形】
 * Sphere / AABB / OBB の全ての組み合わせ。
 * AABB が絡む組み合わせのうち高速な専用処理が無いものは、回転0の OBB へ変換して委譲する.
 *
 * ====================================*/
#include "System/Foundation/Math/Vector.h"
#include "System/Foundation/Math/Geometry.h"

namespace Cake {

// 衝突結果。normal と depth は isHit が true のときだけ意味を持つ.
struct CollisionInfo3D {
	bool isHit = false;
	Vector3 normal = Vector3::Zero; //!< aをbから引き離す向き。正規化済み.
	float depth = 0.0f;             //!< めり込み量。aを normal * depth だけ動かすと接触状態になる.

	// if (Collide(a, b)) と書けるようにする.
	explicit operator bool() const { return isHit; }
};

/*---------------------------------
*
* 図形の変換・包含矩形
*
---------------------------------*/
// AABB を回転0の OBB として表す.
OBB ToOBB(const AABB& aabb);
// 中心と全体サイズから AABB を組み立てる.
AABB MakeAABB(const Vector3& center, const Vector3& size);

// 図形を包む軸平行矩形。広域判定に使う.
AABB GetBounds(const Sphere& shape);
AABB GetBounds(const AABB& shape);
AABB GetBounds(const OBB& shape);

/*---------------------------------
*
* 衝突判定。返る法線は常に「第1引数を第2引数から引き離す向き」.
*
---------------------------------*/
CollisionInfo3D Collide(const Sphere& a, const Sphere& b);
CollisionInfo3D Collide(const AABB& a, const AABB& b);
CollisionInfo3D Collide(const OBB& a, const OBB& b);
CollisionInfo3D Collide(const Sphere& a, const OBB& b);
CollisionInfo3D Collide(const OBB& a, const Sphere& b);

/* 回転0の OBB へ変換して委譲するもの */
CollisionInfo3D Collide(const Sphere& a, const AABB& b);
CollisionInfo3D Collide(const AABB& a, const Sphere& b);
CollisionInfo3D Collide(const OBB& a, const AABB& b);
CollisionInfo3D Collide(const AABB& a, const OBB& b);


// レイの当たり結果。distance 以降の値は isHit が true のときだけ意味を持つ.
struct RaycastHit3D {
	bool isHit = false;
	float distance = 0.0f;          //!< 始点から交点までの距離.
	Vector3 point = Vector3::Zero;  //!< 交点のワールド座標.
	Vector3 normal = Vector3::Zero; //!< 交点で外を向く面の法線。正規化済み.

	// if (Raycast(...)) と書けるようにする.
	explicit operator bool() const { return isHit; }
};

/*---------------------------------
*
* レイと図形の交差。direction は正規化済みであること.
* 始点が図形の内側にある場合は distance 0 で当たったものとして返す.
*
---------------------------------*/
RaycastHit3D Raycast(const Vector3& origin, const Vector3& direction, float distance, const Sphere& shape);
RaycastHit3D Raycast(const Vector3& origin, const Vector3& direction, float distance, const AABB& shape);
RaycastHit3D Raycast(const Vector3& origin, const Vector3& direction, float distance, const OBB& shape);

}	// namespace Cake

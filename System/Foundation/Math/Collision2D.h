#pragma once
/*====================================
 *
 * 2Dの衝突判定。分離軸定理（SAT）にもとづき、衝突の有無に加えて
 * めり込みを解消する最小の押し出しベクトル（MTV）を返す。
 * ゲームオブジェクトには一切依存しない純粋な図形計算のみを行う。
 *
 * ====================================*/
#include "System/Foundation/Math/Vector.h"
#include "System/Foundation/Math/Geometry2D.h"

namespace Cake {

// 衝突結果。normal と depth は isHit が true のときだけ意味を持つ.
struct CollisionInfo2D {
	bool isHit = false;
	Vector2 normal = Vector2::Zero;	//!< aをbから引き離す向き。正規化済み.
	float depth = 0.0f;	//!< めり込み量。aを normal * depth だけ動かすと接触状態になる.

	// if (Collide(a, b)) と書けるようにする.
	explicit operator bool() const { return isHit; }
};

/*---------------------------------
*
* 衝突判定。返る法線は常に「第1引数を第2引数から引き離す向き」.
*
---------------------------------*/
/* 高速な専用処理があるもの */
CollisionInfo2D Collide(const Circle2D& a, const Circle2D& b);
CollisionInfo2D Collide(const AABB2D& a, const AABB2D& b);

/* SATの本体. */
CollisionInfo2D Collide(const ConvexPolygon2D& a, const ConvexPolygon2D& b);
CollisionInfo2D Collide(const ConvexPolygon2D& a, const Circle2D& b);
CollisionInfo2D Collide(const Circle2D& a, const ConvexPolygon2D& b);

/* 凸多角形へ変換して本体へ委譲するもの */
CollisionInfo2D Collide(const AABB2D& a, const Circle2D& b);
CollisionInfo2D Collide(const Circle2D& a, const AABB2D& b);
CollisionInfo2D Collide(const OBB2D& a, const OBB2D& b);
CollisionInfo2D Collide(const OBB2D& a, const AABB2D& b);
CollisionInfo2D Collide(const AABB2D& a, const OBB2D& b);
CollisionInfo2D Collide(const OBB2D& a, const Circle2D& b);
CollisionInfo2D Collide(const Circle2D& a, const OBB2D& b);


// レイの当たり結果。distance 以降の値は isHit が true のときだけ意味を持つ.
struct RaycastHit2D {
	bool isHit = false;
	float distance = 0.0f;          //!< 始点から交点までの距離.
	Vector2 point = Vector2::Zero;  //!< 交点のワールド座標.
	Vector2 normal = Vector2::Zero; //!< 交点で外を向く面の法線。正規化済み.

	// if (Raycast(...)) と書けるようにする.
	explicit operator bool() const { return isHit; }
};

/*---------------------------------
*
* レイと図形の交差。direction は正規化済みであること.
* 始点が図形の内側にある場合は distance 0 で当たったものとして返す.
*
---------------------------------*/
RaycastHit2D Raycast(const Vector2& origin, const Vector2& direction, float distance, const Circle2D& shape);
RaycastHit2D Raycast(const Vector2& origin, const Vector2& direction, float distance, const AABB2D& shape);
RaycastHit2D Raycast(const Vector2& origin, const Vector2& direction, float distance, const OBB2D& shape);

}	// namespace Cake

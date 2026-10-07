#pragma once
/*====================================
 *
 * 2D幾何学的オブジェクト（Circle2, AABB2D, OBB2D, ConvexPolygon2）の定義ファイル。
 * 分離軸定理による衝突判定はいずれも凸多角形へ変換してから行うため、
 * 各図形から ConvexPolygon2D を組み立てる変換関数もここに置く。
 *
 * ====================================*/
#include <cmath>

#include "System/Foundation/Math/Vector.h"

namespace Cake {

// 円.
struct Circle2D {
	Vector2 center;	//!< 中心点.
	float radius = 0.0f;	//!< 半径.
};

// 軸平行境界矩形.
struct AABB2D {
	Vector2 min;	//!< 最小点.
	Vector2 max;	//!< 最大点.
};

// 有向境界矩形.
struct OBB2D {
	Vector2 center;	//!< 中心点.
	float rotate = 0.0f;	//!< 回転角(ラジアン)。Transform2::rotate をそのまま渡せる.
	Vector2 halfSize;	//!< 中心から辺までの距離.
};

// 凸多角形。頂点は反時計回りに並べる.
struct ConvexPolygon2D {
	static constexpr int kMaxVertices = 8;

	Vector2 vertices[kMaxVertices];
	int count = 0;
};

// 中心と全体サイズから AABB2 を組み立てる.
inline AABB2D MakeAABB2(const Vector2& center, const Vector2& size) {
	Vector2 halfSize = size / 2.0f;
	return AABB2D{center - halfSize, center + halfSize};
}

inline ConvexPolygon2D ToPolygon(const AABB2D& aabb) {
	ConvexPolygon2D polygon;
	polygon.count = 4;
	polygon.vertices[0] = {aabb.min.x, aabb.min.y};
	polygon.vertices[1] = {aabb.max.x, aabb.min.y};
	polygon.vertices[2] = {aabb.max.x, aabb.max.y};
	polygon.vertices[3] = {aabb.min.x, aabb.max.y};
	return polygon;
}

inline ConvexPolygon2D ToPolygon(const OBB2D& obb) {
	float sinTheta = std::sin(obb.rotate);
	float cosTheta = std::cos(obb.rotate);

	// 回転後のローカル軸に半サイズを掛けた、中心から辺の中点へ向かうベクトル.
	Vector2 edgeX = Vector2{cosTheta, sinTheta} * obb.halfSize.x;
	Vector2 edgeY = Vector2{-sinTheta, cosTheta} * obb.halfSize.y;

	ConvexPolygon2D polygon;
	polygon.count = 4;
	polygon.vertices[0] = obb.center - edgeX - edgeY;
	polygon.vertices[1] = obb.center + edgeX - edgeY;
	polygon.vertices[2] = obb.center + edgeX + edgeY;
	polygon.vertices[3] = obb.center - edgeX + edgeY;
	return polygon;
}

}	// namespace Cake

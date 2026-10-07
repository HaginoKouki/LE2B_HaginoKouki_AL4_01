#pragma once
/*====================================
 *
 * 3D幾何学的オブジェクト（Line, Ray, Segment, Sphere, AABB, OBB, Plane等）の定義ファイル。
 * 当たり判定や可視化、空間クエリに必要な基本図形を提供する。
 *
 * ====================================*/
#include "System/Foundation/Math/Vector.h"
#include "System/Foundation/Math/Matrix.h"

namespace Cake {
// 直線.
struct Line {
	Cake::Vector3 origin; //!< 始点.
	Cake::Vector3 diff;   //!< 終点への差分ベクトル.
};

// 半直線.
struct Ray {
	Cake::Vector3 origin; //!< 始点.
	Cake::Vector3 diff;   //!< 終点への差分ベクトル.
};

// 線分.
struct Segment {
	Cake::Vector3 origin; //!< 始点.
	Cake::Vector3 diff;   //!< 終点への差分ベクトル.
};

// 三角形.
struct Triangle {
	Cake::Vector3 vertices[3]; //!< 頂点.
};
Cake::Vector3 GetNormal(const Triangle& triangle);

// 四角形.
struct Quad {
	Cake::Vector3 vertices[4]; //!< 頂点.
};

// 平面.
struct Plane {
	Cake::Vector3 normal;   //!< 法線.
	float distance = 0.0f;  //!< 距離.
};
// 法線と点から平面を求める.
Plane GetPlane(const Cake::Vector3& normal, const Cake::Vector3& point);
// 3点で構成される平面を求める.
Plane GetPlane(const Triangle& triangle);

// 球.
struct Sphere {
	Cake::Vector3 center;  //!< 中心点.
	float radius = 0.0f;   //!< 半径.
};
// カプセル.
struct Capsule {
	Segment segment;
	float radius = 0.0f;
};

// AABB.
struct AABB {
	Cake::Vector3 min; //!< 最小点.
	Cake::Vector3 max; //!< 最大点.
};

// OBB.
struct OBB {
	Cake::Vector3 center;          //!< 中心点.
	//! 座標軸。正規化・直交必須。既定はワールド軸そのまま（回転なし）.
	Cake::Vector3 orientations[3] = {Cake::Vector3{1.0f, 0.0f, 0.0f}, Cake::Vector3{0.0f, 1.0f, 0.0f}, Cake::Vector3{0.0f, 0.0f, 1.0f}};
	Cake::Vector3 size;            //!< 座標軸方向の長さの半分。中心から面までの距離.
};

} // namespace Cake

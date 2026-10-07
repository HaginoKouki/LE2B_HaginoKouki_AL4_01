#include "IsCollision.h"

#include <algorithm>
#include <numbers>
#include <cmath>

namespace Cake {
namespace Math {
bool IsCollision(const Sphere& s1, const Sphere& s2) {
	return (s1.radius + s2.radius) > Vector3::Length(s1.center, s2.center);
}
bool IsCollision(const Sphere& sphere, const Plane& plane) {
	return sphere.radius > std::abs(Vector3::DotProduct(plane.normal, sphere.center) - plane.distance);
}

bool IsCollision(const Line& line, const Plane& plane) {
	float dot = Vector3::DotProduct(plane.normal, line.diff);

	if (dot == 0.0f) {
		return false;
	}

	return true;
}
bool IsCollision(const Ray& ray, const Plane& plane) {
	float dot = Vector3::DotProduct(plane.normal, ray.diff);

	if (dot == 0.0f) {
		return false;
	}

	float t = (plane.distance - Vector3::DotProduct(ray.origin, plane.normal)) / dot;

	return 0.0f <= t;
}
bool IsCollision(const Segment& segment, const Plane& plane) {
	float dot = Vector3::DotProduct(plane.normal, segment.diff);

	if (dot == 0.0f) {
		return false;
	}

	float t = (plane.distance - Vector3::DotProduct(segment.origin, plane.normal)) / dot;

	return (0.0f <= t && Vector3::Length(Vector3::Zero, t * segment.diff) <= Vector3::Length(Vector3::Zero, segment.diff));
}

bool IsCollision(const Line& line, const Triangle& triangle) {
	// 三角形から平面を求める.
	Plane plane = GetPlane(triangle);

	// 直線と平面の衝突点pを求める.
	float t = (plane.distance - Vector3::DotProduct(line.origin, plane.normal)) / Vector3::DotProduct(plane.normal, line.diff);
	Vector3 p = line.origin + t * line.diff;

	// 各辺を結んだベクトルと、頂点と衝突点pを結んだベクトルのクロス積を取る.
	Vector3 cross01 = Vector3::CrossProduct(triangle.vertices[1] - triangle.vertices[0], p - triangle.vertices[0]);
	Vector3 cross12 = Vector3::CrossProduct(triangle.vertices[2] - triangle.vertices[1], p - triangle.vertices[1]);
	Vector3 cross20 = Vector3::CrossProduct(triangle.vertices[0] - triangle.vertices[2], p - triangle.vertices[2]);

	// すべての小三角形のクロス積と法線が同じ方向を向いていたら衝突.
	return (
		Vector3::DotProduct(cross01, plane.normal) >= 0.0f &&
		Vector3::DotProduct(cross12, plane.normal) >= 0.0f &&
		Vector3::DotProduct(cross20, plane.normal) >= 0.0f
	);
}
bool IsCollision(const Ray& ray, const Triangle& triangle) {
	// 三角形から平面を求める.
	Plane plane = GetPlane(triangle);

	// レイと平面の衝突点pを求める.
	float t = (plane.distance - Vector3::DotProduct(ray.origin, plane.normal)) / Vector3::DotProduct(plane.normal, ray.diff);

	// 媒介変数tがレイの範囲外ならfalseを返す.
	if (t < 0.0f) {
		return false;
	}

	Vector3 p = ray.origin + t * ray.diff;

	// 各辺を結んだベクトルと、頂点と衝突点pを結んだベクトルのクロス積を取る.
	Vector3 cross01 = Vector3::CrossProduct(triangle.vertices[1] - triangle.vertices[0], p - triangle.vertices[0]);
	Vector3 cross12 = Vector3::CrossProduct(triangle.vertices[2] - triangle.vertices[1], p - triangle.vertices[1]);
	Vector3 cross20 = Vector3::CrossProduct(triangle.vertices[0] - triangle.vertices[2], p - triangle.vertices[2]);

	// すべての小三角形のクロス積と法線が同じ方向を向いていたら衝突.
	return (
		Vector3::DotProduct(cross01, plane.normal) >= 0.0f &&
		Vector3::DotProduct(cross12, plane.normal) >= 0.0f &&
		Vector3::DotProduct(cross20, plane.normal) >= 0.0f
	);
}
bool IsCollision(const Segment& segment, const Triangle& triangle) {
	// 三角形から平面を求める.
	Plane plane = GetPlane(triangle);

	// 線分と平面の衝突点pを求める.
	float t = (plane.distance - Vector3::DotProduct(segment.origin, plane.normal)) / Vector3::DotProduct(plane.normal, segment.diff);

	// 媒介変数tが線分の範囲外ならfalseを返す.
	if (t < 0.0f || t > 1.0f) {
		return false;
	}

	Vector3 p = segment.origin + t * segment.diff;

	// 各辺を結んだベクトルと、頂点と衝突点pを結んだベクトルのクロス積を取る.
	Vector3 cross01 = Vector3::CrossProduct(triangle.vertices[1] - triangle.vertices[0], p - triangle.vertices[0]);
	Vector3 cross12 = Vector3::CrossProduct(triangle.vertices[2] - triangle.vertices[1], p - triangle.vertices[1]);
	Vector3 cross20 = Vector3::CrossProduct(triangle.vertices[0] - triangle.vertices[2], p - triangle.vertices[2]);

	// すべての小三角形のクロス積と法線が同じ方向を向いていたら衝突.
	return (
		Vector3::DotProduct(cross01, plane.normal) >= 0.0f &&
		Vector3::DotProduct(cross12, plane.normal) >= 0.0f &&
		Vector3::DotProduct(cross20, plane.normal) >= 0.0f
	);
}

bool IsCollision(const AABB& aabb, const Line& line) {
	// 特異点の排除.
	assert((line.diff.x != 0.0f || line.diff.y != 0.0f || line.diff.z != 0.0f) && "Line diff cannot be zero vector.");
	// diff成分が0のとき、originがAABB範囲外なら即false
	if (line.diff.x == 0.0f && (line.origin.x < aabb.min.x || line.origin.x > aabb.max.x))
		return false;
	if (line.diff.y == 0.0f && (line.origin.y < aabb.min.y || line.origin.y > aabb.max.y))
		return false;
	if (line.diff.z == 0.0f && (line.origin.z < aabb.min.z || line.origin.z > aabb.max.z))
		return false;

	float tNearX = line.diff.x != 0.0f ? (aabb.min.x - line.origin.x) / line.diff.x : -std::numeric_limits<float>::infinity();
	float tFarX = line.diff.x != 0.0f ? (aabb.max.x - line.origin.x) / line.diff.x : std::numeric_limits<float>::infinity();
	if (tNearX > tFarX) {
		std::swap(tNearX, tFarX);
	}

	float tNearY = line.diff.y != 0.0f ? (aabb.min.y - line.origin.y) / line.diff.y : -std::numeric_limits<float>::infinity();
	float tFarY = line.diff.y != 0.0f ? (aabb.max.y - line.origin.y) / line.diff.y : std::numeric_limits<float>::infinity();
	if (tNearY > tFarY) {
		std::swap(tNearY, tFarY);
	}

	float tNearZ = line.diff.z != 0.0f ? (aabb.min.z - line.origin.z) / line.diff.z : -std::numeric_limits<float>::infinity();
	float tFarZ = line.diff.z != 0.0f ? (aabb.max.z - line.origin.z) / line.diff.z : std::numeric_limits<float>::infinity();
	if (tNearZ > tFarZ) {
		std::swap(tNearZ, tFarZ);
	}

	// AABBとの衝突点（貫通点）のtが小さいほう.
	float tEnter = std::max<float>(std::max<float>(tNearX, tNearY), tNearZ);
	// AABBとの衝突点（貫通点）のtが大きいほう.
	float tExit = std::min<float>(std::min<float>(tFarX, tFarY), tFarZ);

	return tEnter <= tExit;
}
bool IsCollision(const AABB& aabb, const Ray& ray) {
	// 特異点の排除.
	assert((ray.diff.x != 0.0f || ray.diff.y != 0.0f || ray.diff.z != 0.0f) && "Ray diff cannot be zero vector.");
	// diff成分が0のとき、originがAABB範囲外なら即false
	if (ray.diff.x == 0.0f && (ray.origin.x < aabb.min.x || ray.origin.x > aabb.max.x))
		return false;
	if (ray.diff.y == 0.0f && (ray.origin.y < aabb.min.y || ray.origin.y > aabb.max.y))
		return false;
	if (ray.diff.z == 0.0f && (ray.origin.z < aabb.min.z || ray.origin.z > aabb.max.z))
		return false;

	float tNearX = ray.diff.x != 0.0f ? (aabb.min.x - ray.origin.x) / ray.diff.x : -std::numeric_limits<float>::infinity();
	float tFarX = ray.diff.x != 0.0f ? (aabb.max.x - ray.origin.x) / ray.diff.x : std::numeric_limits<float>::infinity();
	if (tNearX > tFarX) {
		std::swap(tNearX, tFarX);
	}

	float tNearY = ray.diff.y != 0.0f ? (aabb.min.y - ray.origin.y) / ray.diff.y : -std::numeric_limits<float>::infinity();
	float tFarY = ray.diff.y != 0.0f ? (aabb.max.y - ray.origin.y) / ray.diff.y : std::numeric_limits<float>::infinity();
	if (tNearY > tFarY) {
		std::swap(tNearY, tFarY);
	}

	float tNearZ = ray.diff.z != 0.0f ? (aabb.min.z - ray.origin.z) / ray.diff.z : -std::numeric_limits<float>::infinity();
	float tFarZ = ray.diff.z != 0.0f ? (aabb.max.z - ray.origin.z) / ray.diff.z : std::numeric_limits<float>::infinity();
	if (tNearZ > tFarZ) {
		std::swap(tNearZ, tFarZ);
	}

	// AABBとの衝突点（貫通点）のtが小さいほう.
	float tEnter = std::max<float>(std::max<float>(tNearX, tNearY), tNearZ);
	// AABBとの衝突点（貫通点）のtが大きいほう.
	float tExit = std::min<float>(std::min<float>(tFarX, tFarY), tFarZ);

	return tEnter <= tExit && tExit >= 0.0f;
}
bool IsCollision(const AABB& aabb, const Segment& segment) {
	// 特異点の排除.
	assert((segment.diff.x != 0.0f || segment.diff.y != 0.0f || segment.diff.z != 0.0f) && "Segment diff cannot be zero vector.");
	// diff成分が0のとき、originがAABB範囲外なら即false
	if (segment.diff.x == 0.0f && (segment.origin.x < aabb.min.x || segment.origin.x > aabb.max.x))
		return false;
	if (segment.diff.y == 0.0f && (segment.origin.y < aabb.min.y || segment.origin.y > aabb.max.y))
		return false;
	if (segment.diff.z == 0.0f && (segment.origin.z < aabb.min.z || segment.origin.z > aabb.max.z))
		return false;

	float tNearX = segment.diff.x != 0.0f ? (aabb.min.x - segment.origin.x) / segment.diff.x : -std::numeric_limits<float>::infinity();
	float tFarX = segment.diff.x != 0.0f ? (aabb.max.x - segment.origin.x) / segment.diff.x : std::numeric_limits<float>::infinity();
	if (tNearX > tFarX) {
		std::swap(tNearX, tFarX);
	}

	float tNearY = segment.diff.y != 0.0f ? (aabb.min.y - segment.origin.y) / segment.diff.y : -std::numeric_limits<float>::infinity();
	float tFarY = segment.diff.y != 0.0f ? (aabb.max.y - segment.origin.y) / segment.diff.y : std::numeric_limits<float>::infinity();
	if (tNearY > tFarY) {
		std::swap(tNearY, tFarY);
	}

	float tNearZ = segment.diff.z != 0.0f ? (aabb.min.z - segment.origin.z) / segment.diff.z : -std::numeric_limits<float>::infinity();
	float tFarZ = segment.diff.z != 0.0f ? (aabb.max.z - segment.origin.z) / segment.diff.z : std::numeric_limits<float>::infinity();
	if (tNearZ > tFarZ) {
		std::swap(tNearZ, tFarZ);
	}

	// AABBとの衝突点（貫通点）のtが小さいほう.
	float tEnter = std::max<float>(std::max<float>(tNearX, tNearY), tNearZ);
	// AABBとの衝突点（貫通点）のtが大きいほう.
	float tExit = std::min<float>(std::min<float>(tFarX, tFarY), tFarZ);

	return tEnter <= tExit && tExit >= 0.0f && tEnter <= 1.0f;
}
bool IsCollision(const AABB& aabb1, const AABB& aabb2) {
	return (
		(aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) &&
		(aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y) &&
		(aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z)
	);
}
bool IsCollision(const AABB& aabb, const Sphere& sphere) {
	Vector3 closestPoint = Vector3::Clamp(sphere.center, aabb.min, aabb.max);
	return Vector3::Length(sphere.center, closestPoint) < sphere.radius;
}

bool IsCollision(const OBB& obb, const Line& line) {
	// OBBのローカル変換行列.
	Matrix4x4 obbWorldMatrixInvercse = Matrix4x4::Inverse({obb.orientations[0].x, obb.orientations[0].y, obb.orientations[0].z, 0.0f, obb.orientations[1].x, obb.orientations[1].y, obb.orientations[1].z, 0.0f, obb.orientations[2].x, obb.orientations[2].y, obb.orientations[2].z, 0.0f, obb.center.x, obb.center.y, obb.center.z, 1.0f});
	// OBBからAABBを生成する.
	AABB aabbOBBLocal{
		.min = {-std::abs(obb.size.x), -std::abs(obb.size.y), -std::abs(obb.size.z)},
		.max = {std::abs(obb.size.x), std::abs(obb.size.y), std::abs(obb.size.z)}
	};
	// 線分をOBBのローカル空間に変換する.
	Line lineOBBLocal;
	lineOBBLocal.origin = Vector3::Transform(line.origin, obbWorldMatrixInvercse);
	lineOBBLocal.diff = Vector3::Transform(line.origin + line.diff, obbWorldMatrixInvercse) - lineOBBLocal.origin;
	// ローカル空間で衝突判定.
	return IsCollision(aabbOBBLocal, lineOBBLocal);
}
bool IsCollision(const OBB& obb, const Ray& ray) {
	// OBBのローカル変換行列.
	Matrix4x4 obbWorldMatrixInvercse = Matrix4x4::Inverse({obb.orientations[0].x, obb.orientations[0].y, obb.orientations[0].z, 0.0f, obb.orientations[1].x, obb.orientations[1].y, obb.orientations[1].z, 0.0f, obb.orientations[2].x, obb.orientations[2].y, obb.orientations[2].z, 0.0f, obb.center.x, obb.center.y, obb.center.z, 1.0f});
	// OBBからAABBを生成する.
	AABB aabbOBBLocal{
		.min = {-std::abs(obb.size.x), -std::abs(obb.size.y), -std::abs(obb.size.z)},
		.max = {std::abs(obb.size.x), std::abs(obb.size.y), std::abs(obb.size.z)}
	};
	// 線分をOBBのローカル空間に変換する.
	Ray rayOBBLocal;
	rayOBBLocal.origin = Vector3::Transform(ray.origin, obbWorldMatrixInvercse);
	rayOBBLocal.diff = Vector3::Transform(ray.origin + ray.diff, obbWorldMatrixInvercse) - rayOBBLocal.origin;
	// ローカル空間で衝突判定.
	return IsCollision(aabbOBBLocal, rayOBBLocal);
}
bool IsCollision(const OBB& obb, const Segment& segment) {
	// OBBのローカル変換行列.
	Matrix4x4 obbWorldMatrixInvercse = Matrix4x4::Inverse({obb.orientations[0].x, obb.orientations[0].y, obb.orientations[0].z, 0.0f, obb.orientations[1].x, obb.orientations[1].y, obb.orientations[1].z, 0.0f, obb.orientations[2].x, obb.orientations[2].y, obb.orientations[2].z, 0.0f, obb.center.x, obb.center.y, obb.center.z, 1.0f});
	// OBBからAABBを生成する.
	AABB aabbOBBLocal{
		.min = {-std::abs(obb.size.x), -std::abs(obb.size.y), -std::abs(obb.size.z)},
		.max = {std::abs(obb.size.x), std::abs(obb.size.y), std::abs(obb.size.z)}
	};
	// 線分をOBBのローカル空間に変換する.
	Segment segmentOBBLocal;
	segmentOBBLocal.origin = Vector3::Transform(segment.origin, obbWorldMatrixInvercse);
	segmentOBBLocal.diff = Vector3::Transform(segment.origin + segment.diff, obbWorldMatrixInvercse) - segmentOBBLocal.origin;

	// ローカル空間で衝突判定.
	return IsCollision(aabbOBBLocal, segmentOBBLocal);
}
bool IsCollision(const OBB& obb, const Sphere& sphere) {
	// OBBのローカル変換行列.
	Matrix4x4 obbWorldMatrixInvercse = Matrix4x4::Inverse({obb.orientations[0].x, obb.orientations[0].y, obb.orientations[0].z, 0.0f, obb.orientations[1].x, obb.orientations[1].y, obb.orientations[1].z, 0.0f, obb.orientations[2].x, obb.orientations[2].y, obb.orientations[2].z, 0.0f, obb.center.x, obb.center.y, obb.center.z, 1.0f});

	// OBBからAABBを生成する.
	AABB aabbOBBLocal{
		.min = {-std::abs(obb.size.x), -std::abs(obb.size.y), -std::abs(obb.size.z)},
		.max = {std::abs(obb.size.x), std::abs(obb.size.y), std::abs(obb.size.z)}
	};

	// 球をOBBのローカル空間に変換する.
	Sphere sphereOBBLocal{
		Vector3::Transform(sphere.center, obbWorldMatrixInvercse),
		sphere.radius
	};

	// ローカル空間で衝突判定.
	return IsCollision(aabbOBBLocal, sphereOBBLocal);
}
bool IsCollision(const OBB& obb, const AABB& aabb) {
	// AABBをOBBに変換する.
	// OBB::size は中心から面までの距離（半分の長さ）なので、AABB の幅も半分にして渡す.
	OBB obbAABB{
		aabb.min + (aabb.max - aabb.min) * 0.5f,
		{Vector3::UnitX,
	     Vector3::UnitY,
	     Vector3::UnitZ},
		(aabb.max - aabb.min) * 0.5f
	};
	// OBB同士の衝突判定.
	return IsCollision(obb, obbAABB);
}
bool IsCollision(const OBB& obb1, const OBB& obb2) {
	// obb1の頂点座標を求める.
	Vector3 verticesOBB1[8] = {
		{+obb1.size.x, +obb1.size.y, +obb1.size.z},
		{-obb1.size.x, +obb1.size.y, +obb1.size.z},
		{-obb1.size.x, -obb1.size.y, +obb1.size.z},
		{+obb1.size.x, -obb1.size.y, +obb1.size.z},
		{+obb1.size.x, +obb1.size.y, -obb1.size.z},
		{-obb1.size.x, +obb1.size.y, -obb1.size.z},
		{-obb1.size.x, -obb1.size.y, -obb1.size.z},
		{+obb1.size.x, -obb1.size.y, -obb1.size.z}
	};
	for (int i = 0; i < 8; ++i) {
		verticesOBB1[i] = Vector3::Transform(verticesOBB1[i], {obb1.orientations[0].x, obb1.orientations[0].y, obb1.orientations[0].z, 0.0f, obb1.orientations[1].x, obb1.orientations[1].y, obb1.orientations[1].z, 0.0f, obb1.orientations[2].x, obb1.orientations[2].y, obb1.orientations[2].z, 0.0f, obb1.center.x, obb1.center.y, obb1.center.z, 1.0f});
	}

	// obb2の頂点座標を求める.
	Vector3 verticesOBB2[8] = {
		{+obb2.size.x, +obb2.size.y, +obb2.size.z},
		{-obb2.size.x, +obb2.size.y, +obb2.size.z},
		{-obb2.size.x, -obb2.size.y, +obb2.size.z},
		{+obb2.size.x, -obb2.size.y, +obb2.size.z},
		{+obb2.size.x, +obb2.size.y, -obb2.size.z},
		{-obb2.size.x, +obb2.size.y, -obb2.size.z},
		{-obb2.size.x, -obb2.size.y, -obb2.size.z},
		{+obb2.size.x, -obb2.size.y, -obb2.size.z}
	};
	for (int i = 0; i < 8; ++i) {
		verticesOBB2[i] = Vector3::Transform(verticesOBB2[i], {obb2.orientations[0].x, obb2.orientations[0].y, obb2.orientations[0].z, 0.0f, obb2.orientations[1].x, obb2.orientations[1].y, obb2.orientations[1].z, 0.0f, obb2.orientations[2].x, obb2.orientations[2].y, obb2.orientations[2].z, 0.0f, obb2.center.x, obb2.center.y, obb2.center.z, 1.0f});
	}

	// verticesOBB1, verticesOBB2をキャプチャしたラムダ式.
	auto isSeparated = [&](const Vector3& axis) -> bool {
		float proj1[8], proj2[8];
		for (int i = 0; i < 8; ++i) {
			proj1[i] = Vector3::DotProduct(verticesOBB1[i], axis);
			proj2[i] = Vector3::DotProduct(verticesOBB2[i], axis);
		}

		float min1 = *std::min_element(proj1, proj1 + 8);
		float max1 = *std::max_element(proj1, proj1 + 8);
		float min2 = *std::min_element(proj2, proj2 + 8);
		float max2 = *std::max_element(proj2, proj2 + 8);

		float L1 = max1 - min1;
		float L2 = max2 - min2;
		float sumSpan = L1 + L2;
		float longSpan = std::max(max1, max2) - std::min(min1, min2);

		return sumSpan < longSpan;
	};

	/*---------------------------------
	*
	* 面法線の判定
	*
	---------------------------------*/
	/*
	* obb1のX,Y,Z軸を分離軸とした判定
	———————————————*/
	for (int i = 0; i < 3; ++i) {
		// obb1の面法線X軸を求める.
		Vector3 axis = obb1.orientations[i];

		if (isSeparated(axis)) {
			// 分離しているので分離軸.
			return false;
		}
	}
	/*
	* obb2のX,Y,Z軸を分離軸とした判定
	———————————————*/
	for (int i = 0; i < 3; ++i) {
		// obb2の面法線X軸を求める.
		Vector3 axis = obb2.orientations[i];

		if (isSeparated(axis)) {
			// 分離しているので分離軸.
			return false;
		}
	}

	/*---------------------------------
	*
	* 各辺の組み合わせのクロス積の判定
	*
	---------------------------------*/
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			// obb1の辺iとobb2の辺jのクロス積を求める.
			Vector3 axis = Vector3::CrossProduct(obb1.orientations[i], obb2.orientations[j]);
			if (Vector3::Length(Vector3::Zero, axis) > 1e-6f) { // クロス積がゼロベクトルに近い場合は分離軸にならないので除外.
				if (isSeparated(axis)) {
					// 分離しているので分離軸.
					return false;
				}
			}
		}
	}

	// 分離軸が見つからなかったので衝突している.
	return true;
}
} // namespace Math
} // namespace Cake

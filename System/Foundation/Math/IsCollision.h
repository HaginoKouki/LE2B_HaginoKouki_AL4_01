#pragma once

#include "System/Foundation/Math/Vector.h"
#include "System/Foundation/Math/Matrix.h"
#include "System/Foundation/Math/Geometry.h"

namespace Cake {
namespace Math {
// 球同士の衝突判定.
bool IsCollision(const Sphere& s1, const Sphere& s2);
// 球と平面の衝突判定.
bool IsCollision(const Sphere& sphere, const Plane& plane);

// 線と平面の衝突判定.
bool IsCollision(const Line& line, const Plane& plane);
bool IsCollision(const Ray& ray, const Plane& plane);
bool IsCollision(const Segment& segment, const Plane& plane);

// 線と三角形の衝突判定.
bool IsCollision(const Line& line, const Triangle& triangle);
bool IsCollision(const Ray& ray, const Triangle& triangle);
bool IsCollision(const Segment& segment, const Triangle& triangle);

// AABBと線の衝突判定.
bool IsCollision(const AABB& aabb, const Line& line);
bool IsCollision(const AABB& aabb, const Ray& ray);
bool IsCollision(const AABB& aabb, const Segment& segment);
// AABB同士の衝突判定.
bool IsCollision(const AABB& aabb1, const AABB& aabb2);
// AABBと球の衝突判定.
bool IsCollision(const AABB& aabb, const Sphere& sphere);

// OBBと線分の衝突判定.
bool IsCollision(const OBB& obb, const Line& line);
bool IsCollision(const OBB& obb, const Ray& ray);
bool IsCollision(const OBB& obb, const Segment& segment);
// OBBと球の衝突判定.
bool IsCollision(const OBB& obb, const Sphere& sphere);
// OBBとAABBの衝突判定.
bool IsCollision(const OBB& obb, const AABB& aabb);
// OBB同士の衝突判定.
bool IsCollision(const OBB& obb1, const OBB& obb2);
} // namespace Math
} // namespace Cake

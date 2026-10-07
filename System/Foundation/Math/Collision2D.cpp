#include "Collision2D.h"
#include <utility>
#include <algorithm>
#include <cmath>
#include <limits>

namespace Cake {

namespace {

// 長さがこれ以下の軸は退化しているとみなす.
constexpr float kAxisEpsilon = 1e-6f;

// 軸に投影して得られる区間.
struct Projection2D {
	float min;
	float max;
};

Projection2D Project(const ConvexPolygon2D& polygon, const Vector2& axis) {
	float value = Vector2::DotProduct(polygon.vertices[0], axis);
	Projection2D result{value, value};

	for (int i = 1; i < polygon.count; ++i) {
		value = Vector2::DotProduct(polygon.vertices[i], axis);
		result.min = std::min(result.min, value);
		result.max = std::max(result.max, value);
	}
	return result;
}

Projection2D Project(const Circle2D& circle, const Vector2& axis) {
	// 円の影は、中心の投影値を半径ぶん両側へ広げたもの.
	float value = Vector2::DotProduct(circle.center, axis);
	return Projection2D{value - circle.radius, value + circle.radius};
}

Vector2 GetCenter(const ConvexPolygon2D& polygon) {
	Vector2 sum = Vector2::Zero;
	for (int i = 0; i < polygon.count; ++i) {
		sum += polygon.vertices[i];
	}
	return sum / static_cast<float>(polygon.count);
}

// point に最も近い頂点を返す.
Vector2 GetClosestVertex(const ConvexPolygon2D& polygon, const Vector2& point) {
	Vector2 closest = polygon.vertices[0];
	Vector2 diff = closest - point;
	float minDistanceSquared = Vector2::DotProduct(diff, diff);

	for (int i = 1; i < polygon.count; ++i) {
		diff = polygon.vertices[i] - point;
		float distanceSquared = Vector2::DotProduct(diff, diff);
		if (distanceSquared < minDistanceSquared) {
			minDistanceSquared = distanceSquared;
			closest = polygon.vertices[i];
		}
	}
	return closest;
}

// 軸の符号は求め方によって前後どちらにも転ぶので、aをbから引き離す向きへ揃える.
Vector2 OrientNormal(const Vector2& axis, const Vector2& centerA, const Vector2& centerB) {
	if (Vector2::DotProduct(centerA - centerB, axis) < 0.0f) {
		return -axis;
	}
	return axis;
}

// 候補軸を1本評価する。分離していれば false を返し、呼び出し側に探索を打ち切らせる.
template<class ShapeA, class ShapeB>
bool TestAxis(const ShapeA& a, const ShapeB& b, const Vector2& rawAxis, float& minOverlap, Vector2& bestAxis) {
	float lengthSquared = Vector2::DotProduct(rawAxis, rawAxis);
	if (lengthSquared <= kAxisEpsilon * kAxisEpsilon) {
		// 長さ0の軸は分離軸になり得ないので読み飛ばす.
		return true;
	}
	// 重なり量を長さとして比較するため、正規化は必須.
	Vector2 axis = rawAxis / std::sqrt(lengthSquared);

	Projection2D projectionA = Project(a, axis);
	Projection2D projectionB = Project(b, axis);

	float overlap = std::min(projectionA.max, projectionB.max) - std::max(projectionA.min, projectionB.min);
	if (overlap <= 0.0f) {
		// 隙間ができた。この1本で非衝突が確定する.
		return false;
	}
	// 重なりが最も浅い軸が、そのまま最小の押し出し方向になる.
	if (overlap < minOverlap) {
		minOverlap = overlap;
		bestAxis = axis;
	}
	return true;
}

// source の全辺の法線を候補軸として評価する.
template<class ShapeA, class ShapeB>
bool TestEdgeNormals(const ShapeA& a, const ShapeB& b, const ConvexPolygon2D& source, float& minOverlap, Vector2& bestAxis) {
	for (int i = 0; i < source.count; ++i) {
		Vector2 edge = source.vertices[(i + 1) % source.count] - source.vertices[i];
		// 2Dの法線は、成分を入れ替えて片方の符号を反転するだけで求まる.
		if (!TestAxis(a, b, Vector2{-edge.y, edge.x}, minOverlap, bestAxis)) {
			return false;
		}
	}
	return true;
}

// 引数を入れ替えて呼んだ結果を、元の並びの結果へ変換する.
CollisionInfo2D FlipNormal(CollisionInfo2D info) {
	info.normal = -info.normal;
	return info;
}


// レイがこれ以下しか進まない軸は、進んでいないものとして扱う.
constexpr float kRayEpsilon = 1e-6f;

// スラブ法。軸平行な箱に対して、入口までの距離と入口面の法線を求める.
// 始点が箱の中にある場合は、距離0で当たったことにする.
bool RaycastSlab(
	const Vector2& origin, const Vector2& direction, const Vector2& minPoint,
	const Vector2& maxPoint, float maxDistance, float& outDistance, Vector2& outNormal
) {

	const float originArray[2] = {origin.x, origin.y};
	const float directionArray[2] = {direction.x, direction.y};
	const float minArray[2] = {minPoint.x, minPoint.y};
	const float maxArray[2] = {maxPoint.x, maxPoint.y};

	float tNear = 0.0f;
	float tFar = maxDistance;
	Vector2 entryNormal = Vector2::Zero;
	bool hasEntry = false;

	for (int axis = 0; axis < 2; ++axis) {
		const Vector2 axisUnit = (axis == 0) ? Vector2::UnitX : Vector2::UnitY;

		if (std::abs(directionArray[axis]) <= kRayEpsilon) {
			// この軸へは進まないので、範囲の外に居るなら永久に入らない.
			if (originArray[axis] < minArray[axis] || originArray[axis] > maxArray[axis]) {
				return false;
			}
			continue;
		}

		const float inverse = 1.0f / directionArray[axis];
		float tMin = (minArray[axis] - originArray[axis]) * inverse;
		float tMax = (maxArray[axis] - originArray[axis]) * inverse;
		// 先に当たるのがどちらの面かで、入口面の法線が決まる.
		Vector2 normal = -axisUnit;
		if (tMin > tMax) {
			std::swap(tMin, tMax);
			normal = axisUnit;
		}

		// 全軸の区間が重なる範囲が、箱の中に居る区間になる.
		if (tMin > tNear) {
			tNear = tMin;
			entryNormal = normal;
			hasEntry = true;
		}
		tFar = std::min(tFar, tMax);
		if (tNear > tFar) {
			return false;
		}
	}

	outDistance = tNear;
	// 始点が中に居ると入口面が決まらないので、来た方向へ押し返す形にしておく.
	outNormal = hasEntry ? entryNormal : -direction;
	return true;
}

}	// namespace


CollisionInfo2D Collide(const Circle2D& a, const Circle2D& b) {
	CollisionInfo2D info;

	Vector2 diff = a.center - b.center;
	float distanceSquared = Vector2::DotProduct(diff, diff);
	float radiusSum = a.radius + b.radius;

	// 平方根を取らずに比較して、離れている場合は早期に抜ける.
	if (distanceSquared >= radiusSum * radiusSum) {
		return info;
	}

	float distance = std::sqrt(distanceSquared);

	info.isHit = true;
	info.depth = radiusSum - distance;
	// 中心が完全に一致していると向きが決まらないので、便宜上Y+へ押し出す.
	info.normal = (distance > kAxisEpsilon) ? (diff / distance) : Vector2::UnitY;
	return info;
}

CollisionInfo2D Collide(const AABB2D& a, const AABB2D& b) {
	CollisionInfo2D info;

	// 候補軸がX,Yに固定されるので、SATを展開した形で書ける.
	float overlapX = std::min(a.max.x, b.max.x) - std::max(a.min.x, b.min.x);
	if (overlapX <= 0.0f) {
		return info;
	}
	float overlapY = std::min(a.max.y, b.max.y) - std::max(a.min.y, b.min.y);
	if (overlapY <= 0.0f) {
		return info;
	}

	Vector2 centerA = (a.min + a.max) / 2.0f;
	Vector2 centerB = (b.min + b.max) / 2.0f;

	info.isHit = true;
	if (overlapX < overlapY) {
		info.depth = overlapX;
		info.normal = OrientNormal(Vector2::UnitX, centerA, centerB);
	} else {
		info.depth = overlapY;
		info.normal = OrientNormal(Vector2::UnitY, centerA, centerB);
	}
	return info;
}

CollisionInfo2D Collide(const ConvexPolygon2D& a, const ConvexPolygon2D& b) {
	CollisionInfo2D info;
	if (a.count < 3 || b.count < 3) {
		return info;
	}

	float minOverlap = std::numeric_limits<float>::max();
	Vector2 bestAxis = Vector2::Zero;

	// 候補軸は両者の辺の法線のみ。3Dと違い、辺同士のクロス積は不要.
	if (!TestEdgeNormals(a, b, a, minOverlap, bestAxis)) {
		return info;
	}
	if (!TestEdgeNormals(a, b, b, minOverlap, bestAxis)) {
		return info;
	}

	// すべての軸で重なった。分離軸が存在しないので衝突している.
	info.isHit = true;
	info.depth = minOverlap;
	info.normal = OrientNormal(bestAxis, GetCenter(a), GetCenter(b));
	return info;
}

CollisionInfo2D Collide(const ConvexPolygon2D& a, const Circle2D& b) {
	CollisionInfo2D info;
	if (a.count < 3) {
		return info;
	}

	float minOverlap = std::numeric_limits<float>::max();
	Vector2 bestAxis = Vector2::Zero;

	if (!TestEdgeNormals(a, b, a, minOverlap, bestAxis)) {
		return info;
	}
	// 円に最も近い頂点へ向かう軸。角をかすめる場合、この軸でしか分離を検出できない.
	if (!TestAxis(a, b, GetClosestVertex(a, b.center) - b.center, minOverlap, bestAxis)) {
		return info;
	}

	info.isHit = true;
	info.depth = minOverlap;
	info.normal = OrientNormal(bestAxis, GetCenter(a), b.center);
	return info;
}

CollisionInfo2D Collide(const Circle2D& a, const ConvexPolygon2D& b) {
	return FlipNormal(Collide(b, a));
}

CollisionInfo2D Collide(const AABB2D& a, const Circle2D& b) {
	return Collide(ToPolygon(a), b);
}

CollisionInfo2D Collide(const Circle2D& a, const AABB2D& b) {
	return FlipNormal(Collide(ToPolygon(b), a));
}

CollisionInfo2D Collide(const OBB2D& a, const OBB2D& b) {
	return Collide(ToPolygon(a), ToPolygon(b));
}

CollisionInfo2D Collide(const OBB2D& a, const AABB2D& b) {
	return Collide(ToPolygon(a), ToPolygon(b));
}

CollisionInfo2D Collide(const AABB2D& a, const OBB2D& b) {
	return Collide(ToPolygon(a), ToPolygon(b));
}

CollisionInfo2D Collide(const OBB2D& a, const Circle2D& b) {
	return Collide(ToPolygon(a), b);
}

CollisionInfo2D Collide(const Circle2D& a, const OBB2D& b) {
	return FlipNormal(Collide(ToPolygon(b), a));
}

RaycastHit2D Raycast(const Vector2& origin, const Vector2& direction, float distance, const Circle2D& shape) {
	RaycastHit2D hit;

	const Vector2 toCenter = shape.center - origin;
	const float radiusSquared = shape.radius * shape.radius;
	const float distanceSquared = Vector2::DotProduct(toCenter, toCenter);

	// 始点が円の中。箱と揃えて、距離0で当たったことにする.
	if (distanceSquared <= radiusSquared) {
		hit.isHit = true;
		hit.point = origin;
		hit.normal = -direction;
		return hit;
	}

	// 中心を進行方向へ落とした点までの距離と、そこから中心までの距離を出す.
	const float projection = Vector2::DotProduct(toCenter, direction);
	if (projection < 0.0f) {
		// 円が後ろにあるので当たらない.
		return hit;
	}
	const float perpendicularSquared = distanceSquared - projection * projection;
	if (perpendicularSquared > radiusSquared) {
		return hit;
	}

	const float halfChord = std::sqrt(radiusSquared - perpendicularSquared);
	const float hitDistance = projection - halfChord;
	if (hitDistance > distance) {
		return hit;
	}

	hit.isHit = true;
	hit.distance = hitDistance;
	hit.point = origin + direction * hitDistance;
	hit.normal = Vector2::Normalize(hit.point - shape.center);
	return hit;
}

RaycastHit2D Raycast(const Vector2& origin, const Vector2& direction, float distance, const AABB2D& shape) {
	RaycastHit2D hit;

	float hitDistance = 0.0f;
	Vector2 hitNormal = Vector2::Zero;
	if (!RaycastSlab(origin, direction, shape.min, shape.max, distance, hitDistance, hitNormal)) {
		return hit;
	}

	hit.isHit = true;
	hit.distance = hitDistance;
	hit.point = origin + direction * hitDistance;
	hit.normal = hitNormal;
	return hit;
}

RaycastHit2D Raycast(const Vector2& origin, const Vector2& direction, float distance, const OBB2D& shape) {
	RaycastHit2D hit;

	// OBBのローカル空間へ移せば、そのままスラブ法が使える.
	const float sinTheta = std::sin(-shape.rotate);
	const float cosTheta = std::cos(-shape.rotate);
	const Vector2 offset = origin - shape.center;
	const Vector2 localOrigin = {
		offset.x * cosTheta - offset.y * sinTheta,
		offset.x * sinTheta + offset.y * cosTheta
	};
	const Vector2 localDirection = {
		direction.x * cosTheta - direction.y * sinTheta,
		direction.x * sinTheta + direction.y * cosTheta
	};

	float hitDistance = 0.0f;
	Vector2 localNormal = Vector2::Zero;
	if (!RaycastSlab(
			localOrigin, localDirection, -shape.halfSize, shape.halfSize, distance,
			hitDistance, localNormal
		)) {
		return hit;
	}

	// 法線だけワールドへ戻す。距離は回転で変わらない.
	const float sinBack = std::sin(shape.rotate);
	const float cosBack = std::cos(shape.rotate);
	hit.isHit = true;
	hit.distance = hitDistance;
	hit.point = origin + direction * hitDistance;
	hit.normal = {
		localNormal.x * cosBack - localNormal.y * sinBack,
		localNormal.x * sinBack + localNormal.y * cosBack
	};
	return hit;
}

}	// namespace Cake

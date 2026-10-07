#include "Collision3D.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace Cake {

namespace {

// 長さがこれ以下のベクトルは、向きを持たないものとみなす.
constexpr float kEpsilon = 1e-6f;
// 辺どうしの外積がこれより短い場合、2辺はほぼ平行。分離軸は面の法線で足りている.
// 短い外積を正規化すると誤差が膨らみ、ありえない向きへ押し出されるので読み飛ばす.
constexpr float kParallelEpsilon = 1e-3f;
// 辺どうしの外積軸は、重なりが面の軸のこの割合を下回るときだけ採用する.
// ほぼ同じ深さなら面の法線で押し出したほうが、床に置いた箱が斜めへ弾かれずに済む.
constexpr float kEdgeAxisBias = 0.95f;
// レイがこれ以下しか進まない軸は、進んでいないものとして扱う.
constexpr float kRayEpsilon = 1e-6f;

float Dot(const Vector3& a, const Vector3& b) {
	return Vector3::DotProduct(a, b);
}

// 成分を添字で読み書きする。3軸に同じ処理を回すため.
float GetAxis(const Vector3& value, int axis) {
	switch (axis) {
		case 0:
			return value.x;
		case 1:
			return value.y;
		default:
			return value.z;
	}
}
Vector3 GetUnitAxis(int axis) {
	switch (axis) {
		case 0:
			return Vector3::UnitX;
		case 1:
			return Vector3::UnitY;
		default:
			return Vector3::UnitZ;
	}
}

Vector3 GetCenter(const AABB& aabb) {
	return (aabb.min + aabb.max) * 0.5f;
}

// 軸の符号は求め方によって前後どちらにも転ぶので、aをbから引き離す向きへ揃える.
Vector3 OrientNormal(const Vector3& axis, const Vector3& centerA, const Vector3& centerB) {
	if (Dot(centerA - centerB, axis) < 0.0f) {
		return -axis;
	}
	return axis;
}

// 引数を入れ替えて呼んだ結果を、元の並びの結果へ変換する.
CollisionInfo3D FlipNormal(CollisionInfo3D info) {
	info.normal = -info.normal;
	return info;
}

// OBB を軸へ投影した影の、中心から端までの長さ.
float ProjectRadius(const OBB& obb, const Vector3& axis) {
	return std::abs(obb.size.x) * std::abs(Dot(obb.orientations[0], axis)) +
	       std::abs(obb.size.y) * std::abs(Dot(obb.orientations[1], axis)) +
	       std::abs(obb.size.z) * std::abs(Dot(obb.orientations[2], axis));
}

// 候補軸を1本評価する。分離していれば false を返し、呼び出し側に探索を打ち切らせる.
// axis は正規化済みで渡す。bias は既存の候補に対する採用のしにくさ（1 で対等）.
bool TestAxis(const OBB& a, const OBB& b, const Vector3& axis, float bias, float& minOverlap, Vector3& bestAxis) {
	const float distance = std::abs(Dot(b.center - a.center, axis));
	const float overlap = ProjectRadius(a, axis) + ProjectRadius(b, axis) - distance;
	if (overlap <= 0.0f) {
		// 隙間ができた。この1本で非衝突が確定する.
		return false;
	}
	// 重なりが最も浅い軸が、そのまま最小の押し出し方向になる.
	if (overlap < minOverlap * bias) {
		minOverlap = overlap;
		bestAxis = axis;
	}
	return true;
}

// スラブ法。軸平行な箱に対して、入口までの距離と入口面の法線を求める.
// 始点が箱の中にある場合は、距離0で当たったことにする.
bool RaycastSlab(
	const Vector3& origin, const Vector3& direction, const Vector3& minPoint,
	const Vector3& maxPoint, float maxDistance, float& outDistance, Vector3& outNormal
) {
	float tNear = 0.0f;
	float tFar = maxDistance;
	Vector3 entryNormal = Vector3::Zero;
	bool hasEntry = false;

	for (int axis = 0; axis < 3; ++axis) {
		const float originValue = GetAxis(origin, axis);
		const float directionValue = GetAxis(direction, axis);
		const float minValue = GetAxis(minPoint, axis);
		const float maxValue = GetAxis(maxPoint, axis);

		if (std::abs(directionValue) <= kRayEpsilon) {
			// この軸へは進まないので、範囲の外に居るなら永久に入らない.
			if (originValue < minValue || originValue > maxValue) {
				return false;
			}
			continue;
		}

		const float inverse = 1.0f / directionValue;
		float tMin = (minValue - originValue) * inverse;
		float tMax = (maxValue - originValue) * inverse;
		// 先に当たるのがどちらの面かで、入口面の法線が決まる.
		Vector3 normal = -GetUnitAxis(axis);
		if (tMin > tMax) {
			std::swap(tMin, tMax);
			normal = GetUnitAxis(axis);
		}

		// 全軸の区間が重なる範囲が、箱の中に居る区間になる.
		if (tMin > tNear) {
			tNear = tMin;
			entryNormal = normal;
			hasEntry = true;
		}
		tFar = (std::min)(tFar, tMax);
		if (tNear > tFar) {
			return false;
		}
	}

	outDistance = tNear;
	// 始点が中に居ると入口面が決まらないので、来た方向へ押し返す形にしておく.
	outNormal = hasEntry ? entryNormal : -direction;
	return true;
}

} // namespace


/*---------------------------------
*
* 図形の変換・包含矩形
*
---------------------------------*/
OBB ToOBB(const AABB& aabb) {
	OBB obb; // 軸は既定でワールド軸そのまま.
	obb.center = GetCenter(aabb);
	obb.size = (aabb.max - aabb.min) * 0.5f;
	return obb;
}

AABB MakeAABB(const Vector3& center, const Vector3& size) {
	const Vector3 halfSize = size * 0.5f;
	return AABB{center - halfSize, center + halfSize};
}

AABB GetBounds(const Sphere& shape) {
	const Vector3 extent{shape.radius, shape.radius, shape.radius};
	return AABB{shape.center - extent, shape.center + extent};
}

AABB GetBounds(const AABB& shape) {
	return shape;
}

AABB GetBounds(const OBB& shape) {
	// 回転した箱を包む範囲は、各軸への半サイズの投影の和で求まる.
	Vector3 extent = Vector3::Zero;
	for (int i = 0; i < 3; ++i) {
		const Vector3 axis = shape.orientations[i] * std::abs(GetAxis(shape.size, i));
		extent.x += std::abs(axis.x);
		extent.y += std::abs(axis.y);
		extent.z += std::abs(axis.z);
	}
	return AABB{shape.center - extent, shape.center + extent};
}


/*---------------------------------
*
* 衝突判定
*
---------------------------------*/
CollisionInfo3D Collide(const Sphere& a, const Sphere& b) {
	CollisionInfo3D info;

	const Vector3 diff = a.center - b.center;
	const float distanceSquared = Dot(diff, diff);
	const float radiusSum = a.radius + b.radius;

	// 平方根を取らずに比較して、離れている場合は早期に抜ける.
	if (distanceSquared >= radiusSum * radiusSum) {
		return info;
	}

	const float distance = std::sqrt(distanceSquared);

	info.isHit = true;
	info.depth = radiusSum - distance;
	// 中心が完全に一致していると向きが決まらないので、便宜上Y+へ押し出す.
	info.normal = (distance > kEpsilon) ? (diff / distance) : Vector3::UnitY;
	return info;
}

CollisionInfo3D Collide(const AABB& a, const AABB& b) {
	CollisionInfo3D info;

	// 候補軸がX,Y,Zに固定されるので、SATを展開した形で書ける.
	const float overlaps[3] = {
		(std::min)(a.max.x, b.max.x) - (std::max)(a.min.x, b.min.x),
		(std::min)(a.max.y, b.max.y) - (std::max)(a.min.y, b.min.y),
		(std::min)(a.max.z, b.max.z) - (std::max)(a.min.z, b.min.z),
	};

	int bestAxis = 0;
	for (int i = 0; i < 3; ++i) {
		if (overlaps[i] <= 0.0f) {
			return info;
		}
		if (overlaps[i] < overlaps[bestAxis]) {
			bestAxis = i;
		}
	}

	info.isHit = true;
	info.depth = overlaps[bestAxis];
	info.normal = OrientNormal(GetUnitAxis(bestAxis), GetCenter(a), GetCenter(b));
	return info;
}

CollisionInfo3D Collide(const OBB& a, const OBB& b) {
	CollisionInfo3D info;

	float minOverlap = (std::numeric_limits<float>::max)();
	Vector3 bestAxis = Vector3::Zero;

	// 両者の面の法線（各3本）.
	for (const Vector3& axis : a.orientations) {
		if (!TestAxis(a, b, axis, 1.0f, minOverlap, bestAxis)) {
			return info;
		}
	}
	for (const Vector3& axis : b.orientations) {
		if (!TestAxis(a, b, axis, 1.0f, minOverlap, bestAxis)) {
			return info;
		}
	}
	// 辺どうしの外積（9本）。面の法線だけでは、辺と辺がかすめる配置の分離を見逃す.
	for (const Vector3& axisA : a.orientations) {
		for (const Vector3& axisB : b.orientations) {
			const Vector3 cross = Vector3::CrossProduct(axisA, axisB);
			const float length = Vector3::Length(cross);
			if (length <= kParallelEpsilon) {
				continue;
			}
			if (!TestAxis(a, b, cross / length, kEdgeAxisBias, minOverlap, bestAxis)) {
				return info;
			}
		}
	}

	// すべての軸で重なった。分離軸が存在しないので衝突している.
	info.isHit = true;
	info.depth = minOverlap;
	info.normal = OrientNormal(bestAxis, a.center, b.center);
	return info;
}

CollisionInfo3D Collide(const Sphere& a, const OBB& b) {
	CollisionInfo3D info;

	// 球の中心を箱のローカル座標で表し、箱の中へ押し込んだ点が最近点になる.
	const Vector3 offset = a.center - b.center;
	const float halfSize[3] = {std::abs(b.size.x), std::abs(b.size.y), std::abs(b.size.z)};
	float local[3] = {};
	bool isInside = true;
	Vector3 closest = b.center;
	for (int i = 0; i < 3; ++i) {
		local[i] = Dot(offset, b.orientations[i]);
		if (local[i] < -halfSize[i] || local[i] > halfSize[i]) {
			isInside = false;
		}
		closest += b.orientations[i] * std::clamp(local[i], -halfSize[i], halfSize[i]);
	}

	if (!isInside) {
		const Vector3 diff = a.center - closest;
		const float distanceSquared = Dot(diff, diff);
		if (distanceSquared >= a.radius * a.radius) {
			return info;
		}
		const float distance = std::sqrt(distanceSquared);
		if (distance > kEpsilon) {
			info.isHit = true;
			info.depth = a.radius - distance;
			info.normal = diff / distance;
			return info;
		}
		// 中心がちょうど表面上にあり、向きが決まらない。内側と同じ扱いにする.
	}

	// 中心が箱の内側にある。最も浅い面から外へ押し出す.
	int nearestAxis = 0;
	float minPenetration = (std::numeric_limits<float>::max)();
	for (int i = 0; i < 3; ++i) {
		const float penetration = halfSize[i] - std::abs(local[i]);
		if (penetration < minPenetration) {
			minPenetration = penetration;
			nearestAxis = i;
		}
	}
	const float sign = (local[nearestAxis] >= 0.0f) ? 1.0f : -1.0f;

	info.isHit = true;
	info.depth = minPenetration + a.radius;
	info.normal = b.orientations[nearestAxis] * sign;
	return info;
}

CollisionInfo3D Collide(const OBB& a, const Sphere& b) {
	return FlipNormal(Collide(b, a));
}

CollisionInfo3D Collide(const Sphere& a, const AABB& b) {
	return Collide(a, ToOBB(b));
}

CollisionInfo3D Collide(const AABB& a, const Sphere& b) {
	return FlipNormal(Collide(b, ToOBB(a)));
}

CollisionInfo3D Collide(const OBB& a, const AABB& b) {
	return Collide(a, ToOBB(b));
}

CollisionInfo3D Collide(const AABB& a, const OBB& b) {
	return Collide(ToOBB(a), b);
}


/*---------------------------------
*
* レイ
*
---------------------------------*/
RaycastHit3D Raycast(const Vector3& origin, const Vector3& direction, float distance, const Sphere& shape) {
	RaycastHit3D hit;

	const Vector3 toCenter = shape.center - origin;
	const float radiusSquared = shape.radius * shape.radius;
	const float distanceSquared = Dot(toCenter, toCenter);

	// 始点が球の中。箱と揃えて、距離0で当たったことにする.
	if (distanceSquared <= radiusSquared) {
		hit.isHit = true;
		hit.point = origin;
		hit.normal = -direction;
		return hit;
	}

	// 中心を進行方向へ落とした点までの距離と、そこから中心までの距離を出す.
	const float projection = Dot(toCenter, direction);
	if (projection < 0.0f) {
		// 球が後ろにあるので当たらない.
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
	hit.normal = Vector3::Normalize(hit.point - shape.center);
	return hit;
}

RaycastHit3D Raycast(const Vector3& origin, const Vector3& direction, float distance, const AABB& shape) {
	RaycastHit3D hit;

	float hitDistance = 0.0f;
	Vector3 hitNormal = Vector3::Zero;
	if (!RaycastSlab(origin, direction, shape.min, shape.max, distance, hitDistance, hitNormal)) {
		return hit;
	}

	hit.isHit = true;
	hit.distance = hitDistance;
	hit.point = origin + direction * hitDistance;
	hit.normal = hitNormal;
	return hit;
}

RaycastHit3D Raycast(const Vector3& origin, const Vector3& direction, float distance, const OBB& shape) {
	RaycastHit3D hit;

	// OBBのローカル空間へ移せば、そのままスラブ法が使える.
	const Vector3 offset = origin - shape.center;
	const Vector3 localOrigin{
		Dot(offset, shape.orientations[0]),
		Dot(offset, shape.orientations[1]),
		Dot(offset, shape.orientations[2]),
	};
	const Vector3 localDirection{
		Dot(direction, shape.orientations[0]),
		Dot(direction, shape.orientations[1]),
		Dot(direction, shape.orientations[2]),
	};
	const Vector3 halfSize{std::abs(shape.size.x), std::abs(shape.size.y), std::abs(shape.size.z)};

	float hitDistance = 0.0f;
	Vector3 localNormal = Vector3::Zero;
	if (!RaycastSlab(localOrigin, localDirection, -halfSize, halfSize, distance, hitDistance, localNormal)) {
		return hit;
	}

	// 法線だけワールドへ戻す。距離は回転で変わらない.
	hit.isHit = true;
	hit.distance = hitDistance;
	hit.point = origin + direction * hitDistance;
	hit.normal = shape.orientations[0] * localNormal.x + shape.orientations[1] * localNormal.y + shape.orientations[2] * localNormal.z;
	return hit;
}

}	// namespace Cake

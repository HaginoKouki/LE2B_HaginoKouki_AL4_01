#include "Quaternion.h"

#include <cmath>
#include <numbers>

#include "System/Foundation/Math/Vector.h"

namespace Cake {

Quaternion Quaternion::operator+(const Quaternion& other) const {
	return Quaternion(
		x + other.x,
		y + other.y,
		z + other.z,
		w + other.w
	);
}
Quaternion Quaternion::operator-(const Quaternion& other) const {
	return Quaternion(
		x - other.x,
		y - other.y,
		z - other.z,
		w - other.w
	);
}
Quaternion Quaternion::operator*(const Quaternion& other) const {
	return Quaternion(
		w * other.x + x * other.w + y * other.z - z * other.y, // x.
		w * other.y - x * other.z + y * other.w + z * other.x, // y.
		w * other.z + x * other.y - y * other.x + z * other.w, // z.
		w * other.w - x * other.x - y * other.y - z * other.z  // w.
	);
}

Quaternion Quaternion::Normalize(const Quaternion& q) {
	float length = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
	if (length < 1e-6f) {
		return Identity;
	}
	return Quaternion(q.x / length, q.y / length, q.z / length, q.w / length);
}

Quaternion Quaternion::AngleAxis(float angleRad, const Vector3& axis) {
	// 軸は必ず正規化する.
	Vector3 n = Vector3::Normalize(axis);

	float s = std::sinf(angleRad / 2.0f);
	float c = std::cosf(angleRad / 2.0f);

	return Quaternion(
		n.x * s,
		n.y * s,
		n.z * s,
		c
	);
}

Vector3 Quaternion::ToEuler(const Quaternion& q) {
	Vector3 angles;

	// roll (X軸まわりの回転)
	float sinXCosp = 2.0f * (q.w * q.x + q.y * q.z);
	float cosXCosp = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
	angles.x = atan2(sinXCosp, cosXCosp);

	// pitch (Y軸まわりの回転)
	float sinY = 2.0f * (q.w * q.y - q.z * q.x);
	if (fabs(sinY) >= 1.0f)
		angles.y = static_cast<float>(copysign(std::numbers::pi / 2.0f, sinY)); // ジンバルロック時は±90°に固定
	else
		angles.y = asin(sinY);

	// yaw (Z軸まわりの回転)
	float sinZCosp = 2.0f * (q.w * q.z + q.x * q.y);
	float cosZCosp = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
	angles.z = atan2(sinZCosp, cosZCosp);

	return angles;
}
Quaternion Quaternion::FromEuler(const Vector3& euler) {
	// 各軸の半角の sin / cos
	float cr = cos(euler.x * 0.5f);
	float sr = sin(euler.x * 0.5f);
	float cp = cos(euler.y * 0.5f);
	float sp = sin(euler.y * 0.5f);
	float cy = cos(euler.z * 0.5f);
	float sy = sin(euler.z * 0.5f);

	Quaternion q;
	q.w = cr * cp * cy + sr * sp * sy;
	q.x = sr * cp * cy - cr * sp * sy;
	q.y = cr * sp * cy + sr * cp * sy;
	q.z = cr * cp * sy - sr * sp * cy;

	return q;
}

} // namespace Cake

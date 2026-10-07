#include "Vector.h"

#include "System/Foundation/Math/Matrix.h"

#include <algorithm>
#include <numbers>
#include <cmath>


namespace Cake {

/*---------------------------------
*
* Vector2
*
---------------------------------*/
#pragma region Operators
// 算術演算子.
Vector2 Vector2::operator+(const Vector2& other) const {
	Vector2 num;
	num.x = x + other.x;
	num.y = y + other.y;
	return num;
}
Vector2 Vector2::operator-(const Vector2& other) const {
	Vector2 num;
	num.x = x - other.x;
	num.y = y - other.y;
	return num;
}
Vector2& Vector2::operator+=(const Vector2& other) {
	x += other.x;
	y += other.y;
	return *this;
}
Vector2& Vector2::operator-=(const Vector2& other) {
	x -= other.x;
	y -= other.y;
	return *this;
}
Vector2 Vector2::Multiply(const Vector2& a, const Vector2& b) {
	Vector2 result = {
		a.x * b.x,
		a.y * b.y
	};
	return result;
}

// 比較演算子.
bool Vector2::operator==(const Vector2& other) const {
	return (x == other.x && y == other.y);
}

#pragma endregion

// 同次座標系に変換し、積を代入.
Vector2 Vector2::Transform(const Vector2& vector, const Matrix3x3& matrix) {
	Vector2 result;
	result.x = (vector.x * matrix.m[0][0]) + (vector.y * matrix.m[1][0]) + (1.0f * matrix.m[2][0]);
	result.y = (vector.x * matrix.m[0][1]) + (vector.y * matrix.m[1][1]) + (1.0f * matrix.m[2][1]);
	float w = (vector.x * matrix.m[0][2]) + (vector.y * matrix.m[1][2]) + (1.0f * matrix.m[2][2]);

	assert(w != 0.0f);

	result.x /= w;
	result.y /= w;

	return result;
}

float Vector2::Length(const Vector2& a, const Vector2& b) {
	Vector2 local = b - a;
	return sqrtf(DotProduct(local, local));
}

Vector2 Vector2::Normalize(const Vector2& target) {
	float length = Vector2::Length(Vector2::Zero, target);
	if (length == 0.0f) {
		return Vector2::Zero;
	}
	return target / length;
};

Vector2 Vector2::Clamp(const Vector2& value, const Vector2& min, const Vector2& max) {
	Vector2 result;
	result.x = std::clamp(value.x, min.x, max.x);
	result.y = std::clamp(value.y, min.y, max.y);
	return result;
}

float Vector2::DotProduct(const Vector2& a, const Vector2& b) {
	return (a.x * b.x) + (a.y * b.y);
}
float Vector2::CrossProduct(const Vector2& a, const Vector2& b) {
	return (a.x * b.y) - (a.y * b.x);
}

float Vector2::ToDegree(const Vector2& target) {
	return atan2f(target.y, target.x) * 180.0f / std::numbers::pi_v<float>;
}
Vector2 Vector2::FromDegreeUnit(const float& degree) {
	Vector2 result;
	result.x = cosf(degree * std::numbers::pi_v<float> / 180.0f);
	result.y = sinf(degree * std::numbers::pi_v<float> / 180.0f);
	result = Normalize(result);
	return result;
}

float Vector2::ToRadian(const Vector2& target) {
	return atan2f(target.y, target.x);
}
Vector2 Vector2::FromRadianUnit(const float& radian) {
	Vector2 result;
	result.x = cosf(radian);
	result.y = sinf(radian);
	result = Normalize(result);
	return result;
}

float Vector2::SignedAngleDifference(const Vector2& target) const {
	double angleA = (double)ToRadian(*this);
	double angleB = (double)ToRadian(target);

	// 浮動小数点の剰余演算を使用して-π〜πの範囲に収める.
	float diff = (float)std::remainder(angleB - angleA, 2.0f * std::numbers::pi_v<float>);

	return diff;
}

/*---------------------------------
*
* Vector3
*
---------------------------------*/
#pragma region Operators
// 算術演算子.
Vector3 Vector3::operator+(const Vector3& other) const {
	Vector3 num;
	num.x = x + other.x;
	num.y = y + other.y;
	num.z = z + other.z;
	return num;
}
Vector3 Vector3::operator-(const Vector3& other) const {
	Vector3 num;
	num.x = x - other.x;
	num.y = y - other.y;
	num.z = z - other.z;
	return num;
}
Vector3& Vector3::operator+=(const Vector3& other) {
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}
Vector3& Vector3::operator-=(const Vector3& other) {
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}
Vector3 Vector3::Multiply(const Vector3& a, const Vector3& b) {
	Vector3 result = {
		a.x * b.x,
		a.y * b.y,
		a.z * b.z
	};
	return result;
}

// 比較演算子.
bool Vector3::operator==(const Vector3& other) const {
	return (x == other.x && y == other.y && z == other.z);
}

#pragma endregion

Vector3 Vector3::Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result;
	result.x = (vector.x * matrix.m[0][0]) + (vector.y * matrix.m[1][0]) + (vector.z * matrix.m[2][0]) + (1.0f * matrix.m[3][0]);
	result.y = (vector.x * matrix.m[0][1]) + (vector.y * matrix.m[1][1]) + (vector.z * matrix.m[2][1]) + (1.0f * matrix.m[3][1]);
	result.z = (vector.x * matrix.m[0][2]) + (vector.y * matrix.m[1][2]) + (vector.z * matrix.m[2][2]) + (1.0f * matrix.m[3][2]);
	float w = (vector.x * matrix.m[0][3]) + (vector.y * matrix.m[1][3]) + (vector.z * matrix.m[2][3]) + (1.0f * matrix.m[3][3]);

	if (w != 0.0f) {
		result.x /= w;
		result.y /= w;
		result.z /= w;
	} else {
		result = Vector3::Zero; // wが0の場合は、結果をゼロベクトルにする（または適切な処理を行う）。
	}

	return result;
}
Vector3 Vector3::TransformNormal(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result;
	result.x = (vector.x * matrix.m[0][0]) + (vector.y * matrix.m[1][0]) + (vector.z * matrix.m[2][0]);
	result.y = (vector.x * matrix.m[0][1]) + (vector.y * matrix.m[1][1]) + (vector.z * matrix.m[2][1]);
	result.z = (vector.x * matrix.m[0][2]) + (vector.y * matrix.m[1][2]) + (vector.z * matrix.m[2][2]);
	return result;
}

float Vector3::Length(const Vector3& a, const Vector3& b) {
	Vector3 local = b - a;
	return sqrtf(DotProduct(local, local));
}

Vector3 Vector3::Normalize(const Vector3& target) {
	float length = Vector3::Length(Vector3::Zero, target);
	if (length == 0.0f) {
		return Vector3::Zero;
	}
	return target / length;
};

Vector3 Vector3::Clamp(const Vector3& value, const Vector3& min, const Vector3& max) {
	Vector3 result;
	result.x = std::clamp(value.x, min.x, max.x);
	result.y = std::clamp(value.y, min.y, max.y);
	result.z = std::clamp(value.z, min.z, max.z);
	return result;
}

float Vector3::DotProduct(const Vector3& a, const Vector3& b) {
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}
Vector3 Vector3::CrossProduct(const Vector3& a, const Vector3& b) {
	return Vector3(
		(a.y * b.z) - (a.z * b.y),
		(a.z * b.x) - (a.x * b.z),
		(a.x * b.y) - (a.y * b.x)
	);
}

/*---------------------------------
*
* Vector4
*
---------------------------------*/
#pragma region Operators
// 算術演算子.
Vector4 Vector4::operator+(const Vector4& other) const {
	Vector4 num;
	num.x = x + other.x;
	num.y = y + other.y;
	num.z = z + other.z;
	num.w = w + other.w;
	return num;
}
Vector4 Vector4::operator-(const Vector4& other) const {
	Vector4 num;
	num.x = x - other.x;
	num.y = y - other.y;
	num.z = z - other.z;
	num.w = w - other.w;
	return num;
}
Vector4& Vector4::operator+=(const Vector4& other) {
	x += other.x;
	y += other.y;
	z += other.z;
	w += other.w;
	return *this;
}
Vector4& Vector4::operator-=(const Vector4& other) {
	x -= other.x;
	y -= other.y;
	z -= other.z;
	w -= other.w;
	return *this;
}
Vector4 Vector4::Multiply(const Vector4& a, const Vector4& b) {
	Vector4 result = {
		a.x * b.x,
		a.y * b.y,
		a.z * b.z,
		a.w * b.w
	};
	return result;
}

// 比較演算子.
bool Vector4::operator==(const Vector4& other) const {
	return (x == other.x && y == other.y && z == other.z && w == other.w);
}
#pragma endregion

float Vector4::Length(const Vector4& a, const Vector4& b) {
	Vector4 local = b - a;
	return sqrtf(DotProduct(local, local));
}

Vector4 Vector4::Normalize(const Vector4& target) {
	float length = Vector4::Length(Vector4::Zero, target);
	if (length == 0.0f) {
		return Vector4::Zero;
	}
	return target / length;
};

Vector4 Vector4::Clamp(const Vector4& value, const Vector4& min, const Vector4& max) {
	Vector4 result;
	result.x = std::clamp(value.x, min.x, max.x);
	result.y = std::clamp(value.y, min.y, max.y);
	result.z = std::clamp(value.z, min.z, max.z);
	result.w = std::clamp(value.w, min.w, max.w);
	return result;
}

float Vector4::DotProduct(const Vector4& a, const Vector4& b) {
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

} // namespace Cake

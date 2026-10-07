#pragma once

#include <cmath>
#include <numbers>
#include "Vector.h"

namespace Cake {
namespace Math {
#pragma region 角度系の変換
inline float ToRadian(float degree) {
	return degree * (std::numbers::pi_v<float> / 180.0f);
}
inline float ToRadian(Cake::Vector2 vector) {
	return atan2f(vector.y, vector.x);
}
inline float ToDegree(float radian) {
	return radian * (180.0f / std::numbers::pi_v<float>);
}
inline float ToDegree(Cake::Vector2 vector) {
	return ToDegree(atan2f(vector.y, vector.x));
}
inline Cake::Vector2 ToUnitVector(float radian) {
	return Cake::Vector2{cosf(radian), sinf(radian)};
}
inline Cake::Vector2 ToUnitVector(float radian, float length) {
	return Cake::Vector2{cosf(radian) * length, sinf(radian) * length};
}
#pragma endregion
} // namespace Math
} // namespace Cake

#pragma region KamataEngine との変換
#include "KamataEngine.h"
#include "Matrix.h"

inline KamataEngine::Vector2 ToKamata(const Cake::Vector2& v) {
	return KamataEngine::Vector2{v.x, v.y};
}
inline KamataEngine::Vector3 ToKamata(const Cake::Vector3& v) {
	return KamataEngine::Vector3{v.x, v.y, v.z};
}
inline KamataEngine::Vector4 ToKamata(const Cake::Vector4& v) {
	return KamataEngine::Vector4{v.x, v.y, v.z, v.w};
}
// 行の並び（行ベクトル形式）はどちらも同じなので、要素をそのまま写す.
inline KamataEngine::Matrix4x4 ToKamata(const Cake::Matrix4x4& m) {
	KamataEngine::Matrix4x4 result{};
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			result.m[row][column] = m.m[row][column];
		}
	}
	return result;
}

inline Cake::Vector2 ToCake(const KamataEngine::Vector2& v) {
	return Cake::Vector2{v.x, v.y};
}
inline Cake::Vector3 ToCake(const KamataEngine::Vector3& v) {
	return Cake::Vector3{v.x, v.y, v.z};
}
inline Cake::Vector4 ToCake(const KamataEngine::Vector4& v) {
	return Cake::Vector4{v.x, v.y, v.z, v.w};
}
inline Cake::Matrix4x4 ToCake(const KamataEngine::Matrix4x4& m) {
	Cake::Matrix4x4 result;
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			result.m[row][column] = m.m[row][column];
		}
	}
	return result;
}
#pragma endregion

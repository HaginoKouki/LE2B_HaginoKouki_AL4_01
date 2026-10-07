#pragma once

namespace Cake {

class Vector2;
class Vector3;
class Vector4;

class Matrix2x2;
class Matrix3x3;
class Matrix4x4;

class Quaternion {
public:
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 1.0f;

public:
	Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
	Quaternion(float nX, float nY, float nZ, float nW) : x(nX), y(nY), z(nZ), w(nW) {}
	Quaternion(const Quaternion&) = default;
	Quaternion& operator=(const Quaternion&) = default;

	static const Quaternion Identity;

	Quaternion operator+(const Quaternion& other) const;
	Quaternion operator-(const Quaternion& other) const;
	Quaternion operator*(const Quaternion& other) const;


	/// <returns>正規化済みクォータニオン</returns>
	static Quaternion Normalize(const Quaternion&);

	/// <summary>
	/// 指定した軸と角度に基づいて回転を表すクォータニオンを作成する関数.
	/// </summary>
	/// <param name="angleRad">角度（ラジアン）</param>
	/// <param name="axis">回転軸</param>
	/// <returns>回転を表すクォータニオン</returns>
	static Quaternion AngleAxis(float angleRad, const Vector3& axis);

	/// <param name="">正規化済みクォータニオン</param>
	/// <returns>オイラー角（ラジアン）</returns>
	static Vector3 ToEuler(const Quaternion&);
	/// <param name="">オイラー角（ラジアン）</param>
	/// <returns>クォータニオン</returns>
	static Quaternion FromEuler(const Vector3&);
};
inline const Quaternion Quaternion::Identity{ 0.0f, 0.0f, 0.0f, 1.0f };
}

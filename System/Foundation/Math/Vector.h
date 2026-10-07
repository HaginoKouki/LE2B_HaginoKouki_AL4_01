#pragma once
/*====================================
 *
 * 数学ライブラリの基本：Vector2, Vector3, Vector4を定義する構造体群。
 * 算術演算（加減乗除）、内積・外積、正規化、クランプ等の
 * 各種ベクトル操作を静的メソッドとして提供する。
 * 行列演算等のすべての幾何学的計算の基盤となる。
 *
 * ====================================*/
#include <cassert>
#include <type_traits>


namespace Cake {

class Matrix2x2;
class Matrix3x3;
class Matrix4x4;

class Vector2 {
public:
	float x;
	float y;

public:
	Vector2() : x(0.0f), y(0.0f) {}
	Vector2(float nX, float nY) : x(nX), y(nY) {}
	Vector2(const Vector2&) = default;
	Vector2& operator=(const Vector2&) = default;

	static const Vector2 Zero;
	static const Vector2 One;

	static const Vector2 UnitX;
	static const Vector2 UnitY;

	// 算術演算子.
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector2 operator+(const T& other) const {
		Vector2 num;
		num.x = x + static_cast<float>(other);
		num.y = y + static_cast<float>(other);
		return num;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector2 operator-(const T& other) const {
		Vector2 num;
		num.x = x - static_cast<float>(other);
		num.y = y - static_cast<float>(other);
		return num;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector2 operator*(const T& other) const {
		Vector2 num;
		num.x = x * static_cast<float>(other);
		num.y = y * static_cast<float>(other);
		return num;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector2 operator/(const T& other) const {
		assert(other != 0 && "Division by zero error in Vector2::operator/");
		Vector2 num;
		num.x = x / static_cast<float>(other);
		num.y = y / static_cast<float>(other);
		return num;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector2& operator+=(const T& other) {
		this->x += static_cast<float>(other);
		this->y += static_cast<float>(other);
		return *this;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector2& operator-=(const T& other) {
		this->x -= static_cast<float>(other);
		this->y -= static_cast<float>(other);
		return *this;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector2& operator*=(const T& other) {
		this->x *= static_cast<float>(other);
		this->y *= static_cast<float>(other);
		return *this;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector2& operator/=(const T& other) {
		assert(other != 0 && "Division by zero error in Vector2::operator/=");
		this->x /= static_cast<float>(other);
		this->y /= static_cast<float>(other);
		return *this;
	}

	Vector2 operator+(const Vector2& other) const;
	Vector2 operator-(const Vector2& other) const;
	Vector2& operator+=(const Vector2& other);
	Vector2& operator-=(const Vector2& other);
	static Vector2 Multiply(const Vector2&, const Vector2&);

	// 比較演算子.
	bool operator==(const Vector2& other) const;

	// 単項演算子.
	Vector2 operator-() const {
		return { -x, -y };
	}
	Vector2& operator++() {
		x++;
		y++;
		return *this;
	}
	Vector2& operator--() {
		x--;
		y--;
		return *this;
	}

	// 同次座標系に変換.
	static Vector2 Transform(const Vector2&, const Matrix3x3& matrix);

	/// <returns>引数間の距離</returns>
	static float Length(const Vector2&, const Vector2& = Vector2::Zero);

	/// <returns>正規化済みベクトル</returns>
	static Vector2 Normalize(const Vector2&);

	/// <returns>valueをminとmaxの範囲に収めたベクトル</returns>
	static Vector2 Clamp(const Vector2& value, const Vector2& min, const Vector2& max);

	/// <returns>a, bの内積</returns>
	static float DotProduct(const Vector2& a, const Vector2& b);
	/// <returns>a, bの外積</returns>
	static float CrossProduct(const Vector2& a, const Vector2& b);

	/// <returns>角度(度数法)</returns>
	static float ToDegree(const Vector2&);
	/// <param name="degree">角度(度数法)</param>
	static Vector2 FromDegreeUnit(const float& degree);

	/// <returns>ラジアン</returns>
	static float ToRadian(const Vector2&);
	/// <param name="radian">ラジアン</param>
	static Vector2 FromRadianUnit(const float& radian);

	/// <returns>targetとの符号付き角度差</returns>
	float SignedAngleDifference(const Vector2& target) const;
};
inline const Vector2 Vector2::Zero{ 0.0f, 0.0f };
inline const Vector2 Vector2::One{ 1.0f, 1.0f };
inline const Vector2 Vector2::UnitX{ 1.0f, 0.0f };
inline const Vector2 Vector2::UnitY{ 0.0f, 1.0f };

template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0>
Vector2 operator*(T scalar, const Vector2& v) {
	return v * static_cast<float>(scalar);
}


class Vector3 {
public:
	float x;
	float y;
	float z;

public:
	Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
	Vector3(float nX, float nY, float nZ) : x(nX), y(nY), z(nZ) {}
	Vector3(const Vector3&) = default;
	Vector3& operator=(const Vector3&) = default;

	static const Vector3 Zero;
	static const Vector3 One;

	static const Vector3 UnitX;
	static const Vector3 UnitY;
	static const Vector3 UnitZ;

	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector3 operator+(const T& other) const {
		Vector3 num;
		num.x = x + static_cast<float>(other);
		num.y = y + static_cast<float>(other);
		num.z = z + static_cast<float>(other);
		return num;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector3 operator-(const T& other) const {
		Vector3 num;
		num.x = x - static_cast<float>(other);
		num.y = y - static_cast<float>(other);
		num.z = z - static_cast<float>(other);
		return num;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector3 operator*(const T& other) const {
		Vector3 num;
		num.x = x * static_cast<float>(other);
		num.y = y * static_cast<float>(other);
		num.z = z * static_cast<float>(other);
		return num;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector3 operator/(const T& other) const {
		assert(other != 0 && "Division by zero error in Vector3::operator/");
		Vector3 num;
		num.x = x / static_cast<float>(other);
		num.y = y / static_cast<float>(other);
		num.z = z / static_cast<float>(other);
		return num;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector3& operator+=(const T& other) {
		x += static_cast<float>(other);
		y += static_cast<float>(other);
		z += static_cast<float>(other);
		return *this;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector3& operator-=(const T& other) {
		x -= static_cast<float>(other);
		y -= static_cast<float>(other);
		z -= static_cast<float>(other);
		return *this;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector3& operator*=(const T& other) {
		x *= static_cast<float>(other);
		y *= static_cast<float>(other);
		z *= static_cast<float>(other);
		return *this;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector3& operator/=(const T& other) {
		assert(other != 0 && "Division by zero error in Vector3::operator/=");
		x /= static_cast<float>(other);
		y /= static_cast<float>(other);
		z /= static_cast<float>(other);
		return *this;
	}

	Vector3 operator+(const Vector3& other) const;
	Vector3 operator-(const Vector3& other) const;
	Vector3& operator+=(const Vector3& other);
	Vector3& operator-=(const Vector3& other);
	static Vector3 Multiply(const Vector3&, const Vector3&);

	bool operator==(const Vector3& other) const;

	Vector3 operator-() const {
		return { -x, -y, -z };
	}
	Vector3& operator++() {
		x++;
		y++;
		z++;
		return *this;
	}
	Vector3& operator--() {
		x--;
		y--;
		z--;
		return *this;
	}

	// 同次座標系に変換.
	static Vector3 Transform(const Vector3&, const Matrix4x4& matrix);
	static Vector3 TransformNormal(const Vector3&, const Matrix4x4& matrix);

	/// <returns>引数間の距離</returns>
	static float Length(const Vector3&, const Vector3& = Vector3::Zero);

	/// <returns>正規化済みベクトル</returns>
	static Vector3 Normalize(const Vector3&);

	/// <returns>valueをminとmaxの範囲内に収めたベクトル</returns>
	static Vector3 Clamp(const Vector3& value, const Vector3& min, const Vector3& max);

	/// <returns>a, bの内積</returns>
	static float DotProduct(const Vector3& a, const Vector3& b);
	/// <returns>a, bの外積</returns>
	static Vector3 CrossProduct(const Vector3& a, const Vector3& b);
};
inline const Vector3 Vector3::Zero{ 0.0f, 0.0f, 0.0f };
inline const Vector3 Vector3::One{ 1.0f, 1.0f, 1.0f };
inline const Vector3 Vector3::UnitX{ 1.0f, 0.0f, 0.0f };
inline const Vector3 Vector3::UnitY{ 0.0f, 1.0f, 0.0f };
inline const Vector3 Vector3::UnitZ{ 0.0f, 0.0f, 1.0f };

template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0>
Vector3 operator*(T scalar, const Vector3& v) {
	return v * static_cast<float>(scalar);
}


class Vector4 {
public:
	float x;
	float y;
	float z;
	float w;

public:
	Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
	Vector4(float nX, float nY, float nZ, float nW) : x(nX), y(nY), z(nZ), w(nW) {}
	Vector4(const Vector4&) = default;
	Vector4& operator=(const Vector4&) = default;

	static const Vector4 Zero;
	static const Vector4 One;

	static const Vector4 UnitX;
	static const Vector4 UnitY;
	static const Vector4 UnitZ;
	static const Vector4 UnitW;

	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector4 operator+(const T& other) const {
		Vector4 num;
		num.x = x + static_cast<float>(other);
		num.y = y + static_cast<float>(other);
		num.z = z + static_cast<float>(other);
		num.w = w + static_cast<float>(other);
		return num;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector4 operator-(const T& other) const {
		Vector4 num;
		num.x = x - static_cast<float>(other);
		num.y = y - static_cast<float>(other);
		num.z = z - static_cast<float>(other);
		num.w = w - static_cast<float>(other);
		return num;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector4 operator*(const T& other) const {
		Vector4 num;
		num.x = x * static_cast<float>(other);
		num.y = y * static_cast<float>(other);
		num.z = z * static_cast<float>(other);
		num.w = w * static_cast<float>(other);
		return num;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector4 operator/(const T& other) const {
		assert(other != 0 && "Division by zero error in Vector4::operator/");
		Vector4 num;
		num.x = x / static_cast<float>(other);
		num.y = y / static_cast<float>(other);
		num.z = z / static_cast<float>(other);
		num.w = w / static_cast<float>(other);
		return num;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector4& operator+=(const T& other) {
		x += static_cast<float>(other);
		y += static_cast<float>(other);
		z += static_cast<float>(other);
		w += static_cast<float>(other);
		return *this;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector4& operator-=(const T& other) {
		x -= static_cast<float>(other);
		y -= static_cast<float>(other);
		z -= static_cast<float>(other);
		w -= static_cast<float>(other);
		return *this;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector4& operator*=(const T& other) {
		x *= static_cast<float>(other);
		y *= static_cast<float>(other);
		z *= static_cast<float>(other);
		w *= static_cast<float>(other);
		return *this;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Vector4& operator/=(const T& other) {
		assert(other != 0 && "Division by zero error in Vector4::operator/=");
		x /= static_cast<float>(other);
		y /= static_cast<float>(other);
		z /= static_cast<float>(other);
		w /= static_cast<float>(other);
		return *this;
	}

	Vector4 operator+(const Vector4& other) const;
	Vector4 operator-(const Vector4& other) const;
	Vector4& operator+=(const Vector4& other);
	Vector4& operator-=(const Vector4& other);
	static Vector4 Multiply(const Vector4&, const Vector4&);

	bool operator==(const Vector4& other) const;

	Vector4 operator-() const {
		return { -x, -y, -z, -w };
	}
	Vector4& operator++() {
		x++;
		y++;
		z++;
		w++;
		return *this;
	}
	Vector4& operator--() {
		x--;
		y--;
		z--;
		w--;
		return *this;
	}


	/// <returns>引数間の距離</returns>
	static float Length(const Vector4&, const Vector4& = Vector4::Zero);

	/// <returns>正規化済みベクトル</returns>
	static Vector4 Normalize(const Vector4&);

	/// <returns>valueをminとmaxの範囲内に収めたベクトル</returns>
	static Vector4 Clamp(const Vector4& value, const Vector4& min, const Vector4& max);

	/// <returns>a, bの内積</returns>
	static float DotProduct(const Vector4& a, const Vector4& b);
};
inline const Vector4 Vector4::Zero{ 0.0f, 0.0f, 0.0f, 0.0f };
inline const Vector4 Vector4::One{ 1.0f, 1.0f, 1.0f, 1.0f };
inline const Vector4 Vector4::UnitX{ 1.0f, 0.0f, 0.0f, 0.0f };
inline const Vector4 Vector4::UnitY{ 0.0f, 1.0f, 0.0f, 0.0f };
inline const Vector4 Vector4::UnitZ{ 0.0f, 0.0f, 1.0f, 0.0f };
inline const Vector4 Vector4::UnitW{ 0.0f, 0.0f, 0.0f, 1.0f };

template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0>
Vector4 operator*(T scalar, const Vector4& v) {
	return v * static_cast<float>(scalar);
}

} // namespace Cake

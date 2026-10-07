#pragma once
/*====================================
 *
 * 数学ライブラリの基本：Matrix2x2, Matrix3x3, Matrix4x4を定義する構造体群。
 * 算術演算（加減乗除）、内積・外積、正規化等の
 * 各種操作を静的メソッドとして提供する。
 * 行列演算等のすべての幾何学的計算の基盤となる。
 *
 * ====================================*/
#include <cassert>
#include <type_traits>

namespace Cake {

class Vector2;
class Vector3;
class Vector4;


class Matrix2x2 {
public:
	float m[2][2];

public:
	Matrix2x2(
		float m0_0 = 0.0f, float m0_1 = 0.0f,
		float m1_0 = 0.0f, float m1_1 = 0.0f
	);
	static const Matrix2x2 Zero;
	static const Matrix2x2 One;
	static const Matrix2x2 Identity;

	Matrix2x2 operator+(const Matrix2x2& other) const;
	Matrix2x2 operator-(const Matrix2x2& other) const;
	Matrix2x2 operator*(const Matrix2x2& other) const;
	Matrix2x2& operator=(const Matrix2x2& other);
	Matrix2x2& operator+=(const Matrix2x2& other);
	Matrix2x2& operator-=(const Matrix2x2& other);
	Matrix2x2& operator*=(const Matrix2x2& other);

	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Matrix2x2 operator*(const T& other) const {
		Matrix2x2 result;
		for (int y = 0; y < 2; y++) {
			for (int x = 0; x < 2; x++) {
				result.m[y][x] = m[y][x] * static_cast<float>(other);
			}
		}
		return result;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Matrix2x2& operator*=(const T& other) {
		Matrix2x2 result;
		for (int y = 0; y < 2; y++) {
			for (int x = 0; x < 2; x++) {
				result.m[y][x] = m[y][x] * static_cast<float>(other);
			}
		}
		*this = result;
		return *this;
	}

	/// <returns>逆行列</returns>
	static Matrix2x2 Inverse(const Matrix2x2& matrix);
	/// <returns>転置行列</returns>
	static Matrix2x2 Transpose(const Matrix2x2& matrix);

	/// <returns>回転行列</returns>
	static Matrix2x2 MakeRotationMatrix(const float& theta);
};
inline const Matrix2x2 Matrix2x2::Zero = Matrix2x2(
	0.0f, 0.0f,
	0.0f, 0.0f
);
inline const Matrix2x2 Matrix2x2::One = Matrix2x2(
	1.0f, 1.0f,
	1.0f, 1.0f
);
inline const Matrix2x2 Matrix2x2::Identity = Matrix2x2(
	1.0f, 0.0f,
	0.0f, 1.0f
);

template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0>
Matrix2x2 operator*(T scalar, const Matrix2x2& m) {
	return m * static_cast<float>(scalar);
}



class Matrix3x3 {
public:
	float m[3][3];

public:
	// コンストラクタ.
	Matrix3x3(
		float m00 = 0.0f, float m01 = 0.0f, float m02 = 0.0f,
		float m10 = 0.0f, float m11 = 0.0f, float m12 = 0.0f,
		float m20 = 0.0f, float m21 = 0.0f, float m22 = 0.0f
	);
	static const Matrix3x3 Zero;
	static const Matrix3x3 One;
	static const Matrix3x3 Identity;

	Matrix3x3 operator+(const Matrix3x3& other) const;
	Matrix3x3 operator-(const Matrix3x3& other) const;
	Matrix3x3 operator*(const Matrix3x3& other) const;
	Matrix3x3& operator=(const Matrix3x3& other);
	Matrix3x3& operator+=(const Matrix3x3& other);
	Matrix3x3& operator-=(const Matrix3x3& other);
	Matrix3x3& operator*=(const Matrix3x3& other);

	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Matrix3x3 operator*(const T& other) const {
		Matrix3x3 result;
		for (int y = 0; y < 3; y++) {
			for (int x = 0; x < 3; x++) {
				result.m[y][x] = m[y][x] * static_cast<float>(other);
			}
		}
		return result;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Matrix3x3& operator*=(const T& other) {
		Matrix3x3 result;
		for (int y = 0; y < 3; y++) {
			for (int x = 0; x < 3; x++) {
				result.m[y][x] = m[y][x] * static_cast<float>(other);
			}
		}
		*this = result;
		return *this;
	}

	/// <returns>逆行列</returns>
	static Matrix3x3 Inverse(const Matrix3x3&);
	/// <returns>転置行列</returns>
	static Matrix3x3 Transpose(const Matrix3x3&);

	/// <returns>平行移動行列</returns>
	static Matrix3x3 MakeTranslateMatrix(const Vector2& translate);
	/// <returns>回転行列</returns>
	static Matrix3x3 MakeRotationMatrix(const float& theta);
	/// <returns>拡大縮小行列</returns>
	static Matrix3x3 MakeScalingMatrix(const Vector2& scale);
	/// <returns>アフィン変換行列</returns>
	static Matrix3x3 MakeAffineMatrix(const Vector2& scale, const float& theta, const Vector2& translate);

	/// <summary>
	/// カメラ座標系から正規化デバイス座標系(NDC)へ変換するための正射影行列を作成する.
	/// </summary>
	/// <param name="left">カメラ座標系での左端</param>
	/// <param name="top">カメラ座標系での上端</param>
	/// <param name="right">カメラ座標系での右端</param>
	/// <param name="bottom">カメラ座標系での下端</param>
	/// <returns>正射影行列</returns>
	static Matrix3x3 MakeOrthoMatrix(const float& left, const float& top, const float& right, const float& bottom);
	/// <summary>
	/// 正規化デバイス座標系(NDC)からビューポート座標系へ変換するためのビューポート行列を作成する.
	/// </summary>
	/// <param name="left">スクリーン座標系での左端</param>
	/// <param name="top">スクリーン座標系での上端</param>
	/// <param name="width">横幅</param>
	/// <param name="height">縦幅</param>
	/// <returns>ビューポート行列</returns>
	static Matrix3x3 MakeViewportMatrix(const float& left, const float& top, const float& width, const float& height);
};
inline const Matrix3x3 Matrix3x3::Zero = Matrix3x3(
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f
);
inline const Matrix3x3 Matrix3x3::One = Matrix3x3(
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f
);
inline const Matrix3x3 Matrix3x3::Identity = Matrix3x3(
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f
);

template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0>
Matrix3x3 operator*(T scalar, const Matrix3x3& m) {
	return m * static_cast<float>(scalar);
}



class Matrix4x4 {
public:
	float m[4][4];

public:
	// コンストラクタ.
	Matrix4x4(
		float m00 = 0.0f, float m01 = 0.0f, float m02 = 0.0f, float m03 = 0.0f,
		float m10 = 0.0f, float m11 = 0.0f, float m12 = 0.0f, float m13 = 0.0f,
		float m20 = 0.0f, float m21 = 0.0f, float m22 = 0.0f, float m23 = 0.0f,
		float m30 = 0.0f, float m31 = 0.0f, float m32 = 0.0f, float m33 = 0.0f
	);
	static const Matrix4x4 Zero;
	static const Matrix4x4 One;
	static const Matrix4x4 Identity;

	Matrix4x4 operator+(const Matrix4x4& other) const;
	Matrix4x4 operator-(const Matrix4x4& other) const;
	Matrix4x4 operator*(const Matrix4x4& other) const;
	Matrix4x4& operator=(const Matrix4x4& other);
	Matrix4x4& operator+=(const Matrix4x4& other);
	Matrix4x4& operator-=(const Matrix4x4& other);
	Matrix4x4& operator*=(const Matrix4x4& other);

	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Matrix4x4 operator*(const T& other) const {
		Matrix4x4 result;
		for (int y = 0; y < 4; y++) {
			for (int x = 0; x < 4; x++) {
				result.m[y][x] = m[y][x] * static_cast<float>(other);
			}
		}
		return result;
	}
	template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0> Matrix4x4& operator*=(const T& other) {
		Matrix4x4 result;
		for (int y = 0; y < 4; y++) {
			for (int x = 0; x < 4; x++) {
				result.m[y][x] = m[y][x] * static_cast<float>(other);
			}
		}
		*this = result;
		return *this;
	}

	/// <returns>逆行列</returns>
	static Matrix4x4 Inverse(const Matrix4x4& matrix);
	/// <returns>転置行列</returns>
	static Matrix4x4 Transpose(const Matrix4x4& matrix);

	/// <returns>拡大縮小行列.</returns>
	static Matrix4x4 MakeScalingMatrix(const Vector3& scale);
	/// <returns>平行移動行列.</returns>
	static Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
	/// <returns>X軸回転行列.</returns>
	static Matrix4x4 MakeXRotationMatrix(const float& theta);
	/// <returns>Y軸回転行列.</returns>
	static Matrix4x4 MakeYRotationMatrix(const float& theta);
	/// <returns>Z軸回転行列.</returns>
	static Matrix4x4 MakeZRotationMatrix(const float& theta);
	/// <returns>回転行列.</returns>
	static Matrix4x4 MakeRotationMatrix(const float& rotationX, const float& rotationY, const float& rotationZ);
	/// <returns>回転行列.</returns>
	static Matrix4x4 MakeRotationMatrix(const Vector3& theta);
	/// <returns>アフィン変換行列.</returns>
	static Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotation, const Vector3& translate);

	/// <summary>
	/// カメラ座標系から同次クリップ空間へ変換する正射影行列を作成する.
	/// </summary>
	/// <param name="left">カメラ座標系での左端</param>
	/// <param name="top">カメラ座標系での上端</param>
	/// <param name="right">カメラ座標系での右端</param>
	/// <param name="bottom">カメラ座標系での下端</param>
	/// <param name="nearClip">ニアクリップ</param>
	/// <param name="farClip">ファークリップ</param>
	/// <returns>正射影行列</returns>
	static Matrix4x4 MakeOrthographicMatrix(const float& left, const float& top, const float& right, const float& bottom, const float& nearClip, const float& farClip);
	/// <summary>
	/// 同次クリップ空間から正規化デバイス座標系(NDC)へ変換する透視投影行列を作成する.
	/// </summary>
	/// <param name="fovY">視野角</param>
	/// <param name="aspectRatio">アスペクト比</param>
	/// <param name="nearClip">ニアクリップ</param>
	/// <param name="farClip">ファークリップ</param>
	/// <returns>透視投影行列</returns>
	static Matrix4x4 MakePerspectiveFovMatrix(const float& fovY, const float& aspectRatio, const float& nearClip, const float& farClip);
	/// <summary>
	/// 正規化デバイス座標系(NDC)からスクリーン座標系(SCS)へ変換するビューポート行列を作成する.
	/// </summary>
	/// <param name="left">スクリーン座標系での左端</param>
	/// <param name="top">スクリーン座標系での上端</param>
	/// <param name="width">横幅</param>
	/// <param name="height">縦幅</param>
	/// <param name="minDepth">最小深度値</param>
	/// <param name="maxDepth">最大深度値</param>
	/// <returns>ビューポート変換行列</returns>
	static Matrix4x4 MakeViewportMatrix(const float& left, const float& top, const float& width, const float& height, const float& minDepth, const float& maxDepth);
};
inline const Matrix4x4 Matrix4x4::Zero = Matrix4x4(
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f
);
inline const Matrix4x4 Matrix4x4::One = Matrix4x4(
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f
);
inline const Matrix4x4 Matrix4x4::Identity = Matrix4x4(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
);

template<typename T, std::enable_if_t<std::is_convertible_v<T, float>, int> = 0>
Matrix4x4 operator*(T scalar, const Matrix4x4& m) {
	return m * static_cast<float>(scalar);
}


} // namespace Cake

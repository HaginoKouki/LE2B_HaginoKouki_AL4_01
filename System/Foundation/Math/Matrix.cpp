#include "Matrix.h"

#include "System/Foundation/Math/Vector.h"

#include <cmath>
#include <cassert>

namespace Cake {

/*---------------------------------
*
* Matrix2x2
*
---------------------------------*/
Matrix2x2::Matrix2x2(float m0_0, float m0_1, float m1_0, float m1_1) {
	m[0][0] = m0_0;
	m[0][1] = m0_1;

	m[1][0] = m1_0;
	m[1][1] = m1_1;
};

#pragma region Operators
Matrix2x2 Matrix2x2::operator+(const Matrix2x2& other) const {
	Matrix2x2 result;
	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 2; x++) {
			result.m[y][x] = m[y][x] + other.m[y][x];
		}
	}
	return result;
}
Matrix2x2 Matrix2x2::operator-(const Matrix2x2& other) const {
	Matrix2x2 result;
	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 2; x++) {
			result.m[y][x] = m[y][x] - other.m[y][x];
		}
	}
	return result;
}
Matrix2x2 Matrix2x2::operator*(const Matrix2x2& other) const {
	Matrix2x2 result;
	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 2; x++) {
			Vector2 vi = {
				m[y][0],
				m[y][1],
			};
			Vector2 vj = {
				other.m[0][x],
				other.m[1][x]
			};
			result.m[y][x] = Vector2::DotProduct(vi, vj);
		}
	}
	return result;
}

Matrix2x2& Matrix2x2::operator=(const Matrix2x2& other) {
	m[0][0] = other.m[0][0];
	m[0][1] = other.m[0][1];
	m[1][0] = other.m[1][0];
	m[1][1] = other.m[1][1];
	return *this;
}
Matrix2x2& Matrix2x2::operator+=(const Matrix2x2& other) {
	Matrix2x2 result;
	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 2; x++) {
			result.m[y][x] = m[y][x] + other.m[y][x];
		}
	}
	*this = result;
	return *this;
}
Matrix2x2& Matrix2x2::operator-=(const Matrix2x2& other) {
	Matrix2x2 result;
	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 2; x++) {
			result.m[y][x] = m[y][x] - other.m[y][x];
		}
	}
	*this = result;
	return *this;
}
Matrix2x2& Matrix2x2::operator*=(const Matrix2x2& matrix) {
	Matrix2x2 result;
	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 2; x++) {
			Vector2 vi = {
				m[y][0],
				m[y][1],
			};
			Vector2 vj = {
				matrix.m[0][x],
				matrix.m[1][x]
			};
			result.m[y][x] = Vector2::DotProduct(vi, vj);
		}
	}
	*this = result;
	return *this;
}
#pragma endregion

Matrix2x2 Matrix2x2::Inverse(const Matrix2x2& target) {
	float scalar = 1.0f / (target.m[0][0] * target.m[1][1] - target.m[0][1] * target.m[1][0]);

	Matrix2x2 result;

	result.m[0][0] = target.m[1][1] * scalar;
	result.m[0][1] = -target.m[0][1] * scalar;

	result.m[1][0] = -target.m[1][0] * scalar;
	result.m[1][1] = target.m[0][0] * scalar;

	return result;
}
Matrix2x2 Matrix2x2::Transpose(const Matrix2x2& target) {
	Matrix2x2 result = target;

	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 2; x++) {
			result.m[y][x] = target.m[x][y];
		}
	}

	return result;
}

Matrix2x2 Matrix2x2::MakeRotationMatrix(const float& theta) {
	Matrix2x2 result;
	result.m[0][0] = cosf(theta);
	result.m[0][1] = sinf(theta);
	result.m[1][0] = -sinf(theta);
	result.m[1][1] = cosf(theta);
	return result;
}

/*---------------------------------
*
* Matrix3x3
*
---------------------------------*/
Matrix3x3::Matrix3x3(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22) {
	m[0][0] = m00;
	m[0][1] = m01;
	m[0][2] = m02;

	m[1][0] = m10;
	m[1][1] = m11;
	m[1][2] = m12;

	m[2][0] = m20;
	m[2][1] = m21;
	m[2][2] = m22;
}

#pragma region Operators
Matrix3x3 Matrix3x3::operator+(const Matrix3x3& other) const {
	Matrix3x3 result;
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			result.m[y][x] = m[y][x] + other.m[y][x];
		}
	}
	return result;
}
Matrix3x3 Matrix3x3::operator-(const Matrix3x3& other) const {
	Matrix3x3 result;
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			result.m[y][x] = m[y][x] - other.m[y][x];
		}
	}
	return result;
}
Matrix3x3 Matrix3x3::operator*(const Matrix3x3& other) const {
	Matrix3x3 result;
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			result.m[y][x] = m[y][0] * other.m[0][x] + m[y][1] * other.m[1][x] + m[y][2] * other.m[2][x];
		}
	}
	return result;
}

Matrix3x3& Matrix3x3::operator=(const Matrix3x3& other) {
	m[0][0] = other.m[0][0];
	m[0][1] = other.m[0][1];
	m[0][2] = other.m[0][2];

	m[1][0] = other.m[1][0];
	m[1][1] = other.m[1][1];
	m[1][2] = other.m[1][2];

	m[2][0] = other.m[2][0];
	m[2][1] = other.m[2][1];
	m[2][2] = other.m[2][2];
	return *this;
}
Matrix3x3& Matrix3x3::operator+=(const Matrix3x3& other) {
	Matrix3x3 result;
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			result.m[y][x] = m[y][x] + other.m[y][x];
		}
	}
	*this = result;
	return *this;
}
Matrix3x3& Matrix3x3::operator-=(const Matrix3x3& other) {
	Matrix3x3 result;
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			result.m[y][x] = m[y][x] - other.m[y][x];
		}
	}
	*this = result;
	return *this;
}
Matrix3x3& Matrix3x3::operator*=(const Matrix3x3& other) {
	Matrix3x3 result;
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			result.m[y][x] = m[y][0] * other.m[0][x] + m[y][1] * other.m[1][x] + m[y][2] * other.m[2][x];
		}
	}
	*this = result;
	return *this;
}
#pragma endregion

Matrix3x3 Matrix3x3::Inverse(const Matrix3x3& target) {
	Matrix3x3 result;

	float a11 = target.m[0][0];
	float a12 = target.m[0][1];
	float a13 = target.m[0][2];

	float a21 = target.m[1][0];
	float a22 = target.m[1][1];
	float a23 = target.m[1][2];

	float a31 = target.m[2][0];
	float a32 = target.m[2][1];
	float a33 = target.m[2][2];

	float absolute = (a11 * a22 * a33) + (a12 * a23 * a31) + (a13 * a21 * a32) - (a13 * a22 * a31) - (a12 * a21 * a33) - (a11 * a23 * a32);
	absolute = 1.0f / absolute;

	result.m[0][0] = (a22 * a33 - a23 * a32) * absolute;
	result.m[0][1] = -(a12 * a33 - a13 * a32) * absolute;
	result.m[0][2] = (a12 * a23 - a13 * a22) * absolute;

	result.m[1][0] = -(a21 * a33 - a23 * a31) * absolute;
	result.m[1][1] = (a11 * a33 - a13 * a31) * absolute;
	result.m[1][2] = -(a11 * a23 - a13 * a21) * absolute;

	result.m[2][0] = (a21 * a32 - a22 * a31) * absolute;
	result.m[2][1] = -(a11 * a32 - a12 * a31) * absolute;
	result.m[2][2] = (a11 * a22 - a12 * a21) * absolute;

	return result;
}
Matrix3x3 Matrix3x3::Transpose(const Matrix3x3& target) {
	Matrix3x3 result = target;

	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			result.m[y][x] = target.m[x][y];
		}
	}

	return result;
}

Matrix3x3 Matrix3x3::MakeTranslateMatrix(const Vector2& translate) {
	Matrix3x3 result;
	result.m[0][0] = 1.0f;
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;

	result.m[1][0] = 0.0f;
	result.m[1][1] = 1.0f;
	result.m[1][2] = 0.0f;

	result.m[2][0] = translate.x;
	result.m[2][1] = translate.y;
	result.m[2][2] = 1.0f;

	return result;
}
Matrix3x3 Matrix3x3::MakeRotationMatrix(const float& theta) {
	Matrix3x3 result;
	result.m[0][0] = cosf(theta);
	result.m[0][1] = sinf(theta);
	result.m[0][2] = 0.0f;

	result.m[1][0] = -sinf(theta);
	result.m[1][1] = cosf(theta);
	result.m[1][2] = 0.0f;

	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = 1.0f;

	return result;
}
Matrix3x3 Matrix3x3::MakeScalingMatrix(const Vector2& scale) {
	Matrix3x3 result;
	result.m[0][0] = scale.x;
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;

	result.m[1][0] = 0.0f;
	result.m[1][1] = scale.y;
	result.m[1][2] = 0.0f;

	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = 1.0f;

	return result;
}
Matrix3x3 Matrix3x3::MakeAffineMatrix(const Vector2& scale, const float& theta, const Vector2& translate) {
	return MakeScalingMatrix(scale) * MakeRotationMatrix(theta) * MakeTranslateMatrix(translate);
}

Matrix3x3 Matrix3x3::MakeOrthoMatrix(const float& left, const float& top, const float& right, const float& bottom) {
	Matrix3x3 result;
	result.m[0][0] = 2.0f / (right - left);
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;

	result.m[1][0] = 0.0f;
	result.m[1][1] = 2.0f / (top - bottom);
	result.m[1][2] = 0.0f;

	result.m[2][0] = (right + left) / (left - right);
	result.m[2][1] = (top + bottom) / (bottom - top);
	result.m[2][2] = 1.0f;
	return result;
}
Matrix3x3 Matrix3x3::MakeViewportMatrix(const float& left, const float& top, const float& width, const float& height) {
	Matrix3x3 result;
	result.m[0][0] = width / 2.0f;
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;

	result.m[1][0] = 0.0f;
	result.m[1][1] = -(height / 2.0f);
	result.m[1][2] = 0.0f;

	result.m[2][0] = left + width / 2.0f;
	result.m[2][1] = top + height / 2.0f;
	result.m[2][2] = 1.0f;
	return result;
}

/*---------------------------------
*
* Matrix4x4
*
---------------------------------*/
Matrix4x4::Matrix4x4(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33
) {
	m[0][0] = m00;
	m[0][1] = m01;
	m[0][2] = m02;
	m[0][3] = m03;
	m[1][0] = m10;
	m[1][1] = m11;
	m[1][2] = m12;
	m[1][3] = m13;
	m[2][0] = m20;
	m[2][1] = m21;
	m[2][2] = m22;
	m[2][3] = m23;
	m[3][0] = m30;
	m[3][1] = m31;
	m[3][2] = m32;
	m[3][3] = m33;
}

#pragma region Operators
Matrix4x4 Matrix4x4::operator+(const Matrix4x4& other) const {
	Matrix4x4 result;
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			result.m[y][x] = m[y][x] + other.m[y][x];
		}
	}
	return result;
}
Matrix4x4 Matrix4x4::operator-(const Matrix4x4& other) const {
	Matrix4x4 result;
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			result.m[y][x] = m[y][x] - other.m[y][x];
		}
	}
	return result;
}
Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const {
	Matrix4x4 result;
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			result.m[y][x] = m[y][0] * other.m[0][x] + m[y][1] * other.m[1][x] + m[y][2] * other.m[2][x] + m[y][3] * other.m[3][x];
		}
	}
	return result;
}
Matrix4x4& Matrix4x4::operator=(const Matrix4x4& other) {
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			m[y][x] = other.m[y][x];
		}
	}
	return *this;
}
Matrix4x4& Matrix4x4::operator+=(const Matrix4x4& other) {
	Matrix4x4 result;
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			result.m[y][x] = m[y][x] + other.m[y][x];
		}
	}
	*this = result;
	return *this;
}
Matrix4x4& Matrix4x4::operator-=(const Matrix4x4& other) {
	Matrix4x4 result;
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			result.m[y][x] = m[y][x] - other.m[y][x];
		}
	}
	*this = result;
	return *this;
}
Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& other) {
	Matrix4x4 result;
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			result.m[y][x] = m[y][0] * other.m[0][x] + m[y][1] * other.m[1][x] + m[y][2] * other.m[2][x] + m[y][3] * other.m[3][x];
		}
	}
	*this = result;
	return *this;
}
#pragma endregion

Matrix4x4 Matrix4x4::Inverse(const Matrix4x4& target) {
	float a11 = target.m[0][0];
	float a12 = target.m[0][1];
	float a13 = target.m[0][2];
	float a14 = target.m[0][3];

	float a21 = target.m[1][0];
	float a22 = target.m[1][1];
	float a23 = target.m[1][2];
	float a24 = target.m[1][3];

	float a31 = target.m[2][0];
	float a32 = target.m[2][1];
	float a33 = target.m[2][2];
	float a34 = target.m[2][3];

	float a41 = target.m[3][0];
	float a42 = target.m[3][1];
	float a43 = target.m[3][2];
	float a44 = target.m[3][3];

	float absolute = 0.0f;
	absolute += +(a11 * a22 * a33 * a44) + (a11 * a23 * a34 * a42) + (a11 * a24 * a32 * a43);
	absolute += -(a11 * a24 * a33 * a42) - (a11 * a23 * a32 * a44) - (a11 * a22 * a34 * a43);
	absolute += -(a12 * a21 * a33 * a44) - (a13 * a21 * a34 * a42) - (a14 * a21 * a32 * a43);
	absolute += +(a14 * a21 * a33 * a42) + (a13 * a21 * a32 * a44) + (a12 * a21 * a34 * a43);
	absolute += +(a12 * a23 * a31 * a44) + (a13 * a24 * a31 * a42) + (a14 * a22 * a31 * a43);
	absolute += -(a14 * a23 * a31 * a42) - (a13 * a22 * a31 * a44) - (a12 * a24 * a31 * a43);
	absolute += -(a12 * a23 * a34 * a41) - (a13 * a24 * a32 * a41) - (a14 * a22 * a33 * a41);
	absolute += +(a14 * a23 * a32 * a41) + (a13 * a22 * a34 * a41) + (a12 * a24 * a33 * a41);

	if (absolute == 0.0f) {
		return Matrix4x4::Zero;
	}

	Matrix4x4 result;

	result.m[0][0] = (a22 * a33 * a44) + (a23 * a34 * a42) + (a24 * a32 * a43) - (a24 * a33 * a42) - (a23 * a32 * a44) - (a22 * a34 * a43);
	result.m[0][1] = -(a12 * a33 * a44) - (a13 * a34 * a42) - (a14 * a32 * a43) + (a14 * a33 * a42) + (a13 * a32 * a44) + (a12 * a34 * a43);
	result.m[0][2] = (a12 * a23 * a44) + (a13 * a24 * a42) + (a14 * a22 * a43) - (a14 * a23 * a42) - (a13 * a22 * a44) - (a12 * a24 * a43);
	result.m[0][3] = -(a12 * a23 * a34) - (a13 * a24 * a32) - (a14 * a22 * a33) + (a14 * a23 * a32) + (a13 * a22 * a34) + (a12 * a24 * a33);

	result.m[1][0] = -(a21 * a33 * a44) - (a23 * a34 * a41) - (a24 * a31 * a43) + (a24 * a33 * a41) + (a23 * a31 * a44) + (a21 * a34 * a43);
	result.m[1][1] = (a11 * a33 * a44) + (a13 * a34 * a41) + (a14 * a31 * a43) - (a14 * a33 * a41) - (a13 * a31 * a44) - (a11 * a34 * a43);
	result.m[1][2] = -(a11 * a23 * a44) - (a13 * a24 * a41) - (a14 * a21 * a43) + (a14 * a23 * a41) + (a13 * a21 * a44) + (a11 * a24 * a43);
	result.m[1][3] = (a11 * a23 * a34) + (a13 * a24 * a31) + (a14 * a21 * a33) - (a14 * a23 * a31) - (a13 * a21 * a34) - (a11 * a24 * a33);

	result.m[2][0] = (a21 * a32 * a44) + (a22 * a34 * a41) + (a24 * a31 * a42) - (a24 * a32 * a41) - (a22 * a31 * a44) - (a21 * a34 * a42);
	result.m[2][1] = -(a11 * a32 * a44) - (a12 * a34 * a41) - (a14 * a31 * a42) + (a14 * a32 * a41) + (a12 * a31 * a44) + (a11 * a34 * a42);
	result.m[2][2] = (a11 * a22 * a44) + (a12 * a24 * a41) + (a14 * a21 * a42) - (a14 * a22 * a41) - (a12 * a21 * a44) - (a11 * a24 * a42);
	result.m[2][3] = -(a11 * a22 * a34) - (a12 * a24 * a31) - (a14 * a21 * a32) + (a14 * a22 * a31) + (a12 * a21 * a34) + (a11 * a24 * a32);

	result.m[3][0] = -(a21 * a32 * a43) - (a22 * a33 * a41) - (a23 * a31 * a42) + (a23 * a32 * a41) + (a22 * a31 * a43) + (a21 * a33 * a42);
	result.m[3][1] = (a11 * a32 * a43) + (a12 * a33 * a41) + (a13 * a31 * a42) - (a13 * a32 * a41) - (a12 * a31 * a43) - (a11 * a33 * a42);
	result.m[3][2] = -(a11 * a22 * a43) - (a12 * a23 * a41) - (a13 * a21 * a42) + (a13 * a22 * a41) + (a12 * a21 * a43) + (a11 * a23 * a42);
	result.m[3][3] = (a11 * a22 * a33) + (a12 * a23 * a31) + (a13 * a21 * a32) - (a13 * a22 * a31) - (a12 * a21 * a33) - (a11 * a23 * a32);

	result *= 1.0f / absolute;

	return result;
}
Matrix4x4 Matrix4x4::Transpose(const Matrix4x4& target) {
	Matrix4x4 result = target;
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			result.m[y][x] = target.m[x][y];
		}
	}
	return result;
}

Matrix4x4 Matrix4x4::MakeScalingMatrix(const Vector3& scale) {
	Matrix4x4 result;
	result.m[0][0] = scale.x;
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;
	result.m[1][0] = 0.0f;
	result.m[1][1] = scale.y;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = scale.z;
	result.m[2][3] = 0.0f;
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;
	return result;
}
Matrix4x4 Matrix4x4::MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 result;
	result.m[0][0] = 1.0f;
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;
	result.m[1][0] = 0.0f;
	result.m[1][1] = 1.0f;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = 1.0f;
	result.m[2][3] = 0.0f;
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 Matrix4x4::MakeXRotationMatrix(const float& theta) {
	Matrix4x4 result;
	result.m[0][0] = 1.0f;
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;
	result.m[1][0] = 0.0f;
	result.m[1][1] = std::cos(theta);
	result.m[1][2] = std::sin(theta);
	result.m[1][3] = 0.0f;
	result.m[2][0] = 0.0f;
	result.m[2][1] = -std::sin(theta);
	result.m[2][2] = std::cos(theta);
	result.m[2][3] = 0.0f;
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;
	return result;
}
Matrix4x4 Matrix4x4::MakeYRotationMatrix(const float& theta) {
	Matrix4x4 result;
	result.m[0][0] = std::cos(theta);
	result.m[0][1] = 0.0f;
	result.m[0][2] = -std::sin(theta);
	result.m[0][3] = 0.0f;
	result.m[1][0] = 0.0f;
	result.m[1][1] = 1.0f;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	result.m[2][0] = std::sin(theta);
	result.m[2][1] = 0.0f;
	result.m[2][2] = std::cos(theta);
	result.m[2][3] = 0.0f;
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;
	return result;
}
Matrix4x4 Matrix4x4::MakeZRotationMatrix(const float& theta) {
	Matrix4x4 result;
	result.m[0][0] = std::cos(theta);
	result.m[0][1] = std::sin(theta);
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;
	result.m[1][0] = -std::sin(theta);
	result.m[1][1] = std::cos(theta);
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = 1.0f;
	result.m[2][3] = 0.0f;
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;
	return result;
}
Matrix4x4 Matrix4x4::MakeRotationMatrix(const float& rotationX, const float& rotationY, const float& rotationZ) {
	Matrix4x4 pitchMatrix = MakeXRotationMatrix(rotationX);
	Matrix4x4 yawMatrix = MakeYRotationMatrix(rotationY);
	Matrix4x4 rollMatrix = MakeZRotationMatrix(rotationZ);
	return pitchMatrix * (yawMatrix * rollMatrix);
}
Matrix4x4 Matrix4x4::MakeRotationMatrix(const Vector3& rotation) {
	return Matrix4x4::MakeRotationMatrix(rotation.x, rotation.y, rotation.z);
}
Matrix4x4 Matrix4x4::MakeAffineMatrix(const Vector3& scale, const Vector3& rotation, const Vector3& translate) {
	Matrix4x4 scalingMatrix = MakeScalingMatrix(scale);
	Matrix4x4 rotationMatrix = MakeRotationMatrix(rotation);
	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);
	return scalingMatrix * rotationMatrix * translateMatrix;
}

Matrix4x4 Matrix4x4::MakeOrthographicMatrix(const float& left, const float& top, const float& right, const float& bottom, const float& nearClip, const float& farClip) {
	Matrix4x4 result;
	result.m[0][0] = 2.0f / (right - left);
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;

	result.m[1][0] = 0.0f;
	result.m[1][1] = 2.0f / (top - bottom);
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;

	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = 1.0f / (farClip - nearClip);
	result.m[2][3] = 0.0f;

	result.m[3][0] = (right + left) / (left - right);
	result.m[3][1] = (top + bottom) / (bottom - top);
	result.m[3][2] = nearClip / (nearClip - farClip);
	result.m[3][3] = 1.0f;

	return result;
}
Matrix4x4 Matrix4x4::MakePerspectiveFovMatrix(const float& fovY, const float& aspectRatio, const float& nearClip, const float& farClip) {
	assert(nearClip != 0.0f && "Near clip was specified as 0");
	Matrix4x4 result;

	result.m[0][0] = (1.0f / aspectRatio) * (1.0f / std::tan(fovY / 2.0f));
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;

	result.m[1][0] = 0.0f;
	result.m[1][1] = 1.0f / std::tan(fovY / 2.0f);
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;

	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1.0f;

	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = (-nearClip * farClip) / (farClip - nearClip);
	result.m[3][3] = 0.0f;

	return result;
}
Matrix4x4 Matrix4x4::MakeViewportMatrix(const float& left, const float& top, const float& width, const float& height, const float& minDepth, const float& maxDepth) {
	assert(minDepth <= maxDepth && "minDepth is greater than maxDepth");
	Matrix4x4 result;

	result.m[0][0] = width / 2.0f;
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;

	result.m[1][0] = 0.0f;
	result.m[1][1] = -(height / 2.0f);
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;

	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = maxDepth - minDepth;
	result.m[2][3] = 0.0f;

	result.m[3][0] = left + width / 2.0f;
	result.m[3][1] = top + height / 2.0f;
	result.m[3][2] = minDepth;
	result.m[3][3] = 1.0f;

	return result;
}

} // namespace Cake

#include "Transform.h"

#include <cmath>

namespace Cake {

Matrix4x4 Math::MakeAffineMatrix(const Transform3& transform) {
	return Matrix4x4::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
}

Vector3 Math::GetTranslate(const Matrix4x4& matrix) {
	return Vector3{matrix.m[3][0], matrix.m[3][1], matrix.m[3][2]};
}

Vector3 Math::GetScale(const Matrix4x4& matrix) {
	// 行ベクトル形式なので、i 行目がローカルの i 軸をワールドへ写したものになる.
	return Vector3{
		Vector3::Length(Vector3{matrix.m[0][0], matrix.m[0][1], matrix.m[0][2]}),
		Vector3::Length(Vector3{matrix.m[1][0], matrix.m[1][1], matrix.m[1][2]}),
		Vector3::Length(Vector3{matrix.m[2][0], matrix.m[2][1], matrix.m[2][2]}),
	};
}

Transform2 Math::Combine(const Transform2& parent, const Transform2& local) {
	Transform2 world;

	// 拡縮は掛け算で積み上がる.
	world.scale.x = parent.scale.x * local.scale.x;
	world.scale.y = parent.scale.y * local.scale.y;

	// 回転は足し算で積み上がる.
	world.rotate = parent.rotate + local.rotate;

	// 位置は「親の拡縮 → 親の回転 → 親の位置を足す」の順で変換する.
	const float offsetX = local.translate.x * parent.scale.x;
	const float offsetY = local.translate.y * parent.scale.y;

	const float s = std::sin(parent.rotate);
	const float c = std::cos(parent.rotate);

	world.translate.x = parent.translate.x + offsetX * c - offsetY * s;
	world.translate.y = parent.translate.y + offsetX * s + offsetY * c;

	return world;
}

}

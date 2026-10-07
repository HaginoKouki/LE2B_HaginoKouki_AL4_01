#pragma once
/*====================================
 *
 * 位置（translate）、回転（rotate）、スケール（scale）を保持する構造体と、
 * それを行列へ変換する関数を定義する。
 *
 * 【Transform3 が本体、Transform2 は UI 用】
 * GameObject の姿勢は Transform3 で表し、親子の合成は行列（GetWorldMatrix()）で行う。
 * オイラー角どうしの足し算では、非一様スケールの親の下で回転したときに正しく合成できないため.
 * Transform2 はスクリーン座標で動かすスプライト（SpriteRenderer::DrawScreen()）向けに残してある.
 *
 * 【回転の順番】
 * Transform3::rotate はオイラー角（ラジアン）で、X → Y → Z の順に回す。
 * KamataEngine::WorldTransform::rotation_ と同じ扱いなので、値をそのまま渡せる.
 *
 * ====================================*/
#include "System/Foundation/Math/Vector.h"
#include "System/Foundation/Math/Matrix.h"

namespace Cake {

struct Transform3 {
	Vector3 scale = Vector3::One;
	Vector3 rotate = Vector3::Zero; //!< オイラー角(ラジアン)。X → Y → Z の順に回す.
	Vector3 translate = Vector3::Zero;
};

struct Transform2 {
	Vector2 scale = Vector2::One;
	float rotate = 0.0f;
	Vector2 translate = Vector2::Zero;
};


namespace Math {
/// <summary>
/// Transform3 をアフィン行列にする。拡縮 → 回転(X → Y → Z) → 平行移動 の順に掛かる.
/// </summary>
Matrix4x4 MakeAffineMatrix(const Transform3& transform);

/// <summary>
/// 行列の平行移動成分を取り出す。ワールド行列なら、そのままワールド座標になる.
/// </summary>
Vector3 GetTranslate(const Matrix4x4& matrix);

/// <summary>
/// 行列の各軸の長さ（＝拡縮）を取り出す。せん断が無い前提.
/// </summary>
Vector3 GetScale(const Matrix4x4& matrix);

/// <summary>
/// 親のワールド変換に子のローカル変換を合成する（2D）.
/// </summary>
/// <param name="parent">親のワールド変換</param>
/// <param name="local">子のローカル変換</param>
/// <returns>子のワールド変換</returns>
Transform2 Combine(const Transform2& parent, const Transform2& local);
} // namespace Math

}	// namespace Cake

#pragma once

#include "System/Foundation/Math/Vector.h"

/// <summary>
/// カメラの振動演出。減衰する乱数オフセットを、カメラから見た左右(x)・上下(y)の量として提供する.
/// 画面に平行な面で揺らすので、カメラがどちらを向いていても見た目の揺れ方は変わらない.
/// 単位はワールド単位（KamataEngine の既定カメラなら 0.1～1.0 程度が目安）.
/// </summary>
class CameraShake {
private:
	bool isShaking_ = false;
	Cake::Vector2 shakedOffset_; // シェイクによる現在のオフセット（カメラのローカル x, y）.

	Cake::Vector2 shakeAmplitudeMax_;     // シェイク開始時の振幅.
	Cake::Vector2 currentShakeAmplitude_; // 減衰中の振幅.

	float duration_ = 0.0f; // シェイクの総時間(秒).
	float elapsed_ = 0.0f;  // 経過時間(秒).

public:
	void Initialize();

	/// <param name="deltaTime">前フレームからの経過時間(秒)</param>
	void Update(float deltaTime);

	/// <param name="amplitude">振幅(縦横同じ大きさ、ワールド単位)</param>
	/// <param name="duration">シェイクが収まるまでの時間(秒)</param>
	void CreateShake(float amplitude, float duration);
	/// <param name="amplitude">振幅(x: 左右, y: 上下、ワールド単位)</param>
	/// <param name="duration">シェイクが収まるまでの時間(秒)</param>
	void CreateShake(const Cake::Vector2& amplitude, float duration);

	// カメラのローカル x（右）・y（上）方向のずれ.
	const Cake::Vector2& GetShakedOffset() const { return shakedOffset_; }
	bool IsShaking() const { return isShaking_; }

#pragma region デバッグ用
#ifdef USE_IMGUI
public:
	/// <summary>
	/// 「Shake」セクションを描画する。ImGui::Begin()/End() は呼び出し側の責任.
	/// </summary>
	void DrawDebugUI();

private:
	// 手動でシェイクを試すための値。ゲーム本体からは参照しない.
	float debugAmplitude_ = 0.5f;
	float debugDuration_ = 0.3f;
#endif // USE_IMGUI
#pragma endregion
};

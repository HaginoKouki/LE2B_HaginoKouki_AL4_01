#pragma once
/*====================================
 *
 * Unity のシーンビューと同じ操作で飛び回れる、Debug ビルド専用のカメラ。
 * Camera が持ち、ESC で切り替える。ゲーム側のカメラ（注視点・追従・シェイク）には一切触れない.
 *
 * 【操作】（Unity のシーンビューに合わせてある）
 * 右ドラッグ             : 視点を回す.
 * 右ボタン + W / S       : 前 / 後ろへ移動.
 * 右ボタン + A / D       : 左 / 右へ移動.
 * 右ボタン + Q / E       : 下 / 上へ移動（ワールドの上下）.
 * 右ボタン + Shift       : 移動を速くする.
 * 右ボタン + ホイール    : 移動速度を変える.
 * ホイール               : 前後へ移動（ズーム）.
 * 中ドラッグ             : 上下左右へ平行移動（パン）.
 *
 * 【マウスが ImGui の上にあるとき】
 * ボタンを押した瞬間に ImGui のウィンドウ上なら、その操作は始めない。
 * スライダーを右クリックしたら視点まで回った、を防ぐため。押し始めた後は ImGui の上を通っても続く.
 * ImGui の入力欄に文字を打っている間は、キーボードでは動かない.
 *
 * 【時間】
 * timeScale やポーズの影響を受けない、実時間の経過時間で動かす。ゲームを止めて眺められるようにするため.
 *
 * ====================================*/
#include "System/Foundation/Math/Vector.h"
#include "System/Foundation/Math/Matrix.h"

// デバッグカメラを使えるのは Debug ビルドだけ。ImGui の状態も見るので USE_IMGUI も要る.
// Develop でも使いたい場合は、ここの条件から defined(_DEBUG) を外す.
#if defined(_DEBUG) && defined(USE_IMGUI)
#define CAKE_ENABLE_DEBUG_CAMERA
#endif

#ifdef CAKE_ENABLE_DEBUG_CAMERA
class DebugFlyCamera {
public:
	// 移動速度の範囲 [unit/s].
	static constexpr float kMinMoveSpeed = 0.1f;
	static constexpr float kMaxMoveSpeed = 1000.0f;

private:
	Cake::Vector3 position_;
	// オイラー角(ラジアン)。x: 見下ろし角(+で下を向く), y: 水平方向の向き。ロールは持たない.
	Cake::Vector3 rotation_;
	// カメラ自身の姿勢。行0が右、行1が上、行2が視線の向き.
	Cake::Matrix4x4 worldMatrix_;

	float moveSpeed_ = 10.0f;        // 基本の移動速度 [unit/s].
	float boostMultiplier_ = 3.0f;   // Shift を押している間の倍率.
	float lookSensitivity_ = 0.003f; // マウス1カウントあたりの回転量 [rad].

	bool isLooking_ = false; // 右ボタンで操作中か.
	bool isPanning_ = false; // 中ボタンで操作中か.

public:
	DebugFlyCamera();

	/// <summary>
	/// 位置と向きを直接決める。ゲームのカメラの視点から始めたい場合に使う.
	/// </summary>
	/// <param name="rotation">x: 見下ろし角, y: 水平方向の向き。z は無視する</param>
	void SetPose(const Cake::Vector3& position, const Cake::Vector3& rotation);

	/// <summary>
	/// マウスとキーボードを読んで動かす。有効な間だけ、1フレームに1回呼ぶ.
	/// </summary>
	/// <param name="deltaTime">実時間の経過時間(秒)。Time::GetUnscaledDeltaTime() を渡す</param>
	void Update(float deltaTime);

	/// <summary>
	/// 操作を打ち切る。デバッグカメラを外したときに呼ぶ.
	/// 右ボタンを押したまま外すと、次に有効にしたときに勝手に回り出すのを防ぐ.
	/// </summary>
	void CancelControl();

	// 右ボタンで飛んでいる間 true。この間はゲーム側へキー入力を渡さない.
	bool IsFlying() const { return isLooking_; }

	const Cake::Vector3& GetPosition() const { return position_; }
	const Cake::Vector3& GetRotation() const { return rotation_; }
	const Cake::Matrix4x4& GetWorldMatrix() const { return worldMatrix_; }

	/// <summary>
	/// 速度・感度の調整と、操作方法の一覧を描く。ImGui::Begin()/End() は呼び出し側の責任.
	/// </summary>
	void DrawDebugUI();

private:
	void UpdateWorldMatrix();
};
#endif // CAKE_ENABLE_DEBUG_CAMERA

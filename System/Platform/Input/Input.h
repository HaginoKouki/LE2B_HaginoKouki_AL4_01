#pragma once
/*====================================
 *
 * KamataEngineのInputをゲーム用にラップするクラス。
 * キーボードとゲームパッド（XInput の 0 番）を区別せずに読めるようにする。
 *
 * 【ゲーム固有の操作はここに足す】
 * 「ジャンプ」「攻撃」のような操作は、IsPadTriggered() と TriggerKey() を
 * 組み合わせた関数を Cake::Input に追加して、キー割り当てを1箇所に集める。
 * 呼び出し側は DIK_〇〇 や XINPUT_GAMEPAD_〇〇 を直接書かない.
 *
 * 【現在の割り当て】
 *   移動     : WASD / 矢印キー / 左スティック
 *   ジャンプ : SPACE / パッド A
 *   攻撃     : J / パッド X
 *   回避     : K / パッド B
 *   決定     : SPACE / Enter / パッド A（タイトルやメニュー用）
 *
 * ====================================*/
#include "KamataEngine.h"

#include "System/Foundation/Math/Vector.h"
#include "System/Platform/Input/KeyState.h"

namespace Cake {
class Input final {
public:
	// 静的関数だけで使うので実体は作らせない.
	Input() = delete;

	// ゲームパッドが1つ以上つながっているか.
	static bool IsEnableController();

	/* ===== 受け付けの停止 ===== */

	/// <summary>
	/// false の間、ボタンと操作の関数（IsPadPressed() 以降）は全て「入力なし」を返す.
	/// Debug ビルドでデバッグカメラを右ボタンで飛ばしている間、プレイヤーまで WASD で動かないように Camera が使う.
	/// KamataEngine::Input を直接読んでいる箇所には効かない.
	/// </summary>
	static void SetGameInputEnabled(bool isEnabled) { isGameInputEnabled_ = isEnabled; }
	static bool IsGameInputEnabled() { return isGameInputEnabled_; }

	/* ===== ゲームパッドのボタン ===== */
	/* button には XINPUT_GAMEPAD_A などを渡す。未接続なら常に false. */

	// 押されているか.
	static bool IsPadPressed(WORD button);
	// 押された瞬間か.
	static bool IsPadTriggered(WORD button);

	/* ===== 操作 ===== */

	// 決定。Space / Enter / パッドの A.
	static bool GetDecideButton();

	/// <summary>
	/// 移動入力。WASD・矢印キー・左スティックのどれからでも取れる。Y+ が上.
	/// 長さは最大 1。キーボードの斜め入力も 1 に揃える.
	/// 2軸アクションでは x だけを使い、y は梯子やしゃがみなどに使う.
	/// </summary>
	static Cake::Vector2 GetMoveAxis();

	// ジャンプ。SPACE / パッドの A。押した瞬間だけ true.
	static bool GetJumpButton();

	// ジャンプボタンを押し続けているか。長押しで高く跳ぶ（可変ジャンプ）の判定に使う.
	static bool IsJumpButtonHeld();

	// 攻撃。J / パッドの X。押した瞬間だけ true.
	static bool GetAttackButton();

	// 回避。K / パッドの B。押した瞬間だけ true.
	static bool GetDodgeButton();

private:
	static inline bool isGameInputEnabled_ = true;

	static Cake::Vector2 GetLeftJoystickAxis();
	static Cake::Vector2 GetRightJoystickAxis();
};
} // namespace Cake

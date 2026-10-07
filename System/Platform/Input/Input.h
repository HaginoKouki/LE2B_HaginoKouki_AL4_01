#pragma once
/*====================================
 *
 * KamataEngineのInputをゲーム用にラップするクラス。
 * キーボードとゲームパッド（XInput の 0 番）を区別せずに読めるようにする。
 *
 * 【ゲーム固有の操作はここに足す】
 * 「ジャンプ」「攻撃」のような操作は、IsPadTriggered() と TriggerKey() を
 * 組み合わせた関数を Cake::Input に追加して、キー割り当てを1箇所に集める。
 * 例:
 *   bool Input::GetJumpButton() {
 *       return KamataEngine::Input::GetInstance()->TriggerKey(DIK_SPACE) || IsPadTriggered(XINPUT_GAMEPAD_A);
 *   }
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
	/// </summary>
	static Cake::Vector2 GetMoveAxis();

	// ステージ回転。SPACE / パッドの A。押した瞬間だけ true.
	static bool GetRotateButton();

	// 回転ボタンを押し続けているか。長押しでのやり直しに使う.
	static bool IsRotateButtonHeld();

private:
	static inline bool isGameInputEnabled_ = true;

	static Cake::Vector2 GetLeftJoystickAxis();
	static Cake::Vector2 GetRightJoystickAxis();
};
} // namespace Cake

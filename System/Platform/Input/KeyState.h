#pragma once
/*====================================
 *
 * 入力デバイス共通のボタン状態を表す列挙型。
 * キーボード・マウス・ゲームパッドで同じ押下判定を使い回せるよう、
 * Input から分離して定義する。
 *
 * ====================================*/

namespace Cake {

enum class KeyState {
	None = 0b00,    //!< 離されている.
	Pressed = 0b01, //!< 押された瞬間.
	Release = 0b10, //!< 離された瞬間.
	Held = 0b11,    //!< 押されている.
};

// 押されている（押した瞬間 or 押しっぱなし）か.
inline bool IsPressed(KeyState state) {
	return state == KeyState::Pressed || state == KeyState::Held;
}

// 今フレーム／前フレームの押下状態からKeyStateを作る.
inline KeyState MakeKeyState(bool current, bool previous) {
	// KeyStateはbit0=今フレーム、bit1=前フレームの押下状態として定義されている.
	return static_cast<KeyState>((static_cast<int>(previous) << 1) | static_cast<int>(current));
}

} // namespace Cake

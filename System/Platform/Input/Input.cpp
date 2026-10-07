#include "Input.h"

#include <algorithm>

namespace Cake {

namespace {
// スティックの遊び。これより小さい傾きは入力なしとみなす.
constexpr float kStickDeadZone = 0.1f;

// GetJoystickState() が false を返した場合（未接続など）に未初期化の値を読まないよう、
// 必ずゼロ初期化した状態から取得する.
XINPUT_STATE GetPadState() {
	XINPUT_STATE state{};
	KamataEngine::Input::GetInstance()->GetJoystickState(0, state);
	return state;
}
XINPUT_STATE GetPadStatePrevious() {
	XINPUT_STATE state{};
	KamataEngine::Input::GetInstance()->GetJoystickStatePrevious(0, state);
	return state;
}

// 長さが 1 を超えないように縮める.
Cake::Vector2 ClampLength(const Cake::Vector2& value) {
	const float length = Cake::Vector2::Length(value);
	if (length <= 1.0f) {
		return value;
	}
	return value / length;
}

Cake::Vector2 ToStickAxis(SHORT x, SHORT y) {
	// -32768 / 32767 は -1 をわずかに超えるので、丸めてから長さも 1 に揃える.
	const Cake::Vector2 axis = {
		(std::max)(static_cast<float>(x) / 32767.0f, -1.0f),
		(std::max)(static_cast<float>(y) / 32767.0f, -1.0f),
	};
	return ClampLength(axis);
}
} // namespace

bool Input::IsEnableController() {
	return KamataEngine::Input::GetInstance()->GetNumberOfJoysticks() > 0;
}

bool Input::IsPadPressed(WORD button) {
	if (!isGameInputEnabled_) {
		return false;
	}
	return (GetPadState().Gamepad.wButtons & button) != 0;
}

bool Input::IsPadTriggered(WORD button) {
	if (!isGameInputEnabled_) {
		return false;
	}
	const bool isPressed = (GetPadState().Gamepad.wButtons & button) != 0;
	const bool wasPressed = (GetPadStatePrevious().Gamepad.wButtons & button) != 0;
	return isPressed && !wasPressed;
}

bool Input::GetDecideButton() {
	if (!isGameInputEnabled_) {
		return false;
	}
	const KamataEngine::Input* input = KamataEngine::Input::GetInstance();
	const bool isKeyTriggered = input->TriggerKey(DIK_SPACE) || input->TriggerKey(DIK_RETURN);
	return isKeyTriggered || IsPadTriggered(XINPUT_GAMEPAD_A);
}

Cake::Vector2 Input::GetLeftJoystickAxis() {
	const XINPUT_STATE state = GetPadState();
	return ToStickAxis(state.Gamepad.sThumbLX, state.Gamepad.sThumbLY);
}
Cake::Vector2 Input::GetRightJoystickAxis() {
	const XINPUT_STATE state = GetPadState();
	return ToStickAxis(state.Gamepad.sThumbRX, state.Gamepad.sThumbRY);
}

Cake::Vector2 Input::GetMoveAxis() {
	if (!isGameInputEnabled_) {
		return Cake::Vector2::Zero;
	}
	// スティックが倒れていればそちらを優先する.
	const Cake::Vector2 stick = GetLeftJoystickAxis();
	if (Cake::Vector2::Length(stick) > kStickDeadZone) {
		return stick;
	}

	const KamataEngine::Input* input = KamataEngine::Input::GetInstance();
	const bool isRightPressed = input->PushKey(DIK_RIGHT) || input->PushKey(DIK_D);
	const bool isLeftPressed = input->PushKey(DIK_LEFT) || input->PushKey(DIK_A);
	const bool isUpPressed = input->PushKey(DIK_UP) || input->PushKey(DIK_W);
	const bool isDownPressed = input->PushKey(DIK_DOWN) || input->PushKey(DIK_S);

	// 左右（上下）同時押しは打ち消し合って 0 になる.
	Cake::Vector2 axis;
	axis.x = static_cast<float>(isRightPressed) - static_cast<float>(isLeftPressed);
	axis.y = static_cast<float>(isUpPressed) - static_cast<float>(isDownPressed);

	// 斜めだけ速くならないよう、長さを 1 に揃える.
	return ClampLength(axis);
}

bool Input::GetRotateButton() {
	if (!isGameInputEnabled_) {
		return false;
	}
	const KamataEngine::Input* input = KamataEngine::Input::GetInstance();
	return input->TriggerKey(DIK_SPACE) || IsPadTriggered(XINPUT_GAMEPAD_A);
}

bool Input::IsRotateButtonHeld() {
	if (!isGameInputEnabled_) {
		return false;
	}
	const KamataEngine::Input* input = KamataEngine::Input::GetInstance();
	return input->PushKey(DIK_SPACE) || IsPadPressed(XINPUT_GAMEPAD_A);
}

} // namespace Cake

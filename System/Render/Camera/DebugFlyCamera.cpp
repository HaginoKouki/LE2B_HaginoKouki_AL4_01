#include "DebugFlyCamera.h"

#ifdef CAKE_ENABLE_DEBUG_CAMERA
#include <algorithm>
#include <cmath>
#include <numbers>
#include <imgui.h>

#include "KamataEngine.h"

#include "System/Foundation/Math/Convert.h"

namespace {
// マウスボタンの番号（KamataEngine::Input::IsPressMouse() の引数）.
constexpr int32_t kMouseRight = 1;
constexpr int32_t kMouseMiddle = 2;
// ホイール1目盛りあたりの値（Windows の WHEEL_DELTA）.
constexpr float kWheelDelta = 120.0f;
// 見下ろし角の上限（約89度）。真上・真下を越えると画面の上下が反転する.
constexpr float kMaxPitch = 1.55f;
// 右ボタン + ホイール1目盛りで、移動速度を何倍にするか.
constexpr float kSpeedStepRatio = 1.2f;
// ホイール1目盛りで前後へ進む距離。移動速度に掛ける割合.
constexpr float kWheelZoomRatio = 0.2f;
// 中ドラッグで、マウス1カウントあたりに動く距離。移動速度に掛ける割合.
constexpr float kPanRatio = 0.002f;

// 行ベクトル形式なので、i 行目がカメラのローカル i 軸のワールドでの向きになる.
Cake::Vector3 GetRow(const Cake::Matrix4x4& matrix, int row) {
	return Cake::Vector3{matrix.m[row][0], matrix.m[row][1], matrix.m[row][2]};
}
} // namespace

DebugFlyCamera::DebugFlyCamera() {
	UpdateWorldMatrix();
}

void DebugFlyCamera::SetPose(const Cake::Vector3& position, const Cake::Vector3& rotation) {
	position_ = position;
	rotation_ = {std::clamp(rotation.x, -kMaxPitch, kMaxPitch), rotation.y, 0.0f};
	UpdateWorldMatrix();
}

void DebugFlyCamera::CancelControl() {
	isLooking_ = false;
	isPanning_ = false;
}

void DebugFlyCamera::Update(float deltaTime) {
	KamataEngine::Input* input = KamataEngine::Input::GetInstance();
	const ImGuiIO& io = ImGui::GetIO();

	// 押した瞬間に ImGui の上でなければ操作を始める。押している間は ImGui に関係なく続ける.
	if (input->IsTriggerMouse(kMouseRight) && !io.WantCaptureMouse) {
		isLooking_ = true;
	}
	if (!input->IsPressMouse(kMouseRight)) {
		isLooking_ = false;
	}
	if (input->IsTriggerMouse(kMouseMiddle) && !io.WantCaptureMouse) {
		isPanning_ = true;
	}
	if (!input->IsPressMouse(kMouseMiddle)) {
		isPanning_ = false;
	}

	const KamataEngine::Input::MouseMove mouseMove = input->GetMouseMove();
	const float mouseX = static_cast<float>(mouseMove.lX);
	const float mouseY = static_cast<float>(mouseMove.lY);
	// 奥へ回すと + になる。目盛りの数に直す.
	const float wheel = static_cast<float>(input->GetWheel()) / kWheelDelta;

	/* ===== 視点の回転 ===== */
	if (isLooking_) {
		// マウスを右へ動かすと右を向き、下へ動かすと下を向く.
		rotation_.y += mouseX * lookSensitivity_;
		rotation_.x = std::clamp(rotation_.x + mouseY * lookSensitivity_, -kMaxPitch, kMaxPitch);
		// 回し続けても値が膨らまないよう、-π～π に収める.
		rotation_.y = std::remainder(rotation_.y, 2.0f * std::numbers::pi_v<float>);
	}
	// 移動はこのフレームの向きで行う.
	UpdateWorldMatrix();

	const Cake::Vector3 right = GetRow(worldMatrix_, 0);
	const Cake::Vector3 up = GetRow(worldMatrix_, 1);
	const Cake::Vector3 forward = GetRow(worldMatrix_, 2);

	/* ===== 移動 ===== */
	if (isLooking_) {
		// 右ボタン + ホイールで速度を変える（Unity と同じ）.
		if (wheel != 0.0f) {
			moveSpeed_ = std::clamp(moveSpeed_ * std::pow(kSpeedStepRatio, wheel), kMinMoveSpeed, kMaxMoveSpeed);
		}

		// ImGui の入力欄に文字を打っている間は動かない.
		if (!io.WantTextInput) {
			Cake::Vector3 direction = Cake::Vector3::Zero;
			if (input->PushKey(DIK_W)) {
				direction += forward;
			}
			if (input->PushKey(DIK_S)) {
				direction -= forward;
			}
			if (input->PushKey(DIK_D)) {
				direction += right;
			}
			if (input->PushKey(DIK_A)) {
				direction -= right;
			}
			// 上下は視線の傾きに関係なく、ワールドの上下へ動く.
			if (input->PushKey(DIK_E)) {
				direction += Cake::Vector3::UnitY;
			}
			if (input->PushKey(DIK_Q)) {
				direction -= Cake::Vector3::UnitY;
			}

			if (Cake::Vector3::Length(direction) > 0.0f) {
				const bool isBoosted = input->PushKey(DIK_LSHIFT) || input->PushKey(DIK_RSHIFT);
				const float speed = moveSpeed_ * (isBoosted ? boostMultiplier_ : 1.0f);
				// 斜めだけ速くならないよう、向きは長さ1に揃える.
				position_ += Cake::Vector3::Normalize(direction) * (speed * deltaTime);
			}
		}
	} else if (wheel != 0.0f && !io.WantCaptureMouse) {
		// ホイールだけなら前後へ動く（ズーム）.
		position_ += forward * (wheel * moveSpeed_ * kWheelZoomRatio);
	}

	/* ===== パン ===== */
	if (isPanning_) {
		// 掴んだ景色がカーソルについてくるよう、カメラはカーソルと逆へ動かす.
		const float panScale = moveSpeed_ * kPanRatio;
		position_ += (right * -mouseX + up * mouseY) * panScale;
	}

	UpdateWorldMatrix();
}

void DebugFlyCamera::UpdateWorldMatrix() {
	// ロールを持たないので、見下ろし(X) → 水平の向き(Y) の順に回す（Camera と同じ順番）.
	worldMatrix_ = Cake::Matrix4x4::MakeXRotationMatrix(rotation_.x) *
	               Cake::Matrix4x4::MakeYRotationMatrix(rotation_.y) *
	               Cake::Matrix4x4::MakeTranslateMatrix(position_);
}

void DebugFlyCamera::DrawDebugUI() {
	ImGui::PushID("DebugFlyCamera");

	ImGui::Text("Position: (%.2f, %.2f, %.2f)", position_.x, position_.y, position_.z);
	ImGui::Text("Rotation: (%.1f, %.1f) deg", Cake::Math::ToDegree(rotation_.x), Cake::Math::ToDegree(rotation_.y));
	ImGui::Text("Flying: %s", isLooking_ ? "Yes (game input is blocked)" : "No");

	ImGui::SliderFloat("Move Speed", &moveSpeed_, kMinMoveSpeed, kMaxMoveSpeed, "%.2f", ImGuiSliderFlags_Logarithmic);
	ImGui::SliderFloat("Shift Multiplier", &boostMultiplier_, 1.0f, 10.0f, "%.1f x");
	ImGui::SliderFloat("Look Sensitivity", &lookSensitivity_, 0.0005f, 0.01f, "%.4f");

	if (ImGui::TreeNode("Controls")) {
		ImGui::BulletText("ESC : switch debug / game camera");
		ImGui::BulletText("RMB drag : look around");
		ImGui::BulletText("RMB + WASD : move");
		ImGui::BulletText("RMB + Q / E : down / up");
		ImGui::BulletText("RMB + Shift : move faster");
		ImGui::BulletText("RMB + Wheel : change move speed");
		ImGui::BulletText("Wheel : move forward / back");
		ImGui::BulletText("MMB drag : pan");
		ImGui::TreePop();
	}

	ImGui::PopID();
}
#endif // CAKE_ENABLE_DEBUG_CAMERA

#include "CameraShake.h"

#include <random>
#include "System/Foundation/Math/Easing.h"

namespace {
// -amplitude ～ +amplitude の一様乱数。
// 2D版は rand() % 整数 で求めていたが、3D はワールド単位が小さく（1未満が普通）、整数に丸めると揺れなくなる.
float RandomRange(float amplitude) {
	if (amplitude <= 0.0f) {
		return 0.0f;
	}
	static std::mt19937 engine{std::random_device{}()};
	std::uniform_real_distribution<float> distribution(-amplitude, amplitude);
	return distribution(engine);
}
} // namespace

void CameraShake::Initialize() {
	isShaking_ = false;
	shakedOffset_ = Cake::Vector2::Zero;

	shakeAmplitudeMax_ = Cake::Vector2::Zero;
	currentShakeAmplitude_ = Cake::Vector2::Zero;

	duration_ = 0.0f;
	elapsed_ = 0.0f;
}

void CameraShake::Update(float deltaTime) {
	if (!isShaking_) {
		shakedOffset_ = Cake::Vector2::Zero;
		return;
	}

	elapsed_ += deltaTime;
	if (duration_ <= 0.0f || elapsed_ >= duration_) {
		// 時間切れ。振動を止める.
		isShaking_ = false;
		shakedOffset_ = Cake::Vector2::Zero;
		return;
	}

	// 振幅を時間とともに減衰させる.
	const float ratio = elapsed_ / duration_;
	currentShakeAmplitude_ = shakeAmplitudeMax_ - Cake::Easing::EaseOutExpo(Cake::Vector2::Zero, shakeAmplitudeMax_, ratio);

	shakedOffset_.x = RandomRange(currentShakeAmplitude_.x);
	shakedOffset_.y = RandomRange(currentShakeAmplitude_.y);
}

void CameraShake::CreateShake(float amplitude, float duration) {
	CreateShake(Cake::Vector2{amplitude, amplitude}, duration);
}
void CameraShake::CreateShake(const Cake::Vector2& amplitude, float duration) {
	shakeAmplitudeMax_ = amplitude;
	currentShakeAmplitude_ = amplitude;

	duration_ = duration;
	elapsed_ = 0.0f;
	isShaking_ = true;
}



#ifdef USE_IMGUI
#include <algorithm>
#include <imgui.h>
void CameraShake::DrawDebugUI() {
	if (!ImGui::CollapsingHeader("Shake", ImGuiTreeNodeFlags_DefaultOpen)) {
		return;
	}
	// 他セクションと同名ラベルが衝突しないようにする.
	ImGui::PushID("CameraShake");

	ImGui::Text("Shaking: %s", isShaking_ ? "Yes" : "No");
	ImGui::Text("Offset: (%.3f, %.3f)", shakedOffset_.x, shakedOffset_.y);

	// 減衰の進み具合は数値より棒で見たほうが早い.
	const float progress = duration_ > 0.0f ? std::clamp(elapsed_ / duration_, 0.0f, 1.0f) : 0.0f;
	ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f));
	ImGui::Text("Elapsed: %.2f / %.2f sec", elapsed_, duration_);
	ImGui::Text("Amplitude: (%.2f, %.2f)  Max: (%.2f, %.2f)", currentShakeAmplitude_.x, currentShakeAmplitude_.y, shakeAmplitudeMax_.x, shakeAmplitudeMax_.y);

	ImGui::Separator();
	ImGui::DragFloat("Test Amplitude", &debugAmplitude_, 0.01f, 0.0f, 10.0f, "%.2f");
	ImGui::DragFloat("Test Duration", &debugDuration_, 0.01f, 0.0f, 10.0f, "%.2f sec");
	if (ImGui::Button("Shake")) {
		CreateShake(debugAmplitude_, debugDuration_);
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop")) {
		Initialize();
	}

	ImGui::PopID();
}
#endif // USE_IMGUI

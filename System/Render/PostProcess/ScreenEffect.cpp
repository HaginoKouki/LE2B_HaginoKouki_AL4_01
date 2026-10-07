#include "ScreenEffect.h"

#include <algorithm>

#include "System/Platform/Time/Time.h"
#include "System/Render/Renderer/SpriteRenderer.h"

namespace {
// 白1x1。色を掛けるだけなので、これ1枚で全ての色を賄える.
const char* const kOverlayTexturePath = "white1x1.png";

// 「覆いきった」とみなす濃さ。1.0f との == 比較は丸め誤差で外れる.
constexpr float kCoveredThreshold = 0.999f;
} // namespace

ScreenEffect::Layer ScreenEffect::fade_{};
ScreenEffect::Layer ScreenEffect::flash_{};
uint32_t ScreenEffect::textureHandle_ = 0;
KamataEngine::Sprite* ScreenEffect::fadeSprite_ = nullptr;
KamataEngine::Sprite* ScreenEffect::flashSprite_ = nullptr;

/*
* Layer
———————————————*/
void ScreenEffect::Layer::Start(const Cake::Vector3& newColor, float from, float to, float newDuration, Cake::Easing::EaseType newEase) {
	color = newColor;
	startAlpha = std::clamp(from, 0.0f, 1.0f);
	targetAlpha = std::clamp(to, 0.0f, 1.0f);
	duration = (std::max)(0.0f, newDuration);
	elapsed = 0.0f;
	ease = newEase;

	// duration が 0 の場合、Update() を待たずにこの場で終点まで進める。
	// 「即座に暗転」を1フレーム遅れずに反映するため.
	alpha = (duration > 0.0f) ? startAlpha : targetAlpha;
}

void ScreenEffect::Layer::Update(float deltaTime) {
	// 補間が終わった層は、alpha を保持したまま止まる。
	// フェードはここで覆いを維持し、フラッシュは 0 のまま何もしない.
	if (!IsPlaying()) {
		return;
	}

	elapsed = (std::min)(elapsed + (std::max)(0.0f, deltaTime), duration);
	const float t = elapsed / duration; // IsPlaying() が true の時点で duration > 0.
	alpha = Cake::Easing::Ease(startAlpha, targetAlpha, t, ease);
}

/*
* 初期化・後始末
———————————————*/
void ScreenEffect::Initialize() {
	// 二度呼ばれてもスプライトが漏れないように、先に畳む.
	Finalize();

	textureHandle_ = KamataEngine::TextureManager::Load(kOverlayTexturePath);

	// 層ごとに実体を分ける。1枚を2回描くと、両方が最後に書いた色になる.
	fadeSprite_ = KamataEngine::Sprite::Create(textureHandle_, {0.0f, 0.0f});
	flashSprite_ = KamataEngine::Sprite::Create(textureHandle_, {0.0f, 0.0f});

	fade_ = Layer{};
	flash_ = Layer{};
}

void ScreenEffect::Finalize() {
	// Sprite::Create() の実体はエンジンが解放しない.
	delete fadeSprite_;
	fadeSprite_ = nullptr;
	delete flashSprite_;
	flashSprite_ = nullptr;
}

/*
* 更新・描画
———————————————*/
void ScreenEffect::Update(const Cake::Time* time) {
	// ポーズやヒットストップで止めたくないので、timeScale の影響を受けない方を使う.
	const float deltaTime = (time != nullptr) ? time->GetUnscaledDeltaTime() : 0.0f;

	fade_.Update(deltaTime);
	flash_.Update(deltaTime);
}

void ScreenEffect::Draw() {
	// 持続する層が先。フラッシュはその上に乗る。
	// 逆にすると、暗転しきった画面でフラッシュが見えなくなる.
	SpriteRenderer::DrawFullScreen(fadeSprite_, fade_.ToColor());
	SpriteRenderer::DrawFullScreen(flashSprite_, flash_.ToColor());
}

/*
* フェード
———————————————*/
void ScreenEffect::FadeOut(const Cake::Vector3& color, float duration, Cake::Easing::EaseType ease) {
	// 今の濃さを始点にする。連続で呼ばれても濃さが飛び戻らない.
	fade_.Start(color, fade_.alpha, 1.0f, duration, ease);
}

void ScreenEffect::FadeIn(float duration, Cake::Easing::EaseType ease) {
	fade_.Start(fade_.color, fade_.alpha, 0.0f, duration, ease);
}

void ScreenEffect::FadeIn(const Cake::Vector3& color, float duration, Cake::Easing::EaseType ease) {
	// 今の色と違う色を渡すと、その瞬間に色が切り替わる。
	// 覆いきっている状態から呼ぶ前提なので、通常は直前の FadeOut() と同じ色を渡す.
	fade_.Start(color, fade_.alpha, 0.0f, duration, ease);
}

void ScreenEffect::SetFade(const Cake::Vector3& color, float alpha) {
	fade_.Start(color, alpha, alpha, 0.0f, Cake::Easing::EaseType::Linear);
}

void ScreenEffect::ClearFade() {
	SetFade(fade_.color, 0.0f);
}

bool ScreenEffect::IsFading() {
	return fade_.IsPlaying();
}

bool ScreenEffect::IsCovered() {
	return fade_.alpha >= kCoveredThreshold;
}

/*
* フラッシュ
———————————————*/
void ScreenEffect::Flash(const Cake::Vector3& color, float duration, float peakAlpha, Cake::Easing::EaseType ease) {
	// 立ち上がりは瞬間。呼んだフレームから最大の明るさで出る。
	// 減衰中の呼び直しでは濃い方を採り、弱い光が強い光を消さないようにする.
	const float from = (std::max)(flash_.alpha, std::clamp(peakAlpha, 0.0f, 1.0f));
	flash_.Start(color, from, 0.0f, duration, ease);
}

bool ScreenEffect::IsFlashing() {
	return flash_.IsPlaying();
}

/*
* デバッグ用
———————————————*/
#ifdef USE_IMGUI
#include <imgui.h>

void ScreenEffect::DrawDebugUI(const char* windowName) {
	if (!ImGui::Begin(windowName)) {
		ImGui::End();
		return;
	}

	ImGui::Text("Fade  : alpha %.3f %s", fade_.alpha, fade_.IsPlaying() ? "(playing)" : "");
	ImGui::Text("Flash : alpha %.3f %s", flash_.alpha, flash_.IsPlaying() ? "(playing)" : "");
	ImGui::Separator();

	ImGui::ColorEdit3("Color", debugColor_);
	ImGui::DragFloat("Duration", &debugDuration_, 0.01f, 0.0f, 5.0f, "%.2f s");
	ImGui::SliderFloat("Peak Alpha", &debugPeakAlpha_, 0.0f, 1.0f, "%.2f");

	const Cake::Vector3 color = {debugColor_[0], debugColor_[1], debugColor_[2]};

	if (ImGui::Button("Fade Out")) {
		FadeOut(color, debugDuration_);
	}
	ImGui::SameLine();
	if (ImGui::Button("Fade In")) {
		FadeIn(debugDuration_);
	}
	ImGui::SameLine();
	if (ImGui::Button("Flash")) {
		Flash(color, debugDuration_, debugPeakAlpha_);
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear")) {
		ClearFade();
	}

	ImGui::End();
}
#endif // USE_IMGUI

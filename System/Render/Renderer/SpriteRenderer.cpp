#include "SpriteRenderer.h"

#include "System/Foundation/Math/Convert.h"
#include "System/Render/Camera/Camera.h"

namespace {
// バックバッファの大きさ。ウィンドウの既定サイズと揃える.
// Camera を引数に取らないのは、全画面演出はカメラの位置や画角と無関係だから.
const Cake::Vector2 kBackBufferSize = {
	static_cast<float>(KamataEngine::WinApp::kWindowWidth),
	static_cast<float>(KamataEngine::WinApp::kWindowHeight),
};
} // namespace

void SpriteRenderer::DrawScreen(KamataEngine::Sprite* sprite, const Cake::Transform2& scs, const Cake::Vector2& baseSize) {
	if (sprite == nullptr) {
		return;
	}
	// スクリーン座標系そのままなので、反転もズームも掛けない.
	DrawSprite(sprite, scs.translate, -scs.rotate, {baseSize.x * scs.scale.x, baseSize.y * scs.scale.y});
}

void SpriteRenderer::DrawAtWorld(const Camera& camera, KamataEngine::Sprite* sprite, const Cake::Vector3& worldPosition, const Cake::Vector2& screenOffset, const Cake::Vector2& size) {
	if (sprite == nullptr) {
		return;
	}
	const Cake::Vector3 screen = camera.ToSCS(worldPosition);
	// z はカメラからの奥行き。後ろにある点を投影すると、画面の反対側に出てしまう.
	if (screen.z <= 0.0f) {
		return;
	}
	DrawSprite(sprite, Cake::Vector2{screen.x, screen.y} + screenOffset, 0.0f, size);
}

void SpriteRenderer::DrawFullScreen(KamataEngine::Sprite* sprite, const Cake::Vector4& color) {
	if (sprite == nullptr) {
		return;
	}
	// 完全に透明なら描いても結果が変わらない。演出が止まっている間の大半はここで抜ける.
	if (color.w <= 0.0f) {
		return;
	}

	// 左上を原点にして画面全体へ伸ばすので、アンカーは左上に固定する.
	sprite->SetAnchorPoint({0.0f, 0.0f});
	sprite->SetColor(ToKamata(color));

	DrawSprite(sprite, Cake::Vector2::Zero, 0.0f, kBackBufferSize);
}

void SpriteRenderer::DrawSprite(KamataEngine::Sprite* sprite, const Cake::Vector2& position, float rotation, const Cake::Vector2& size) {
	if (sprite == nullptr) {
		return;
	}
	sprite->SetPosition(ToKamata(position));
	sprite->SetRotation(rotation);
	sprite->SetSize(ToKamata(size));
	sprite->Draw();
}

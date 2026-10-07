#include "TitleScene.h"

#include "System/Platform/Time/Time.h"
#include "System/Platform/Input/Input.h"
#include "System/Render/PostProcess/ScreenEffect.h"

TitleScene::~TitleScene() {
}

void TitleScene::Initialize() {
}

void TitleScene::Update(Cake::Time* time) {
	// updateQueue_ が小さい順に更新し、その後に衝突判定まで済ませる.
	UpdateGameObjects(time);

	// オブジェクトが動き終わってから、追従先とシェイクを反映する.
	// 注視点の移動範囲を制限したい場合は camera_.Update(time, 範囲) を使う.
	camera_.Update(time);

#ifdef USE_IMGUI
	// カメラ更新の後に描く。そうしないと1フレーム前のカメラで座標変換してしまう.
	camera_.DrawDebugUI();
	collisionManager_.DrawDebugUI();
	collisionManager_.DrawDebugShapes(camera_);
#endif // USE_IMGUI

	if (isExiting_) {
		// 暗転しきってから切り替える.
		if (!ScreenEffect::IsFading()) {
			RequestSceneChange(SceneType::Game);
		}
		return;
	}

	if (Cake::Input::GetDecideButton() && !isExiting_) {
		ScreenEffect::FadeOut(ScreenEffect::kBlack, 0.4f);
		isExiting_ = true;
	}
}

void TitleScene::DrawBackground() {
	DrawGameObjectsBackground();
}

void TitleScene::Draw() {
	DrawGameObjects();
}

void TitleScene::DrawUI() {
	DrawGameObjectsUI();
}

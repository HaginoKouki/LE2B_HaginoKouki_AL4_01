#include "Game.h"
#include "System/Platform/Sound/Sound.h"
#include "System/Platform/Sound/SoundShortcut.h"
#include "System/Render/PostProcess/ScreenEffect.h"
#include "System/Render/Renderer/ModelManager.h"

void Game::Initialize() {
	// KamataEngineの初期化.
	KamataEngine::Initialize(L"TD2_01");

	// DirectXCommonのインスタンスを取得.
	dxCommon_ = KamataEngine::DirectXCommon::GetInstance();
#ifdef USE_IMGUI
	imguiManager_ = KamataEngine::ImGuiManager::GetInstance();
#endif

	time_.Reset();

	// Soundの初期化.
	Cake::Sound::Initialize();
	LoadAllSounds();

	ScreenEffect::Initialize();

	// SceneManagerのインスタンスを取得.
	sceneManager_.Initialize(&time_);
}

void Game::Run() {
	// メインループ.
	while (true) {
		time_.Tick();
		// KamataEngineの更新処理.
		if (KamataEngine::Update()) {
			break;
		}

/*
* 更新処理
———————————————*/
// ImGui受付開始.
#ifdef USE_IMGUI
		imguiManager_->Begin();
#endif // USE_IMGUI

		// シーンの更新処理.
		sceneManager_.Update();

		/*
		* 描画処理
		———————————————*/
		// 描画前処理.
		dxCommon_->PreDraw();

		// 背景スプライト。3D より奥に敷く.
		KamataEngine::Sprite::PreDraw(dxCommon_->GetCommandList());
		sceneManager_.DrawBackground();
		KamataEngine::Sprite::PostDraw();
		// 背景が深度を書いていても、3D が必ずその上に描かれるように消しておく.
		dxCommon_->ClearDepthBuffer();

		// 3D。深度バッファで前後を決める.
		KamataEngine::Model::PreDraw();
		sceneManager_.Draw();
		KamataEngine::Model::PostDraw();

		// 前景スプライト。3D の上に重ねる UI と、全てを覆う全画面演出.
		KamataEngine::Sprite::PreDraw(dxCommon_->GetCommandList());
		sceneManager_.DrawUI();
		KamataEngine::Sprite::PostDraw();

		// シーン内のデバッグ描画も受け付けてからImGuiのフレームを確定する.
#ifdef USE_IMGUI
		imguiManager_->End();
#endif // USE_IMGUI

		// ImGuiの描画.
#ifdef USE_IMGUI
		imguiManager_->Draw();
#endif // USE_IMGUI

		// 描画後処理.
		dxCommon_->PostDraw();
	}
	// シーンが持つ Sprite・定数バッファや音は、エンジンが生きているうちに片付ける.
	// Game のメンバとして自然に破棄されるのを待つと、KamataEngine::Finalize() の後になる.
	sceneManager_.Finalize();

	// シーン側で止め忘れた BGM/SE を止めておく.
	Cake::Sound::StopAll();
	ScreenEffect::Finalize();
	// モデルはシーンを跨いで使い回すので、シーンを畳んだ後に破棄する.
	ModelManager::Finalize();

	// KamataEngineの終了処理.
	KamataEngine::Finalize();
}

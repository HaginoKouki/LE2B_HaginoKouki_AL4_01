#include "SceneManager.h"

#include <cassert>

#include "System/Platform/Time/Time.h"
#include "System/Render/PostProcess/ScreenEffect.h"

#include "System/Platform/Sound/Sound.h"

#include "Game/GameScene.h"
#include "Game/TitleScene.h"

void SceneManager::Initialize(Cake::Time* time) {
	time_ = time;
	ChangeScene(SceneType::Title);
}

void SceneManager::Finalize() { currentScene_.reset(); }

void SceneManager::Update() {
	ScreenEffect::Update(time_);
	if (!currentScene_) {
		return;
	}

	currentScene_->Update(time_);

	// 切り替えは Update() から戻った後に行う。
	// シーンの Update() の内側で破棄すると、実行中の関数の this が消える.
	if (currentScene_->IsSceneChangeRequested()) {
		ChangeScene(currentScene_->GetNextSceneType());
	}
}

void SceneManager::DrawBackground() {
	if (currentScene_) {
		currentScene_->DrawBackground();
	}
}

void SceneManager::Draw() {
	if (currentScene_) {
		currentScene_->Draw();
	}
}

void SceneManager::DrawUI() {
	if (currentScene_) {
		currentScene_->DrawUI();
	}
	// 全画面の覆いは UI も含めて全てを覆うので、最後に描く.
	ScreenEffect::Draw();
}

std::unique_ptr<SceneBase> SceneManager::CreateScene(SceneType type) {
	switch (type) {
	case SceneType::Title:
		return std::make_unique<TitleScene>();
	case SceneType::Game:
		return std::make_unique<GameScene>();
	}

	// SceneType に値を足して、この switch に書き足し忘れた場合の保険.
	assert(false && "未対応の SceneType。SceneManager::CreateScene() に case を足すこと");
	return nullptr;
}

void SceneManager::ChangeScene(SceneType type) {
	// 先に古いシーンを畳んでから新しいシーンを作る。
	// テクスチャやスプライトを持つシーンが一瞬2つ並ぶのを避ける.
	currentScene_.reset();

	currentScene_ = CreateScene(type);

	Cake::Sound::StopAll();

	if (ScreenEffect::IsCovered()) {
		ScreenEffect::FadeIn(kSceneFadeInDuration);
	}

	// 生成に失敗した場合は空のまま進める。Update() / Draw() は空のシーンを読み飛ばす.
	if (!currentScene_) {
		return;
	}
	currentScene_->Initialize();

	// 読み込みにかかった時間を、新しいシーンの最初の deltaTime に混ぜない.
	if (time_) {
		time_->Reset();
	}
}

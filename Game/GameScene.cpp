#include "GameScene.h"

#include <cassert>
#include <optional>
#include <string>

#include "Game/Puzzle/PuzzleRule.h"
#include "Game/Puzzle/StageText.h"
#include "System/Platform/Input/Input.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

#include "System/Platform/Time/Time.h"
#include "System/Render/Renderer/ModelManager.h"

GameScene::~GameScene() {
}

void GameScene::Initialize() {
	cubeModel_.Initialize(ModelManager::Load("cube"));
	cubeObject_ = static_cast<StaticModelObject*>(AddGameObject(std::make_unique<StaticModelObject>()));
	cubeObject_->SetCamera(&camera_);
	cubeObject_->SetModelRenderer(&cubeModel_);

	// 1マス拾って3マスにし、1回回すと■がゴールに届く確認用ステージ.
	const std::optional<Puzzle::PuzzleState> stage = Puzzle::StageText::Parse(
		{
			"..G..",
			".....",
			".....",
			"@o1..",
		},
		1
	);
	assert(stage.has_value() && "ステージの文字が不正");
	initialState_ = stage.value();
	state_ = initialState_;
}

void GameScene::Update(Cake::Time* time) {
	UpdatePuzzle(time);
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

	ImGui::Begin("Puzzle");
	static constexpr const char* kDirectionNames[] = {"None", "Up", "Down", "Left", "Right"};
	ImGui::Text("Floor side: %s", kDirectionNames[static_cast<size_t>(state_.gravity)]);
	ImGui::Text("Rotation: %d / %d", state_.rotationCount, state_.rotationLimit);
	ImGui::Text("%s", state_.isCleared ? "CLEAR" : (state_.isMissed ? "MISS" : "PLAYING"));
	for (const std::string& row : Puzzle::StageText::ToRows(state_)) {
		ImGui::TextUnformatted(row.c_str());
	}
	ImGui::End();
#endif // USE_IMGUI
}

void GameScene::UpdatePuzzle(Cake::Time* time) {
	// 終わった後は SPACE でやり直す（確認用。本番はミスで自動リスタート、クリアで次のステージ）.
	if (state_.isCleared || state_.isMissed) {
		if (Cake::Input::GetRotateButton()) {
			state_ = initialState_;
			stepTimer_ = 0.0f;
		}
		return;
	}

	// 押した瞬間に回す。長押しはやり直し.
	// 長押しの前に1回回ってしまうが、やり直しで初期状態に戻るので問題ない.
	if (Cake::Input::GetRotateButton()) {
		Puzzle::PuzzleRule::Rotate(state_);
	}
	if (Cake::Input::IsRotateButtonHeld()) {
		const float previousHold = holdTimer_;
		holdTimer_ += time->GetUnscaledDeltaTime();
		// しきい値をまたいだ1回だけやり直す。押しっぱなしで何度もやり直さないように.
		if (previousHold < kRetryHoldTime && kRetryHoldTime <= holdTimer_) {
			state_ = initialState_;
			stepTimer_ = 0.0f;
			return;
		}
	} else {
		holdTimer_ = 0.0f;
	}

	// 一定間隔で1歩ずつ進める。フレームレートに関係なく、同じ入力なら同じ結果になる.
	stepTimer_ += time->GetDeltaTime();
	while (stepTimer_ >= kStepInterval) {
		stepTimer_ -= kStepInterval;
		Puzzle::PuzzleRule::Step(state_);
	}
}

void GameScene::DrawBackground() {
	DrawGameObjectsBackground();
}

void GameScene::Draw() {
	DrawGameObjects();
}

void GameScene::DrawUI() {
	DrawGameObjectsUI();
}

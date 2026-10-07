#pragma once

#include "KamataEngine.h"
#include "System/Render/Renderer/ModelRenderer.h"
#include "System/Scene/SceneBase.h"
#include "Game/Temp/StaticModelObject.h"
#include "Game/Puzzle/PuzzleState.h"

namespace Cake {
class Time;
}

class GameScene : public SceneBase {
private:
	ModelRenderer cubeModel_;
	StaticModelObject* cubeObject_ = nullptr;

	// 1歩の間隔 [s]。自機の移動速度になる.
	static constexpr float kStepInterval = 0.25f;
	// 長押しでやり直しになるまでの時間 [s].
	static constexpr float kRetryHoldTime = 1.0f;

	Puzzle::PuzzleState initialState_; // やり直し用に開始時の状態を取っておく.
	Puzzle::PuzzleState state_;
	float stepTimer_ = 0.0f;
	float holdTimer_ = 0.0f;

	void UpdatePuzzle(Cake::Time* time);

public:
	~GameScene() override;

	void Initialize() override;
	void Update(Cake::Time* time) override;
	void DrawBackground() override;
	void Draw() override;
	void DrawUI() override;
};

#include "GameScene.h"

#include <memory>

#include "Game/Object/Block.h"
#include "Game/Object/Player.h"
#include "System/Platform/Time/Time.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

GameScene::~GameScene() {
}

Block* GameScene::AddBlock(const Cake::Vector3& center, const Cake::Vector2& size) {
	Block* block = static_cast<Block*>(AddGameObject(std::make_unique<Block>()));
	block->Setup(center, Cake::Vector3{size.x, size.y, kBlockDepth});
	return block;
}

void GameScene::Initialize() {
	/*
	* ステージ
	———————————————*/
	// 床。上面が kFloorTop になるように、高さの半分だけ下げて置く.
	AddBlock({20.0f, kFloorTop - 1.0f, 0.0f}, {kStageRight - kStageLeft, 2.0f});
	// 左右の壁。ステージの外へ落ちないようにする.
	AddBlock({kStageLeft - 1.0f, 5.0f, 0.0f}, {2.0f, 12.0f});
	AddBlock({kStageRight + 1.0f, 5.0f, 0.0f}, {2.0f, 12.0f});
	// 足場。色を変えて床と見分ける.
	const Cake::Vector4 platformColor{0.6f, 0.8f, 1.0f, 1.0f};
	AddBlock({8.0f, 2.5f, 0.0f}, {4.0f, 1.0f})->SetColor(platformColor);
	AddBlock({14.0f, 4.5f, 0.0f}, {4.0f, 1.0f})->SetColor(platformColor);
	AddBlock({22.0f, 3.0f, 0.0f}, {6.0f, 1.0f})->SetColor(platformColor);
	AddBlock({32.0f, 6.0f, 0.0f}, {4.0f, 1.0f})->SetColor(platformColor);

	/*
	* 自機
	———————————————*/
	player_ = static_cast<Player*>(AddGameObject(std::make_unique<Player>()));
	player_->SetPosition({0.0f, kFloorTop + 1.0f, 0.0f});

	/*
	* カメラ
	———————————————*/
	// 真横から見る（回転なし）。距離だけ近づけて自機を大きく映す.
	camera_.SetRotation(Cake::Vector3::Zero);
	camera_.SetDistance(kCameraDistance);
	camera_.GetCameraFollow().SetBaseObject(player_);

	// 注視点の移動範囲。画面の半分ぶん内側に収めれば、ステージの外が映らない.
	const Cake::Vector2 visibleHalfSize = camera_.GetVisibleSize() * 0.5f;
	cameraClamp_.min = {kStageLeft + visibleHalfSize.x, kFloorTop + visibleHalfSize.y - 2.0f, 0.0f};
	cameraClamp_.max = {kStageRight - visibleHalfSize.x, kCameraTop, 0.0f};
}

void GameScene::Update(Cake::Time* time) {
	// updateQueue_ が小さい順に更新し、その後に衝突判定まで済ませる.
	UpdateGameObjects(time);

	// オブジェクトが動き終わってから、追従先とシェイクを反映する.
	camera_.Update(time, cameraClamp_);

#ifdef USE_IMGUI
	// カメラ更新の後に描く。そうしないと1フレーム前のカメラで座標変換してしまう.
	camera_.DrawDebugUI();
	collisionManager_.DrawDebugUI();
	collisionManager_.DrawDebugShapes(camera_);

	ImGui::Begin("Player");
	if (player_) {
		const Cake::Vector3 position = player_->GetWorldPosition();
		const Cake::Vector3 velocity = player_->GetRigidBody().GetVelocity();
		ImGui::Text("Position : %.2f, %.2f, %.2f", position.x, position.y, position.z);
		ImGui::Text("Velocity : %.2f, %.2f, %.2f", velocity.x, velocity.y, velocity.z);
		ImGui::Text("Grounded : %s", player_->IsGrounded() ? "true" : "false");
		ImGui::Text("Facing   : %s", player_->GetFacing() > 0.0f ? "Right" : "Left");
	}
	ImGui::Separator();
	ImGui::TextUnformatted("Move: A/D, Left/Right, L-Stick");
	ImGui::TextUnformatted("Jump: SPACE / Pad A");
	ImGui::TextUnformatted("Attack: J / Pad X   Dodge: K / Pad B");
	ImGui::End();
#endif // USE_IMGUI
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

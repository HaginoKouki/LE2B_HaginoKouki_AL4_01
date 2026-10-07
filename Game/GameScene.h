#pragma once
/*====================================
 *
 * ゲーム本編のシーン。2軸アクションの最小構成.
 *   床・壁・足場（Block）を置き、自機（Player）をカメラで追う.
 *
 * 【ステージの置き方】
 * Initialize() の AddBlock() で、中心と大きさを指定して並べている.
 * 数が増えたら Grid.h（System/Foundation/Grid）でマップチップ化するか、CSV から読む.
 *
 * 【カメラ】
 * CameraFollow で自機を基準にし、Update(time, cameraClamp_) で注視点の移動範囲を
 * ステージの内側に収める。画面の外に床の下が見えないようにするため.
 *
 * ====================================*/
#include "KamataEngine.h"

#include "System/Foundation/Math/Geometry.h"
#include "System/Foundation/Math/Vector.h"
#include "System/Scene/SceneBase.h"

namespace Cake {
class Time;
}
class Player;
class Block;

class GameScene : public SceneBase {
private:
	// カメラから注視点までの距離。小さいほど自機が大きく映る.
	static constexpr float kCameraDistance = 20.0f;

	// ステージの左右端（X）。この外にはカメラの注視点を出さない.
	static constexpr float kStageLeft = -100.0f;
	static constexpr float kStageRight = 100.0f;
	// 床の上面（Y）.
	static constexpr float kFloorTop = 0.0f;
	// 注視点の高さの上限（Y）.
	static constexpr float kCameraTop = 20.0f;
	// ブロックの奥行き。自機（1）より厚くして、奥へ押し出されないようにする.
	static constexpr float kBlockDepth = 4.0f;

	// 所有権はシーン（gameObjects_）が持つ。ここでは参照用に持つだけ.
	Player* player_ = nullptr;

	// 注視点が動ける範囲。Initialize() で画面に映る範囲から求める.
	Cake::AABB cameraClamp_{};

	// 地形ブロックを1つ置く。center は中心、size は全体の大きさ（幅・高さ。奥行きは kBlockDepth）.
	Block* AddBlock(const Cake::Vector3& center, const Cake::Vector2& size);

public:
	~GameScene() override;

	void Initialize() override;
	void Update(Cake::Time* time) override;
	void DrawBackground() override;
	void Draw() override;
	void DrawUI() override;
};

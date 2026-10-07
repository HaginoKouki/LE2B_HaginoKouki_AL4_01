#pragma once
/*====================================
 *
 * シーンの生成・破棄・切り替えを行うクラス。
 *
 * 【生きているシーンは常に1つ】
 * 全シーンを抱えたまま切り替えると、タイトルへ戻ってもゲームシーンの
 * プレイヤーHPや敵の状態が残る。切り替えのたびに作り直す.
 *
 * ====================================*/
#include <memory>

#include "System/Scene/SceneBase.h"
#include "System/Scene/SceneType.h"

namespace Cake {
class Time;
}

class SceneManager {
private:
	Cake::Time* time_ = nullptr;

	std::unique_ptr<SceneBase> currentScene_;

	// シーンが切り替わった直後に明けるまでの時間 [s].
	static constexpr float kSceneFadeInDuration = 0.4f;

public:
	void Initialize(Cake::Time* time);

	/// <summary>
	/// 現在のシーンを破棄する。KamataEngine::Finalize() より前に呼ぶこと.
	/// シーンが持つ Sprite や再生中の音は、エンジンが生きているうちに片付ける必要がある.
	/// </summary>
	void Finalize();

	void Update();

	// 2D 描画（3D より奥）。Sprite::PreDraw()～PostDraw() の内側で、Draw() より先に呼ぶ.
	void DrawBackground();

	// 3D 描画。Model::PreDraw()～PostDraw() の内側で呼ぶ.
	void Draw();

	// 2D 描画（UI と全画面演出）。Sprite::PreDraw()～PostDraw() の内側で、Draw() の後に呼ぶ.
	void DrawUI();

private:
	// 種類からシーンの実体を作る。シーンを new するのはここだけ.
	static std::unique_ptr<SceneBase> CreateScene(SceneType type);

	// 現在のシーンを破棄して、指定のシーンへ差し替える.
	void ChangeScene(SceneType type);
};

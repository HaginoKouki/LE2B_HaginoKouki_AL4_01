#pragma once
/*====================================
 *
 * ゲームシーンやタイトルシーンなどに共通する基底クラス
 *
 * ====================================*/
#include <vector>
#include <memory>
#include <algorithm>

#include "System/Render/Camera/Camera.h"
#include "System/Scene/GameObject/GameObject.h"
#include "System/Scene/Collider/CollisionManager.h"
#include "System/Scene/SceneType.h"

namespace Cake {
class Time;
}

class SceneBase {
protected:
	std::vector<std::unique_ptr<GameObject>> gameObjects_;
	CollisionManager collisionManager_;
	Camera camera_;

private:
	// 描画順に並べるための一時リスト。毎フレーム作り直すが、確保し直さないよう使い回す.
	struct DrawEntry {
		GameObject* object = nullptr;
		int priority = 0;
		float sortKey = 0.0f; // 3D はカメラからの距離の2乗、2D は spriteOrder_.
	};

	bool needsSort_ = false;
	std::vector<DrawEntry> drawList_;
	// Update() 内で AddGameObject() を呼んだ場合、次のフレームから有効になるようにするための保留リスト.
	std::vector<std::unique_ptr<GameObject>> pendingAdd_;

	bool sceneChangeRequested_ = false;
	SceneType nextSceneType_ = SceneType::Game;

public:
	virtual ~SceneBase();
	virtual void Initialize() = 0;
	virtual void Update(Cake::Time* time) = 0;
	// 2D 描画（3D より奥）。Sprite::PreDraw()～PostDraw() の内側で、3D より先に呼ばれる.
	virtual void DrawBackground() = 0;
	// 3D 描画。Model::PreDraw()～PostDraw() の内側で呼ばれる.
	virtual void Draw() = 0;
	// 2D 描画（3D より手前）。3D を描き終えた後、Sprite::PreDraw()～PostDraw() の内側で呼ばれる.
	virtual void DrawUI() = 0;

	CollisionManager& GetCollisionManager() { return collisionManager_; }
	Camera& GetCamera() { return camera_; }

	/// <summary>
	/// シーン遷移の要求が出ているか。SceneManager が毎フレーム見る.
	/// </summary>
	bool IsSceneChangeRequested() const { return sceneChangeRequested_; }
	SceneType GetNextSceneType() const { return nextSceneType_; }

protected:
	/// <summary>
	/// ゲームオブジェクトを登録する。所有権はシーンが持つ.
	/// シーンへの追加はフレームの先頭にのみ行われるため、Update() 内で AddGameObject() を呼んだ場合、次のフレームから有効になる.
	/// </summary>
	/// <returns>登録したオブジェクトへのポインタ(参照用。delete してはいけない)</returns>
	GameObject* AddGameObject(std::unique_ptr<GameObject> object);

	// updateQueue_ が小さい順に Update() を呼ぶ.
	void UpdateGameObjects(Cake::Time* time);

	/// <summary>
	/// 全オブジェクトの Draw()（3D）を呼ぶ。描画優先度の低い順、同じ優先度の中ではカメラから遠い順.
	/// 遠い順に描くと、半透明のモデルが手前の物を正しく透かして重なる.
	/// </summary>
	void DrawGameObjects();

	/// <summary>
	/// 全オブジェクトの DrawBackground()（2D・3D より奥）を呼ぶ。描画優先度の低い順、同じ優先度の中では spriteOrder_ の小さい順.
	/// </summary>
	void DrawGameObjectsBackground();

	/// <summary>
	/// 全オブジェクトの DrawUI()（2D・3D より手前）を呼ぶ。描画優先度の低い順、同じ優先度の中では spriteOrder_ の小さい順.
	/// </summary>
	void DrawGameObjectsUI();

	/// <summary>
	/// シーン遷移を要求する。実際の切り替えは Update() が戻ってから SceneManager が行う.
	/// 要求を出した後もそのフレームの処理は最後まで走るので、続けて書いても安全.
	/// </summary>
	void RequestSceneChange(SceneType next) {
		// 同じフレームに複数回呼ばれた場合は先勝ち。
		// 「死亡と撃破が同時に成立した」ような場合に、後の判定で上書きされないようにする.
		if (sceneChangeRequested_) {
			return;
		}
		sceneChangeRequested_ = true;
		nextSceneType_ = next;
	}

private:
	// 2D の描画順に drawList_ を並べ直す。DrawGameObjectsBackground() と DrawGameObjectsUI() で共有する.
	void BuildSpriteDrawList();

	void SortIfNeeded() {
		if (!needsSort_) {
			return;
		}
		std::stable_sort(gameObjects_.begin(), gameObjects_.end(), [](const std::unique_ptr<GameObject>& a, const std::unique_ptr<GameObject>& b) {
			return a->GetUpdateQueue() < b->GetUpdateQueue();
		});
		needsSort_ = false;
	}
	void UpdateRecursive(GameObject* object, Cake::Time* time);
	// AddGameObject() されたが、まだ gameObjects_ へ移っていないか.
	bool IsPending(const GameObject* object) const;
	void PropagateDelete(GameObject* object, bool parentDeleted);
};


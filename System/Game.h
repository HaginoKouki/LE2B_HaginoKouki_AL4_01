#pragma once
/*====================================
 *
 * ゲーム全体のメインループを管理するクラス。
 *
 * ====================================*/
#include "KamataEngine.h"

#include "System/Platform/Time/Time.h"
#include "System/Scene/SceneManager.h"

class Game {
private:
	KamataEngine::DirectXCommon* dxCommon_ = nullptr;
	#ifdef USE_IMGUI
	KamataEngine::ImGuiManager* imguiManager_ = nullptr;
	#endif // USE_IMGUI

	Cake::Time time_;

	SceneManager sceneManager_;

public:
	void Initialize();
	void Run();
};

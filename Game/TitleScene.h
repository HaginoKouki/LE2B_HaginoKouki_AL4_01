#pragma once

#include "KamataEngine.h"
#include "System/Scene/SceneBase.h"

namespace Cake {
class Time;
}

class TitleScene : public SceneBase {
	bool isExiting_ = false;

public:
	~TitleScene() override;

	void Initialize() override;
	void Update(Cake::Time* time) override;
	void DrawBackground() override;
	void Draw() override;
	void DrawUI() override;
};

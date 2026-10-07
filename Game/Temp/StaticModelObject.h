#pragma once
#include "System/Render/Renderer/ModelRenderer.h"
#include "System/Render/Camera/Camera.h"
#include "System/Scene/GameObject/GameObject.h"

class StaticModelObject : public GameObject {
private:
	Camera* camera_ = nullptr;
	ModelRenderer* modelRenderer_ = nullptr;

public:
	StaticModelObject() = default;
	~StaticModelObject() override = default;
	void Initialize() override;
	void Update(Cake::Time* time) override;
	void Draw() override;

	void SetCamera(Camera* camera) { camera_ = camera; }
	void SetModelRenderer(ModelRenderer* modelRenderer) { modelRenderer_ = modelRenderer; }
};
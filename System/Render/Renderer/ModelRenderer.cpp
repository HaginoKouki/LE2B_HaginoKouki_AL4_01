#include "ModelRenderer.h"

#include <algorithm>
#include <cassert>

#include "System/Foundation/Math/Convert.h"
#include "System/Foundation/Math/Geometry.h"
#include "System/Foundation/Math/Transform.h"
#include "System/Render/Camera/Camera.h"

void ModelRenderer::Initialize(KamataEngine::Model* model) {
	model_ = model;

	// 二度呼ばれても定数バッファを作り直すだけで済む.
	worldTransform_.Initialize();
	objectColor_.Initialize();
	isInitialized_ = true;
}

void ModelRenderer::SetTexture(uint32_t textureHandle) {
	textureHandle_ = textureHandle;
	hasTextureOverride_ = true;
}

void ModelRenderer::Draw(const Camera& camera, const Cake::Matrix4x4& worldMatrix) {
	assert(isInitialized_ && "ModelRenderer::Initialize() を呼んでから描画すること");
	if (!isInitialized_ || model_ == nullptr || !isVisible_) {
		return;
	}

	// 画面外なら描画コマンドを積まずに抜ける.
	if (cullingRadius_ > 0.0f) {
		const Cake::Vector3 scale = Cake::Math::GetScale(worldMatrix);
		const float maxScale = (std::max)({scale.x, scale.y, scale.z});
		const Cake::Sphere bounds{Cake::Math::GetTranslate(worldMatrix), cullingRadius_ * maxScale};
		if (!camera.IsVisible(bounds)) {
			return;
		}
	}

	// WorldTransform::TransferMatrix() は matWorld_ をそのまま送る（scale_ などから組み直さない）.
	// 親子の合成は GameObject::GetWorldMatrix() で済んでいるので、parent_ も使わない.
	worldTransform_.matWorld_ = ToKamata(worldMatrix);
	worldTransform_.TransferMatrix();
	objectColor_.SetColor(ToKamata(color_));

	if (hasTextureOverride_) {
		model_->Draw(worldTransform_, camera.GetRenderCamera(), textureHandle_, &objectColor_);
	} else {
		model_->Draw(worldTransform_, camera.GetRenderCamera(), &objectColor_);
	}
}

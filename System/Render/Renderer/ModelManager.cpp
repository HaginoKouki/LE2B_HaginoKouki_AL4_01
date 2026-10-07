#include "ModelManager.h"

#include <cassert>

std::unordered_map<std::string, std::unique_ptr<KamataEngine::Model>>& ModelManager::GetModels() {
	static std::unordered_map<std::string, std::unique_ptr<KamataEngine::Model>> models;
	return models;
}

KamataEngine::Model* ModelManager::Load(const std::string& name, bool smoothing) {
	// 平滑化の有無で頂点が変わるので、キーを分ける.
	const std::string key = smoothing ? name + "#smooth" : name;

	auto& models = GetModels();
	if (const auto it = models.find(key); it != models.end()) {
		return it->second.get();
	}

	KamataEngine::Model* model = KamataEngine::Model::CreateFromOBJ(name, smoothing);
	assert(model != nullptr && "モデルの読み込みに失敗した。Resources/{name}/{name}.obj があるか確認すること");
	models.emplace(key, std::unique_ptr<KamataEngine::Model>(model));
	return model;
}

KamataEngine::Model* ModelManager::LoadSphere(uint32_t division) {
	// OBJ のモデル名と区別するため、先頭に '#' を付ける.
	const std::string key = "#sphere" + std::to_string(division);

	auto& models = GetModels();
	if (const auto it = models.find(key); it != models.end()) {
		return it->second.get();
	}

	KamataEngine::Model* model = KamataEngine::Model::CreateSphere(division, division);
	models.emplace(key, std::unique_ptr<KamataEngine::Model>(model));
	return model;
}

void ModelManager::Finalize() {
	GetModels().clear();
}

#include "CameraFollow.h"

#include <algorithm>

#include "System/Scene/GameObject/GameObject.h"

namespace {
// 回転行列の i 行目。行ベクトル形式なので、カメラのローカル i 軸がワールドでどちらを向くかになる.
Cake::Vector3 GetRow(const Cake::Matrix4x4& matrix, int row) {
	return Cake::Vector3{matrix.m[row][0], matrix.m[row][1], matrix.m[row][2]};
}
} // namespace

void CameraFollow::Update(const Cake::Matrix4x4& cameraRotation, const Cake::Vector3& baseObjectHardLimitHalfSize) {
	hardLimitHalfSize_ = baseObjectHardLimitHalfSize;
	if (!IsActive()) {
		return;
	}
	const Cake::Vector3 basePosition = baseObject_->GetWorldPosition();

	// baseObject自身も比重に含めた重心を求める.
	Cake::Vector3 weightedSum = basePosition * baseWeight_;
	float totalWeight = baseWeight_;

	for (const FollowTarget& target : followTargets_) {
		if (target.object == nullptr) {
			continue;
		}
		weightedSum += target.object->GetWorldPosition() * target.weight;
		totalWeight += target.weight;
	}

	const Cake::Vector3 desiredPosition = totalWeight > 0.0f ? weightedSum / totalWeight : basePosition;
	desiredPosition_ = desiredPosition;

	// baseObjectからの引っ張られ量を、カメラから見た 右・上・奥 の成分に分ける.
	const Cake::Vector3 right = GetRow(cameraRotation, 0);
	const Cake::Vector3 up = GetRow(cameraRotation, 1);
	const Cake::Vector3 forward = GetRow(cameraRotation, 2);
	const Cake::Vector3 offset = desiredPosition - basePosition;
	localOffset_ = {
		Cake::Vector3::DotProduct(offset, right),
		Cake::Vector3::DotProduct(offset, up),
		Cake::Vector3::DotProduct(offset, forward),
	};

	// 成分ごとに上限で止めることで、baseObjectが画面外に出ないことを保証する.
	const float x = std::clamp(localOffset_.x, -baseObjectHardLimitHalfSize.x, baseObjectHardLimitHalfSize.x);
	const float y = std::clamp(localOffset_.y, -baseObjectHardLimitHalfSize.y, baseObjectHardLimitHalfSize.y);
	const float z = std::clamp(localOffset_.z, -baseObjectHardLimitHalfSize.z, baseObjectHardLimitHalfSize.z);

	followedPosition_ = basePosition + right * x + up * y + forward * z;
}

void CameraFollow::AddFollowTarget(GameObject* target, float weight) {
	followTargets_.push_back({target, weight});
}

void CameraFollow::RemoveFollowTarget(const GameObject* target) {
	std::erase_if(followTargets_, [target](const FollowTarget& t) {
		return t.object == target;
	});
}

void CameraFollow::Forget(const GameObject* object) {
	if (baseObject_ == object) {
		baseObject_ = nullptr;
	}
	RemoveFollowTarget(object);
}


#ifdef USE_IMGUI
#include <cmath>
#include <imgui.h>

namespace {
// GameObject に名前が無いので、せめてレイヤーで見分けられるようにする.
const char* ToLayerName(GameObjectLayer layer) {
	const uint32_t bits = static_cast<uint32_t>(layer);
	if (bits & static_cast<uint32_t>(GameObjectLayer::Player)) {
		return "Player";
	}
	if (bits & static_cast<uint32_t>(GameObjectLayer::Enemy)) {
		return "Enemy";
	}
	if (bits & static_cast<uint32_t>(GameObjectLayer::UI)) {
		return "UI";
	}
	if (bits & static_cast<uint32_t>(GameObjectLayer::Background)) {
		return "Background";
	}
	return "Default";
}
} // namespace

void CameraFollow::DrawDebugUI() {
	if (!ImGui::CollapsingHeader("Follow", ImGuiTreeNodeFlags_DefaultOpen)) {
		return;
	}
	ImGui::PushID("CameraFollow");

	ImGui::Text("Active: %s", IsActive() ? "Yes" : "No");
	if (!IsActive()) {
		ImGui::TextDisabled("No base object. Camera target is not driven by follow.");
		ImGui::PopID();
		return;
	}

	const Cake::Vector3 basePosition = baseObject_->GetWorldPosition();

	ImGui::Text("Base [%s]: (%.2f, %.2f, %.2f)", ToLayerName(baseObject_->GetLayer()), basePosition.x, basePosition.y, basePosition.z);
	ImGui::DragFloat("Base Weight", &baseWeight_, 0.05f, 0.0f, 100.0f, "%.2f");

	ImGui::Separator();
	ImGui::Text("Desired (raw): (%.2f, %.2f, %.2f)", desiredPosition_.x, desiredPosition_.y, desiredPosition_.z);
	ImGui::Text("Followed:      (%.2f, %.2f, %.2f)", followedPosition_.x, followedPosition_.y, followedPosition_.z);

	// baseObject を画面内に留めるためのクランプが効いているかを可視化する.
	const bool clampedX = std::abs(localOffset_.x) > hardLimitHalfSize_.x;
	const bool clampedY = std::abs(localOffset_.y) > hardLimitHalfSize_.y;
	const bool clampedZ = std::abs(localOffset_.z) > hardLimitHalfSize_.z;
	ImGui::Text("Offset (camera local): (%.2f, %.2f, %.2f)", localOffset_.x, localOffset_.y, localOffset_.z);
	ImGui::Text("Limit: (%.2f, %.2f, %.2f)", hardLimitHalfSize_.x, hardLimitHalfSize_.y, hardLimitHalfSize_.z);
	ImGui::Text("Hard Limit Hit: X %s / Y %s / Z %s", clampedX ? "YES" : "no", clampedY ? "YES" : "no", clampedZ ? "YES" : "no");

	ImGui::Separator();
	ImGui::Text("Targets: %d", static_cast<int>(followTargets_.size()));
	for (size_t index = 0; index < followTargets_.size(); ++index) {
		FollowTarget& target = followTargets_[index];
		ImGui::PushID(static_cast<int>(index));

		if (target.object == nullptr) {
			ImGui::TextDisabled("[%d] (null)", static_cast<int>(index));
			ImGui::PopID();
			continue;
		}
		const Cake::Vector3 targetPosition = target.object->GetWorldPosition();
		if (ImGui::TreeNode("Target", "[%d] %s", static_cast<int>(index), ToLayerName(target.object->GetLayer()))) {
			ImGui::Text("Position: (%.2f, %.2f, %.2f)", targetPosition.x, targetPosition.y, targetPosition.z);
			ImGui::Text("Distance From Base: %.2f", Cake::Vector3::Length(targetPosition, basePosition));
			ImGui::DragFloat("Weight", &target.weight, 0.05f, 0.0f, 100.0f, "%.2f");
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	ImGui::PopID();
}
#endif // USE_IMGUI

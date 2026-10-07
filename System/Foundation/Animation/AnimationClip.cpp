#include "AnimationClip.h"

#include <algorithm>
#include <cassert>

namespace Cake {

AnimationCurve<float>& AnimationClip::Float(uint16_t channel) {
	assert(channel < Channel::Count);

	const auto it = std::find_if(floatCurves_.begin(), floatCurves_.end(), [channel](const std::pair<uint16_t, AnimationCurve<float>>& entry) {
		return entry.first == channel;
	});
	if (it != floatCurves_.end()) {
		return it->second;
	}

	floatCurves_.emplace_back(channel, AnimationCurve<float>{});
	return floatCurves_.back().second;
}

void AnimationClip::SetTextureFrames(const std::vector<uint32_t>& handles, float fps) {
	assert(fps > 0.0f);
	if (handles.empty() || fps <= 0.0f) {
		return;
	}

	// 呼び直したときに前の内容が混ざらないよう、作り直す.
	textureCurve_ = StepCurve<uint32_t>{};

	const float interval = 1.0f / fps;
	for (size_t i = 0; i < handles.size(); ++i) {
		textureCurve_.AddKey(static_cast<float>(i) * interval, handles[i]);
	}

	// 終端のキー。これが無いと最後の1枚だけ表示時間が 0 になり、
	// ループ時にその1枚が飛ばされたように見える.
	textureCurve_.AddKey(static_cast<float>(handles.size()) * interval, handles.back());
}

float AnimationClip::GetDuration() const {
	float duration = textureCurve_.GetDuration();
	for (const std::pair<uint16_t, AnimationCurve<float>>& entry : floatCurves_) {
		duration = std::max(duration, entry.second.GetDuration());
	}
	return duration;
}

} // namespace Cake

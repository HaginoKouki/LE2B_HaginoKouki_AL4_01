#include "Animator.h"

#include <cmath>
#include <cassert>

#include "System/Platform/Time/Time.h"

void Animator::BindTransform(Cake::Transform3& transform) {
	floatTargets_[Cake::Channel::PositionX] = &transform.translate.x;
	floatTargets_[Cake::Channel::PositionY] = &transform.translate.y;
	floatTargets_[Cake::Channel::PositionZ] = &transform.translate.z;
	floatTargets_[Cake::Channel::RotationX] = &transform.rotate.x;
	floatTargets_[Cake::Channel::RotationY] = &transform.rotate.y;
	floatTargets_[Cake::Channel::RotationZ] = &transform.rotate.z;
	floatTargets_[Cake::Channel::ScaleX] = &transform.scale.x;
	floatTargets_[Cake::Channel::ScaleY] = &transform.scale.y;
	floatTargets_[Cake::Channel::ScaleZ] = &transform.scale.z;
}

void Animator::BindTransform(Cake::Transform2& transform) {
	floatTargets_[Cake::Channel::PositionX] = &transform.translate.x;
	floatTargets_[Cake::Channel::PositionY] = &transform.translate.y;
	floatTargets_[Cake::Channel::RotationZ] = &transform.rotate;
	floatTargets_[Cake::Channel::ScaleX] = &transform.scale.x;
	floatTargets_[Cake::Channel::ScaleY] = &transform.scale.y;
}

void Animator::BindFloat(uint16_t channel, float* target) {
	assert(channel < Cake::Channel::Count);
	floatTargets_[channel] = target;
}

void Animator::SetTextureSetter(std::function<void(uint32_t)> setter) {
	textureSetter_ = std::move(setter);
}

void Animator::Play(const Cake::AnimationClip* clip, bool restart) {
	if (clip == nullptr) {
		return;
	}
	// 流し直さない指定なら、同じクリップを再生中はそのまま続ける.
	// 「歩行中は毎フレーム Play を呼ぶ」という書き方を許すための逃げ道.
	if (!restart && clip_ == clip && isPlaying_) {
		return;
	}

	clip_ = clip;
	time_ = 0.0f;
	isReversing_ = false;
	isPlaying_ = true;

	// 呼んだフレームから正しい見た目になるよう、先頭の値をここで書き込む.
	// 次の Update() を待つと、1フレームだけ前のクリップの絵が残る.
	Apply();
}

void Animator::Update(Cake::Time* time) {
	if (!isPlaying_ || clip_ == nullptr || time == nullptr) {
		return;
	}

	const float deltaTime = useUnscaledTime_ ? time->GetUnscaledDeltaTime() : time->GetDeltaTime();
	AdvanceTime(deltaTime * speed_);
	Apply();
}

void Animator::Sample(float time) {
	if (clip_ == nullptr) {
		return;
	}
	time_ = time;
	Apply();
}

void Animator::AdvanceTime(float deltaTime) {
	const float duration = clip_->GetDuration();

	// 長さ 0 のクリップは進めようがない。先頭の値だけ残して止める.
	if (duration <= 0.0f) {
		time_ = 0.0f;
		isPlaying_ = false;
		return;
	}

	time_ += isReversing_ ? -deltaTime : deltaTime;

	switch (clip_->GetLoop()) {
		case Cake::LoopMode::Once:
			if (time_ >= duration) {
				time_ = duration; // 最終キーの値で固定する.
				isPlaying_ = false;
			} else if (time_ < 0.0f) {
				time_ = 0.0f; // speed_ が負のとき、先頭で止まる.
				isPlaying_ = false;
			}
			break;

		case Cake::LoopMode::Loop:
			// 1フレームで複数周してもズレないよう、剰余で戻す.
			time_ = std::fmod(time_, duration);
			if (time_ < 0.0f) {
				time_ += duration;
			}
			break;

		case Cake::LoopMode::PingPong:
			// 端を跨いだ分をそのまま反射させる.
			// 行き過ぎた量は反射のたびに減るので、この while は必ず抜ける.
			while (time_ > duration || time_ < 0.0f) {
				if (time_ > duration) {
					time_ = duration - (time_ - duration);
					isReversing_ = true;
				} else {
					time_ = -time_;
					isReversing_ = false;
				}
			}
			break;
	}
}

void Animator::Apply() const {
	if (clip_ == nullptr) {
		return;
	}

	for (const std::pair<uint16_t, Cake::AnimationCurve<float>>& entry : clip_->GetFloatCurves()) {
		float* target = floatTargets_[entry.first];
		// 繋がれていないチャンネルは黙って捨てる.
		if (target != nullptr) {
			*target = entry.second.Evaluate(time_);
		}
	}

	const Cake::StepCurve<uint32_t>& textureCurve = clip_->GetTextureCurve();
	if (textureSetter_ && !textureCurve.IsEmpty()) {
		textureSetter_(textureCurve.Evaluate(time_));
	}
}

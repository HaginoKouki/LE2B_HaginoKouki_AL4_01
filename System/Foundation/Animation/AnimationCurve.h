#pragma once
/*====================================
 *
 * キーフレーム列を時刻で評価するカーブ。
 * 時刻を渡すと値が返るだけの計算に徹し、誰の何を動かすのかは一切知らない。
 *
 * 【補間するカーブとしないカーブを分けてある】
 * AnimationCurve は Lerp を通す。StepCurve は通さない。
 * 1つのクラスで両方を賄うと、テクスチャハンドル（uint32_t）に対しても
 * Lerp が実体化され、float から整数への変換警告が出る。
 * このプロジェクトは /W4 かつ TreatWarningAsError なのでビルドが止まる。
 *
 * ====================================*/
#include <vector>
#include <utility>
#include <algorithm>
#include <cstdint>

#include "System/Foundation/Math/Easing.h"

namespace Cake {

template <class T>
struct Keyframe {
	float time = 0.0f;
	T value{};
	// このキー「から次のキーまで」の繋ぎ方。最後のキーの ease は使われない.
	Easing::EaseType ease = Easing::EaseType::Linear;
};

/// <summary>
/// キーの間を補間するカーブ。float など、途中の値に意味があるものへ使う.
/// </summary>
template <class T>
class AnimationCurve {
private:
	// time の昇順を常に維持する.
	std::vector<Keyframe<T>> keys_;

public:
	/// <summary>
	/// キーを追加する。途中に挿しても昇順が崩れないよう、位置を探してから入れる.
	/// </summary>
	/// <returns>続けて AddKey() を書けるように自分自身を返す</returns>
	AnimationCurve& AddKey(float time, const T& value, Easing::EaseType ease = Easing::EaseType::Linear) {
		const auto it = std::upper_bound(keys_.begin(), keys_.end(), time, [](float t, const Keyframe<T>& key) {
			return t < key.time;
		});
		keys_.insert(it, Keyframe<T>{time, value, ease});
		return *this;
	}

	bool IsEmpty() const { return keys_.empty(); }

	// 最後のキーの時刻。キーが無ければ 0.
	float GetDuration() const { return keys_.empty() ? 0.0f : keys_.back().time; }

	/// <summary>
	/// 指定時刻の値を求める。範囲外は端のキーでクランプする.
	/// </summary>
	T Evaluate(float time) const {
		if (keys_.empty()) {
			return T{};
		}
		if (time <= keys_.front().time) {
			return keys_.front().value;
		}
		if (time >= keys_.back().time) {
			return keys_.back().value;
		}

		// time を超える最初のキーを探す。その1つ手前が区間の始点になる.
		const auto next = std::upper_bound(keys_.begin(), keys_.end(), time, [](float t, const Keyframe<T>& key) {
			return t < key.time;
		});
		const Keyframe<T>& begin = *(next - 1);
		const Keyframe<T>& end = *next;

		const float span = end.time - begin.time;
		if (span <= 0.0f) {
			// 同じ時刻にキーが並んだ場合。0除算を避け、後ろのキーを採用する.
			return end.value;
		}

		const float t = (time - begin.time) / span;
		return Easing::Ease(begin.value, end.value, t, begin.ease);
	}
};

/// <summary>
/// 補間しないカーブ。テクスチャハンドルのように、途中の値に意味が無いものへ使う.
/// </summary>
template <class T>
class StepCurve {
private:
	// time の昇順を常に維持する.
	std::vector<std::pair<float, T>> keys_;

public:
	StepCurve& AddKey(float time, const T& value) {
		const auto it = std::upper_bound(keys_.begin(), keys_.end(), time, [](float t, const std::pair<float, T>& key) {
			return t < key.first;
		});
		keys_.insert(it, std::pair<float, T>{time, value});
		return *this;
	}

	bool IsEmpty() const { return keys_.empty(); }

	float GetDuration() const { return keys_.empty() ? 0.0f : keys_.back().first; }

	/// <summary>
	/// 指定時刻の値を求める。time 以下で最も遅いキーの値をそのまま返す.
	/// </summary>
	T Evaluate(float time) const {
		if (keys_.empty()) {
			return T{};
		}
		if (time <= keys_.front().first) {
			return keys_.front().second;
		}

		const auto next = std::upper_bound(keys_.begin(), keys_.end(), time, [](float t, const std::pair<float, T>& key) {
			return t < key.first;
		});
		return (next - 1)->second;
	}
};

} // namespace Cake

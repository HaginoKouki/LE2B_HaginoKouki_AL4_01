#pragma once
#include "Vector.h"
#include <cstdint>
#include <numbers>
#include <cmath>
#include <algorithm>
#include <type_traits>

namespace Cake {
namespace Easing {

namespace {
constexpr float kEpsilon = 1e-5f;
}

enum class EaseType : uint8_t {
	Step,
	Linear,
	EaseInSine,
	EaseOutSine,
	EaseInOutSine,
	EaseInQuad,
	EaseOutQuad,
	EaseInOutQuad,
	EaseInCubic,
	EaseOutCubic,
	EaseInOutCubic,
	EaseInQuart,
	EaseOutQuart,
	EaseInOutQuart,
	EaseInQuint,
	EaseOutQuint,
	EaseInOutQuint,
	EaseInExpo,
	EaseOutExpo,
	EaseInOutExpo,
	EaseInCirc,
	EaseOutCirc,
	EaseInOutCirc,
	EaseInBack,
	EaseOutBack,
	EaseInOutBack,
	EaseInElastic,
	EaseOutElastic,
	EaseInOutElastic,
	EaseInBounce,
	EaseOutBounce,
	EaseInOutBounce
};

template <typename T>
T Ease(const T& start, const T& end, const float& time, EaseType type);
// Ease() の本体。float などスカラー値専用.
template <typename T>
T EaseScalar(const T& start, const T& end, const float& time, EaseType type);

template <typename T>
T Lerp(const T& start, const T& end, const float& time);

template <typename T>
T EaseInSine(const T& start, const T& end, const float& time);
template <typename T>
T EaseOutSine(const T& start, const T& end, const float& time);
template <typename T>
T EaseInOutSine(const T& start, const T& end, const float& time);

template <typename T>
T EaseInQuad(const T& start, const T& end, const float& time);
template <typename T>
T EaseOutQuad(const T& start, const T& end, const float& time);
template <typename T>
T EaseInOutQuad(const T& start, const T& end, const float& time);

template <typename T>
T EaseInCubic(const T& start, const T& end, const float& time);
template <typename T>
T EaseOutCubic(const T& start, const T& end, const float& time);
template <typename T>
T EaseInOutCubic(const T& start, const T& end, const float& time);

template <typename T>
T EaseInQuart(const T& start, const T& end, const float& time);
template <typename T>
T EaseOutQuart(const T& start, const T& end, const float& time);
template <typename T>
T EaseInOutQuart(const T& start, const T& end, const float& time);

template <typename T>
T EaseInQuint(const T& start, const T& end, const float& time);
template <typename T>
T EaseOutQuint(const T& start, const T& end, const float& time);
template <typename T>
T EaseInOutQuint(const T& start, const T& end, const float& time);

template <typename T>
T EaseInExpo(const T& start, const T& end, const float& time);
template <typename T>
T EaseOutExpo(const T& start, const T& end, const float& time);
template <typename T>
T EaseInOutExpo(const T& start, const T& end, const float& time);

template <typename T>
T EaseInCirc(const T& start, const T& end, const float& time);
template <typename T>
T EaseOutCirc(const T& start, const T& end, const float& time);
template <typename T>
T EaseInOutCirc(const T& start, const T& end, const float& time);

template <typename T>
T EaseInBack(const T& start, const T& end, const float& time);
template <typename T>
T EaseOutBack(const T& start, const T& end, const float& time);
template <typename T>
T EaseInOutBack(const T& start, const T& end, const float& time);

template <typename T>
T EaseInElastic(const T& start, const T& end, const float& time);
template <typename T>
T EaseOutElastic(const T& start, const T& end, const float& time);
template <typename T>
T EaseInOutElastic(const T& start, const T& end, const float& time);

template <typename T>
T EaseInBounce(const T& start, const T& end, const float& time);
template <typename T>
T EaseOutBounce(const T& start, const T& end, const float& time);
template <typename T>
T EaseInOutBounce(const T& start, const T& end, const float& time);

template <typename T>
T QuadraticFunctions(const T& start, const T& end, const float& time, float a, float b, float c);

/*
* 以下実装.
———————————————*/
template <typename T>
T Ease(const T& start, const T& end, const float& time, EaseType type) {
	// ベクトルは成分ごとに float 版へ流す.
	// 下の各関数のベクトル版はこの定義より後ろにあり、テンプレートの中からは見えないため.
	if constexpr (std::is_same_v<T, Vector2>) {
		return Vector2{Ease(start.x, end.x, time, type), Ease(start.y, end.y, time, type)};
	} else if constexpr (std::is_same_v<T, Vector3>) {
		return Vector3{Ease(start.x, end.x, time, type), Ease(start.y, end.y, time, type), Ease(start.z, end.z, time, type)};
	} else if constexpr (std::is_same_v<T, Vector4>) {
		return Vector4{Ease(start.x, end.x, time, type), Ease(start.y, end.y, time, type), Ease(start.z, end.z, time, type), Ease(start.w, end.w, time, type)};
	} else {
		return EaseScalar(start, end, time, type);
	}
}

template <typename T>
T EaseScalar(const T& start, const T& end, const float& time, EaseType type) {
	switch (type) {
		case EaseType::Step:
			return (time < 1.0f) ? start : end;
		case EaseType::Linear:
			return Lerp(start, end, time);
		case EaseType::EaseInSine:
			return EaseInSine(start, end, time);
		case EaseType::EaseOutSine:
			return EaseOutSine(start, end, time);
		case EaseType::EaseInOutSine:
			return EaseInOutSine(start, end, time);
		case EaseType::EaseInQuad:
			return EaseInQuad(start, end, time);
		case EaseType::EaseOutQuad:
			return EaseOutQuad(start, end, time);
		case EaseType::EaseInOutQuad:
			return EaseInOutQuad(start, end, time);
		case EaseType::EaseInCubic:
			return EaseInCubic(start, end, time);
		case EaseType::EaseOutCubic:
			return EaseOutCubic(start, end, time);
		case EaseType::EaseInOutCubic:
			return EaseInOutCubic(start, end, time);
		case EaseType::EaseInQuart:
			return EaseInQuart(start, end, time);
		case EaseType::EaseOutQuart:
			return EaseOutQuart(start, end, time);
		case EaseType::EaseInOutQuart:
			return EaseInOutQuart(start, end, time);
		case EaseType::EaseInQuint:
			return EaseInQuint(start, end, time);
		case EaseType::EaseOutQuint:
			return EaseOutQuint(start, end, time);
		case EaseType::EaseInOutQuint:
			return EaseInOutQuint(start, end, time);
		case EaseType::EaseInExpo:
			return EaseInExpo(start, end, time);
		case EaseType::EaseOutExpo:
			return EaseOutExpo(start, end, time);
		case EaseType::EaseInOutExpo:
			return EaseInOutExpo(start, end, time);
		case EaseType::EaseInCirc:
			return EaseInCirc(start, end, time);
		case EaseType::EaseOutCirc:
			return EaseOutCirc(start, end, time);
		case EaseType::EaseInOutCirc:
			return EaseInOutCirc(start, end, time);
		case EaseType::EaseInBack:
			return EaseInBack(start, end, time);
		case EaseType::EaseOutBack:
			return EaseOutBack(start, end, time);
		case EaseType::EaseInOutBack:
			return EaseInOutBack(start, end, time);
		case EaseType::EaseInElastic:
			return EaseInElastic(start, end, time);
		case EaseType::EaseOutElastic:
			return EaseOutElastic(start, end, time);
		case EaseType::EaseInOutElastic:
			return EaseInOutElastic(start, end, time);
		case EaseType::EaseInBounce:
			return EaseInBounce(start, end, time);
		case EaseType::EaseOutBounce:
			return EaseOutBounce(start, end, time);
		case EaseType::EaseInOutBounce:
			return EaseInOutBounce(start, end, time);
		default:
			return Lerp(start, end, time);
	}
}

template <typename T>
T Lerp(const T& start, const T& end, const float& time) {
	float result;

	result = (float)(end - start) * time;

	result += start;
	return (T)result;
}
#pragma region EaseSine
template <typename T>
T EaseInSine(const T& start, const T& end, const float& time) {
	float result;

	result = 1.0f - cosf((time * std::numbers::pi_v<float>) / 2.0f);

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
template <typename T>
T EaseOutSine(const T& start, const T& end, const float& time) {
	float result;

	result = sinf((time * std::numbers::pi_v<float>) / 2.0f);

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
template <typename T>
T EaseInOutSine(const T& start, const T& end, const float& time) {
	float result;

	result = -(cosf(std::numbers::pi_v<float> * time) - 1.0f) / 2.0f;

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
#pragma endregion
#pragma region EaseQuad
template <typename T>
T EaseInQuad(const T& start, const T& end, const float& time) {
	float result;

	result = time * time;

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
template <typename T>
T EaseOutQuad(const T& start, const T& end, const float& time) {
	float result;

	result = 1.0f - (1.0f - time) * (1.0f - time);

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
template <typename T>
T EaseInOutQuad(const T& start, const T& end, const float& time) {
	float result;

	if (time < 0.5f) {
		result = 2.0f * time * time;
	} else {
		result = 1.0f - powf(-2.0f * time + 2.0f, 2.0f) / 2.0f;
	}

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
#pragma endregion
#pragma region EaseCubic
template <typename T>
T EaseInCubic(const T& start, const T& end, const float& time) {
	float result;

	result = time * time * time;

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
template <typename T>
T EaseOutCubic(const T& start, const T& end, const float& time) {
	float result;

	result = 1.0f - powf(1.0f - time, 3.0f);

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
template <typename T>
T EaseInOutCubic(const T& start, const T& end, const float& time) {
	float result;

	if (time < 0.5f) {
		result = 4.0f * time * time * time;
	} else {
		result = 1.0f - powf(-2.0f * time + 2.0f, 3.0f) / 2.0f;
	}

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
#pragma endregion
#pragma region EaseQuart
template <typename T>
T EaseInQuart(const T& start, const T& end, const float& time) {
	float result;

	result = time * time * time * time;

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
template <typename T>
T EaseOutQuart(const T& start, const T& end, const float& time) {
	float result;

	result = 1.0f - powf(1.0f - time, 4.0f);

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
template <typename T>
T EaseInOutQuart(const T& start, const T& end, const float& time) {
	float result;

	if (time < 0.5f) {
		result = 8.0f * time * time * time * time;
	} else {
		result = 1.0f - powf(-2.0f * time + 2.0f, 4.0f) / 2.0f;
	}

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
#pragma endregion
#pragma region EaseQuint
template <typename T>
T EaseInQuint(const T& start, const T& end, const float& time) {
	float result;

	result = time * time * time * time * time;

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
template <typename T>
T EaseOutQuint(const T& start, const T& end, const float& time) {
	float result;

	result = 1.0f - powf(1.0f - time, 5.0f);

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
template <typename T>
T EaseInOutQuint(const T& start, const T& end, const float& time) {
	float result;

	if (time < 0.5f) {
		result = 16.0f * time * time * time * time * time;
	} else {
		result = 1.0f - powf(-2.0f * time + 2.0f, 5.0f) / 2.0f;
	}

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
#pragma endregion
#pragma region EaseCirc
template <typename T>
T EaseInCirc(const T& start, const T& end, const float& time) {
	float result;

	result = 1.0f - sqrtf(1.0f - powf(time, 2.0f));

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
template <typename T>
T EaseOutCirc(const T& start, const T& end, const float& time) {
	float result;

	result = sqrtf(1.0f - powf(time - 1.0f, 2.0f));

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
template <typename T>
T EaseInOutCirc(const T& start, const T& end, const float& time) {
	float result;

	if (time < 0.5f) {
		result = (1.0f - sqrtf(1.0f - powf(2.0f * time, 2.0f))) / 2.0f;
	} else {
		result = (sqrtf(1.0f - powf(-2.0f * time + 2.0f, 2.0f)) + 1.0f) / 2.0f;
	}

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
#pragma endregion
#pragma region EaseExpo
template <typename T>
T EaseInExpo(const T& start, const T& end, const float& time) {
	float result;

	if (time == 0.0f) {
		result = 0.0f;
	} else {
		result = powf(2.0f, 10.0f * time - 10.0f);
	}

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
template <typename T>
T EaseOutExpo(const T& start, const T& end, const float& time) {
	float result;

	if (time == 1.0f) {
		return end;
	} else {
		result = 1.0f - powf(2.0f, -10.0f * time);
	}

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
template <typename T>
T EaseInOutExpo(const T& start, const T& end, const float& time) {
	float result;

	if (time == 0.0f) {
		return start;
	} else if (time == 1.0f) {
		return end;
	} else if (time < 0.5f) {
		result = powf(2.0f, 20.0f * time - 10.0f) / 2.0f;
	} else {
		result = (2.0f - powf(2.0f, -20.0f * time + 10.0f)) / 2.0f;
	}

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
#pragma endregion
#pragma region EaseElastic
template <typename T>
T EaseInElastic(const T& start, const T& end, const float& time) {
	float result;

	const float c4 = (2.0f * std::numbers::pi_v<float>) / 3.0f;

	if (time == 0.0f) {
		result = 0.0f;
	} else if (time == 1.0f) {
		result = 1.0f;
	} else {
		result = -powf(2.0f, 10.0f * time - 10.0f) * sinf((time * 10.0f - 10.75f) * c4);
	}

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
template <typename T>
T EaseOutElastic(const T& start, const T& end, const float& time) {
	float result;

	const float c4 = (2.0f * std::numbers::pi_v<float>) / 3.0f;

	if (time == 0.0f) {
		result = 0.0f;
	} else if (time == 1.0f) {
		result = 1.0f;
	} else {
		result = powf(2.0f, -10.0f * time) * sinf((time * 10.0f - 0.75f) * c4) + 1.0f;
	}

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
template <typename T>
T EaseInOutElastic(const T& start, const T& end, const float& time) {
	float result;

	const float c5 = (2.0f * std::numbers::pi_v<float>) / 4.5f;

	if (time == 0.0f) {
		result = 0.0f;
	} else if (time == 1.0f) {
		result = 1.0f;
	} else if (time < 0.5f) {
		result = -(powf(2.0f, 20.0f * time - 10.0f) * sinf((20.0f * time - 11.125f) * c5)) / 2.0f;
	} else {
		result = (powf(2.0f, -20.0f * time + 10.0f) * sinf((20.0f * time - 11.125f) * c5)) / 2.0f + 1.0f;
	}

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
#pragma endregion
#pragma region EaseBack
template <typename T>
T EaseInBack(const T& start, const T& end, const float& time) {
	float result;

	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;

	if (time == 0.0f) {
		result = 0.0f;
	} else {
		result = (c3 * time * time * time - c1 * time * time);
	}

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
template <typename T>
T EaseOutBack(const T& start, const T& end, const float& time) {
	float result;

	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;

	if (time == 1.0f) {
		return end;
	} else {
		result = 1.0f + c3 * powf(time - 1.0f, 3.0f) + c1 * powf(time - 1.0f, 2.0f);
	}

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
template <typename T>
T EaseInOutBack(const T& start, const T& end, const float& time) {
	float result;
	const float c1 = 1.70158f;
	const float c2 = c1 * 1.525f;

	if (time < 0.5f) {
		result = (powf(2.0f * time, 2.0f) * ((c2 + 1.0f) * 2.0f * time - c2)) / 2.0f;
	} else {
		result = (powf(2.0f * time - 2.0f, 2.0f) * ((c2 + 1.0f) * (time * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
	}

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
#pragma endregion
#pragma region EaseBounce
template <typename T>
T EaseInBounce(const T& start, const T& end, const float& time) {
	float result;

	result = 1.0f - EaseOutBounce(0.0f, 1.0f, 1.0f - time);

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
template <typename T>
T EaseOutBounce(const T& start, const T& end, const float& time) {
	float result;

	const float n1 = 7.5625f;
	const float d1 = 2.75f;

	if (time < 1.0f / d1) {
		result = n1 * time * time;
	} else if (time < 2.0f / d1) {
		float t = time - 1.5f / d1;
		result = n1 * t * t + 0.75f;
	} else if (time < 2.5f / d1) {
		float t = time - 2.25f / d1;
		result = n1 * t * t + 0.9375f;
	} else {
		float t = time - 2.625f / d1;
		result = n1 * t * t + 0.984375f;
	}

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
template <typename T>
T EaseInOutBounce(const T& start, const T& end, const float& time) {
	float result;

	if (time < 0.5f) {
		result = (1.0f - EaseOutBounce(0.0f, 1.0f, 1.0f - 2.0f * time)) / 2.0f;
	} else {
		result = (1.0f + EaseOutBounce(0.0f, 1.0f, 2.0f * time - 1.0f)) / 2.0f;
	}

	result = (end >= start) ? start + result * (end - start) : start - result * (start - end);
	return (T)result;
}
#pragma endregion

template <typename T>
T QuadraticFunctions(const T& start, const T& end, const float& time, float a, float b, float c) {
	float result;
	result = (float)pow((double)(a * time), 2.0) + b * time + c;

	return (T)result;
}

/*
* Vector2.
———————————————*/
template <typename Func>
Vector2 ApplyComponentWise(const Vector2& start, const Vector2& end, const float& time, Func func) {
	return {func(start.x, end.x, time), func(start.y, end.y, time)};
}
inline Vector2 Lerp(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::Lerp<float>);
}

#pragma region EaseSine
inline Vector2 EaseInSine(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInSine<float>);
}
inline Vector2 EaseOutSine(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutSine<float>);
}
inline Vector2 EaseInOutSine(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutSine<float>);
}
#pragma endregion
#pragma region EaseQuad
inline Vector2 EaseInQuad(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInQuad<float>);
}
inline Vector2 EaseOutQuad(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutQuad<float>);
}
inline Vector2 EaseInOutQuad(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutQuad<float>);
}
#pragma endregion
#pragma region EaseCubic
inline Vector2 EaseInCubic(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInCubic<float>);
}
inline Vector2 EaseOutCubic(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutCubic<float>);
}
inline Vector2 EaseInOutCubic(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutCubic<float>);
}
#pragma endregion
#pragma region EaseQuart
inline Vector2 EaseInQuart(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInQuart<float>);
}
inline Vector2 EaseOutQuart(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutQuart<float>);
}
inline Vector2 EaseInOutQuart(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutQuart<float>);
}
#pragma endregion
#pragma region EaseQuint
inline Vector2 EaseInQuint(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInQuint<float>);
}
inline Vector2 EaseOutQuint(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutQuint<float>);
}
inline Vector2 EaseInOutQuint(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutQuint<float>);
}
#pragma endregion
#pragma region EaseExpo
inline Vector2 EaseInExpo(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInExpo<float>);
}
inline Vector2 EaseOutExpo(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutExpo<float>);
}
inline Vector2 EaseInOutExpo(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutExpo<float>);
}
#pragma endregion
#pragma region EaseCirc
inline Vector2 EaseInCirc(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInCirc<float>);
}
inline Vector2 EaseOutCirc(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutCirc<float>);
}
inline Vector2 EaseInOutCirc(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutCirc<float>);
}
#pragma endregion
#pragma region EaseBack
inline Vector2 EaseInBack(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInBack<float>);
}
inline Vector2 EaseOutBack(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutBack<float>);
}
inline Vector2 EaseInOutBack(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutBack<float>);
}
#pragma endregion
#pragma region EaseElastic
inline Vector2 EaseInElastic(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInElastic<float>);
}
inline Vector2 EaseOutElastic(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutElastic<float>);
}
inline Vector2 EaseInOutElastic(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutElastic<float>);
}
#pragma endregion
#pragma region EaseBounce
inline Vector2 EaseInBounce(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInBounce<float>);
}
inline Vector2 EaseOutBounce(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutBounce<float>);
}
inline Vector2 EaseInOutBounce(const Vector2& start, const Vector2& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutBounce<float>);
}
#pragma endregion

/*
* Vector3.
———————————————*/
template <typename Func>
Vector3 ApplyComponentWise(const Vector3& start, const Vector3& end, const float& time, Func func) {
	return {func(start.x, end.x, time), func(start.y, end.y, time), func(start.z, end.z, time)};
}
inline Vector3 Lerp(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::Lerp<float>);
}
#pragma region EaseSine
inline Vector3 EaseInSine(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInSine<float>);
}
inline Vector3 EaseOutSine(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutSine<float>);
}
inline Vector3 EaseInOutSine(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutSine<float>);
}
#pragma endregion
#pragma region EaseQuad
inline Vector3 EaseInQuad(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInQuad<float>);
}
inline Vector3 EaseOutQuad(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutQuad<float>);
}
inline Vector3 EaseInOutQuad(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutQuad<float>);
}
#pragma endregion
#pragma region EaseCubic
inline Vector3 EaseInCubic(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInCubic<float>);
}
inline Vector3 EaseOutCubic(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutCubic<float>);
}
inline Vector3 EaseInOutCubic(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutCubic<float>);
}
#pragma endregion
#pragma region EaseQuart
inline Vector3 EaseInQuart(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInQuart<float>);
}
inline Vector3 EaseOutQuart(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutQuart<float>);
}
inline Vector3 EaseInOutQuart(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutQuart<float>);
}
#pragma endregion
#pragma region EaseQuint
inline Vector3 EaseInQuint(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInQuint<float>);
}
inline Vector3 EaseOutQuint(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutQuint<float>);
}
inline Vector3 EaseInOutQuint(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutQuint<float>);
}
#pragma endregion
#pragma region EaseExpo
inline Vector3 EaseInExpo(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInExpo<float>);
}
inline Vector3 EaseOutExpo(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutExpo<float>);
}
inline Vector3 EaseInOutExpo(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutExpo<float>);
}
#pragma endregion
#pragma region EaseCirc
inline Vector3 EaseInCirc(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInCirc<float>);
}
inline Vector3 EaseOutCirc(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutCirc<float>);
}
inline Vector3 EaseInOutCirc(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutCirc<float>);
}
#pragma endregion
#pragma region EaseBack
inline Vector3 EaseInBack(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInBack<float>);
}
inline Vector3 EaseOutBack(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutBack<float>);
}
inline Vector3 EaseInOutBack(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutBack<float>);
}
#pragma endregion
#pragma region EaseElastic
inline Vector3 EaseInElastic(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInElastic<float>);
}
inline Vector3 EaseOutElastic(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutElastic<float>);
}
inline Vector3 EaseInOutElastic(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutElastic<float>);
}
#pragma endregion
#pragma region EaseBounce
inline Vector3 EaseInBounce(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInBounce<float>);
}
inline Vector3 EaseOutBounce(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutBounce<float>);
}
inline Vector3 EaseInOutBounce(const Vector3& start, const Vector3& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutBounce<float>);
}
#pragma endregion

// 球面線形補間.
inline Vector3 Slerp(const Vector3& start, const Vector3& end, const float& time) {
	// 正規化.
	Vector3 nStart = Vector3::Normalize(start);
	Vector3 nEnd = Vector3::Normalize(end);

	// 球面線形補間したベクトル.
	Vector3 result;

	// cosθを内積から求める.
	float dot = Vector3::DotProduct(nStart, nEnd);
	// 誤差でdotが[-1,1]を超えないようにクランプ.
	dot = std::clamp(dot, -1.0f, 1.0f);

	if (dot > 1.0f - kEpsilon) {
		// ほぼ同じ方向.
		result = nEnd;
	} else if (dot < -1.0f + kEpsilon) {
		// ほぼ真逆の方向.
		result = Vector3::Normalize(Cake::Easing::Lerp(nStart, nEnd, time));
	} else {
		// acosでθを求める.
		float theta = std::acos(dot);
		// sinθを求める.
		float sinTheta = std::sin(theta);
		// sin(θ(1-t))を求める,
		float s0 = std::sin((1.0f - time) * theta);
		// sinθtを求める.
		float s1 = std::sin(time * theta);

		result.x = (s0 * nStart.x + s1 * nEnd.x) / sinTheta;
		result.y = (s0 * nStart.y + s1 * nEnd.y) / sinTheta;
		result.z = (s0 * nStart.z + s1 * nEnd.z) / sinTheta;
	}

	// ベクトルの長さを線形補間して反映.
	float length = Cake::Easing::Lerp(Vector3::Length(start), Vector3::Length(end), time);
	return result * length;
}

/*
* Vector4.
———————————————*/
template <typename Func>
Vector4 ApplyComponentWise(const Vector4& start, const Vector4& end, const float& time, Func func) {
	return {func(start.x, end.x, time), func(start.y, end.y, time), func(start.z, end.z, time), func(start.w, end.w, time)};
}
inline Vector4 Lerp(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::Lerp<float>);
}
#pragma region EaseSine
inline Vector4 EaseInSine(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInSine<float>);
}
inline Vector4 EaseOutSine(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutSine<float>);
}
inline Vector4 EaseInOutSine(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutSine<float>);
}
#pragma endregion
#pragma region EaseQuad
inline Vector4 EaseInQuad(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInQuad<float>);
}
inline Vector4 EaseOutQuad(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutQuad<float>);
}
inline Vector4 EaseInOutQuad(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutQuad<float>);
}
#pragma endregion
#pragma region EaseCubic
inline Vector4 EaseInCubic(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInCubic<float>);
}
inline Vector4 EaseOutCubic(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutCubic<float>);
}
inline Vector4 EaseInOutCubic(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutCubic<float>);
}
#pragma endregion
#pragma region EaseQuart
inline Vector4 EaseInQuart(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInQuart<float>);
}
inline Vector4 EaseOutQuart(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutQuart<float>);
}
inline Vector4 EaseInOutQuart(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutQuart<float>);
}
#pragma endregion
#pragma region EaseQuint
inline Vector4 EaseInQuint(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInQuint<float>);
}
inline Vector4 EaseOutQuint(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutQuint<float>);
}
inline Vector4 EaseInOutQuint(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutQuint<float>);
}
#pragma endregion
#pragma region EaseExpo
inline Vector4 EaseInExpo(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInExpo<float>);
}
inline Vector4 EaseOutExpo(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutExpo<float>);
}
inline Vector4 EaseInOutExpo(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutExpo<float>);
}
#pragma endregion
#pragma region EaseCirc
inline Vector4 EaseInCirc(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInCirc<float>);
}
inline Vector4 EaseOutCirc(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutCirc<float>);
}
inline Vector4 EaseInOutCirc(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutCirc<float>);
}
#pragma endregion
#pragma region EaseBack
inline Vector4 EaseInBack(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInBack<float>);
}
inline Vector4 EaseOutBack(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutBack<float>);
}
inline Vector4 EaseInOutBack(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutBack<float>);
}
#pragma endregion
#pragma region EaseElastic
inline Vector4 EaseInElastic(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInElastic<float>);
}
inline Vector4 EaseOutElastic(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutElastic<float>);
}
inline Vector4 EaseInOutElastic(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutElastic<float>);
}
#pragma endregion
#pragma region EaseBounce
inline Vector4 EaseInBounce(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInBounce<float>);
}
inline Vector4 EaseOutBounce(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseOutBounce<float>);
}
inline Vector4 EaseInOutBounce(const Vector4& start, const Vector4& end, const float& time) {
	return ApplyComponentWise(start, end, time, Cake::Easing::EaseInOutBounce<float>);
}
#pragma endregion

} // namespace Easing

} // namespace Cake

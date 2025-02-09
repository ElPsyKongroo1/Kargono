#include "kgpch.h"

#include "Kargono/Math/Interpolation.h"

namespace Kargono::Math
{
	float Interpolation::Linear(float startPoint, float endPoint, float progress)
	{
		return startPoint + (endPoint - startPoint) * progress;
	}

	Math::vec2 Interpolation::Linear(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Math::vec2
		(
			startPoint.x + (endPoint.x - startPoint.x) * progress,
			startPoint.y + (endPoint.y - startPoint.y) * progress
		);
	}

	Math::vec3 Interpolation::Linear(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Math::vec3
		(
			startPoint.x + (endPoint.x - startPoint.x) * progress,
			startPoint.y + (endPoint.y - startPoint.y) * progress,
			startPoint.z + (endPoint.z - startPoint.z) * progress
		);
	}

	Math::vec4 Interpolation::Linear(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Math::vec4
		(
			startPoint.x + (endPoint.x - startPoint.x) * progress,
			startPoint.y + (endPoint.y - startPoint.y) * progress,
			startPoint.z + (endPoint.z - startPoint.z) * progress,
			startPoint.w + (endPoint.w - startPoint.w) * progress
		);
	}

	static float EaseInSinCalc(float progress)
	{
		return 1.0f - std::cosf((progress * std::numbers::pi_v<float>) / 2.0f);
	}

	float Interpolation::EaseInSin(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInSinCalc(progress));
	}

	Math::vec2 Interpolation::EaseInSin(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInSinCalc(progress));
	}

	Math::vec3 Interpolation::EaseInSin(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInSinCalc(progress));
	}

	Math::vec4 Interpolation::EaseInSin(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInSinCalc(progress));
	}

	static float EaseOutSinCalc(float progress)
	{
		return std::sinf((progress * std::numbers::pi_v<float>) / 2.0f);
	}

	float Interpolation::EaseOutSin(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutSinCalc(progress));
	}

	Math::vec2 Interpolation::EaseOutSin(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutSinCalc(progress));
	}

	Math::vec3 Interpolation::EaseOutSin(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutSinCalc(progress));
	}

	Math::vec4 Interpolation::EaseOutSin(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutSinCalc(progress));
	}

	static float EaseInOutSinCalc(float progress)
	{
		return -(std::cosf(std::numbers::pi_v<float> *progress) - 1.0f) / 2.0f;
	}

	float Interpolation::EaseInOutSin(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutSinCalc(progress));
	}

	Math::vec2 Interpolation::EaseInOutSin(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutSinCalc(progress));
	}

	Math::vec3 Interpolation::EaseInOutSin(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutSinCalc(progress));
	}

	Math::vec4 Interpolation::EaseInOutSin(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutSinCalc(progress));
	}

	static float EaseInQuadCalc(float progress)
	{
		return progress * progress;
	}

	float Interpolation::EaseInQuad(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInQuadCalc(progress));
	}

	Math::vec2 Interpolation::EaseInQuad(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInQuadCalc(progress));
	}

	Math::vec3 Interpolation::EaseInQuad(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInQuadCalc(progress));
	}

	Math::vec4 Interpolation::EaseInQuad(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInQuadCalc(progress));
	}

	static float EaseOutQuadCalc(float progress)
	{
		return 1.0f - (1.0f - progress) * (1.0f - progress);
	}

	float Interpolation::EaseOutQuad(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutQuadCalc(progress));
	}

	Math::vec2 Interpolation::EaseOutQuad(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutQuadCalc(progress));
	}

	Math::vec3 Interpolation::EaseOutQuad(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutQuadCalc(progress));
	}

	Math::vec4 Interpolation::EaseOutQuad(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutQuadCalc(progress));
	}

	static float EaseInOutQuadCalc(float progress)
	{
		return progress < 0.5f ? 2.0f * progress * progress : 1.0f - std::powf(-2.0f * progress + 2.0f, 2.0f) / 2.0f;
	}

	float Interpolation::EaseInOutQuad(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutQuadCalc(progress));
	}

	Math::vec2 Interpolation::EaseInOutQuad(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutQuadCalc(progress));
	}

	Math::vec3 Interpolation::EaseInOutQuad(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutQuadCalc(progress));
	}

	Math::vec4 Interpolation::EaseInOutQuad(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutQuadCalc(progress));
	}

	static float EaseInCubicCalc(float progress)
	{
		return progress * progress * progress;
	}

	float Interpolation::EaseInCubic(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInCubicCalc(progress));
	}

	Math::vec2 Interpolation::EaseInCubic(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInCubicCalc(progress));
	}

	Math::vec3 Interpolation::EaseInCubic(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInCubicCalc(progress));
	}

	Math::vec4 Interpolation::EaseInCubic(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInCubicCalc(progress));
	}

	static float EaseOutCubicCalc(float progress)
	{
		return 1.0f - std::powf(1.0f - progress, 3.0f);
	}

	float Interpolation::EaseOutCubic(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutCubicCalc(progress));
	}

	Math::vec2 Interpolation::EaseOutCubic(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutCubicCalc(progress));
	}

	Math::vec3 Interpolation::EaseOutCubic(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutCubicCalc(progress));
	}

	Math::vec4 Interpolation::EaseOutCubic(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutCubicCalc(progress));
	}

	static float EaseInOutCubicCalc(float progress)
	{
		return progress < 0.5f ? 4.0f * progress * progress * progress : 1.0f - std::powf(-2.0f * progress + 2.0f, 3.0f) / 2.0f;
	}

	float Interpolation::EaseInOutCubic(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutCubicCalc(progress));
	}

	Math::vec2 Interpolation::EaseInOutCubic(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutCubicCalc(progress));
	}

	Math::vec3 Interpolation::EaseInOutCubic(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutCubicCalc(progress));
	}

	Math::vec4 Interpolation::EaseInOutCubic(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutCubicCalc(progress));
	}

	static float EaseInQuartCalc(float progress)
	{
		return progress * progress * progress * progress;
	}

	float Interpolation::EaseInQuart(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInQuartCalc(progress));
	}

	Math::vec2 Interpolation::EaseInQuart(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInQuartCalc(progress));
	}

	Math::vec3 Interpolation::EaseInQuart(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInQuartCalc(progress));
	}

	Math::vec4 Interpolation::EaseInQuart(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInQuartCalc(progress));
	}

	static float EaseOutQuartCalc(float progress)
	{
		return 1.0f - std::powf(1.0f - progress, 4.0f);
	}

	float Interpolation::EaseOutQuart(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutQuartCalc(progress));
	}

	Math::vec2 Interpolation::EaseOutQuart(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutQuartCalc(progress));
	}

	Math::vec3 Interpolation::EaseOutQuart(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutQuartCalc(progress));
	}

	Math::vec4 Interpolation::EaseOutQuart(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutQuartCalc(progress));
	}

	static float EaseInOutQuartCalc(float progress)
	{
		return progress < 0.5f ? 8.0f * progress * progress * progress * progress : 1.0f - std::powf(-2.0f * progress + 2.0f, 4.0f) / 2.0f;
	}

	float Interpolation::EaseInOutQuart(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutQuartCalc(progress));
	}

	Math::vec2 Interpolation::EaseInOutQuart(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutQuartCalc(progress));
	}

	Math::vec3 Interpolation::EaseInOutQuart(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutQuartCalc(progress));
	}

	Math::vec4 Interpolation::EaseInOutQuart(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutQuartCalc(progress));
	}

	static float EaseInQuintCalc(float progress)
	{
		return progress * progress * progress * progress * progress;
	}

	float Interpolation::EaseInQuint(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInQuintCalc(progress));
	}

	Math::vec2 Interpolation::EaseInQuint(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInQuintCalc(progress));
	}

	Math::vec3 Interpolation::EaseInQuint(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInQuintCalc(progress));
	}

	Math::vec4 Interpolation::EaseInQuint(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInQuintCalc(progress));
	}

	static float EaseOutQuintCalc(float progress)
	{
		return 1.0f - std::powf(1.0f - progress, 5.0f);
	}

	float Interpolation::EaseOutQuint(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutQuintCalc(progress));
	}

	Math::vec2 Interpolation::EaseOutQuint(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutQuintCalc(progress));
	}

	Math::vec3 Interpolation::EaseOutQuint(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutQuintCalc(progress));
	}

	Math::vec4 Interpolation::EaseOutQuint(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutQuintCalc(progress));
	}

	static float EaseInOutQuintCalc(float progress)
	{
		return progress < 0.5 ?
			16.0f * progress * progress * progress * progress * progress :
			1 - std::powf(-2.0f * progress + 2.0f, 5.0f) / 2.0f;
	}

	float Interpolation::EaseInOutQuint(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutQuintCalc(progress));
	}

	Math::vec2 Interpolation::EaseInOutQuint(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutQuintCalc(progress));
	}

	Math::vec3 Interpolation::EaseInOutQuint(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutQuintCalc(progress));
	}

	Math::vec4 Interpolation::EaseInOutQuint(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutQuintCalc(progress));
	}

	static float EaseInExpoCalc(float progress)
	{
		constexpr float k_Tolerance{ 0.0001f };
		return progress < k_Tolerance ? 0.0f : std::powf(2.0f, 10.0f * progress - 10.0f);
	}

	float Interpolation::EaseInExpo(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInExpoCalc(progress));
	}

	Math::vec2 Interpolation::EaseInExpo(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInExpoCalc(progress));
	}

	Math::vec3 Interpolation::EaseInExpo(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInExpoCalc(progress));
	}

	Math::vec4 Interpolation::EaseInExpo(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInExpoCalc(progress));
	}

	static float EaseOutExpoCalc(float progress)
	{
		constexpr float k_Tolerance{ 0.9999f };
		return progress > k_Tolerance ? 1.0f : 1.0f - std::powf(2.0f, -10.0f * progress);
	}

	float Interpolation::EaseOutExpo(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutExpoCalc(progress));
	}

	Math::vec2 Interpolation::EaseOutExpo(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutExpoCalc(progress));
	}

	Math::vec3 Interpolation::EaseOutExpo(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutExpoCalc(progress));
	}

	Math::vec4 Interpolation::EaseOutExpo(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutExpoCalc(progress));
	}

	static float EaseInOutExpoCalc(float progress)
	{
		constexpr float k_LowTolerance{ 0.0001f };

		float returnValue;

		if (progress < k_LowTolerance)
		{
			returnValue = 0.0f;
		}
		else if (progress > k_LowTolerance)
		{
			returnValue = 1.0f;
		}
		else if (progress < 0.5)
		{
			returnValue = std::powf(2.0f, 20.0f * progress - 10.0f) / 2.0f;
		}
		else
		{
			returnValue = (2.0f - std::powf(2.0f, -20.0f * progress + 10.0f)) / 2.0f;
		}

		return returnValue;
	}

	float Interpolation::EaseInOutExpo(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutExpoCalc(progress));
	}

	Math::vec2 Interpolation::EaseInOutExpo(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutExpoCalc(progress));
	}

	Math::vec3 Interpolation::EaseInOutExpo(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutExpoCalc(progress));
	}

	Math::vec4 Interpolation::EaseInOutExpo(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutExpoCalc(progress));
	}

	static float EaseInCircCalc(float progress)
	{
		return 1.0f - std::sqrtf(1.0f - std::powf(progress, 2.0f));
	}

	float Interpolation::EaseInCirc(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInCircCalc(progress));
	}

	Math::vec2 Interpolation::EaseInCirc(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInCircCalc(progress));
	}

	Math::vec3 Interpolation::EaseInCirc(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInCircCalc(progress));
	}

	Math::vec4 Interpolation::EaseInCirc(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInCircCalc(progress));
	}

	static float EaseOutCircCalc(float progress)
	{
		return std::sqrtf(1.0f - std::powf(progress - 1.0f, 2.0f));
	}

	float Interpolation::EaseOutCirc(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutCircCalc(progress));
	}

	Math::vec2 Interpolation::EaseOutCirc(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutCircCalc(progress));
	}

	Math::vec3 Interpolation::EaseOutCirc(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutCircCalc(progress));
	}

	Math::vec4 Interpolation::EaseOutCirc(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutCircCalc(progress));
	}

	static float EaseInOutCircCalc(float progress)
	{
		return progress < 0.5f
			? (1.0f - std::sqrtf(1.0f - std::powf(2.0f * progress, 2.0f))) / 2.0f
			: (std::sqrtf(1.0f - std::powf(-2.0f * progress + 2.0f, 2.0f)) + 1.0f) / 2.0f;
	}

	float Interpolation::EaseInOutCirc(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutCircCalc(progress));
	}

	Math::vec2 Interpolation::EaseInOutCirc(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutCircCalc(progress));
	}

	Math::vec3 Interpolation::EaseInOutCirc(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutCircCalc(progress));
	}

	Math::vec4 Interpolation::EaseInOutCirc(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutCircCalc(progress));
	}

	static float EaseInBackCalc(float progress)
	{
		constexpr float c1 = 1.70158f;
		constexpr float c3 = { c1 + 1.0f };

		return c3 * progress * progress * progress - c1 * progress * progress;
	}

	float Interpolation::EaseInBack(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInBackCalc(progress));
	}

	Math::vec2 Interpolation::EaseInBack(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInBackCalc(progress));
	}

	Math::vec3 Interpolation::EaseInBack(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInBackCalc(progress));
	}

	Math::vec4 Interpolation::EaseInBack(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInBackCalc(progress));
	}

	static float EaseOutBackCalc(float progress)
	{
		constexpr float c1{ 1.70158f };
		constexpr float c3{ c1 + 1.0f };

		return 1.0f + c3 * std::powf(progress - 1.0f, 3.0f) + c1 * std::powf(progress - 1.0f, 2.0f);
	}

	float Interpolation::EaseOutBack(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutBackCalc(progress));
	}

	Math::vec2 Interpolation::EaseOutBack(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutBackCalc(progress));
	}

	Math::vec3 Interpolation::EaseOutBack(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutBackCalc(progress));
	}

	Math::vec4 Interpolation::EaseOutBack(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutBackCalc(progress));
	}

	static float EaseInOutBackCalc(float progress)
	{
		constexpr float c1 { 1.70158f };
		constexpr float c2 {c1 * 1.525f};

		return progress < 0.5f
			? (std::powf(2.0f * progress, 2.0f) * ((c2 + 1.0f) * 2.0f * progress - c2)) / 2.0f
			: (std::powf(2.0f * progress - 2.0f, 2.0f) * ((c2 + 1.0f) * (progress * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
	}

	float Interpolation::EaseInOutBack(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutBackCalc(progress));
	}

	Math::vec2 Interpolation::EaseInOutBack(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutBackCalc(progress));
	}

	Math::vec3 Interpolation::EaseInOutBack(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutBackCalc(progress));
	}

	Math::vec4 Interpolation::EaseInOutBack(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutBackCalc(progress));
	}

	static float EaseInElasticCalc(float progress)
	{
		constexpr float c4{ (2.0f * std::numbers::pi_v<float>) / 3.0f };
		constexpr float k_LowTolerance{ 0.0001f };
		constexpr float k_HighTolerance{ 0.9999f };

		return progress < k_LowTolerance
			? 0.0f
			: progress > k_HighTolerance
			? 1.0f
			: -std::powf(2.0f, 10.0f * progress - 10.0f) * std::sinf((progress * 10.0f - 10.75f) * c4);
	}

	float Interpolation::EaseInElastic(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInElasticCalc(progress));
	}

	Math::vec2 Interpolation::EaseInElastic(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInElasticCalc(progress));
	}

	Math::vec3 Interpolation::EaseInElastic(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInElasticCalc(progress));
	}

	Math::vec4 Interpolation::EaseInElastic(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInElasticCalc(progress));
	}

	static float EaseOutElasticCalc(float progress)
	{
		constexpr float c4{ (2.0f * std::numbers::pi_v<float>) / 3.0f };
		constexpr float k_LowTolerance{ 0.0001f };
		constexpr float k_HighTolerance{ 0.9999f };

		return progress < k_LowTolerance
			? 0.0f
			: progress > k_HighTolerance
			? 1.0f
			: std::powf(2.0f, -10.0f * progress) * std::sinf((progress * 10.0f - 0.75f) * c4) + 1.0f;
	}

	float Interpolation::EaseOutElastic(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutElasticCalc(progress));
	}

	Math::vec2 Interpolation::EaseOutElastic(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutElasticCalc(progress));
	}

	Math::vec3 Interpolation::EaseOutElastic(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutElasticCalc(progress));
	}

	Math::vec4 Interpolation::EaseOutElastic(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutElasticCalc(progress));
	}

	static float EaseInOutElasticCalc(float progress)
	{
		constexpr float c5{ (2.0f * std::numbers::pi_v<float>) / 4.5f };
		constexpr float k_LowTolerance{ 0.0001f };
		constexpr float k_HighTolerance{ 0.9999f };

		return progress < k_LowTolerance
			? 0.0f
			: progress > k_HighTolerance
			? 1.0f
			: progress < 0.5f
			? -(std::powf(2.0f, 20.0f * progress - 10.0f) * std::sinf((20.0f * progress - 11.125f) * c5)) / 2.0f
			: (std::powf(2.0f, -20.0f * progress + 10.0f) * std::sinf((20.0f * progress - 11.125f) * c5)) / 2.0f + 1.0f;
	}

	float Interpolation::EaseInOutElastic(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutElasticCalc(progress));
	}

	Math::vec2 Interpolation::EaseInOutElastic(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutElasticCalc(progress));
	}

	Math::vec3 Interpolation::EaseInOutElastic(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutElasticCalc(progress));
	}

	Math::vec4 Interpolation::EaseInOutElastic(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutElasticCalc(progress));
	}

	static float EaseOutBounceCalc(float progress)
	{
		constexpr float n1{ 7.5625f };
		constexpr float d1{ 2.75f };

		if (progress < 1.0f / d1) 
		{
			return n1 * progress * progress;
		}
		else if (progress < 2.0f / d1) 
		{
			return n1 * (progress -= 1.5f / d1) * progress + 0.75f;
		}
		else if (progress < 2.5f / d1) 
		{
			return n1 * (progress -= 2.25f / d1) * progress + 0.9375f;
		}
		else 
		{
			return n1 * (progress -= 2.625f / d1) * progress + 0.984375f;
		}
	}

	static float EaseInBounceCalc(float progress)
	{
		return 1.0f - EaseOutBounceCalc(1.0f - progress);
	}

	float Interpolation::EaseInBounce(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInBounceCalc(progress));
	}

	Math::vec2 Interpolation::EaseInBounce(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInBounceCalc(progress));
	}

	Math::vec3 Interpolation::EaseInBounce(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInBounceCalc(progress));
	}

	Math::vec4 Interpolation::EaseInBounce(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInBounceCalc(progress));
	}

	float Interpolation::EaseOutBounce(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutBounceCalc(progress));
	}

	Math::vec2 Interpolation::EaseOutBounce(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutBounceCalc(progress));
	}

	Math::vec3 Interpolation::EaseOutBounce(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutBounceCalc(progress));
	}

	Math::vec4 Interpolation::EaseOutBounce(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseOutBounceCalc(progress));
	}

	static float EaseInOutBounceCalc(float progress)
	{
		return progress < 0.5f
			? (1.0f - EaseOutBounceCalc(1.0f - 2.0f * progress)) / 2.0f
			: (1.0f + EaseOutBounceCalc(2.0f * progress - 1.0f)) / 2.0f;
	}

	float Interpolation::EaseInOutBounce(float startPoint, float endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutBounceCalc(progress));
	}

	Math::vec2 Interpolation::EaseInOutBounce(Math::vec2 startPoint, Math::vec2 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutBounceCalc(progress));
	}

	Math::vec3 Interpolation::EaseInOutBounce(Math::vec3 startPoint, Math::vec3 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutBounceCalc(progress));
	}

	Math::vec4 Interpolation::EaseInOutBounce(Math::vec4 startPoint, Math::vec4 endPoint, float progress)
	{
		return Linear(startPoint, endPoint, EaseInOutBounceCalc(progress));
	}
	float Interpolation::CallEasingFunctionByType(float startPoint, float endPoint, float progress, Math::InterpolationType type)
	{
		switch (type)
		{
		case Math::InterpolationType::None: return 0.0f;
		case Math::InterpolationType::Linear: return Linear(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInSin: return EaseInSin(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutSin: return EaseOutSin(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutSin: return EaseInOutSin(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInQuad: return EaseInQuad(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutQuad: return EaseOutQuad(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutQuad: return EaseInOutQuad(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInCubic: return EaseInCubic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutCubic: return EaseOutCubic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutCubic: return EaseInOutCubic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInQuart: return EaseInQuart(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutQuart: return EaseOutQuart(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutQuart: return EaseInOutQuart(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInQuint: return EaseInQuint(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutQuint: return EaseOutQuint(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutQuint: return EaseInOutQuint(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInExpo: return EaseInExpo(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutExpo: return EaseOutExpo(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutExpo: return EaseInOutExpo(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInCirc: return EaseInCirc(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutCirc: return EaseOutCirc(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutCirc: return EaseInOutCirc(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInBack: return EaseInBack(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutBack: return EaseOutBack(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutBack: return EaseInOutBack(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInElastic: return EaseInElastic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutElastic: return EaseOutElastic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutElastic: return EaseInOutElastic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInBounce: return EaseInBounce(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutBounce: return EaseOutBounce(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutBounce: return EaseInOutBounce(startPoint, endPoint, progress);
		default:
			KG_WARN("Could not find interpolation type when attempting to make an interpolation function call");
			return 0.0f;
		}
	}
	Math::vec2 Interpolation::CallEasingFunctionByType(Math::vec2 startPoint, Math::vec2 endPoint, float progress, Math::InterpolationType type)
	{
		switch (type)
		{
		case Math::InterpolationType::None: return Math::vec2(0.0f);
		case Math::InterpolationType::Linear: return Linear(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInSin: return EaseInSin(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutSin: return EaseOutSin(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutSin: return EaseInOutSin(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInQuad: return EaseInQuad(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutQuad: return EaseOutQuad(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutQuad: return EaseInOutQuad(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInCubic: return EaseInCubic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutCubic: return EaseOutCubic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutCubic: return EaseInOutCubic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInQuart: return EaseInQuart(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutQuart: return EaseOutQuart(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutQuart: return EaseInOutQuart(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInQuint: return EaseInQuint(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutQuint: return EaseOutQuint(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutQuint: return EaseInOutQuint(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInExpo: return EaseInExpo(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutExpo: return EaseOutExpo(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutExpo: return EaseInOutExpo(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInCirc: return EaseInCirc(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutCirc: return EaseOutCirc(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutCirc: return EaseInOutCirc(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInBack: return EaseInBack(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutBack: return EaseOutBack(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutBack: return EaseInOutBack(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInElastic: return EaseInElastic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutElastic: return EaseOutElastic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutElastic: return EaseInOutElastic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInBounce: return EaseInBounce(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutBounce: return EaseOutBounce(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutBounce: return EaseInOutBounce(startPoint, endPoint, progress);
		default:
			KG_WARN("Could not find interpolation type when attempting to make an interpolation function call");
			return Math::vec2(0.0f);
		}
	}
	Math::vec3 Interpolation::CallEasingFunctionByType(Math::vec3 startPoint, Math::vec3 endPoint, float progress, Math::InterpolationType type)
	{
		switch (type)
		{
		case Math::InterpolationType::None: return Math::vec3(0.0f);
		case Math::InterpolationType::Linear: return Linear(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInSin: return EaseInSin(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutSin: return EaseOutSin(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutSin: return EaseInOutSin(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInQuad: return EaseInQuad(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutQuad: return EaseOutQuad(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutQuad: return EaseInOutQuad(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInCubic: return EaseInCubic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutCubic: return EaseOutCubic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutCubic: return EaseInOutCubic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInQuart: return EaseInQuart(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutQuart: return EaseOutQuart(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutQuart: return EaseInOutQuart(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInQuint: return EaseInQuint(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutQuint: return EaseOutQuint(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutQuint: return EaseInOutQuint(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInExpo: return EaseInExpo(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutExpo: return EaseOutExpo(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutExpo: return EaseInOutExpo(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInCirc: return EaseInCirc(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutCirc: return EaseOutCirc(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutCirc: return EaseInOutCirc(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInBack: return EaseInBack(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutBack: return EaseOutBack(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutBack: return EaseInOutBack(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInElastic: return EaseInElastic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutElastic: return EaseOutElastic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutElastic: return EaseInOutElastic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInBounce: return EaseInBounce(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutBounce: return EaseOutBounce(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutBounce: return EaseInOutBounce(startPoint, endPoint, progress);
		default:
			KG_WARN("Could not find interpolation type when attempting to make an interpolation function call");
			return Math::vec3(0.0f);
		}
	}
	Math::vec4 Interpolation::CallEasingFunctionByType(Math::vec4 startPoint, Math::vec4 endPoint, float progress, Math::InterpolationType type)
	{
		switch (type)
		{
		case Math::InterpolationType::None: return Math::vec4(0.0f);
		case Math::InterpolationType::Linear: return Linear(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInSin: return EaseInSin(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutSin: return EaseOutSin(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutSin: return EaseInOutSin(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInQuad: return EaseInQuad(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutQuad: return EaseOutQuad(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutQuad: return EaseInOutQuad(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInCubic: return EaseInCubic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutCubic: return EaseOutCubic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutCubic: return EaseInOutCubic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInQuart: return EaseInQuart(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutQuart: return EaseOutQuart(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutQuart: return EaseInOutQuart(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInQuint: return EaseInQuint(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutQuint: return EaseOutQuint(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutQuint: return EaseInOutQuint(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInExpo: return EaseInExpo(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutExpo: return EaseOutExpo(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutExpo: return EaseInOutExpo(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInCirc: return EaseInCirc(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutCirc: return EaseOutCirc(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutCirc: return EaseInOutCirc(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInBack: return EaseInBack(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutBack: return EaseOutBack(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutBack: return EaseInOutBack(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInElastic: return EaseInElastic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutElastic: return EaseOutElastic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutElastic: return EaseInOutElastic(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInBounce: return EaseInBounce(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseOutBounce: return EaseOutBounce(startPoint, endPoint, progress);
		case Math::InterpolationType::EaseInOutBounce: return EaseInOutBounce(startPoint, endPoint, progress);
		default:
			KG_WARN("Could not find interpolation type when attempting to make an interpolation function call");
			return Math::vec4(0.0f);
		}
	}
	EaseFloatFunction Interpolation::GetEasingFunctionFloat(Math::InterpolationType type)
	{
		switch (type)
		{
		case Math::InterpolationType::None: return nullptr;
		case Math::InterpolationType::Linear: return Linear;
		case Math::InterpolationType::EaseInSin: return EaseInSin;
		case Math::InterpolationType::EaseOutSin: return EaseOutSin;
		case Math::InterpolationType::EaseInOutSin: return EaseInOutSin;
		case Math::InterpolationType::EaseInQuad: return EaseInQuad;
		case Math::InterpolationType::EaseOutQuad: return EaseOutQuad;
		case Math::InterpolationType::EaseInOutQuad: return EaseInOutQuad;
		case Math::InterpolationType::EaseInCubic: return EaseInCubic;
		case Math::InterpolationType::EaseOutCubic: return EaseOutCubic;
		case Math::InterpolationType::EaseInOutCubic: return EaseInOutCubic;
		case Math::InterpolationType::EaseInQuart: return EaseInQuart;
		case Math::InterpolationType::EaseOutQuart: return EaseOutQuart;
		case Math::InterpolationType::EaseInOutQuart: return EaseInOutQuart;
		case Math::InterpolationType::EaseInQuint: return EaseInQuint;
		case Math::InterpolationType::EaseOutQuint: return EaseOutQuint;
		case Math::InterpolationType::EaseInOutQuint: return EaseInOutQuint;
		case Math::InterpolationType::EaseInExpo: return EaseInExpo;
		case Math::InterpolationType::EaseOutExpo: return EaseOutExpo;
		case Math::InterpolationType::EaseInOutExpo: return EaseInOutExpo;
		case Math::InterpolationType::EaseInCirc: return EaseInCirc;
		case Math::InterpolationType::EaseOutCirc: return EaseOutCirc;
		case Math::InterpolationType::EaseInOutCirc: return EaseInOutCirc;
		case Math::InterpolationType::EaseInBack: return EaseInBack;
		case Math::InterpolationType::EaseOutBack: return EaseOutBack;
		case Math::InterpolationType::EaseInOutBack: return EaseInOutBack;
		case Math::InterpolationType::EaseInElastic: return EaseInElastic;
		case Math::InterpolationType::EaseOutElastic: return EaseOutElastic;
		case Math::InterpolationType::EaseInOutElastic: return EaseInOutElastic;
		case Math::InterpolationType::EaseInBounce: return EaseInBounce;
		case Math::InterpolationType::EaseOutBounce: return EaseOutBounce;
		case Math::InterpolationType::EaseInOutBounce: return EaseInOutBounce;
		default:
			KG_WARN("Could not find interpolation type when attempting to get an easing function");
			return nullptr;
		}
	}
	EaseVec2Function Interpolation::GetEasingFunctionVec2(Math::InterpolationType type)
	{
		switch (type)
		{
		case Math::InterpolationType::None: return nullptr;
		case Math::InterpolationType::Linear: return Linear;
		case Math::InterpolationType::EaseInSin: return EaseInSin;
		case Math::InterpolationType::EaseOutSin: return EaseOutSin;
		case Math::InterpolationType::EaseInOutSin: return EaseInOutSin;
		case Math::InterpolationType::EaseInQuad: return EaseInQuad;
		case Math::InterpolationType::EaseOutQuad: return EaseOutQuad;
		case Math::InterpolationType::EaseInOutQuad: return EaseInOutQuad;
		case Math::InterpolationType::EaseInCubic: return EaseInCubic;
		case Math::InterpolationType::EaseOutCubic: return EaseOutCubic;
		case Math::InterpolationType::EaseInOutCubic: return EaseInOutCubic;
		case Math::InterpolationType::EaseInQuart: return EaseInQuart;
		case Math::InterpolationType::EaseOutQuart: return EaseOutQuart;
		case Math::InterpolationType::EaseInOutQuart: return EaseInOutQuart;
		case Math::InterpolationType::EaseInQuint: return EaseInQuint;
		case Math::InterpolationType::EaseOutQuint: return EaseOutQuint;
		case Math::InterpolationType::EaseInOutQuint: return EaseInOutQuint;
		case Math::InterpolationType::EaseInExpo: return EaseInExpo;
		case Math::InterpolationType::EaseOutExpo: return EaseOutExpo;
		case Math::InterpolationType::EaseInOutExpo: return EaseInOutExpo;
		case Math::InterpolationType::EaseInCirc: return EaseInCirc;
		case Math::InterpolationType::EaseOutCirc: return EaseOutCirc;
		case Math::InterpolationType::EaseInOutCirc: return EaseInOutCirc;
		case Math::InterpolationType::EaseInBack: return EaseInBack;
		case Math::InterpolationType::EaseOutBack: return EaseOutBack;
		case Math::InterpolationType::EaseInOutBack: return EaseInOutBack;
		case Math::InterpolationType::EaseInElastic: return EaseInElastic;
		case Math::InterpolationType::EaseOutElastic: return EaseOutElastic;
		case Math::InterpolationType::EaseInOutElastic: return EaseInOutElastic;
		case Math::InterpolationType::EaseInBounce: return EaseInBounce;
		case Math::InterpolationType::EaseOutBounce: return EaseOutBounce;
		case Math::InterpolationType::EaseInOutBounce: return EaseInOutBounce;
		default:
			KG_WARN("Could not find interpolation type when attempting to get an easing function");
			return nullptr;
		}
	}
	EaseVec3Function Interpolation::GetEasingFunctionVec3(Math::InterpolationType type)
	{
		switch (type)
		{
		case Math::InterpolationType::None: return nullptr;
		case Math::InterpolationType::Linear: return Linear;
		case Math::InterpolationType::EaseInSin: return EaseInSin;
		case Math::InterpolationType::EaseOutSin: return EaseOutSin;
		case Math::InterpolationType::EaseInOutSin: return EaseInOutSin;
		case Math::InterpolationType::EaseInQuad: return EaseInQuad;
		case Math::InterpolationType::EaseOutQuad: return EaseOutQuad;
		case Math::InterpolationType::EaseInOutQuad: return EaseInOutQuad;
		case Math::InterpolationType::EaseInCubic: return EaseInCubic;
		case Math::InterpolationType::EaseOutCubic: return EaseOutCubic;
		case Math::InterpolationType::EaseInOutCubic: return EaseInOutCubic;
		case Math::InterpolationType::EaseInQuart: return EaseInQuart;
		case Math::InterpolationType::EaseOutQuart: return EaseOutQuart;
		case Math::InterpolationType::EaseInOutQuart: return EaseInOutQuart;
		case Math::InterpolationType::EaseInQuint: return EaseInQuint;
		case Math::InterpolationType::EaseOutQuint: return EaseOutQuint;
		case Math::InterpolationType::EaseInOutQuint: return EaseInOutQuint;
		case Math::InterpolationType::EaseInExpo: return EaseInExpo;
		case Math::InterpolationType::EaseOutExpo: return EaseOutExpo;
		case Math::InterpolationType::EaseInOutExpo: return EaseInOutExpo;
		case Math::InterpolationType::EaseInCirc: return EaseInCirc;
		case Math::InterpolationType::EaseOutCirc: return EaseOutCirc;
		case Math::InterpolationType::EaseInOutCirc: return EaseInOutCirc;
		case Math::InterpolationType::EaseInBack: return EaseInBack;
		case Math::InterpolationType::EaseOutBack: return EaseOutBack;
		case Math::InterpolationType::EaseInOutBack: return EaseInOutBack;
		case Math::InterpolationType::EaseInElastic: return EaseInElastic;
		case Math::InterpolationType::EaseOutElastic: return EaseOutElastic;
		case Math::InterpolationType::EaseInOutElastic: return EaseInOutElastic;
		case Math::InterpolationType::EaseInBounce: return EaseInBounce;
		case Math::InterpolationType::EaseOutBounce: return EaseOutBounce;
		case Math::InterpolationType::EaseInOutBounce: return EaseInOutBounce;
		default:
			KG_WARN("Could not find interpolation type when attempting to get an easing function");
			return nullptr;
		}
	}
	EaseVec4Function Interpolation::GetEasingFunctionVec4(Math::InterpolationType type)
	{
		switch (type)
		{
		case Math::InterpolationType::None: return nullptr;
		case Math::InterpolationType::Linear: return Linear;
		case Math::InterpolationType::EaseInSin: return EaseInSin;
		case Math::InterpolationType::EaseOutSin: return EaseOutSin;
		case Math::InterpolationType::EaseInOutSin: return EaseInOutSin;
		case Math::InterpolationType::EaseInQuad: return EaseInQuad;
		case Math::InterpolationType::EaseOutQuad: return EaseOutQuad;
		case Math::InterpolationType::EaseInOutQuad: return EaseInOutQuad;
		case Math::InterpolationType::EaseInCubic: return EaseInCubic;
		case Math::InterpolationType::EaseOutCubic: return EaseOutCubic;
		case Math::InterpolationType::EaseInOutCubic: return EaseInOutCubic;
		case Math::InterpolationType::EaseInQuart: return EaseInQuart;
		case Math::InterpolationType::EaseOutQuart: return EaseOutQuart;
		case Math::InterpolationType::EaseInOutQuart: return EaseInOutQuart;
		case Math::InterpolationType::EaseInQuint: return EaseInQuint;
		case Math::InterpolationType::EaseOutQuint: return EaseOutQuint;
		case Math::InterpolationType::EaseInOutQuint: return EaseInOutQuint;
		case Math::InterpolationType::EaseInExpo: return EaseInExpo;
		case Math::InterpolationType::EaseOutExpo: return EaseOutExpo;
		case Math::InterpolationType::EaseInOutExpo: return EaseInOutExpo;
		case Math::InterpolationType::EaseInCirc: return EaseInCirc;
		case Math::InterpolationType::EaseOutCirc: return EaseOutCirc;
		case Math::InterpolationType::EaseInOutCirc: return EaseInOutCirc;
		case Math::InterpolationType::EaseInBack: return EaseInBack;
		case Math::InterpolationType::EaseOutBack: return EaseOutBack;
		case Math::InterpolationType::EaseInOutBack: return EaseInOutBack;
		case Math::InterpolationType::EaseInElastic: return EaseInElastic;
		case Math::InterpolationType::EaseOutElastic: return EaseOutElastic;
		case Math::InterpolationType::EaseInOutElastic: return EaseInOutElastic;
		case Math::InterpolationType::EaseInBounce: return EaseInBounce;
		case Math::InterpolationType::EaseOutBounce: return EaseOutBounce;
		case Math::InterpolationType::EaseInOutBounce: return EaseInOutBounce;
		default:
			KG_WARN("Could not find interpolation type when attempting to get an easing function");
			return nullptr;
		}
	}
}

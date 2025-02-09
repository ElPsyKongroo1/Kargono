#pragma once
#include "Kargono/Math/MathAliases.h"

#include <string>

namespace Kargono::Math
{
	enum class InterpolationType
	{
		None = 0,
		Linear,
		EaseInSin, EaseOutSin, EaseInOutSin,
		EaseInQuad, EaseOutQuad, EaseInOutQuad,
		EaseInCubic, EaseOutCubic, EaseInOutCubic,
		EaseInQuart, EaseOutQuart, EaseInOutQuart,
		EaseInQuint, EaseOutQuint, EaseInOutQuint,
		EaseInExpo, EaseOutExpo, EaseInOutExpo,
		EaseInCirc, EaseOutCirc, EaseInOutCirc,
		EaseInBack, EaseOutBack, EaseInOutBack,
		EaseInElastic, EaseOutElastic, EaseInOutElastic,
		EaseInBounce, EaseOutBounce, EaseInOutBounce,
	};

	inline std::array<InterpolationType, 32> s_AllInterpolationTypes
	{
		InterpolationType::None,
		InterpolationType::Linear,
		InterpolationType::EaseInSin, InterpolationType::EaseOutSin, InterpolationType::EaseInOutSin,
		InterpolationType::EaseInQuad, InterpolationType::EaseOutQuad, InterpolationType::EaseInOutQuad,
		InterpolationType::EaseInCubic, InterpolationType::EaseOutCubic, InterpolationType::EaseInOutCubic,
		InterpolationType::EaseInQuart, InterpolationType::EaseOutQuart, InterpolationType::EaseInOutQuart,
		InterpolationType::EaseInQuint, InterpolationType::EaseOutQuint, InterpolationType::EaseInOutQuint,
		InterpolationType::EaseInExpo, InterpolationType::EaseOutExpo, InterpolationType::EaseInOutExpo,
		InterpolationType::EaseInCirc, InterpolationType::EaseOutCirc, InterpolationType::EaseInOutCirc,
		InterpolationType::EaseInBack, InterpolationType::EaseOutBack, InterpolationType::EaseInOutBack,
		InterpolationType::EaseInElastic, InterpolationType::EaseOutElastic, InterpolationType::EaseInOutElastic,
		InterpolationType::EaseInBounce, InterpolationType::EaseOutBounce, InterpolationType::EaseInOutBounce
	};

	//==============================
	// Function Ptr Definitions for Interpolations
	//==============================
	typedef float(*EaseFloatFunction)(float, float, float);
	typedef Math::vec2(*EaseVec2Function)(Math::vec2, Math::vec2, float);
	typedef Math::vec3(*EaseVec3Function)(Math::vec3, Math::vec3, float);
	typedef Math::vec4(*EaseVec4Function)(Math::vec4, Math::vec4, float);

	class Interpolation
	{
	public:

		// Note: The easing calculations where mostly found here: https://easings.net/

		//==============================
		// Linear Interpolations
		//==============================
		static float Linear(float startPoint, float endPoint, float progress);
		static Math::vec2 Linear(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 Linear(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 Linear(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease In Sine Interpolations (https://easings.net/#easeInSine)
		//==============================
		static float EaseInSin(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseInSin(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseInSin(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseInSin(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease Out Sine Interpolations (https://easings.net/#easeOutSine)
		//==============================
		static float EaseOutSin(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseOutSin(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseOutSin(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseOutSin(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease In Out Sine Interpolations (https://easings.net/#easeInOutSine)
		//==============================
		static float EaseInOutSin(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseInOutSin(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseInOutSin(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseInOutSin(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease In Quad Interpolations (https://easings.net/#easeInQuad)
		//==============================
		static float EaseInQuad(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseInQuad(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseInQuad(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseInQuad(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease Out Quad Interpolations (https://easings.net/#easeOutQuad)
		//==============================
		static float EaseOutQuad(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseOutQuad(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseOutQuad(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseOutQuad(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease In Out Quad Interpolations (https://easings.net/#easeInOutQuad)
		//==============================
		static float EaseInOutQuad(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseInOutQuad(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseInOutQuad(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseInOutQuad(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease In Cubic Interpolations (https://easings.net/#easeInCubic)
		//==============================
		static float EaseInCubic(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseInCubic(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseInCubic(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseInCubic(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease Out Cubic Interpolations (https://easings.net/#easeOutCubic)
		//==============================
		static float EaseOutCubic(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseOutCubic(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseOutCubic(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseOutCubic(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease In Out Cubic Interpolations (https://easings.net/#easeInOutCubic)
		//==============================
		static float EaseInOutCubic(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseInOutCubic(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseInOutCubic(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseInOutCubic(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease In Quart Interpolations (https://easings.net/#easeInQuart)
		//==============================
		static float EaseInQuart(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseInQuart(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseInQuart(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseInQuart(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease Out Quart Interpolations (https://easings.net/#easeOutQuart)
		//==============================
		static float EaseOutQuart(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseOutQuart(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseOutQuart(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseOutQuart(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease In Out Quart Interpolations (https://easings.net/#easeInOutQuart)
		//==============================
		static float EaseInOutQuart(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseInOutQuart(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseInOutQuart(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseInOutQuart(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease In Quint Interpolations (https://easings.net/#easeInQuint)
		//==============================
		static float EaseInQuint(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseInQuint(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseInQuint(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseInQuint(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease Out Quint Interpolations (https://easings.net/#easeOutQuint)
		//==============================
		static float EaseOutQuint(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseOutQuint(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseOutQuint(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseOutQuint(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease In Out Quint Interpolations (https://easings.net/#easeInOutQuint)
		//==============================
		static float EaseInOutQuint(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseInOutQuint(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseInOutQuint(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseInOutQuint(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease In Expo Interpolations (https://easings.net/#easeInExpo)
		//==============================
		static float EaseInExpo(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseInExpo(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseInExpo(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseInExpo(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease Out Expo Interpolations (https://easings.net/#easeOutExpo)
		//==============================
		static float EaseOutExpo(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseOutExpo(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseOutExpo(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseOutExpo(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease In Out Expo Interpolations (https://easings.net/#easeInOutExpo)
		//==============================
		static float EaseInOutExpo(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseInOutExpo(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseInOutExpo(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseInOutExpo(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease In Circ Interpolations (https://easings.net/#easeInCirc)
		//==============================
		static float EaseInCirc(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseInCirc(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseInCirc(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseInCirc(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease Out Circ Interpolations (https://easings.net/#easeOutCirc)
		//==============================
		static float EaseOutCirc(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseOutCirc(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseOutCirc(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseOutCirc(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease In Out Circ Interpolations (https://easings.net/#easeInOutCirc)
		//==============================
		static float EaseInOutCirc(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseInOutCirc(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseInOutCirc(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseInOutCirc(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease In Back Interpolations (https://easings.net/#easeInBack)
		//==============================
		static float EaseInBack(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseInBack(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseInBack(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseInBack(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease Out Back Interpolations (https://easings.net/#easeOutBack)
		//==============================
		static float EaseOutBack(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseOutBack(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseOutBack(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseOutBack(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease In Out Back Interpolations (https://easings.net/#easeInOutBack)
		//==============================
		static float EaseInOutBack(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseInOutBack(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseInOutBack(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseInOutBack(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease In Elastic Interpolations (https://easings.net/#easeInElastic)
		//==============================
		static float EaseInElastic(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseInElastic(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseInElastic(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseInElastic(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease Out Elastic Interpolations (https://easings.net/#easeOutElastic)
		//==============================
		static float EaseOutElastic(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseOutElastic(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseOutElastic(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseOutElastic(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease In Out Elastic Interpolations (https://easings.net/#easeInOutElastic)
		//==============================
		static float EaseInOutElastic(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseInOutElastic(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseInOutElastic(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseInOutElastic(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease In Bounce Interpolations (https://easings.net/#easeInBounce)
		//==============================
		static float EaseInBounce(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseInBounce(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseInBounce(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseInBounce(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease Out Bounce Interpolations (https://easings.net/#easeOutBounce)
		//==============================
		static float EaseOutBounce(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseOutBounce(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseOutBounce(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseOutBounce(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Ease In Out Bounce Interpolations (https://easings.net/#easeInOutBounce)
		//==============================
		static float EaseInOutBounce(float startPoint, float endPoint, float progress);
		static Math::vec2 EaseInOutBounce(Math::vec2 startPoint, Math::vec2 endPoint, float progress);
		static Math::vec3 EaseInOutBounce(Math::vec3 startPoint, Math::vec3 endPoint, float progress);
		static Math::vec4 EaseInOutBounce(Math::vec4 startPoint, Math::vec4 endPoint, float progress);

		//==============================
		// Call Based On Interpolation Type
		//==============================
		static float CallEasingFunctionByType (float startPoint, float endPoint, 
												float progress, Math::InterpolationType type);
		static Math::vec2 CallEasingFunctionByType(Math::vec2 startPoint, Math::vec2 endPoint, 
													float progress,  Math::InterpolationType type);
		static Math::vec3 CallEasingFunctionByType(Math::vec3 startPoint, Math::vec3 endPoint,
													float progress, Math::InterpolationType type);
		static Math::vec4 CallEasingFunctionByType(Math::vec4 startPoint, Math::vec4 endPoint,
													float progress, Math::InterpolationType type);

		//==============================
		// Get Interpolation Function Based on Type
		//==============================
		static EaseFloatFunction GetEasingFunctionFloat(Math::InterpolationType type);
		static EaseVec2Function GetEasingFunctionVec2(Math::InterpolationType type);
		static EaseVec3Function GetEasingFunctionVec3(Math::InterpolationType type);
		static EaseVec4Function GetEasingFunctionVec4(Math::InterpolationType type);
	};
}


namespace Kargono::Utility
{
	inline const char* InterpolationTypeToString(Math::InterpolationType type)
	{
		switch (type)
		{
		case Math::InterpolationType::None: return "None";
		case Math::InterpolationType::Linear: return "Linear";
		case Math::InterpolationType::EaseInSin: return "EaseInSin";
		case Math::InterpolationType::EaseOutSin: return "EaseOutSin";
		case Math::InterpolationType::EaseInOutSin: return "EaseInOutSin";
		case Math::InterpolationType::EaseInQuad: return "EaseInQuad";
		case Math::InterpolationType::EaseOutQuad: return "EaseOutQuad";
		case Math::InterpolationType::EaseInOutQuad: return "EaseInOutQuad";
		case Math::InterpolationType::EaseInCubic: return "EaseInCubic";
		case Math::InterpolationType::EaseOutCubic: return "EaseOutCubic";
		case Math::InterpolationType::EaseInOutCubic: return "EaseInOutCubic";
		case Math::InterpolationType::EaseInQuart: return "EaseInQuart";
		case Math::InterpolationType::EaseOutQuart: return "EaseOutQuart";
		case Math::InterpolationType::EaseInOutQuart: return "EaseInOutQuart";
		case Math::InterpolationType::EaseInQuint: return "EaseInQuint";
		case Math::InterpolationType::EaseOutQuint: return "EaseOutQuint";
		case Math::InterpolationType::EaseInOutQuint: return "EaseInOutQuint";
		case Math::InterpolationType::EaseInExpo: return "EaseInExpo";
		case Math::InterpolationType::EaseOutExpo: return "EaseOutExpo";
		case Math::InterpolationType::EaseInOutExpo: return "EaseInOutExpo";
		case Math::InterpolationType::EaseInCirc: return "EaseInCirc";
		case Math::InterpolationType::EaseOutCirc: return "EaseOutCirc";
		case Math::InterpolationType::EaseInOutCirc: return "EaseInOutCirc";
		case Math::InterpolationType::EaseInBack: return "EaseInBack";
		case Math::InterpolationType::EaseOutBack: return "EaseOutBack";
		case Math::InterpolationType::EaseInOutBack: return "EaseInOutBack";
		case Math::InterpolationType::EaseInElastic: return "EaseInElastic";
		case Math::InterpolationType::EaseOutElastic: return "EaseOutElastic";
		case Math::InterpolationType::EaseInOutElastic: return "EaseInOutElastic";
		case Math::InterpolationType::EaseInBounce: return "EaseInBounce";
		case Math::InterpolationType::EaseOutBounce: return "EaseOutBounce";
		case Math::InterpolationType::EaseInOutBounce: return "EaseInOutBounce";
		default:
			return "None";
		}
	}

	inline Math::InterpolationType StringToInterpolationType(std::string_view str)
	{
		if (str == "None") return Math::InterpolationType::None;
		else if (str == "Linear") return Math::InterpolationType::Linear;
		else if (str == "EaseInSin") return Math::InterpolationType::EaseInSin;
		else if (str == "EaseOutSin") return Math::InterpolationType::EaseOutSin;
		else if (str == "EaseInOutSin") return Math::InterpolationType::EaseInOutSin;
		else if (str == "EaseInQuad") return Math::InterpolationType::EaseInQuad;
		else if (str == "EaseOutQuad") return Math::InterpolationType::EaseOutQuad;
		else if (str == "EaseInOutQuad") return Math::InterpolationType::EaseInOutQuad;
		else if (str == "EaseInCubic") return Math::InterpolationType::EaseInCubic;
		else if (str == "EaseOutCubic") return Math::InterpolationType::EaseOutCubic;
		else if (str == "EaseInOutCubic") return Math::InterpolationType::EaseInOutCubic;
		else if (str == "EaseInQuart") return Math::InterpolationType::EaseInQuart;
		else if (str == "EaseOutQuart") return Math::InterpolationType::EaseOutQuart;
		else if (str == "EaseInOutQuart") return Math::InterpolationType::EaseInOutQuart;
		else if (str == "EaseInQuint") return Math::InterpolationType::EaseInQuint;
		else if (str == "EaseOutQuint") return Math::InterpolationType::EaseOutQuint;
		else if (str == "EaseInOutQuint") return Math::InterpolationType::EaseInOutQuint;
		else if (str == "EaseInExpo") return Math::InterpolationType::EaseInExpo;
		else if (str == "EaseOutExpo") return Math::InterpolationType::EaseOutExpo;
		else if (str == "EaseInOutExpo") return Math::InterpolationType::EaseInOutExpo;
		else if (str == "EaseInCirc") return Math::InterpolationType::EaseInCirc;
		else if (str == "EaseOutCirc") return Math::InterpolationType::EaseOutCirc;
		else if (str == "EaseInOutCirc") return Math::InterpolationType::EaseInOutCirc;
		else if (str == "EaseInBack") return Math::InterpolationType::EaseInBack;
		else if (str == "EaseOutBack") return Math::InterpolationType::EaseOutBack;
		else if (str == "EaseInOutBack") return Math::InterpolationType::EaseInOutBack;
		else if (str == "EaseInElastic") return Math::InterpolationType::EaseInElastic;
		else if (str == "EaseOutElastic") return Math::InterpolationType::EaseOutElastic;
		else if (str == "EaseInOutElastic") return Math::InterpolationType::EaseInOutElastic;
		else if (str == "EaseInBounce") return Math::InterpolationType::EaseInBounce;
		else if (str == "EaseOutBounce") return Math::InterpolationType::EaseOutBounce;
		else if (str == "EaseInOutBounce") return Math::InterpolationType::EaseInOutBounce;
		else
		{
			return Math::InterpolationType::None;
		}
	}

	inline const char* InterpolationTypeToCategory(Math::InterpolationType type)
	{
		switch (type)
		{
		case Math::InterpolationType::None: 
			return "Clear";
		case Math::InterpolationType::Linear: 
			return "Linear";
		case Math::InterpolationType::EaseInSin:
		case Math::InterpolationType::EaseOutSin:
		case Math::InterpolationType::EaseInOutSin:
			return "Sine";
		case Math::InterpolationType::EaseInQuad:
		case Math::InterpolationType::EaseOutQuad:
		case Math::InterpolationType::EaseInOutQuad:
			return "Quadratic";
		case Math::InterpolationType::EaseInCubic:
		case Math::InterpolationType::EaseOutCubic:
		case Math::InterpolationType::EaseInOutCubic:
			return "Cubic";
		case Math::InterpolationType::EaseInQuart:
		case Math::InterpolationType::EaseOutQuart:
		case Math::InterpolationType::EaseInOutQuart:
			return "Quartic";
		case Math::InterpolationType::EaseInQuint:
		case Math::InterpolationType::EaseOutQuint:
		case Math::InterpolationType::EaseInOutQuint:
			return "Quintic";
		case Math::InterpolationType::EaseInExpo:
		case Math::InterpolationType::EaseOutExpo:
		case Math::InterpolationType::EaseInOutExpo:
			return "Exponential";
		case Math::InterpolationType::EaseInCirc:
		case Math::InterpolationType::EaseOutCirc:
		case Math::InterpolationType::EaseInOutCirc:
			return "Circular";
		case Math::InterpolationType::EaseInBack:
		case Math::InterpolationType::EaseOutBack:
		case Math::InterpolationType::EaseInOutBack:
			return "Back";
		case Math::InterpolationType::EaseInElastic:
		case Math::InterpolationType::EaseOutElastic:
		case Math::InterpolationType::EaseInOutElastic:
			return "Elastic";
		case Math::InterpolationType::EaseInBounce:
		case Math::InterpolationType::EaseOutBounce:
		case Math::InterpolationType::EaseInOutBounce:
			return "Bounce";
		default:
			return "None";
		}
	}
}


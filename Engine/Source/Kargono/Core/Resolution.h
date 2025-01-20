#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Math/Math.h"

#include <array>
#include <string>

namespace Kargono
{
	//=========================
	// Screen Resolution Options Enum
	//=========================
	// This enum provides the different screen resolution options available
	//		at a project level. This enum is used to set the default and stored
	//		screen resolution for the project.
	enum class ScreenResolution
	{
		None = 0,
		R800x800, R400x400,									// 1x1

		R122880x69120, R61440x34560, R30720x17280,			// 16x9
		R15360x8640, R7680x4320, R5120x2880,				// 16x9
		R3840x2160, R3200x1800, R2560x1440, R2048x1152,		// 16x9
		R1920x1080, R1600x900, R1366x768, R1280x720,		// 16x9
		R1024x576, R960x540, R854x480, R640x360,			// 16x9

		R1600x1200, R1280x960, R1152x864, R1024x768,		// 4x3
		MatchDevice											// Automatic
	};

	inline constexpr std::array<ScreenResolution, 26> s_AllScreenResolutions
	{
		ScreenResolution::None,

		// 1x1
		ScreenResolution::R800x800,
		ScreenResolution::R400x400,

		//16x9
		ScreenResolution::R122880x69120,
		ScreenResolution::R61440x34560,
		ScreenResolution::R30720x17280,
		ScreenResolution::R15360x8640,
		ScreenResolution::R7680x4320,
		ScreenResolution::R5120x2880,
		ScreenResolution::R3840x2160,
		ScreenResolution::R3200x1800,
		ScreenResolution::R2560x1440,
		ScreenResolution::R2048x1152,
		ScreenResolution::R1920x1080,
		ScreenResolution::R1600x900,
		ScreenResolution::R1366x768,
		ScreenResolution::R1280x720,
		ScreenResolution::R1024x576,
		ScreenResolution::R960x540,
		ScreenResolution::R854x480,
		ScreenResolution::R640x360,

		// 4x3
		ScreenResolution::R1600x1200,
		ScreenResolution::R1280x960,
		ScreenResolution::R1152x864,
		ScreenResolution::R1024x768,

		// Other
		ScreenResolution::MatchDevice
	};
}

namespace Kargono::Utility
{

	//=========================
	// Conversion Functions
	//=========================

	// These functions help convert the screen resolution and aspect ratio enum into a string
	//		to serialization purposes.

	inline std::string ScreenResolutionToString(ScreenResolution option)
	{
		switch (option)
		{
		case ScreenResolution::R800x800: return "800x800";
		case ScreenResolution::R400x400: return "400x400";

			// 16:9
		case ScreenResolution::R122880x69120: return "122880x69120";
		case ScreenResolution::R61440x34560: return "61440x34560";
		case ScreenResolution::R30720x17280: return "30720x17280";
		case ScreenResolution::R15360x8640: return "15360x8640";
		case ScreenResolution::R7680x4320: return "7680x4320";
		case ScreenResolution::R5120x2880: return "5120x2880";
		case ScreenResolution::R3840x2160: return "3840x2160";
		case ScreenResolution::R3200x1800: return "3200x1800";
		case ScreenResolution::R2560x1440: return "2560x1440";
		case ScreenResolution::R2048x1152: return "2048x1152";
		case ScreenResolution::R1920x1080: return "1920x1080";
		case ScreenResolution::R1600x900: return "1600x900";
		case ScreenResolution::R1366x768: return "1366x768";
		case ScreenResolution::R1280x720: return "1280x720";
		case ScreenResolution::R1024x576: return "1024x576";
		case ScreenResolution::R960x540: return "960x540";
		case ScreenResolution::R854x480: return "854x480";
		case ScreenResolution::R640x360: return "640x360";

		case ScreenResolution::R1600x1200: return "1600x1200";
		case ScreenResolution::R1280x960: return "1280x960";
		case ScreenResolution::R1152x864: return "1152x864";
		case ScreenResolution::R1024x768: return "1024x768";

		case ScreenResolution::MatchDevice: return "Match Device";
		case ScreenResolution::None: return "None";
		}
		KG_ERROR("Invalid ScreenResolution enum provided to ScreenResolutionToString function");
		return "None";
	}

	inline std::string ScreenResolutionToCategoryTitle(ScreenResolution option)
	{
		switch (option)
		{
		case ScreenResolution::R800x800:
		case ScreenResolution::R400x400:
			return "Aspect Ratio: 1:1 (Box)";
			// 16:9
		case ScreenResolution::R122880x69120:
		case ScreenResolution::R61440x34560:
		case ScreenResolution::R30720x17280:
		case ScreenResolution::R15360x8640:
		case ScreenResolution::R7680x4320:
		case ScreenResolution::R5120x2880:
		case ScreenResolution::R3840x2160:
		case ScreenResolution::R3200x1800:
		case ScreenResolution::R2560x1440:
		case ScreenResolution::R2048x1152:
		case ScreenResolution::R1920x1080:
		case ScreenResolution::R1600x900:
		case ScreenResolution::R1366x768:
		case ScreenResolution::R1280x720:
		case ScreenResolution::R1024x576:
		case ScreenResolution::R960x540:
		case ScreenResolution::R854x480:
		case ScreenResolution::R640x360:
			return "Aspect Ratio: 16:9 (Widescreen)";
		case ScreenResolution::R1600x1200:
		case ScreenResolution::R1280x960:
		case ScreenResolution::R1152x864:
		case ScreenResolution::R1024x768:
			return "Aspect Ratio: 4:3 (Fullscreen)";
		case ScreenResolution::MatchDevice:
			return "Aspect Ratio: Automatic (Based on Device Used)";
		case ScreenResolution::None:
			return "Invalid Aspect Ratio";
		}
		KG_ERROR("Invalid ScreenResolution enum provided to ScreenResolutionToString function");
		return "None";
	}

	inline ScreenResolution StringToScreenResolution(const std::string& optionStr)
	{
		if (optionStr == "800x800") { return ScreenResolution::R800x800; }
		if (optionStr == "400x400") { return ScreenResolution::R400x400; }

		if (optionStr == "122880x69120") { return ScreenResolution::R122880x69120; }
		if (optionStr == "61440x34560") { return ScreenResolution::R61440x34560; }
		if (optionStr == "30720x17280") { return ScreenResolution::R30720x17280; }
		if (optionStr == "15360x8640") { return ScreenResolution::R15360x8640; }
		if (optionStr == "7680x4320") { return ScreenResolution::R7680x4320; }
		if (optionStr == "5120x2880") { return ScreenResolution::R5120x2880; }
		if (optionStr == "3840x2160") { return ScreenResolution::R3840x2160; }
		if (optionStr == "3200x1800") { return ScreenResolution::R3200x1800; }
		if (optionStr == "2560x1440") { return ScreenResolution::R2560x1440; }
		if (optionStr == "2048x1152") { return ScreenResolution::R2048x1152; }
		if (optionStr == "1920x1080") { return ScreenResolution::R1920x1080; }
		if (optionStr == "1600x900") { return ScreenResolution::R1600x900; }
		if (optionStr == "1366x768") { return ScreenResolution::R1366x768; }
		if (optionStr == "1280x720") { return ScreenResolution::R1280x720; }
		if (optionStr == "1024x576") { return ScreenResolution::R1024x576; }
		if (optionStr == "960x540") { return ScreenResolution::R960x540; }
		if (optionStr == "854x480") { return ScreenResolution::R854x480; }
		if (optionStr == "640x360") { return ScreenResolution::R640x360; }


		if (optionStr == "1600x1200") { return ScreenResolution::R1600x1200; }
		if (optionStr == "1280x960") { return ScreenResolution::R1280x960; }
		if (optionStr == "1152x864") { return ScreenResolution::R1152x864; }
		if (optionStr == "1024x768") { return ScreenResolution::R1024x768; }

		if (optionStr == "Match Device") { return ScreenResolution::MatchDevice; }
		if (optionStr == "None") { return ScreenResolution::None; }

		KG_ERROR("Invalid ScreenResolution enum provided to StringToScreenResolution function");
		return ScreenResolution::None;
	}



	Math::uvec2 ScreenResolutionToAspectRatio(ScreenResolution option);
	Math::vec2 ScreenResolutionToVec2(ScreenResolution option);
}

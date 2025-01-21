#pragma once

#include "Kargono/Math/MathAliases.h"

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

	//// 21:9 resolutions TODO: Add 21:9 for ultra wide. Does not divide into same value!
	//R2560×1080, R3440×1440, R3840×1600,
	//R5120×2160, R5760×2400, R7680×3200, R10240×4320,

	enum class ScreenResolution : uint16_t
	{
		None = 0,

		// 1x1 resolutions
		R256x256,
		R512x512, R1024x1024, R2048x2048, R3072x3072,
		R4096x4096, R5120x5120, R6144x6144, R8192x8192,
		R10240x10240, R12288x12288, R16384x16384, R32768x32768,

		// 16x9 resolutions
		R122880x69120, R61440x34560, R30720x17280,
		R15360x8640, R7680x4320, R5120x2880,
		R3840x2160, R3200x1800, R2560x1440, R2048x1152,
		R1920x1080, R1600x900, R1366x768, R1280x720,
		R1024x576, R960x540, R854x480, R640x360,

		// 9:16 resolutions
		R69120x122880, R34560x61440, R17280x30720,
		R8640x15360, R4320x7680, R2880x5120,
		R2160x3840, R1800x3200, R1440x2560, R1152x2048,
		R1080x1920, R900x1600, R768x1366, R720x1280,
		R576x1024, R540x960, R480x854, R360x640,
		
		// 32:9 resolutions
		R1280x360, R1920x540, R2560x720, R2880x810, R3200x900,
		R3840x1080, R5120x1440, R7680x2160, R10240x2880,
		
		// 4x3 resolutions
		R160x120, R256x192, R320x240, R640x480,
		R800x600, R960x720, R1024x768, R1152x864,
		R1280x960, R1400x1050, R1440x1080, R1600x1200,
		R1856x1392, R1920x1440, R2048x1536, R2560x1920,
		R2880x2160, R3072x2304, R3840x2880, R4096x3072,
		R5120x3840, R6144x4608, R7680x5760, R8192x6144,

		// 5x4 resolutions
		R1280x1024, R2560x2048, R5120x4096,

		// Automatic
		MatchDevice
	};


	inline constexpr std::array<ScreenResolution, 87> s_AllScreenResolutions
	{
		ScreenResolution::None,

		// 1x1 resolutions
		ScreenResolution::R256x256,
		ScreenResolution::R512x512,
		ScreenResolution::R1024x1024,
		ScreenResolution::R2048x2048,
		ScreenResolution::R3072x3072,
		ScreenResolution::R4096x4096,
		ScreenResolution::R5120x5120,
		ScreenResolution::R6144x6144,
		ScreenResolution::R8192x8192,
		ScreenResolution::R10240x10240,
		ScreenResolution::R12288x12288,
		ScreenResolution::R16384x16384,
		ScreenResolution::R32768x32768,

		// 16x9 resolutions
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

		// 9:16 resolutions
		ScreenResolution::R69120x122880,
		ScreenResolution::R34560x61440,
		ScreenResolution::R17280x30720,
		ScreenResolution::R8640x15360,
		ScreenResolution::R4320x7680,
		ScreenResolution::R2880x5120,
		ScreenResolution::R2160x3840,
		ScreenResolution::R1800x3200,
		ScreenResolution::R1440x2560,
		ScreenResolution::R1152x2048,
		ScreenResolution::R1080x1920,
		ScreenResolution::R900x1600,
		ScreenResolution::R768x1366,
		ScreenResolution::R720x1280,
		ScreenResolution::R576x1024,
		ScreenResolution::R540x960,
		ScreenResolution::R480x854,
		ScreenResolution::R360x640,

		// 32:9 resolutions
		ScreenResolution::R1280x360,
		ScreenResolution::R1920x540,
		ScreenResolution::R2560x720,
		ScreenResolution::R2880x810,
		ScreenResolution::R3200x900,
		ScreenResolution::R3840x1080,
		ScreenResolution::R5120x1440,
		ScreenResolution::R7680x2160,
		ScreenResolution::R10240x2880,

		// 4x3 resolutions
		ScreenResolution::R160x120,
		ScreenResolution::R256x192,
		ScreenResolution::R320x240,
		ScreenResolution::R640x480,
		ScreenResolution::R800x600,
		ScreenResolution::R960x720,
		ScreenResolution::R1024x768,
		ScreenResolution::R1152x864,
		ScreenResolution::R1280x960,
		ScreenResolution::R1400x1050,
		ScreenResolution::R1440x1080,
		ScreenResolution::R1600x1200,
		ScreenResolution::R1856x1392,
		ScreenResolution::R1920x1440,
		ScreenResolution::R2048x1536,
		ScreenResolution::R2560x1920,
		ScreenResolution::R2880x2160,
		ScreenResolution::R3072x2304,
		ScreenResolution::R3840x2880,
		ScreenResolution::R4096x3072,
		ScreenResolution::R5120x3840,
		ScreenResolution::R6144x4608,
		ScreenResolution::R7680x5760,
		ScreenResolution::R8192x6144,

		// 5:4 resolutions
		ScreenResolution::R1280x1024,
		ScreenResolution::R2560x2048,
		ScreenResolution::R5120x4096,

		// Automatic
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
			// 1x1 resolutions
			case ScreenResolution::R256x256: return "256x256";
			case ScreenResolution::R512x512: return "512x512";
			case ScreenResolution::R1024x1024: return "1024x1024";
			case ScreenResolution::R2048x2048: return "2048x2048";
			case ScreenResolution::R3072x3072: return "3072x3072";
			case ScreenResolution::R4096x4096: return "4096x4096";
			case ScreenResolution::R5120x5120: return "5120x5120";
			case ScreenResolution::R6144x6144: return "6144x6144";
			case ScreenResolution::R8192x8192: return "8192x8192";
			case ScreenResolution::R10240x10240: return "10240x10240";
			case ScreenResolution::R12288x12288: return "12288x12288";
			case ScreenResolution::R16384x16384: return "16384x16384";
			case ScreenResolution::R32768x32768: return "32768x32768";

				// 16x9 resolutions
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

				// 9:16 resolutions
			case ScreenResolution::R69120x122880: return "69120x122880";
			case ScreenResolution::R34560x61440: return "34560x61440";
			case ScreenResolution::R17280x30720: return "17280x30720";
			case ScreenResolution::R8640x15360: return "8640x15360";
			case ScreenResolution::R4320x7680: return "4320x7680";
			case ScreenResolution::R2880x5120: return "2880x5120";
			case ScreenResolution::R2160x3840: return "2160x3840";
			case ScreenResolution::R1800x3200: return "1800x3200";
			case ScreenResolution::R1440x2560: return "1440x2560";
			case ScreenResolution::R1152x2048: return "1152x2048";
			case ScreenResolution::R1080x1920: return "1080x1920";
			case ScreenResolution::R900x1600: return "900x1600";
			case ScreenResolution::R768x1366: return "768x1366";
			case ScreenResolution::R720x1280: return "720x1280";
			case ScreenResolution::R576x1024: return "576x1024";
			case ScreenResolution::R540x960: return "540x960";
			case ScreenResolution::R480x854: return "480x854";
			case ScreenResolution::R360x640: return "360x640";

				// 32:9 resolutions
			case ScreenResolution::R1280x360: return "1280x360";
			case ScreenResolution::R1920x540: return "1920x540";
			case ScreenResolution::R2560x720: return "2560x720";
			case ScreenResolution::R2880x810: return "2880x810";
			case ScreenResolution::R3200x900: return "3200x900";
			case ScreenResolution::R3840x1080: return "3840x1080";
			case ScreenResolution::R5120x1440: return "5120x1440";
			case ScreenResolution::R7680x2160: return "7680x2160";
			case ScreenResolution::R10240x2880: return "10240x2880";

				// 4x3 resolutions
			case ScreenResolution::R160x120: return "160x120";
			case ScreenResolution::R256x192: return "256x192";
			case ScreenResolution::R320x240: return "320x240";
			case ScreenResolution::R640x480: return "640x480";
			case ScreenResolution::R800x600: return "800x600";
			case ScreenResolution::R960x720: return "960x720";
			case ScreenResolution::R1024x768: return "1024x768";
			case ScreenResolution::R1152x864: return "1152x864";
			case ScreenResolution::R1280x960: return "1280x960";
			case ScreenResolution::R1400x1050: return "1400x1050";
			case ScreenResolution::R1440x1080: return "1440x1080";
			case ScreenResolution::R1600x1200: return "1600x1200";
			case ScreenResolution::R1856x1392: return "1856x1392";
			case ScreenResolution::R1920x1440: return "1920x1440";
			case ScreenResolution::R2048x1536: return "2048x1536";
			case ScreenResolution::R2560x1920: return "2560x1920";
			case ScreenResolution::R2880x2160: return "2880x2160";
			case ScreenResolution::R3072x2304: return "3072x2304";
			case ScreenResolution::R3840x2880: return "3840x2880";
			case ScreenResolution::R4096x3072: return "4096x3072";
			case ScreenResolution::R5120x3840: return "5120x3840";
			case ScreenResolution::R6144x4608: return "6144x4608";
			case ScreenResolution::R7680x5760: return "7680x5760";
			case ScreenResolution::R8192x6144: return "8192x6144";

				// 5:4 resolutions
			case ScreenResolution::R1280x1024: return "1280x1024";
			case ScreenResolution::R2560x2048: return "2560x2048";
			case ScreenResolution::R5120x4096: return "5120x4096";

				// Automatic
			case ScreenResolution::MatchDevice: return "Match Device";
			case ScreenResolution::None: return "None";
		}
		return "None";
	}

	inline ScreenResolution StringToScreenResolution(const std::string& optionStr)
	{

		// 1x1 resolutions
		if (optionStr == "256x256") return ScreenResolution::R256x256;
		if (optionStr == "512x512") return ScreenResolution::R512x512;
		if (optionStr == "1024x1024") return ScreenResolution::R1024x1024;
		if (optionStr == "2048x2048") return ScreenResolution::R2048x2048;
		if (optionStr == "3072x3072") return ScreenResolution::R3072x3072;
		if (optionStr == "4096x4096") return ScreenResolution::R4096x4096;
		if (optionStr == "5120x5120") return ScreenResolution::R5120x5120;
		if (optionStr == "6144x6144") return ScreenResolution::R6144x6144;
		if (optionStr == "8192x8192") return ScreenResolution::R8192x8192;
		if (optionStr == "10240x10240") return ScreenResolution::R10240x10240;
		if (optionStr == "12288x12288") return ScreenResolution::R12288x12288;
		if (optionStr == "16384x16384") return ScreenResolution::R16384x16384;
		if (optionStr == "32768x32768") return  ScreenResolution::R32768x32768;

		// 16x9 resolutions
		if (optionStr == "122880x69120") return ScreenResolution::R122880x69120;
		if (optionStr == "61440x34560") return ScreenResolution::R61440x34560;
		if (optionStr == "30720x17280") return ScreenResolution::R30720x17280;
		if (optionStr == "15360x8640") return ScreenResolution::R15360x8640;
		if (optionStr == "7680x4320") return ScreenResolution::R7680x4320;
		if (optionStr == "5120x2880") return ScreenResolution::R5120x2880;
		if (optionStr == "3840x2160") return ScreenResolution::R3840x2160;
		if (optionStr == "3200x1800") return ScreenResolution::R3200x1800;
		if (optionStr == "2560x1440") return ScreenResolution::R2560x1440;
		if (optionStr == "2048x1152") return ScreenResolution::R2048x1152;
		if (optionStr == "1920x1080") return ScreenResolution::R1920x1080;
		if (optionStr == "1600x900") return ScreenResolution::R1600x900;
		if (optionStr == "1366x768") return ScreenResolution::R1366x768;
		if (optionStr == "1280x720") return ScreenResolution::R1280x720;
		if (optionStr == "1024x576") return ScreenResolution::R1024x576;
		if (optionStr == "960x540") return ScreenResolution::R960x540;
		if (optionStr == "854x480") return ScreenResolution::R854x480;
		if (optionStr == "640x360") return  ScreenResolution::R640x360;

		// 9:16 resolutions
		if (optionStr == "69120x122880") return ScreenResolution::R69120x122880;
		if (optionStr == "34560x61440") return ScreenResolution::R34560x61440;
		if (optionStr == "17280x30720") return ScreenResolution::R17280x30720;
		if (optionStr == "8640x15360") return ScreenResolution::R8640x15360;
		if (optionStr == "4320x7680") return ScreenResolution::R4320x7680;
		if (optionStr == "2880x5120") return ScreenResolution::R2880x5120;
		if (optionStr == "2160x3840") return ScreenResolution::R2160x3840;
		if (optionStr == "1800x3200") return ScreenResolution::R1800x3200;
		if (optionStr == "1440x2560") return ScreenResolution::R1440x2560;
		if (optionStr == "1152x2048") return ScreenResolution::R1152x2048;
		if (optionStr == "1080x1920") return ScreenResolution::R1080x1920;
		if (optionStr == "900x1600") return ScreenResolution::R900x1600;
		if (optionStr == "768x1366") return ScreenResolution::R768x1366;
		if (optionStr == "720x1280") return ScreenResolution::R720x1280;
		if (optionStr == "576x1024") return ScreenResolution::R576x1024;
		if (optionStr == "540x960") return ScreenResolution::R540x960;
		if (optionStr == "480x854") return ScreenResolution::R480x854;
		if (optionStr == "360x640") return  ScreenResolution::R360x640;

		// 32:9 resolutions
		if (optionStr == "1280x360") return ScreenResolution::R1280x360;
		if (optionStr == "1920x540") return ScreenResolution::R1920x540;
		if (optionStr == "2560x720") return ScreenResolution::R2560x720;
		if (optionStr == "2880x810") return ScreenResolution::R2880x810;
		if (optionStr == "3200x900") return ScreenResolution::R3200x900;
		if (optionStr == "3840x1080") return ScreenResolution::R3840x1080;
		if (optionStr == "5120x1440") return ScreenResolution::R5120x1440;
		if (optionStr == "7680x2160") return ScreenResolution::R7680x2160;
		if (optionStr == "10240x2880") return  ScreenResolution::R10240x2880;

		// 4x3 resolutions
		if (optionStr == "160x120") return ScreenResolution::R160x120;
		if (optionStr == "256x192") return ScreenResolution::R256x192;
		if (optionStr == "320x240") return ScreenResolution::R320x240;
		if (optionStr == "640x480") return ScreenResolution::R640x480;
		if (optionStr == "800x600") return ScreenResolution::R800x600;
		if (optionStr == "960x720") return ScreenResolution::R960x720;
		if (optionStr == "1024x768") return ScreenResolution::R1024x768;
		if (optionStr == "1152x864") return ScreenResolution::R1152x864;
		if (optionStr == "1280x960") return ScreenResolution::R1280x960;
		if (optionStr == "1400x1050") return ScreenResolution::R1400x1050;
		if (optionStr == "1440x1080") return ScreenResolution::R1440x1080;
		if (optionStr == "1600x1200") return ScreenResolution::R1600x1200;
		if (optionStr == "1856x1392") return ScreenResolution::R1856x1392;
		if (optionStr == "1920x1440") return ScreenResolution::R1920x1440;
		if (optionStr == "2048x1536") return ScreenResolution::R2048x1536;
		if (optionStr == "2560x1920") return ScreenResolution::R2560x1920;
		if (optionStr == "2880x2160") return ScreenResolution::R2880x2160;
		if (optionStr == "3072x2304") return ScreenResolution::R3072x2304;
		if (optionStr == "3840x2880") return ScreenResolution::R3840x2880;
		if (optionStr == "4096x3072") return ScreenResolution::R4096x3072;
		if (optionStr == "5120x3840") return ScreenResolution::R5120x3840;
		if (optionStr == "6144x4608") return ScreenResolution::R6144x4608;
		if (optionStr == "7680x5760") return ScreenResolution::R7680x5760;
		if (optionStr == "8192x6144") return  ScreenResolution::R8192x6144;

		// 5:4 resolutions
		if (optionStr == "1280x1024") { return ScreenResolution::R1280x1024; }
		if (optionStr == "2560x2048") { return ScreenResolution::R2560x2048; }
		if (optionStr == "5120x4096") { return ScreenResolution::R5120x4096; }

		if (optionStr == "Match Device") { return ScreenResolution::MatchDevice; }
		if (optionStr == "None") { return ScreenResolution::None; }

		return ScreenResolution::None;
	}

	inline std::string ScreenResolutionToCategoryTitle(ScreenResolution option)
	{
		switch (option)
		{
		// 1x1 resolutions
		case ScreenResolution::R256x256:
		case ScreenResolution::R512x512:
		case ScreenResolution::R1024x1024:
		case ScreenResolution::R2048x2048:
		case ScreenResolution::R3072x3072:
		case ScreenResolution::R4096x4096:
		case ScreenResolution::R5120x5120:
		case ScreenResolution::R6144x6144:
		case ScreenResolution::R8192x8192:
		case ScreenResolution::R10240x10240:
		case ScreenResolution::R12288x12288:
		case ScreenResolution::R16384x16384:
		case ScreenResolution::R32768x32768:
			return "Aspect Ratio: 1:1 (Box)";

			// 16x9 resolutions
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
			// 9:16 resolutions
		case ScreenResolution::R69120x122880:
		case ScreenResolution::R34560x61440:
		case ScreenResolution::R17280x30720:
		case ScreenResolution::R8640x15360:
		case ScreenResolution::R4320x7680:
		case ScreenResolution::R2880x5120:
		case ScreenResolution::R2160x3840:
		case ScreenResolution::R1800x3200:
		case ScreenResolution::R1440x2560:
		case ScreenResolution::R1152x2048:
		case ScreenResolution::R1080x1920:
		case ScreenResolution::R900x1600:
		case ScreenResolution::R768x1366:
		case ScreenResolution::R720x1280:
		case ScreenResolution::R576x1024:
		case ScreenResolution::R540x960:
		case ScreenResolution::R480x854:
		case ScreenResolution::R360x640:
			return "Aspect Ratio: 9:16 (Default Phone Aspect Ratio)";
			// 32:9 resolutions
		case ScreenResolution::R1280x360:
		case ScreenResolution::R1920x540:
		case ScreenResolution::R2560x720:
		case ScreenResolution::R2880x810:
		case ScreenResolution::R3200x900:
		case ScreenResolution::R3840x1080:
		case ScreenResolution::R5120x1440:
		case ScreenResolution::R7680x2160:
		case ScreenResolution::R10240x2880:
			return "Aspect Ratio: 32:9 (Super Ultrawide)";
			// 4x3 resolutions
		case ScreenResolution::R160x120:
		case ScreenResolution::R256x192:
		case ScreenResolution::R320x240:
		case ScreenResolution::R640x480:
		case ScreenResolution::R800x600:
		case ScreenResolution::R960x720:
		case ScreenResolution::R1024x768:
		case ScreenResolution::R1152x864:
		case ScreenResolution::R1280x960:
		case ScreenResolution::R1400x1050:
		case ScreenResolution::R1440x1080:
		case ScreenResolution::R1600x1200:
		case ScreenResolution::R1856x1392:
		case ScreenResolution::R1920x1440:
		case ScreenResolution::R2048x1536:
		case ScreenResolution::R2560x1920:
		case ScreenResolution::R2880x2160:
		case ScreenResolution::R3072x2304:
		case ScreenResolution::R3840x2880:
		case ScreenResolution::R4096x3072:
		case ScreenResolution::R5120x3840:
		case ScreenResolution::R6144x4608:
		case ScreenResolution::R7680x5760:
		case ScreenResolution::R8192x6144:
			return "Aspect Ratio: 4:3 (Fullscreen)";
			// 5:4 resolutions
		case ScreenResolution::R1280x1024:
		case ScreenResolution::R2560x2048:
		case ScreenResolution::R5120x4096:
			return "Aspect Ratio: 5:4 (Older Monitor/TV's)";
			// Automatic
		case ScreenResolution::MatchDevice:
			return "Aspect Ratio: Automatic (Based on Device Used)";
		case ScreenResolution::None:
			return "Invalid Aspect Ratio";
		}
		return "None";
	}


	Math::uvec2 GetClosestAspectRatio(Math::vec2 monitorResolution);
	Math::uvec2 ScreenResolutionToAspectRatio(ScreenResolution option);
	Math::vec2 ScreenResolutionToVec2(ScreenResolution option);
}

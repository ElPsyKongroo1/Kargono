#include "kgpch.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/Resolution.h"
#include "Kargono/Core/Engine.h"

namespace Kargono::Utility
{
	Math::uvec2 ScreenResolutionToVec2(ScreenResolution option)
	{
		switch (option)
		{
		// 1x1 resolutions
		case ScreenResolution::R256x256: return Math::uvec2(256, 256);
		case ScreenResolution::R512x512: return Math::uvec2(512, 512);
		case ScreenResolution::R1024x1024: return Math::uvec2(1024, 1024);
		case ScreenResolution::R2048x2048: return Math::uvec2(2048, 2048);
		case ScreenResolution::R3072x3072: return Math::uvec2(3072, 3072);
		case ScreenResolution::R4096x4096: return Math::uvec2(4096, 4096);
		case ScreenResolution::R5120x5120: return Math::uvec2(5120, 5120);
		case ScreenResolution::R6144x6144: return Math::uvec2(6144, 6144);
		case ScreenResolution::R8192x8192: return Math::uvec2(8192, 8192);
		case ScreenResolution::R10240x10240: return Math::uvec2(10240, 10240);
		case ScreenResolution::R12288x12288: return Math::uvec2(12288, 12288);
		case ScreenResolution::R16384x16384: return Math::uvec2(16384, 16384);
		case ScreenResolution::R32768x32768: return Math::vec2(32768.0f, 32768.0f);

		// 16x9 resolutions
		case ScreenResolution::R122880x69120: return Math::uvec2(122880, 69120);
		case ScreenResolution::R61440x34560: return Math::uvec2(61440, 34560);
		case ScreenResolution::R30720x17280: return Math::uvec2(30720, 17280);
		case ScreenResolution::R15360x8640: return Math::uvec2(15360, 8640);
		case ScreenResolution::R7680x4320: return Math::uvec2(7680, 4320);
		case ScreenResolution::R5120x2880: return Math::uvec2(5120, 2880);
		case ScreenResolution::R3840x2160: return Math::uvec2(3840, 2160);
		case ScreenResolution::R3200x1800: return Math::uvec2(3200, 1800);
		case ScreenResolution::R2560x1440: return Math::uvec2(2560, 1440);
		case ScreenResolution::R2048x1152: return Math::uvec2(2048, 1152);
		case ScreenResolution::R1920x1080: return Math::uvec2(1920, 1080);
		case ScreenResolution::R1600x900: return Math::uvec2(1600, 900);
		case ScreenResolution::R1366x768: return Math::uvec2(1366, 768);
		case ScreenResolution::R1280x720: return Math::uvec2(1280, 720);
		case ScreenResolution::R1024x576: return Math::uvec2(1024, 576);
		case ScreenResolution::R960x540: return Math::uvec2(960, 540);
		case ScreenResolution::R854x480: return Math::uvec2(854, 480);
		case ScreenResolution::R640x360: return Math::uvec2(640, 360);

		// 9:16 resolutions
		case ScreenResolution::R69120x122880: return Math::uvec2(69120, 122880);
		case ScreenResolution::R34560x61440: return Math::uvec2(34560, 61440);
		case ScreenResolution::R17280x30720: return Math::uvec2(17280, 30720);
		case ScreenResolution::R8640x15360: return Math::uvec2(8640, 15360);
		case ScreenResolution::R4320x7680: return Math::uvec2(4320, 7680);
		case ScreenResolution::R2880x5120: return Math::uvec2(2880, 5120);
		case ScreenResolution::R2160x3840: return Math::uvec2(2160, 3840);
		case ScreenResolution::R1800x3200: return Math::uvec2(1800, 3200);
		case ScreenResolution::R1440x2560: return Math::uvec2(1440, 2560);
		case ScreenResolution::R1152x2048: return Math::uvec2(1152, 2048);
		case ScreenResolution::R1080x1920: return Math::uvec2(1080, 1920);
		case ScreenResolution::R900x1600: return Math::uvec2(900, 1600);
		case ScreenResolution::R768x1366: return Math::uvec2(768, 1366);
		case ScreenResolution::R720x1280: return Math::uvec2(720, 1280);
		case ScreenResolution::R576x1024: return Math::uvec2(576, 1024);
		case ScreenResolution::R540x960: return Math::uvec2(540, 960);
		case ScreenResolution::R480x854: return Math::uvec2(480, 854);
		case ScreenResolution::R360x640: return Math::uvec2(360, 640);

		// 32:9 resolutions
		case ScreenResolution::R1280x360: return Math::uvec2(1280, 360);
		case ScreenResolution::R1920x540: return Math::uvec2(1920, 540);
		case ScreenResolution::R2560x720: return Math::uvec2(2560, 720);
		case ScreenResolution::R2880x810: return Math::uvec2(2880, 810);
		case ScreenResolution::R3200x900: return Math::uvec2(3200, 900);
		case ScreenResolution::R3840x1080: return Math::uvec2(3840, 1080);
		case ScreenResolution::R5120x1440: return Math::uvec2(5120, 1440);
		case ScreenResolution::R7680x2160: return Math::uvec2(7680, 2160);
		case ScreenResolution::R10240x2880: return Math::uvec2(10240, 2880);

		// 4x3 resolutions
		case ScreenResolution::R160x120: return Math::uvec2(160, 120);
		case ScreenResolution::R256x192: return Math::uvec2(256, 192);
		case ScreenResolution::R320x240: return Math::uvec2(320, 240);
		case ScreenResolution::R640x480: return Math::uvec2(640, 480);
		case ScreenResolution::R800x600: return Math::uvec2(800, 600);
		case ScreenResolution::R960x720: return Math::uvec2(960, 720);
		case ScreenResolution::R1024x768: return Math::uvec2(1024, 768);
		case ScreenResolution::R1152x864: return Math::uvec2(1152, 864);
		case ScreenResolution::R1280x960: return Math::uvec2(1280, 960);
		case ScreenResolution::R1400x1050: return Math::uvec2(1400, 1050);
		case ScreenResolution::R1440x1080: return Math::uvec2(1440, 1080);
		case ScreenResolution::R1600x1200: return Math::uvec2(1600, 1200);
		case ScreenResolution::R1856x1392: return Math::uvec2(1856, 1392);
		case ScreenResolution::R1920x1440: return Math::uvec2(1920, 1440);
		case ScreenResolution::R2048x1536: return Math::uvec2(2048, 1536);
		case ScreenResolution::R2560x1920: return Math::uvec2(2560, 1920);
		case ScreenResolution::R2880x2160: return Math::uvec2(2880, 2160);
		case ScreenResolution::R3072x2304: return Math::uvec2(3072, 2304);
		case ScreenResolution::R3840x2880: return Math::uvec2(3840, 2880);
		case ScreenResolution::R4096x3072: return Math::uvec2(4096, 3072);
		case ScreenResolution::R5120x3840: return Math::uvec2(5120, 3840);
		case ScreenResolution::R6144x4608: return Math::uvec2(6144, 4608);
		case ScreenResolution::R7680x5760: return Math::uvec2(7680, 5760);
		case ScreenResolution::R8192x6144: return Math::uvec2(8192, 6144);

		// 5:4 resolutions
		case ScreenResolution::R1280x1024: return Math::uvec2(1280, 1024);
		case ScreenResolution::R2560x2048: return Math::uvec2(2560, 2048);
		case ScreenResolution::R5120x4096: return Math::uvec2(5120, 4096);

		// Automatic
		case ScreenResolution::None: return Math::uvec2(400, 400);
		case ScreenResolution::MatchDevice: return EngineService::GetActiveEngine().GetWindow().GetMonitorDimensions();
		}

		KG_ERROR("Invalid ScreenResolution enum provided to ScreenResolutionToVec2 function");
		return Math::vec2(400.0f, 400.0f);
	}

	Math::uvec2 GetClosestAspectRatio(Math::vec2 monitorResolution)
	{

		// Converting float to uint32_t to allow use of switch statement
		constexpr uint32_t widescreen16_9{ static_cast<uint32_t>((16.0f / 9.0f) * 100) };
		constexpr uint32_t fullscreen4_3{ static_cast<uint32_t>((4.0f / 3.0f) * 100) };
		constexpr uint32_t phone9_16{ static_cast<uint32_t>((9.0f / 16.0f) * 100) };
		constexpr uint32_t superUltraWide32_9{ static_cast<uint32_t>((32.0f / 9.0f) * 100) };
		constexpr uint32_t olderMonitor5_4{ static_cast<uint32_t>((5.0f / 4.0f) * 100) };
		constexpr uint32_t square1_1{ 1 * 100 };

		uint32_t monitorAspectRatio = static_cast<uint32_t>((monitorResolution.x / monitorResolution.y) * 100);

		switch (monitorAspectRatio)
		{
		case widescreen16_9: return Math::uvec2(16, 9);
		case fullscreen4_3: return Math::uvec2(4, 3);
		case square1_1: return Math::uvec2(1, 1);
		case olderMonitor5_4: return Math::uvec2(5, 4);
		case superUltraWide32_9: return Math::uvec2(32, 9);
		case phone9_16: return Math::uvec2(9, 16);
		}

		KG_ERROR("Unknown Screen Resolution Used");
		return Math::uvec2(1, 1);

	}

	Math::uvec2 ScreenResolutionToAspectRatio(ScreenResolution option)
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
			return Math::uvec2(1, 1);

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
			return Math::uvec2(16, 9);
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
			return Math::uvec2(9, 16);
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
			return Math::uvec2(32, 9);
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
			return Math::uvec2(4, 3);
			// 5:4 resolutions
		case ScreenResolution::R1280x1024:
		case ScreenResolution::R2560x2048:
		case ScreenResolution::R5120x4096:
			return Math::uvec2(5, 4);
			// Automatic
		case ScreenResolution::MatchDevice: return GetClosestAspectRatio(EngineService::GetActiveEngine().GetWindow().GetMonitorDimensions());
		case ScreenResolution::None: return Math::uvec2(1, 1);
		}
		KG_ERROR("Invalid ScreenResolution enum provided to ScreenResolutionToString function");
		return Math::uvec2(1, 1);
	}
}

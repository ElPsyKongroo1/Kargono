#include "kgpch.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/Resolution.h"
#include "Kargono/Core/Engine.h"

namespace Kargono::Utility
{
	Math::vec2 ScreenResolutionToVec2(ScreenResolution option)
	{
		switch (option)
		{
		// 1x1 resolutions
		case ScreenResolution::R256x256: return Math::vec2(256.0f, 256.0f);
		case ScreenResolution::R512x512: return Math::vec2(512.0f, 512.0f);
		case ScreenResolution::R1024x1024: return Math::vec2(1024.0f, 1024.0f);
		case ScreenResolution::R2048x2048: return Math::vec2(2048.0f, 2048.0f);
		case ScreenResolution::R3072x3072: return Math::vec2(3072.0f, 3072.0f);
		case ScreenResolution::R4096x4096: return Math::vec2(4096.0f, 4096.0f);
		case ScreenResolution::R5120x5120: return Math::vec2(5120.0f, 5120.0f);
		case ScreenResolution::R6144x6144: return Math::vec2(6144.0f, 6144.0f);
		case ScreenResolution::R8192x8192: return Math::vec2(8192.0f, 8192.0f);
		case ScreenResolution::R10240x10240: return Math::vec2(10240.0f, 10240.0f);
		case ScreenResolution::R12288x12288: return Math::vec2(12288.0f, 12288.0f);
		case ScreenResolution::R16384x16384: return Math::vec2(16384.0f, 16384.0f);
		case ScreenResolution::R32768x32768: return Math::vec2(32768.0f, 32768.0f);

		// 16x9 resolutions
		case ScreenResolution::R122880x69120: return Math::vec2(122880.0f, 69120.0f);
		case ScreenResolution::R61440x34560: return Math::vec2(61440.0f, 34560.0f);
		case ScreenResolution::R30720x17280: return Math::vec2(30720.0f, 17280.0f);
		case ScreenResolution::R15360x8640: return Math::vec2(15360.0f, 8640.0f);
		case ScreenResolution::R7680x4320: return Math::vec2(7680.0f, 4320.0f);
		case ScreenResolution::R5120x2880: return Math::vec2(5120.0f, 2880.0f);
		case ScreenResolution::R3840x2160: return Math::vec2(3840.0f, 2160.0f);
		case ScreenResolution::R3200x1800: return Math::vec2(3200.0f, 1800.0f);
		case ScreenResolution::R2560x1440: return Math::vec2(2560.0f, 1440.0f);
		case ScreenResolution::R2048x1152: return Math::vec2(2048.0f, 1152.0f);
		case ScreenResolution::R1920x1080: return Math::vec2(1920.0f, 1080.0f);
		case ScreenResolution::R1600x900: return Math::vec2(1600.0f, 900.0f);
		case ScreenResolution::R1366x768: return Math::vec2(1366.0f, 768.0f);
		case ScreenResolution::R1280x720: return Math::vec2(1280.0f, 720.0f);
		case ScreenResolution::R1024x576: return Math::vec2(1024.0f, 576.0f);
		case ScreenResolution::R960x540: return Math::vec2(960.0f, 540.0f);
		case ScreenResolution::R854x480: return Math::vec2(854.0f, 480.0f);
		case ScreenResolution::R640x360: return Math::vec2(640.0f, 360.0f);

		// 9:16 resolutions
		case ScreenResolution::R69120x122880: return Math::vec2(69120.0f, 122880.0f);
		case ScreenResolution::R34560x61440: return Math::vec2(34560.0f, 61440.0f);
		case ScreenResolution::R17280x30720: return Math::vec2(17280.0f, 30720.0f);
		case ScreenResolution::R8640x15360: return Math::vec2(8640.0f, 15360.0f);
		case ScreenResolution::R4320x7680: return Math::vec2(4320.0f, 7680.0f);
		case ScreenResolution::R2880x5120: return Math::vec2(2880.0f, 5120.0f);
		case ScreenResolution::R2160x3840: return Math::vec2(2160.0f, 3840.0f);
		case ScreenResolution::R1800x3200: return Math::vec2(1800.0f, 3200.0f);
		case ScreenResolution::R1440x2560: return Math::vec2(1440.0f, 2560.0f);
		case ScreenResolution::R1152x2048: return Math::vec2(1152.0f, 2048.0f);
		case ScreenResolution::R1080x1920: return Math::vec2(1080.0f, 1920.0f);
		case ScreenResolution::R900x1600: return Math::vec2(900.0f, 1600.0f);
		case ScreenResolution::R768x1366: return Math::vec2(768.0f, 1366.0f);
		case ScreenResolution::R720x1280: return Math::vec2(720.0f, 1280.0f);
		case ScreenResolution::R576x1024: return Math::vec2(576.0f, 1024.0f);
		case ScreenResolution::R540x960: return Math::vec2(540.0f, 960.0f);
		case ScreenResolution::R480x854: return Math::vec2(480.0f, 854.0f);
		case ScreenResolution::R360x640: return Math::vec2(360.0f, 640.0f);

		// 32:9 resolutions
		case ScreenResolution::R1280x360: return Math::vec2(1280.0f, 360.0f);
		case ScreenResolution::R1920x540: return Math::vec2(1920.0f, 540.0f);
		case ScreenResolution::R2560x720: return Math::vec2(2560.0f, 720.0f);
		case ScreenResolution::R2880x810: return Math::vec2(2880.0f, 810.0f);
		case ScreenResolution::R3200x900: return Math::vec2(3200.0f, 900.0f);
		case ScreenResolution::R3840x1080: return Math::vec2(3840.0f, 1080.0f);
		case ScreenResolution::R5120x1440: return Math::vec2(5120.0f, 1440.0f);
		case ScreenResolution::R7680x2160: return Math::vec2(7680.0f, 2160.0f);
		case ScreenResolution::R10240x2880: return Math::vec2(10240.0f, 2880.0f);

		// 4x3 resolutions
		case ScreenResolution::R160x120: return Math::vec2(160.0f, 120.0f);
		case ScreenResolution::R256x192: return Math::vec2(256.0f, 192.0f);
		case ScreenResolution::R320x240: return Math::vec2(320.0f, 240.0f);
		case ScreenResolution::R640x480: return Math::vec2(640.0f, 480.0f);
		case ScreenResolution::R800x600: return Math::vec2(800.0f, 600.0f);
		case ScreenResolution::R960x720: return Math::vec2(960.0f, 720.0f);
		case ScreenResolution::R1024x768: return Math::vec2(1024.0f, 768.0f);
		case ScreenResolution::R1152x864: return Math::vec2(1152.0f, 864.0f);
		case ScreenResolution::R1280x960: return Math::vec2(1280.0f, 960.0f);
		case ScreenResolution::R1400x1050: return Math::vec2(1400.0f, 1050.0f);
		case ScreenResolution::R1440x1080: return Math::vec2(1440.0f, 1080.0f);
		case ScreenResolution::R1600x1200: return Math::vec2(1600.0f, 1200.0f);
		case ScreenResolution::R1856x1392: return Math::vec2(1856.0f, 1392.0f);
		case ScreenResolution::R1920x1440: return Math::vec2(1920.0f, 1440.0f);
		case ScreenResolution::R2048x1536: return Math::vec2(2048.0f, 1536.0f);
		case ScreenResolution::R2560x1920: return Math::vec2(2560.0f, 1920.0f);
		case ScreenResolution::R2880x2160: return Math::vec2(2880.0f, 2160.0f);
		case ScreenResolution::R3072x2304: return Math::vec2(3072.0f, 2304.0f);
		case ScreenResolution::R3840x2880: return Math::vec2(3840.0f, 2880.0f);
		case ScreenResolution::R4096x3072: return Math::vec2(4096.0f, 3072.0f);
		case ScreenResolution::R5120x3840: return Math::vec2(5120.0f, 3840.0f);
		case ScreenResolution::R6144x4608: return Math::vec2(6144.0f, 4608.0f);
		case ScreenResolution::R7680x5760: return Math::vec2(7680.0f, 5760.0f);
		case ScreenResolution::R8192x6144: return Math::vec2(8192.0f, 6144.0f);

		// 5:4 resolutions
		case ScreenResolution::R1280x1024: return Math::vec2(1280.0f, 1024.0f);
		case ScreenResolution::R2560x2048: return Math::vec2(2560.0f, 2048.0f);
		case ScreenResolution::R5120x4096: return Math::vec2(5120.0f, 4096.0f);

		// Automatic
		case ScreenResolution::None: return Math::vec2(400.0f, 400.0f);
		case ScreenResolution::MatchDevice: return EngineService::GetActiveWindow().GetMonitorDimensions();
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
		case ScreenResolution::MatchDevice: return GetClosestAspectRatio(EngineService::GetActiveWindow().GetMonitorDimensions());
		case ScreenResolution::None: return Math::uvec2(1, 1);
		}
		KG_ERROR("Invalid ScreenResolution enum provided to ScreenResolutionToString function");
		return Math::uvec2(1, 1);
	}
}

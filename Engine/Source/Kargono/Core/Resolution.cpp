#include "kgpch.h"

#include "Kargono/Core/Resolution.h"
#include "Kargono/Core/Engine.h"

namespace Kargono::Utility
{
	Math::vec2 ScreenResolutionToVec2(ScreenResolution option)
	{
		switch (option)
		{
		case ScreenResolution::R400x400: return Math::vec2(400.0f, 400.0f);
		case ScreenResolution::R800x800: return Math::vec2(800.0f, 800.0f);


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

		case ScreenResolution::R1600x1200: return Math::vec2(1600.0f, 1200.0f);
		case ScreenResolution::R1280x960: return Math::vec2(1280.0f, 960.0f);
		case ScreenResolution::R1152x864: return Math::vec2(1152.0f, 864.0f);
		case ScreenResolution::R1024x768: return Math::vec2(1024.0f, 768.0f);

		case ScreenResolution::None: return Math::vec2(400.0f, 400.0f);
		case ScreenResolution::MatchDevice: return EngineService::GetActiveWindow().GetMonitorDimensions();

		}

		KG_ERROR("Invalid ScreenResolution enum provided to ScreenResolutionToVec2 function");
		return Math::vec2(400.0f, 400.0f);
	}

	static Math::uvec2 GetClosestAspectRatio(Math::vec2 monitorResolution)
	{

		// Converting float to uint32_t to allow use of switch statement
		constexpr uint32_t widescreen16_9{ static_cast<uint32_t>((16.0f / 9.0f) * 100) };
		constexpr uint32_t fullscreen4_3{ static_cast<uint32_t>((4.0f / 3.0f) * 100) };
		constexpr uint32_t square1_1{ 1 * 100 };

		uint32_t monitorAspectRatio = static_cast<uint32_t>((monitorResolution.x / monitorResolution.y) * 100);

		switch (monitorAspectRatio)
		{
		case widescreen16_9: return Math::uvec2(16, 9);
		case fullscreen4_3: return Math::uvec2(4, 3);
		case square1_1: return Math::uvec2(1, 1);
		}

		KG_ERROR("Unknown Screen Resolution Used");
		return Math::uvec2(1, 1);

	}

	Math::uvec2 ScreenResolutionToAspectRatio(ScreenResolution option)
	{
		switch (option)
		{

		case ScreenResolution::R800x800: return Math::uvec2(1, 1);
		case ScreenResolution::R400x400: return Math::uvec2(1, 1);

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

		case ScreenResolution::R1600x1200:
		case ScreenResolution::R1280x960:
		case ScreenResolution::R1152x864:
		case ScreenResolution::R1024x768: return Math::uvec2(4, 3);

		case ScreenResolution::MatchDevice: return GetClosestAspectRatio(EngineService::GetActiveWindow().GetMonitorDimensions());
		case ScreenResolution::None: return Math::uvec2(1, 1);
		}
		KG_ERROR("Invalid ScreenResolution enum provided to ScreenResolutionToString function");
		return Math::uvec2(1, 1);
	}
}

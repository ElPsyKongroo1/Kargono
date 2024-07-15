#include "kgpch.h"

#include "Kargono/Projects/Project.h"

#include "Kargono/Core/Engine.h"
#include "Kargono/Assets/AssetManager.h"

namespace Kargono::Utility
{
	Math::vec2 ScreenResolutionToVec2(Projects::ScreenResolutionOptions option)
	{
		switch (option)
		{
		case Projects::ScreenResolutionOptions::R400x400: return Math::vec2(400.0f, 400.0f);
		case Projects::ScreenResolutionOptions::R800x800: return Math::vec2(800.0f, 800.0f);

		case Projects::ScreenResolutionOptions::R1920x1080: return Math::vec2(1920.0f, 1080.0f);
		case Projects::ScreenResolutionOptions::R1600x900: return Math::vec2(1600.0f, 900.0f);
		case Projects::ScreenResolutionOptions::R1366x768: return Math::vec2(1366.0f, 768.0f);
		case Projects::ScreenResolutionOptions::R1280x720: return Math::vec2(1280.0f, 720.0f);

		case Projects::ScreenResolutionOptions::R1600x1200: return Math::vec2(1600.0f, 1200.0f);
		case Projects::ScreenResolutionOptions::R1280x960: return Math::vec2(1280.0f, 960.0f);
		case Projects::ScreenResolutionOptions::R1152x864: return Math::vec2(1152.0f, 864.0f);
		case Projects::ScreenResolutionOptions::R1024x768: return Math::vec2(1024.0f, 768.0f);

		case Projects::ScreenResolutionOptions::None: return Math::vec2(400.0f, 400.0f);
		case Projects::ScreenResolutionOptions::MatchDevice: return EngineService::GetActiveWindow().GetMonitorDimensions();

		}

		KG_ERROR("Invalid ScreenResolutionOptions enum provided to ScreenResolutionToVec2 function");
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

	Math::uvec2 ScreenResolutionToAspectRatio(Projects::ScreenResolutionOptions option)
	{
		switch (option)
		{

		case Projects::ScreenResolutionOptions::R800x800: return Math::uvec2(1, 1);
		case Projects::ScreenResolutionOptions::R400x400: return Math::uvec2(1, 1);

		case Projects::ScreenResolutionOptions::R1280x720:
		case Projects::ScreenResolutionOptions::R1600x900:
		case Projects::ScreenResolutionOptions::R1366x768:
		case Projects::ScreenResolutionOptions::R1920x1080: return Math::uvec2(16, 9);

		case Projects::ScreenResolutionOptions::R1600x1200:
		case Projects::ScreenResolutionOptions::R1280x960:
		case Projects::ScreenResolutionOptions::R1152x864:
		case Projects::ScreenResolutionOptions::R1024x768: return Math::uvec2(4, 3);

		case Projects::ScreenResolutionOptions::MatchDevice: return GetClosestAspectRatio(EngineService::GetActiveWindow().GetMonitorDimensions());
		case Projects::ScreenResolutionOptions::None: return Math::uvec2(1, 1);
		}
		KG_ERROR("Invalid ScreenResolutionOptions enum provided to ScreenResolutionToString function");
		return Math::uvec2(1, 1);
	}
}

namespace Kargono::Projects
{
	Ref<Project> ProjectService::s_ActiveProject { nullptr };

	void ProjectService::ExportProject(const std::filesystem::path& exportLocation)
	{
		KG_ASSERT(s_ActiveProject, "Failed to export project since no active project is open!")

		std::filesystem::path projectDirectory = exportLocation / s_ActiveProject->Name;

		KG_INFO("Creating {} Project Directory", s_ActiveProject->Name);
		Utility::FileSystem::CreateNewDirectory(projectDirectory);

		KG_INFO("Copying project files over");
		bool success = Utility::FileSystem::CopyDirectory(
			GetActiveProjectDirectory(), 
			projectDirectory);
		if (!success)
		{
			KG_WARN("Failed to export project due to inability to copy project's directory!");
			return;
		}

		KG_INFO("Copying runtime shared libraries");
		// Copy OpenAL.dll file
		success = Utility::FileSystem::CopySingleFile(
			std::filesystem::current_path().parent_path() / "Dependencies/OpenAL/lib/dist/OpenAL32.dll",
			projectDirectory / "OpenAL32.dll");
		if (!success)
		{
			KG_WARN("Failed to move OpenAL32.dll file into export directory!");
			return;
		}
	}
}



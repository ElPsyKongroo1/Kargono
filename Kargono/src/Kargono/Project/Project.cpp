#include "kgpch.h"

#include "Kargono/Project/Project.h"

#include "Kargono/Core/Application.h"
#include "Kargono/Assets/AssetManager.h"

namespace Kargono
{
	namespace Utility
	{
		glm::vec2 ScreenResolutionToVec2(ScreenResolutionOptions option)
		{
			switch (option)
			{
			case ScreenResolutionOptions::R400x400: return glm::vec2(400.0f, 400.0f);
			case ScreenResolutionOptions::R800x800: return glm::vec2(800.0f, 800.0f);

			case ScreenResolutionOptions::R1920x1080: return glm::vec2(1920.0f, 1080.0f);
			case ScreenResolutionOptions::R1600x900: return glm::vec2(1600.0f, 900.0f);
			case ScreenResolutionOptions::R1366x768: return glm::vec2(1366.0f, 768.0f);
			case ScreenResolutionOptions::R1280x720: return glm::vec2(1280.0f, 720.0f);

			case ScreenResolutionOptions::R1600x1200: return glm::vec2(1600.0f, 1200.0f);
			case ScreenResolutionOptions::R1280x960: return glm::vec2(1280.0f, 960.0f);
			case ScreenResolutionOptions::R1152x864: return glm::vec2(1152.0f, 864.0f);
			case ScreenResolutionOptions::R1024x768: return glm::vec2(1024.0f, 768.0f);

			case ScreenResolutionOptions::None: return glm::vec2(400.0f, 400.0f);
			case ScreenResolutionOptions::MatchDevice: return Application::GetCurrentApp().GetWindow().GetMonitorDimensions();

			}

			KG_CORE_ASSERT(false, "Invalid ScreenResolutionOptions enum provided to ScreenResolutionToVec2 function");
			return glm::vec2(400.0f, 400.0f);
		}

		static glm::uvec2 GetClosestAspectRatio(glm::vec2 monitorResolution)
		{

			// Converting float to uint32_t to allow use of switch statement
			constexpr uint32_t widescreen16_9{ static_cast<uint32_t>((16.0f / 9.0f) * 100) };
			constexpr uint32_t fullscreen4_3{ static_cast<uint32_t>((4.0f / 3.0f) * 100) };
			constexpr uint32_t square1_1{ 1 * 100 };

			uint32_t monitorAspectRatio = static_cast<uint32_t>((monitorResolution.x / monitorResolution.y) * 100);

			switch (monitorAspectRatio)
			{
			case widescreen16_9: return glm::uvec2(16, 9);
			case fullscreen4_3: return glm::uvec2(4, 3);
			case square1_1: return glm::uvec2(1, 1);
			}

			KG_CORE_ASSERT(false, "Unknown Screen Resolution Used");
			return glm::uvec2(1, 1);

		}

		glm::uvec2 ScreenResolutionToAspectRatio(ScreenResolutionOptions option)
		{
			switch (option)
			{
				

			case ScreenResolutionOptions::R800x800: return glm::uvec2(1, 1);
			case ScreenResolutionOptions::R400x400: return glm::uvec2(1, 1);

			case ScreenResolutionOptions::R1280x720:
			case ScreenResolutionOptions::R1600x900:
			case ScreenResolutionOptions::R1366x768:
			case ScreenResolutionOptions::R1920x1080: return glm::uvec2(16, 9);

			case ScreenResolutionOptions::R1600x1200:
			case ScreenResolutionOptions::R1280x960:
			case ScreenResolutionOptions::R1152x864:
			case ScreenResolutionOptions::R1024x768: return glm::uvec2(4, 3);

			case ScreenResolutionOptions::MatchDevice: return GetClosestAspectRatio(Application::GetCurrentApp().GetWindow().GetMonitorDimensions());
			case ScreenResolutionOptions::None: return glm::uvec2(1, 1);
			}
			KG_CORE_ASSERT(false, "Invalid ScreenResolutionOptions enum provided to ScreenResolutionToString function");
			return glm::uvec2(1, 1);
		}
	}


	Ref<Project> Project::New()
	{
		s_ActiveProject = CreateRef<Project>();
		return s_ActiveProject;
	}
	Ref<Project> Project::Load(const std::filesystem::path& path)
	{
		Ref<Project> project = CreateRef<Project>();
		if (AssetManager::DeserializeProject(project, path))
		{
			project->m_ProjectDirectory = path.parent_path();
			s_ActiveProject = project;
			return s_ActiveProject;
		}

		return nullptr;
		
	}
	bool Project::SaveActive(const std::filesystem::path& path)
	{
		if(AssetManager::SerializeProject(s_ActiveProject, path))
		{
			s_ActiveProject->m_ProjectDirectory = path.parent_path();
			return true;
		}
		return false;
	}
}



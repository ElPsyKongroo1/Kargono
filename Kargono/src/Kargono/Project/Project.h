#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Assets/Asset.h"

#include <glm/glm.hpp>

#include <string>
#include <filesystem>


namespace Kargono
{
	enum class ScreenResolutionOptions
	{
		None = 0,
		R800x800, R400x400,									// 1x1
		R1920x1080, R1600x900, R1366x768, R1280x720,		// 16x9
		R1600x1200, R1280x960, R1152x864, R1024x768,		// 4x3
		MatchDevice											// Automatic
	};
	namespace Utility
	{
		inline std::string ScreenResolutionToString(ScreenResolutionOptions option)
		{
			switch (option)
			{
			case ScreenResolutionOptions::R800x800: return "800x800";
			case ScreenResolutionOptions::R400x400: return "400x400";

			case ScreenResolutionOptions::R1920x1080: return "1920x1080";
			case ScreenResolutionOptions::R1600x900: return "1600x900";
			case ScreenResolutionOptions::R1366x768: return "1366x768";
			case ScreenResolutionOptions::R1280x720: return "1280x720";

			case ScreenResolutionOptions::R1600x1200: return "1600x1200";
			case ScreenResolutionOptions::R1280x960: return "1280x960";
			case ScreenResolutionOptions::R1152x864: return "1152x864";
			case ScreenResolutionOptions::R1024x768: return "1024x768";

			case ScreenResolutionOptions::MatchDevice: return "Match Device";
			case ScreenResolutionOptions::None: return "None";
			}
			KG_CORE_ASSERT(false, "Invalid ScreenResolutionOptions enum provided to ScreenResolutionToString function");
			return "None";
		}

		inline ScreenResolutionOptions StringToScreenResolution(const std::string& optionStr)
		{
			if (optionStr == "800x800") { return ScreenResolutionOptions::R800x800; }
			if (optionStr == "400x400") { return ScreenResolutionOptions::R400x400; }

			if (optionStr == "1920x1080") { return ScreenResolutionOptions::R1920x1080; }
			if (optionStr == "1600x900") { return ScreenResolutionOptions::R1600x900; }
			if (optionStr == "1366x768") { return ScreenResolutionOptions::R1366x768; }
			if (optionStr == "1280x720") { return ScreenResolutionOptions::R1280x720; }

			if (optionStr == "1600x1200") { return ScreenResolutionOptions::R1600x1200; }
			if (optionStr == "1280x960") { return ScreenResolutionOptions::R1280x960; }
			if (optionStr == "1152x864") { return ScreenResolutionOptions::R1152x864; }
			if (optionStr == "1024x768") { return ScreenResolutionOptions::R1024x768; }

			if (optionStr == "Match Device") { return ScreenResolutionOptions::MatchDevice; }
			if (optionStr == "None") { return ScreenResolutionOptions::None; }
			
			KG_CORE_ASSERT(false, "Invalid ScreenResolutionOptions enum provided to StringToScreenResolution function");
			return ScreenResolutionOptions::None;
		}

		glm::uvec2 ScreenResolutionToAspectRatio(ScreenResolutionOptions option);

		glm::vec2 ScreenResolutionToVec2(ScreenResolutionOptions option);
	}
	 
	struct ProjectConfig
	{
		std::string Name = "Untitled";
		std::filesystem::path StartScenePath;
		AssetHandle StartSceneHandle {0};
		std::filesystem::path AssetDirectory;
		std::filesystem::path ScriptModulePath;
		bool DefaultFullscreen = false;
		ScreenResolutionOptions TargetResolution{ ScreenResolutionOptions::R800x800 };
	};

	class Project
	{
	public:
		static const std::filesystem::path& GetProjectDirectory()
		{
			KG_CORE_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_ProjectDirectory;
		}

		static std::filesystem::path GetAssetDirectory()
		{
			KG_CORE_ASSERT(s_ActiveProject);
			return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory;
		}

		static std::filesystem::path GetStartingSceneAbsolutePath()
		{
			KG_CORE_ASSERT(s_ActiveProject);
			return GetProjectDirectory() / s_ActiveProject->m_Config.StartScenePath;
		}

		static std::filesystem::path GetStartingSceneRelativePath()
		{
			KG_CORE_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.StartScenePath;
		}

		static AssetHandle GetStartingSceneHandle()
		{
			KG_CORE_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.StartSceneHandle;
		}

		static bool GetIsFullscreen()
		{
			KG_CORE_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.DefaultFullscreen;
		}

		static void SetIsFullscreen(bool fullscreen)
		{
			KG_CORE_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.DefaultFullscreen = fullscreen;
		}

		static ScreenResolutionOptions GetTargetResolution()
		{
			KG_CORE_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.TargetResolution;
		}

		static void SetTargetResolution(ScreenResolutionOptions option)
		{
			KG_CORE_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.TargetResolution = option;
		}

		static void SetStartingSceneHandle(AssetHandle handle)
		{
			KG_CORE_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.StartSceneHandle = handle;
		}

		static void SetStartingScenePath(const std::filesystem::path& path)
		{
			KG_CORE_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.StartScenePath = path;
		}

		static std::string GetProjectName()
		{
			KG_CORE_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.Name;
		}

		static void SetProjectName(std::string name)
		{
			KG_CORE_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.Name = name;
		}

		

		static std::filesystem::path AppendToAssetDirPath(const std::filesystem::path& path)
		{
			KG_CORE_ASSERT(s_ActiveProject);
			return GetAssetDirectory() / path;
		}

		ProjectConfig& GetConfig()
		{
			return m_Config;
		}

		static Ref<Project> GetActive() { return s_ActiveProject; }

		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& path);
		static bool SaveActive(const std::filesystem::path& path);


	private:
		ProjectConfig m_Config;
		std::filesystem::path m_ProjectDirectory;
		inline static Ref<Project> s_ActiveProject;
	};
}

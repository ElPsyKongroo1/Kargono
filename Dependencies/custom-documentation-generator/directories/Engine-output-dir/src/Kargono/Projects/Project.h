#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/Math/Math.h"

#include <string>
#include <filesystem>

namespace Kargono::Assets { class AssetManager; }

//============================================================
// Projects Namespace
//============================================================
namespace Kargono::Projects
{

	enum class ScreenResolutionOptions
	{
		None = 0,
		R800x800, R400x400,									// 1x1
		R1920x1080, R1600x900, R1366x768, R1280x720,		// 16x9
		R1600x1200, R1280x960, R1152x864, R1024x768,		// 4x3
		MatchDevice											// Automatic
	};

	struct ProjectConfig
	{
		std::string Name = "Untitled";
		std::filesystem::path StartScenePath;
		Assets::AssetHandle StartSceneHandle {0};
		std::filesystem::path AssetDirectory;
		std::filesystem::path ScriptModulePath;
		bool DefaultFullscreen = false;
		ScreenResolutionOptions TargetResolution{ ScreenResolutionOptions::MatchDevice };
	};

	//============================================================
	// Class Project
	//============================================================
	class Project
	{
	public:

		//=========================
		// Getters/Setters
		//=========================
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

		static Assets::AssetHandle GetStartingSceneHandle()
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

		static void SetStartingSceneHandle(Assets::AssetHandle handle)
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

		//=========================
		// Constructor(s)
		//=========================
		


	private:
		ProjectConfig m_Config;
		std::filesystem::path m_ProjectDirectory;
		inline static Ref<Project> s_ActiveProject;
	public:
		friend class Assets::AssetManager;
	};
}

namespace Kargono::Utility
{
	inline std::string ScreenResolutionToString(Projects::ScreenResolutionOptions option)
	{
		switch (option)
		{
		case Projects::ScreenResolutionOptions::R800x800: return "800x800";
		case Projects::ScreenResolutionOptions::R400x400: return "400x400";

		case Projects::ScreenResolutionOptions::R1920x1080: return "1920x1080";
		case Projects::ScreenResolutionOptions::R1600x900: return "1600x900";
		case Projects::ScreenResolutionOptions::R1366x768: return "1366x768";
		case Projects::ScreenResolutionOptions::R1280x720: return "1280x720";

		case Projects::ScreenResolutionOptions::R1600x1200: return "1600x1200";
		case Projects::ScreenResolutionOptions::R1280x960: return "1280x960";
		case Projects::ScreenResolutionOptions::R1152x864: return "1152x864";
		case Projects::ScreenResolutionOptions::R1024x768: return "1024x768";

		case Projects::ScreenResolutionOptions::MatchDevice: return "Match Device";
		case Projects::ScreenResolutionOptions::None: return "None";
		}
		KG_CORE_ASSERT(false, "Invalid ScreenResolutionOptions enum provided to ScreenResolutionToString function");
		return "None";
	}

	inline Projects::ScreenResolutionOptions StringToScreenResolution(const std::string& optionStr)
	{
		if (optionStr == "800x800") { return Projects::ScreenResolutionOptions::R800x800; }
		if (optionStr == "400x400") { return Projects::ScreenResolutionOptions::R400x400; }

		if (optionStr == "1920x1080") { return Projects::ScreenResolutionOptions::R1920x1080; }
		if (optionStr == "1600x900") { return Projects::ScreenResolutionOptions::R1600x900; }
		if (optionStr == "1366x768") { return Projects::ScreenResolutionOptions::R1366x768; }
		if (optionStr == "1280x720") { return Projects::ScreenResolutionOptions::R1280x720; }

		if (optionStr == "1600x1200") { return Projects::ScreenResolutionOptions::R1600x1200; }
		if (optionStr == "1280x960") { return Projects::ScreenResolutionOptions::R1280x960; }
		if (optionStr == "1152x864") { return Projects::ScreenResolutionOptions::R1152x864; }
		if (optionStr == "1024x768") { return Projects::ScreenResolutionOptions::R1024x768; }

		if (optionStr == "Match Device") { return Projects::ScreenResolutionOptions::MatchDevice; }
		if (optionStr == "None") { return Projects::ScreenResolutionOptions::None; }

		KG_CORE_ASSERT(false, "Invalid ScreenResolutionOptions enum provided to StringToScreenResolution function");
		return Projects::ScreenResolutionOptions::None;
	}

	Math::uvec2 ScreenResolutionToAspectRatio(Projects::ScreenResolutionOptions option);

	Math::vec2 ScreenResolutionToVec2(Projects::ScreenResolutionOptions option);
}

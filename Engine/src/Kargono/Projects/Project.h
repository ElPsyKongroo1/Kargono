#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/Math/Math.h"

#include <string>
#include <filesystem>
#include <unordered_set>

namespace Kargono::Assets { class AssetManager; }

//============================================================
// Projects Namespace
//============================================================
// This namespace holds the Project class which describes a Game/Rendering application.
//		Other accessory classes such as the ProjectConfig and ScreenResolutionOptions serve
//		to add functionality to the Project class. 
namespace Kargono::Projects
{
	//=========================
	// Screen Resolution Options Enum
	//=========================
	// This enum provides the different screen resolution options available
	//		at a project level. This enum is used to set the default and stored
	//		screen resolution for the project.
	enum class ScreenResolutionOptions
	{
		None = 0,
		R800x800, R400x400,									// 1x1
		R1920x1080, R1600x900, R1366x768, R1280x720,		// 16x9
		R1600x1200, R1280x960, R1152x864, R1024x768,		// 4x3
		MatchDevice											// Automatic
	};
	//=========================
	// Project Configuration Struct
	//=========================
	// This struct holds the data necessary to describe a project such as its
	//		Asset Directory, Script Path, Starting Scene, Resolution/Fullscreen
	//		options, etc... 
	struct ProjectConfig
	{
		// Name simply provides a method to identify the project for debugging
		//		purposes.
		std::string Name = "Untitled";
		// StartScenePath describes the location of the StartScene relative to
		//		the current asset directory. This is for debugging and editor
		//		purposes.
		std::filesystem::path StartScenePath;
		// StartSceneHandle holds the actual reference to the starting scene for the
		//		project.
		Assets::AssetHandle StartSceneHandle {0};
		// AssetDirectory holds a relative path from the project directory to its
		//		asset directory which is displayed by default in the content browser.
		std::filesystem::path AssetDirectory;
		// ScriptModulePath describes the path from the asset directory to the script
		//		.dll that holds the scripts for the project.
		std::filesystem::path ScriptModulePath;
		// DefaultFullscreen describes the preference to start the application in the
		//		runtime as fullscreen.
		bool DefaultFullscreen = false;
		// TargetResolution describes the screen resolution the application will attempt
		//		to display when starting the runtime application.
		ScreenResolutionOptions TargetResolution{ ScreenResolutionOptions::MatchDevice };
		// OnRuntimeStartFunction holds the name of the custom call that is run when
		//		the application is started.
		std::string OnRuntimeStartFunction {"None"};

		std::string OnUpdateUserCountFunction {"None"};

		std::string OnApproveJoinSessionFunction {"None"};

		std::string OnUserLeftSessionFunction {"None"};

		std::string OnCurrentSessionInitFunction {"None"};

		std::string OnConnectionTerminatedFunction {"None"};

		std::string OnUpdateSessionUserSlotFunction {"None"};

		std::string OnStartSessionFunction {"None"};

		std::string OnSessionReadyCheckConfirmFunction {"None"};

		std::string OnReceiveSignalFunction {"None"};

		std::unordered_set<uint64_t> AppTickGenerators{};

		bool AppIsNetworked = false;

		// Networking Variables

		std::string ServerIP{};
		uint16_t ServerPort{};
		// LocalMachine or Remote are only options currently
		std::string ServerLocation{"LocalMachine"};
		uint64_t SecretOne{};
		uint64_t SecretTwo{};
		uint64_t SecretThree{};
		uint64_t SecretFour{};
	};

	//============================================================
	// Class Project
	//============================================================
	// This class represents a game/rendering application. This class holds data in
	//		its m_Config field that describes relevant paths and user preferences
	//		that are necessary for the application to run such as the location
	//		of the Asset Directory, the starting scene, and the target resolution.
	//		There can be multiple projects that exist, however, only one project
	//		can be loaded into the engine at a time. The reference to this single
	//		project is held in s_ActiveProject.
	class Project
	{
	public:

		//=========================
		// Getters/Setters
		//=========================

		// This function simply returns the project directory
		//		associated with the currently active project
		//		in s_ActiveProject.
		static const std::filesystem::path& GetProjectDirectory()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_ProjectDirectory;
		}
		// This function simply returns the absolute path to the
		//		Asset Directory (Default Directory in the Content Browser)
		//		associated with the active project in s_ActiveProject.
		static std::filesystem::path GetAssetDirectory()
		{
			KG_ASSERT(s_ActiveProject);
			return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory;
		}

		// This function returns the current StartScenePath associated with the active
		//		project in s_ActiveProject.
		static std::filesystem::path GetStartScenePath(bool isAbsolute = true)
		{
			KG_ASSERT(s_ActiveProject);
			if (isAbsolute)
			{
				// Return Absolute Path
				return GetAssetDirectory() / s_ActiveProject->m_Config.StartScenePath;
			}
			// Return Relative Path
			return s_ActiveProject->m_Config.StartScenePath;
		}

		// This function returns the current ScriptModulePath associated with the active
		//		project in s_ActiveProject.
		static std::filesystem::path GetScriptModulePath(bool isAbsolute = true)
		{
			KG_ASSERT(s_ActiveProject);
			if (isAbsolute)
			{
				// Return Absolute Path
				return GetAssetDirectory() / s_ActiveProject->m_Config.ScriptModulePath;
			}
			// Return Relative Path
			return s_ActiveProject->m_Config.ScriptModulePath;
		}

		// This function returns the AssetHandle associated with the starting
		//		scene associated with the active project, in s_ActiveProject.
		//		This handle is reflected in the current s_SceneRegistry in
		//		the AssetManager.
		static Assets::AssetHandle GetStartSceneHandle()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.StartSceneHandle;
		}

		// This functions returns the current s_ActiveProject's DefaultFullscreen
		//		boolean value.
		static bool GetIsFullscreen()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.DefaultFullscreen;
		}

		// This function provides an API to set the fullscreen option on the currently
		//		active project, s_ActiveProject
		static void SetIsFullscreen(bool fullscreen)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.DefaultFullscreen = fullscreen;
		}
		// This function returns the current target resolution associated with
		//		the current project in s_ActiveProject.
		static ScreenResolutionOptions GetTargetResolution()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.TargetResolution;
		}
		// This function provides an API to set the target resolution on the currently
		//		active project, s_ActiveProject
		static void SetTargetResolution(ScreenResolutionOptions option)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.TargetResolution = option;
		}
		// This function sets the starting scene of the current project in s_ActiveProject.
		static void SetStartingScene(Assets::AssetHandle handle, const std::filesystem::path& path)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.StartSceneHandle = handle;
			s_ActiveProject->m_Config.StartScenePath = path;
		}

		// This function gets the project name associated with the current project in s_ActiveProject.
		//		This value is mostly for debugging purposes.
		static std::string GetProjectName()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.Name;
		}

		// This function allows the project name to be changed in the currently active project in
		//		s_ActiveProject.
		static void SetProjectName(const std::string& name)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.Name = name;
		}

		static std::unordered_set<uint64_t>& GetAppTickGenerators()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.AppTickGenerators;
		}

		static bool GetAppIsNetworked()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.AppIsNetworked;
		}

		
		static void SetAppIsNetworked(bool isNetworked)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.AppIsNetworked = isNetworked;
		}

		// This function gets the name of the function from the ScriptEngine CustomCalls that is
		//		run when the application starts.
		static std::string& GetProjectOnRuntimeStart()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.OnRuntimeStartFunction;
		}

		// This function allows the function pointer to be updated when the application begins.
		static void SetProjectOnRuntimeStart(const std::string& name)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.OnRuntimeStartFunction = name;
		}

		static std::string& GetProjectOnUpdateUserCount()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.OnUpdateUserCountFunction;
		}

		static void SetProjectOnUpdateUserCount(const std::string& name)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.OnUpdateUserCountFunction = name;
		}

		static std::string& GetProjectOnApproveJoinSession()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.OnApproveJoinSessionFunction;
		}

		static void SetProjectOnApproveJoinSession(const std::string& name)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.OnApproveJoinSessionFunction = name;
		}

		static std::string& GetProjectOnUserLeftSession()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.OnUserLeftSessionFunction;
		}

		static void SetProjectOnUserLeftSession(const std::string& name)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.OnUserLeftSessionFunction = name;
		}

		static std::string& GetProjectOnCurrentSessionInit()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.OnCurrentSessionInitFunction;
		}

		static void SetProjectOnCurrentSessionInit(const std::string& name)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.OnCurrentSessionInitFunction = name;
		}

		static std::string& GetProjectOnConnectionTerminated()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.OnConnectionTerminatedFunction;
		}

		static void SetProjectOnConnectionTerminated(const std::string& name)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.OnConnectionTerminatedFunction = name;
		}

		static std::string& GetProjectOnUpdateSessionUserSlot()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.OnUpdateSessionUserSlotFunction;
		}

		static void SetProjectOnUpdateSessionUserSlot(const std::string& name)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.OnUpdateSessionUserSlotFunction = name;
		}

		static std::string& GetProjectOnStartSession()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.OnStartSessionFunction;
		}

		static void SetProjectOnStartSession(const std::string& name)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.OnStartSessionFunction = name;
		}

		static std::string& GetProjectOnSessionReadyCheckConfirm()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.OnSessionReadyCheckConfirmFunction;
		}

		static void SetProjectOnSessionReadyCheckConfirm(const std::string& name)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.OnSessionReadyCheckConfirmFunction = name;
		}

		static std::string& GetProjectOnReceiveSignal()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.OnReceiveSignalFunction;
		}

		static void SetProjectOnReceiveSignal(const std::string& name)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.OnReceiveSignalFunction = name;
		}

		static std::string GetServerIP()
		{
			KG_ASSERT(s_ActiveProject);
			if (s_ActiveProject->m_Config.ServerLocation == "LocalMachine")
			{
				return "127.0.0.1";
			}
			return s_ActiveProject->m_Config.ServerIP;
		}

		static void SetServerIP(const std::string& name)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.ServerIP = name;
		}

		static uint16_t GetServerPort()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.ServerPort;
		}

		static void SetServerPort(uint16_t newPort)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.ServerPort = newPort;
		}

		static const std::string& GetServerLocation()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.ServerLocation;
		}

		static void SetServerLocation(const std::string& location)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.ServerLocation = location;
		}

		static uint64_t GetSecretOne()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.SecretOne;
		}

		static void SetSecretOne(uint64_t newSecret)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.SecretOne = newSecret;
		}

		static uint64_t GetSecretTwo()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.SecretTwo;
		}

		static void SetSecretTwo(uint64_t newSecret)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.SecretTwo = newSecret;
		}

		static uint64_t GetSecretThree()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.SecretThree;
		}

		static void SetSecretThree(uint64_t newSecret)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.SecretThree = newSecret;
		}

		static uint64_t GetSecretFour()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_Config.SecretFour;
		}

		static void SetSecretFour(uint64_t newSecret)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_Config.SecretFour = newSecret;
		}

		// This function returns the currently active project held in s_ActiveProject.
		static Ref<Project> GetActive() { return s_ActiveProject; }

	private:
		// This config file holds many details about a project such as the starting scene handle/file location
		//		 the asset directory, the script project directory, etc...
		ProjectConfig m_Config;
		// m_ProjectDirectory simply holds the path to a project. This is typically provided when a
		//		project is initially selected from a file dialog.
		std::filesystem::path m_ProjectDirectory;
		// m_ActiveProject holds a static reference to the currently active project. Only one project can be
		//		active at a time and that project is held in this variable.
		inline static Ref<Project> s_ActiveProject;
	public:
		friend class Assets::AssetManager;
	};
}

namespace Kargono::Utility
{

	//=========================
	// Conversion Functions
	//=========================

	// These functions help convert the screen resolution and aspect ratio enum into a string
	//		to serialization purposes.

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
		KG_ASSERT(false, "Invalid ScreenResolutionOptions enum provided to ScreenResolutionToString function");
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

		KG_ASSERT(false, "Invalid ScreenResolutionOptions enum provided to StringToScreenResolution function");
		return Projects::ScreenResolutionOptions::None;
	}

	Math::uvec2 ScreenResolutionToAspectRatio(Projects::ScreenResolutionOptions option);

	Math::vec2 ScreenResolutionToVec2(Projects::ScreenResolutionOptions option);
}

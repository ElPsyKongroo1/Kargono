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
	// Project Class
	//=========================
	// This struct holds the data necessary to describe a project such as its
	//		Asset Directory, Script Path, Starting Scene, Resolution/Fullscreen
	//		options, etc... 
	class Project
	{
	private:
		//=========================
		// Internal Fields
		//=========================
		// m_ProjectDirectory simply holds the path to a project. This is typically provided when a
		//		project is initially selected from a file dialog.
		std::filesystem::path m_ProjectDirectory {};
		// Name simply provides a method to identify the project for debugging
		//		purposes.
		std::string Name {"Untitled"};
		// StartSceneHandle holds the actual reference to the starting scene for the
		//		project.
		Assets::AssetHandle StartSceneHandle {0};
		// AssetDirectory holds a relative path from the project directory to its
		//		asset directory which is displayed by default in the content browser.
		std::filesystem::path AssetDirectory {};
		// ScriptModulePath describes the path from the asset directory to the script
		//		.dll that holds the scripts for the project.
		std::filesystem::path ScriptModulePath {};
		// ScriptDLLPath describes the path from the asset directory to the script
		//		.dll that holds the scripts for the project.
		std::filesystem::path ScriptDLLPath {};
		// DefaultFullscreen describes the preference to start the application in the
		//		runtime as fullscreen.
		bool DefaultFullscreen = false;
		// TargetResolution describes the screen resolution the application will attempt
		//		to display when starting the runtime application.
		ScreenResolutionOptions TargetResolution{ ScreenResolutionOptions::MatchDevice };
		// OnRuntimeStartFunction holds the name of the custom call that is run when
		//		the application is started.
		Assets::AssetHandle OnRuntimeStart {Assets::EmptyHandle};

		Assets::AssetHandle StartGameState {Assets::EmptyHandle};

		Assets::AssetHandle OnUpdateUserCount {Assets::EmptyHandle};

		Assets::AssetHandle OnApproveJoinSession {Assets::EmptyHandle};

		Assets::AssetHandle OnUserLeftSession {Assets::EmptyHandle};

		Assets::AssetHandle OnCurrentSessionInit {Assets::EmptyHandle};

		Assets::AssetHandle OnConnectionTerminated {Assets::EmptyHandle};

		Assets::AssetHandle OnUpdateSessionUserSlot {Assets::EmptyHandle};

		Assets::AssetHandle OnStartSession {Assets::EmptyHandle};

		Assets::AssetHandle OnSessionReadyCheckConfirm {Assets::EmptyHandle};

		Assets::AssetHandle OnReceiveSignal {Assets::EmptyHandle};

		std::unordered_set<uint64_t> AppTickGenerators{};

		bool AppIsNetworked { false };

		// Networking Variables

		std::string ServerIP{"192.168.1.2"};
		uint16_t ServerPort{20000};
		// LocalMachine or Remote are only options currently
		std::string ServerLocation{"LocalMachine"};
		uint64_t SecretOne{1};
		uint64_t SecretTwo{2};
		uint64_t SecretThree{3};
		uint64_t SecretFour{4};

	private:
		friend class ProjectService;
		friend class Assets::AssetManager;
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
	class ProjectService
	{
	public:
		//=========================
		// External API
		//=========================
		static void ExportProject(const std::filesystem::path& exportLocation);
	public:
		//=========================
		// Getters/Setters API
		//=========================
		// This function simply returns the project directory
		//		associated with the currently active project
		//		in s_ActiveProject.
		static const std::filesystem::path& GetActiveProjectDirectory()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_ProjectDirectory;
		}
		// This function simply returns the absolute path to the
		//		Asset Directory (Default Directory in the Content Browser)
		//		associated with the active project in s_ActiveProject.
		static std::filesystem::path GetActiveAssetDirectory()
		{
			KG_ASSERT(s_ActiveProject);
			return GetActiveProjectDirectory() / s_ActiveProject->AssetDirectory;
		}

		// This function returns the current ScriptModulePath associated with the active
		//		project in s_ActiveProject.
		static std::filesystem::path GetActiveScriptModulePath(bool isAbsolute = true)
		{
			KG_ASSERT(s_ActiveProject);
			if (isAbsolute)
			{
				// Return Absolute Path
				return GetActiveAssetDirectory() / s_ActiveProject->ScriptModulePath;
			}
			// Return Relative Path
			return s_ActiveProject->ScriptModulePath;
		}

		static std::filesystem::path GetActiveScriptDLLPath(bool absolute = true)
		{
			KG_ASSERT(s_ActiveProject);
			if (absolute)
			{
				// Return Absolute Path
				return GetActiveAssetDirectory() / s_ActiveProject->ScriptDLLPath;
			}
			// Return Relative Path
			return s_ActiveProject->ScriptDLLPath;
		}

		// This function returns the AssetHandle associated with the starting
		//		scene associated with the active project, in s_ActiveProject.
		//		This handle is reflected in the current s_SceneRegistry in
		//		the AssetManager.
		static Assets::AssetHandle GetActiveStartSceneHandle()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->StartSceneHandle;
		}

		// This functions returns the current s_ActiveProject's DefaultFullscreen
		//		boolean value.
		static bool GetActiveIsFullscreen()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->DefaultFullscreen;
		}

		// This function provides an API to set the fullscreen option on the currently
		//		active project, s_ActiveProject
		static void SetActiveIsFullscreen(bool fullscreen)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->DefaultFullscreen = fullscreen;
		}
		// This function returns the current target resolution associated with
		//		the current project in s_ActiveProject.
		static ScreenResolutionOptions GetActiveTargetResolution()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->TargetResolution;
		}
		// This function provides an API to set the target resolution on the currently
		//		active project, s_ActiveProject
		static void SetActiveTargetResolution(ScreenResolutionOptions option)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->TargetResolution = option;
		}
		// This function sets the starting scene of the current project in s_ActiveProject.
		static void SetActiveStartingScene(Assets::AssetHandle handle)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->StartSceneHandle = handle;
		}

		// This function gets the project name associated with the current project in s_ActiveProject.
		//		This value is mostly for debugging purposes.
		static std::string GetActiveProjectName()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->Name;
		}

		// This function allows the project name to be changed in the currently active project in
		//		s_ActiveProject.
		static void SetActiveProjectName(const std::string& name)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->Name = name;
		}

		static std::unordered_set<uint64_t>& GetActiveAppTickGenerators()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->AppTickGenerators;
		}

		static bool GetActiveAppIsNetworked()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->AppIsNetworked;
		}

		
		static void SetActiveAppIsNetworked(bool isNetworked)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->AppIsNetworked = isNetworked;
		}

		static Assets::AssetHandle GetActiveStartGameState()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->StartGameState;
		}

		static void SetActiveStartGameState(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->StartGameState = id;
		}

		static Assets::AssetHandle GetActiveOnRuntimeStart()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->OnRuntimeStart;
		}

		static void SetActiveOnRuntimeStart(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->OnRuntimeStart = id;
		}

		static Assets::AssetHandle GetActiveOnUpdateUserCount()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->OnUpdateUserCount;
		}

		static void SetActiveOnUpdateUserCount(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->OnUpdateUserCount = id;
		}

		static Assets::AssetHandle GetActiveOnApproveJoinSession()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->OnApproveJoinSession;
		}

		static void SetActiveOnApproveJoinSession(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->OnApproveJoinSession = id;
		}

		static Assets::AssetHandle GetActiveOnUserLeftSession()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->OnUserLeftSession;
		}

		static void SetActiveOnUserLeftSession(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->OnUserLeftSession = id;
		}

		static Assets::AssetHandle GetActiveOnCurrentSessionInit()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->OnCurrentSessionInit;
		}

		static void SetActiveOnCurrentSessionInit(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->OnCurrentSessionInit = id;
		}
		

		static Assets::AssetHandle GetActiveOnConnectionTerminated()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->OnConnectionTerminated;
		}

		static void SetActiveOnConnectionTerminated(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->OnConnectionTerminated = id;
		}

		static Assets::AssetHandle GetActiveOnUpdateSessionUserSlot()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->OnUpdateSessionUserSlot;
		}

		static void SetActiveOnUpdateSessionUserSlot(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->OnUpdateSessionUserSlot = id;
		}

		static Assets::AssetHandle GetActiveOnStartSession()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->OnStartSession;
		}

		static void SetActiveOnStartSession(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->OnStartSession = id;
		}

		static Assets::AssetHandle GetActiveOnSessionReadyCheckConfirm()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->OnSessionReadyCheckConfirm;
		}

		static void SetActiveOnSessionReadyCheckConfirm(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->OnSessionReadyCheckConfirm = id;
		}

		static Assets::AssetHandle GetActiveOnReceiveSignal()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->OnReceiveSignal;
		}

		static void SetActiveOnReceiveSignal(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->OnReceiveSignal = id;
		}

		static std::string GetActiveServerIP()
		{
			KG_ASSERT(s_ActiveProject);
			if (s_ActiveProject->ServerLocation == "LocalMachine")
			{
				return "127.0.0.1";
			}
			return s_ActiveProject->ServerIP;
		}

		static void SetActiveServerIP(const std::string& name)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->ServerIP = name;
		}

		static uint16_t GetActiveServerPort()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->ServerPort;
		}

		static void SetActiveServerPort(uint16_t newPort)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->ServerPort = newPort;
		}

		static const std::string& GetActiveServerLocation()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->ServerLocation;
		}

		static void SetActiveServerLocation(const std::string& location)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->ServerLocation = location;
		}

		static uint64_t GetActiveSecretOne()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->SecretOne;
		}

		static void SetActiveSecretOne(uint64_t newSecret)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->SecretOne = newSecret;
		}

		static uint64_t GetActiveSecretTwo()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->SecretTwo;
		}

		static void SetActiveSecretTwo(uint64_t newSecret)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->SecretTwo = newSecret;
		}

		static uint64_t GetActiveSecretThree()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->SecretThree;
		}

		static void SetActiveSecretThree(uint64_t newSecret)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->SecretThree = newSecret;
		}

		static uint64_t GetActiveSecretFour()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->SecretFour;
		}

		static void SetActiveSecretFour(uint64_t newSecret)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->SecretFour = newSecret;
		}

		// This function returns the currently active project held in s_ActiveProject.
		static Ref<Project> GetActive()
		{
			return s_ActiveProject;
		}

	private:
		//=========================
		// Internal Fields
		//=========================
		// m_ActiveProject holds a static reference to the currently active project. Only one project can be
		//		active at a time and that project is held in this variable.
		static Ref<Project> s_ActiveProject;
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
		KG_ERROR("Invalid ScreenResolutionOptions enum provided to ScreenResolutionToString function");
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

		KG_ERROR("Invalid ScreenResolutionOptions enum provided to StringToScreenResolution function");
		return Projects::ScreenResolutionOptions::None;
	}

	Math::uvec2 ScreenResolutionToAspectRatio(Projects::ScreenResolutionOptions option);

	Math::vec2 ScreenResolutionToVec2(Projects::ScreenResolutionOptions option);
}

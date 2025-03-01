#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/Math/Math.h"
#include "Kargono/Core/Resolution.h"
#include "Kargono/Network/NetworkCommon.h"

#include <string>
#include <filesystem>
#include <unordered_set>
#include <array>

//============================================================
// Projects Namespace
//============================================================
// This namespace holds the Project class which describes a Game/Rendering application.
//		Other accessory classes such as the ProjectConfig  serve
//		to add functionality to the Project class. 
namespace Kargono::Projects
{
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

		std::filesystem::path IntermediateDirectory {};
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
		ScreenResolution TargetResolution{ ScreenResolution::MatchDevice };
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
		Network::ServerConfig m_ServerConfig;

	private:
		friend class ProjectService;
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

		//==============================
		// Modify Active Project
		//==============================
		static std::filesystem::path CreateNewProject(const std::string& projectName, const std::filesystem::path& projectLocation);
		static Ref<Projects::Project> OpenProject(const std::filesystem::path& path);
		static bool SaveActiveProject();
		static bool SaveActiveProject(const std::filesystem::path& path);
		static bool SerializeProject(Ref<Projects::Project> project, const std::filesystem::path& filepath);
		static bool DeserializeProject(Ref<Projects::Project> project, const std::filesystem::path& filepath);

		static bool RemoveScriptFromActiveProject(Assets::AssetHandle scriptHandle);

	private:
		static bool DeserializeServerVariables(Ref<Projects::Project> project, const std::filesystem::path& filepath);

		//=========================
		// Exporting API
		//=========================
	public:
		static void ExportProject(const std::filesystem::path& exportLocation, Network::ServerConfig* serverConfig, bool createServer);
	private:
		static bool BuildExecutableMSVC(const std::filesystem::path& projectDirectory, bool createServer);
		static bool BuildExecutableGCC(const std::filesystem::path& projectDirectory, bool createServer);
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

		static std::filesystem::path GetActiveIntermediateDirectory()
		{
			KG_ASSERT(s_ActiveProject);
			return GetActiveProjectDirectory() / s_ActiveProject->IntermediateDirectory;
		}

		// This function returns the current ScriptModulePath associated with the active
		//		project in s_ActiveProject.
		static std::filesystem::path GetActiveScriptModulePath(bool isAbsolute = true)
		{
			KG_ASSERT(s_ActiveProject);
			if (isAbsolute)
			{
				// Return Absolute Path
				return GetActiveIntermediateDirectory() / s_ActiveProject->ScriptModulePath;
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
				return GetActiveIntermediateDirectory() / s_ActiveProject->ScriptDLLPath;
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
		static ScreenResolution GetActiveTargetResolution()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->TargetResolution;
		}
		// This function provides an API to set the target resolution on the currently
		//		active project, s_ActiveProject
		static void SetActiveTargetResolution(ScreenResolution option)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->TargetResolution = option;
		}
		// This function sets the starting scene of the current project in s_ActiveProject.
		static void SetActiveStartingSceneHandle(Assets::AssetHandle handle)
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

		static Assets::AssetHandle GetActiveStartGameStateHandle()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->StartGameState;
		}

		static void SetActiveStartGameStateHandle(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->StartGameState = id;
		}

		static Assets::AssetHandle GetActiveOnRuntimeStartHandle()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->OnRuntimeStart;
		}

		static void SetActiveOnRuntimeStartHandle(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->OnRuntimeStart = id;
		}

		static Assets::AssetHandle GetActiveOnUpdateUserCountHandle()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->OnUpdateUserCount;
		}

		static void SetActiveOnUpdateUserCountHandle(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->OnUpdateUserCount = id;
		}

		static Assets::AssetHandle GetActiveOnApproveJoinSessionHandle()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->OnApproveJoinSession;
		}

		static void SetActiveOnApproveJoinSessionHandle(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->OnApproveJoinSession = id;
		}

		static Assets::AssetHandle GetActiveOnUserLeftSessionHandle()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->OnUserLeftSession;
		}

		static void SetActiveOnUserLeftSessionHandle(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->OnUserLeftSession = id;
		}

		static Assets::AssetHandle GetActiveOnCurrentSessionInitHandle()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->OnCurrentSessionInit;
		}

		static void SetActiveOnCurrentSessionInitHandle(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->OnCurrentSessionInit = id;
		}
		

		static Assets::AssetHandle GetActiveOnConnectionTerminatedHandle()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->OnConnectionTerminated;
		}

		static void SetActiveOnConnectionTerminatedHandle(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->OnConnectionTerminated = id;
		}

		static Assets::AssetHandle GetActiveOnUpdateSessionUserSlotHandle()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->OnUpdateSessionUserSlot;
		}

		static void SetActiveOnUpdateSessionUserSlotHandle(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->OnUpdateSessionUserSlot = id;
		}

		static Assets::AssetHandle GetActiveOnStartSessionHandle()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->OnStartSession;
		}

		static void SetActiveOnStartSessionHandle(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->OnStartSession = id;
		}

		static Assets::AssetHandle GetActiveOnSessionReadyCheckConfirmHandle()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->OnSessionReadyCheckConfirm;
		}

		static void SetActiveOnSessionReadyCheckConfirmHandle(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->OnSessionReadyCheckConfirm = id;
		}

		static Assets::AssetHandle GetActiveOnReceiveSignalHandle()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->OnReceiveSignal;
		}

		static void SetActiveOnReceiveSignalHandle(Assets::AssetHandle id)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->OnReceiveSignal = id;
		}

		static Math::u8vec4 GetActiveServerIP()
		{
			KG_ASSERT(s_ActiveProject);
			if (s_ActiveProject->m_ServerConfig.m_ServerLocation == Network::ServerLocation::LocalMachine)
			{
				return {127, 0, 0, 1};
			}
			return s_ActiveProject->m_ServerConfig.m_IPv4;
		}

		static void SetActiveServerIP(Math::u8vec4 ip)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_ServerConfig.m_IPv4 = ip;
		}

		static uint16_t GetActiveServerPort()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_ServerConfig.m_Port;
		}

		static void SetActiveServerPort(uint16_t newPort)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_ServerConfig.m_Port = newPort;
		}

		static Network::ServerLocation GetActiveServerLocation()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_ServerConfig.m_ServerLocation;
		}

		static void SetActiveServerLocation(Network::ServerLocation location)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_ServerConfig.m_ServerLocation = location;
		}

		static Math::u64vec4 GetActiveSecrets()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_ServerConfig.m_ValidationSecrets;
		}

		static uint64_t GetActiveSecretOne()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_ServerConfig.m_ValidationSecrets.x;
		}

		static void SetActiveSecretOne(uint64_t newSecret)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_ServerConfig.m_ValidationSecrets.x = newSecret;
		}

		static uint64_t GetActiveSecretTwo()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_ServerConfig.m_ValidationSecrets.y;
		}

		static void SetActiveSecretTwo(uint64_t newSecret)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_ServerConfig.m_ValidationSecrets.y = newSecret;
		}

		static uint64_t GetActiveSecretThree()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_ServerConfig.m_ValidationSecrets.z;
		}

		static void SetActiveSecretThree(uint64_t newSecret)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_ServerConfig.m_ValidationSecrets.z = newSecret;
		}

		static uint64_t GetActiveSecretFour()
		{
			KG_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_ServerConfig.m_ValidationSecrets.w;
		}

		static void SetActiveSecretFour(uint64_t newSecret)
		{
			KG_ASSERT(s_ActiveProject);
			s_ActiveProject->m_ServerConfig.m_ValidationSecrets.w = newSecret;
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
		static inline Ref<Project> s_ActiveProject{ nullptr };
	};
}

#pragma once

#include "Kargono/Core/Base.h"
#include "Modules/Assets/Asset.h"
#include "Kargono/Math/Math.h"
#include "Kargono/Core/Resolution.h"
#include "Modules/Network/ServerConfig.h"
#include "Modules/Network/NetworkScripts.h"

#include <string>
#include <filesystem>
#include <unordered_set>
#include <array>

namespace Kargono::Projects
{
	class ProjectPaths
	{
	private:
		//=========================
		// Constructors/Destructors
		//=========================
		ProjectPaths() = default;
		~ProjectPaths() = default;
	public:
		//=========================
		// Getter/Setters
		//=========================
		std::filesystem::path GetAssetDirectory()
		{
			return m_ProjectDirectory / "Assets";
		}

		std::filesystem::path GetIntermediateDirectory()
		{
			return m_ProjectDirectory / "Intermediates";
		}
	public:
		//=========================
		// Public Fields
		//=========================
		std::filesystem::path m_ProjectDirectory{};

	private:
		friend class Project;
	};

	class Project
	{
	public:
		//==============================
		// Save/Load Project
		//==============================
		bool CreateNewProject(const std::string& projectName, const std::filesystem::path& projectLocation);
		bool OpenProject(const std::filesystem::path& path);
		bool SaveProject();
		bool SaveProject(const std::filesystem::path& path);

	private:
		bool SerializeProject(const std::filesystem::path& filepath);
		bool DeserializeProject(const std::filesystem::path& filepath);

		bool SerializeServerConfig();
		bool DeserializeServerConfig(const std::filesystem::path& filepath);

	public:
		//==============================
		// Remove Asset
		//==============================
		bool RemoveScriptFromActiveProject(Assets::AssetHandle scriptHandle);

	public:
		//=========================
		// Exporting API
		//=========================
		void ExportProject(const std::filesystem::path& exportLocation, bool createServer);
	private:
		bool BuildExecutableMSVC(const std::filesystem::path& projectDirectory, bool createServer);
		bool BuildExecutableGCC(const std::filesystem::path& projectDirectory, bool createServer);
	public:
		//=========================
		// Getters/Setters
		//=========================
		Assets::AssetHandle GetStartSceneHandle()
		{
			return m_StartSceneHandle;
		}
		bool GetIsFullscreen()
		{
			return m_DefaultFullscreen;
		}
		void SetIsFullscreen(bool fullscreen)
		{
			m_DefaultFullscreen = fullscreen;
		}
		ScreenResolution GetTargetResolution()
		{
			return m_TargetResolution;
		}
		void SetTargetResolution(ScreenResolution option)
		{
			m_TargetResolution = option;
		}
		void SetStartingSceneHandle(Assets::AssetHandle handle)
		{
			m_StartSceneHandle = handle;
		}
		std::string GetProjectName()
		{
			return m_Name;
		}
		void SetProjectName(const std::string& name)
		{
			m_Name = name;
		}
		bool GetAppIsNetworked()
		{
			return m_AppIsNetworked;
		}
		void SetAppIsNetworked(bool isNetworked)
		{
			m_AppIsNetworked = isNetworked;
		}
		Assets::AssetHandle GetStartGameStateHandle()
		{
			return m_StartGameState;
		}
		void SetStartGameStateHandle(Assets::AssetHandle id)
		{
			m_StartGameState = id;
		}
		Assets::AssetHandle GetOnRuntimeStartHandle()
		{
			return m_OnRuntimeStart;
		}
		void SetOnRuntimeStartHandle(Assets::AssetHandle id)
		{
			m_OnRuntimeStart = id;
		}

		Network::ClientScripts& GetClientScripts()
		{
			return m_ClientScripts;
		}

		Network::ServerConfig& GetServerConfig()
		{
			return m_ServerConfig;
		}
		void SetServerConfig(const Network::ServerConfig& config)
		{
			m_ServerConfig = config;
		}
		ProjectPaths& GetProjectPaths()
		{
			return m_ProjectPaths;
		}

	private:
		//=========================
		// Internal Fields
		//=========================
		// Debug name
		std::string m_Name {"Untitled"};
		// Project state
		bool m_Active{ false };
		// Project path(s)
		ProjectPaths m_ProjectPaths{};
		// Display options
		bool m_DefaultFullscreen{ false };
		ScreenResolution m_TargetResolution{ ScreenResolution::MatchDevice };
		// Default assets
		Assets::AssetHandle m_StartSceneHandle {0};
		Assets::AssetHandle m_OnRuntimeStart {Assets::EmptyHandle};
		Assets::AssetHandle m_StartGameState {Assets::EmptyHandle};
		// Networking Variables
		Network::ServerConfig m_ServerConfig;
		Network::ClientScripts m_ClientScripts;
		bool m_AppIsNetworked { false }; // TODO: This bool should be addressed by the module system

	private:
		friend class ProjectService;
	};

	class ProjectService
	{
	public:
		static Project& GetActiveContext()
		{
			return s_ActiveProject;
		}

		static bool IsActive()
		{
			return s_ActiveProject.m_Active;
		}

	private:
		//=========================
		// Internal Fields
		//=========================
		static inline Project s_ActiveProject{};
	};
}

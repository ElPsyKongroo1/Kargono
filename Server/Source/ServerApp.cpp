#include "kgpch.h"
#include "ServerApp.h"

#include <filesystem>

namespace Kargono
{
	ServerApp::ServerApp()
		: Application("ServerLayer")
	{
	}

	void ServerApp::Init()
	{
		Scenes::SceneService::Init();
#ifdef KG_TESTING
	OpenProject("../Projects/Pong/Pong.kproj");
#elif defined KG_EXPORT
		std::filesystem::path pathToProject = Utility::FileSystem::FindFileWithExtension(
			std::filesystem::current_path(),
			".kproj");
		if (pathToProject.empty())
		{
			KG_CRITICAL("Could not locate a .kproj file in local directory!");
			EngineService::EndRun();
			return;
	}
		OpenProject(pathToProject);
		if (!Projects::ProjectService::GetActive())
		{
			KG_CRITICAL("Failed to open project!");
			EngineService::EndRun();
			return;
		}
#else
		if (!OpenProject())
		{
			KG_CRITICAL("Failed to select/open a project");
			EngineService::EndRun();
			return;
		}
#endif


		bool isLocal = Projects::ProjectService::GetActiveServerLocation() == "LocalMachine";

		Network::ServerService::SetActiveServer(CreateRef<Network::Server>(Projects::ProjectService::GetActiveServerPort(), isLocal));

		if (!Network::ServerService::GetActiveServer()->StartServer())
		{
			KG_CRITICAL("Failed to start server");
			Network::ServerService::GetActiveServer()->StopServer();
			Network::ServerService::GetActiveServer().reset();
			EngineService::EndRun();
			return;
		}

#ifndef KG_TESTING
		Network::ServerService::GetActiveServer()->RunServer();
#endif
		Network::ServerService::GetActiveServer()->StopServer();
		Network::ServerService::GetActiveServer().reset();
		Network::ServerService::GetActiveServer() = nullptr;
	}

	bool ServerApp::OpenProject()
	{
		std::filesystem::path initialDirectory = std::filesystem::current_path().parent_path() / "Projects";
		if (!std::filesystem::exists(initialDirectory))
		{
			initialDirectory = "";
		}
		std::filesystem::path filepath = Utility::FileDialogs::OpenFile("Kargono Project (*.kproj)\0*.kproj\0", initialDirectory.string().c_str());
		if (filepath.empty()) { return false; }

		OpenProject(filepath);
		return true;
	}

	void ServerApp::OpenProject(const std::filesystem::path& path)
	{
		if (Assets::AssetManager::OpenProject(path)) {}
	}


	void ServerApp::Terminate()
	{
	}

	void ServerApp::OnUpdate(Timestep ts)
	{
		
	}

}

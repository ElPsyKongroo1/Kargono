#include "kgpch.h"
#include "ServerApp.h"

#include <filesystem>

namespace Kargono
{
	// Final Export Values
	//const std::filesystem::path runtimePath = "../Projects/Pong/Pong.kproj";
	//const std::filesystem::path logoPath = "../Projects/Pong/pong_logo.png";

	const std::filesystem::path runtimePath = "./Pong/Pong.kproj";
	const std::filesystem::path logoPath = "./Pong/pong_logo.png";

	ServerApp::ServerApp()
		: Application("ServerLayer")
	{
	}

	void ServerApp::OnAttach()
	{
		Scenes::SceneService::Init();
#ifdef KG_TESTING
		OpenProject("../Projects/Pong/Pong.kproj");
#else
		if (!OpenProject())
		{
			KG_CRITICAL("Failed to select/open a project");
			EngineCore::GetCurrentEngineCore().Close();
			return;
		}
#endif

		bool isLocal = Projects::Project::GetServerLocation() == "LocalMachine";

		Network::Server::SetActiveServer(CreateRef<Network::Server>(Projects::Project::GetServerPort(), isLocal));

		if (!Network::Server::GetActiveServer()->StartServer())
		{
			KG_CRITICAL("Failed to start server");
			Network::Server::GetActiveServer()->StopServer();
			Network::Server::GetActiveServer().reset();
			EngineCore::GetCurrentEngineCore().Close();
			return;
		}

#ifndef KG_TESTING
		Network::Server::GetActiveServer()->RunServer();
#endif
		Network::Server::GetActiveServer()->StopServer();
		Network::Server::GetActiveServer().reset();
		Network::Server::GetActiveServer() = nullptr;
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


	void ServerApp::OnDetach()
	{
	}

	void ServerApp::OnUpdate(Timestep ts)
	{
		
	}

}

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
		if (!OpenProject())
		{
			EngineCore::GetCurrentEngineCore().Close();
			return;
		}

		bool isLocal = Projects::Project::GetServerLocation() == "LocalMachine";

		Network::Server::SetActiveServer(CreateRef<Network::Server>(Projects::Project::GetServerPort(), isLocal));
		Network::Server::GetActiveServer()->RunServer();
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

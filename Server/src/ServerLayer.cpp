#include "kgpch.h"
#include "ServerLayer.h"

#include <filesystem>

namespace Kargono
{
	// Final Export Values
	//const std::filesystem::path runtimePath = "../Projects/Pong/Pong.kproj";
	//const std::filesystem::path logoPath = "../Projects/Pong/pong_logo.png";

	const std::filesystem::path runtimePath = "./Pong/Pong.kproj";
	const std::filesystem::path logoPath = "./Pong/pong_logo.png";

	ServerLayer::ServerLayer()
		: Application("ServerLayer")
	{
	}

	void ServerLayer::OnAttach()
	{
		if (!OpenProject())
		{
			Core::GetCurrentApp().Close();
			return;
		}

		bool isLocal = Projects::Project::GetServerLocation() == "LocalMachine";

		Network::Server::SetActiveServer(CreateRef<Network::Server>(Projects::Project::GetServerPort(), isLocal));
		Network::Server::GetActiveServer()->RunServer();
	}

	bool ServerLayer::OpenProject()
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

	void ServerLayer::OpenProject(const std::filesystem::path& path)
	{
		if (Assets::AssetManager::OpenProject(path)) {}
	}


	void ServerLayer::OnDetach()
	{
	}

	void ServerLayer::OnUpdate(Timestep ts)
	{
		
	}

}

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
	OpenProject("../Projects/TestProject/TestProject.kproj");
#elif defined(KG_EXPORT_RUNTIME) || defined (KG_EXPORT_SERVER)
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
		if (!Network::ServerService::Init())
		{
			EngineService::EndRun();
			return;
		}

#ifndef KG_TESTING
		Network::ServerService::Run();
#endif
		Network::ServerService::Terminate();
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

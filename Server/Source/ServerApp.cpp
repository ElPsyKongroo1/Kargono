#include "kgpch.h"
#include "ServerApp.h"

#include <filesystem>

namespace Kargono
{
	ServerApp::ServerApp()
		: Application("ServerLayer")
	{
	}

	ServerApp::ServerApp(std::filesystem::path projectPath) : Application("ServerLayer"), m_ProjectPath(projectPath)
	{

	}

	bool ServerApp::Init()
	{
		Scenes::SceneService::Init();
#if defined(KG_EXPORT_RUNTIME) || defined (KG_EXPORT_SERVER)
		std::filesystem::path pathToProject = Utility::FileSystem::FindFileWithExtension(
			std::filesystem::current_path(),
			".kproj");
		if (pathToProject.empty())
		{
			KG_CRITICAL("Could not locate a .kproj file in local directory!");
			Scenes::SceneService::Terminate();
			return false;
	}
		OpenProject(pathToProject);
		if (!Projects::ProjectService::GetActive())
		{
			KG_CRITICAL("Failed to open project!");
			Scenes::SceneService::Terminate();
			return false;
		}
#else

		if (m_ProjectPath.empty())
		{
			if (!OpenProject())
			{
				Scenes::SceneService::Terminate();
				return false;
			}
		}
		else
		{
			OpenProject(m_ProjectPath);
		}
#endif
		if (!Network::ServerService::Init())
		{
			Scenes::SceneService::Terminate();
			return false;
		}

		return true;
	}

	bool ServerApp::OpenProject()
	{
		std::filesystem::path initialDirectory = std::filesystem::current_path().parent_path() / "Projects";
		if (!Utility::FileSystem::PathExists(initialDirectory))
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
		Projects::ProjectService::OpenProject(path);
	}

	bool ServerApp::Terminate()
	{
		Network::ServerService::Terminate();
		return true;
	}

	void ServerApp::OnUpdate(Timestep ts)
	{
		//Network::ServerService::Run();
	}

	Math::vec2 ServerApp::GetMouseViewportPosition()
	{
		KG_WARN("Attempt to retrieve mouse location from unimplemented function in Server Application");
		return Math::vec2();
	}

	ViewportData* ServerApp::GetViewportData()
	{
		KG_WARN("Attempt to retrieve mouse location from unimplemented function in Server Application");
		return nullptr;
	}

}

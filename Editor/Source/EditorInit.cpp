#include "Kargono.h"
#ifndef KG_TESTING
#include "Kargono/Core/EntryPoint.h"
#endif

#include "EditorApp.h"
#include "Launcher/LauncherApp.h"

namespace Kargono
{
	void InitEngineAndCreateApp(CommandLineArguments args)
	{
		std::filesystem::path projectPath {};
#ifdef KG_TESTING
		projectPath = "../Projects/Pong/Pong.kproj";
#else
		EngineSpec launcherSpec;
		launcherSpec.Name = "Editor Launcher";
		launcherSpec.CommandLineArgs = args;
		launcherSpec.WorkingDirectory = std::filesystem::current_path();
		launcherSpec.DefaultWindowWidth = 800;
		launcherSpec.DefaultWindowHeight = 320;
		LauncherApp* launcherApp = new LauncherApp();
		EngineService::Init(launcherSpec, launcherApp);
		Kargono::EngineService::Run();
		projectPath = launcherApp->GetSelectedProject();
		EngineService::Terminate();

		if (projectPath.empty())
		{
			return;
		}
#endif
		
		EngineSpec editorSpec;
		editorSpec.Name = "Kargono Editor";
		editorSpec.CommandLineArgs = args;
		editorSpec.WorkingDirectory = std::filesystem::current_path();
		editorSpec.DefaultWindowWidth = 1600;
		editorSpec.DefaultWindowHeight = 900;
		Application* editorApp = new EditorApp(projectPath);
		EngineService::Init(editorSpec, editorApp);
		KG_VERIFY(editorApp, "Editor App Init");
	}
}

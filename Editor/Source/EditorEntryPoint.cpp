
#include "CoreModule/Engine.h"

#ifndef KG_TESTING
#include "CoreModule/EntryPoint.h"
#endif

#include "EditorApp.h"
#include "Launcher/LauncherApp.h"

namespace Kargono
{
	void ExecutableEntryPoint(Engine& engine, CommandLineArguments args)
	{
		std::filesystem::path projectPath {};

		// Init Launcher
		EngineConfig launcherSpec;
		launcherSpec.m_ExecutableName = "Kargono Editor Launcher";
		launcherSpec.m_CmlArgs = args;
		launcherSpec.m_WorkingDirectory = std::filesystem::current_path();
		launcherSpec.m_DefaultWindowDimensions = {800, 320};
		LauncherApp* launcherApp = new LauncherApp();
		engine.Init(launcherSpec, launcherApp);

		// Run launcher
		engine.GetThread().RunThread();

		// Close launcher
		projectPath = launcherApp->GetSelectedProject();
		Kargono::Utility::AsyncBusyTimer::CloseAllTimers();
		engine.Terminate();

		// Change project path
		if (projectPath.empty())
		{
			return;
		}
		
		// Init editor
		EngineConfig editorSpec;
		editorSpec.m_ExecutableName = "Kargono Editor";
		editorSpec.m_CmlArgs = args;
		editorSpec.m_WorkingDirectory = std::filesystem::current_path();
		editorSpec.m_DefaultWindowDimensions = { 1600, 900 };
		Application* editorApp = new EditorApp(projectPath);
		engine.Init(editorSpec, editorApp);
		KG_VERIFY(editorApp, "Editor App Init");

		// Run editor
		engine.GetThread().RunThread();

		// Close editor
		Kargono::Utility::AsyncBusyTimer::CloseAllTimers();
		engine.Terminate();
	}
}

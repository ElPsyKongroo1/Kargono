#include "Kargono.h"
#ifndef KG_TESTING
#include "Kargono/Core/EntryPoint.h"
#endif

#include "EditorApp.h"

namespace Kargono
{
	void InitEngineAndCreateApp(CommandLineArguments args)
	{
		EngineSpec spec;
		spec.Name = "Kargono Editor";
		spec.CommandLineArgs = args;
		spec.WorkingDirectory = std::filesystem::current_path();
		spec.DefaultWindowWidth = 1600;
		spec.DefaultWindowHeight = 900;

		Application* editorApp = new EditorApp();
		KG_VERIFY(editorApp, "Editor App Init");
		EngineService::Init(spec, editorApp);
	}
}

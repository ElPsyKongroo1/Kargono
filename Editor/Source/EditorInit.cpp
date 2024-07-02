#include "Kargono.h"
#ifndef KG_TESTING
#include "Kargono/Core/EntryPoint.h"
#endif

#include "EditorApp.h"

namespace Kargono
{
	Engine* InitEngineAndCreateApp(CommandLineArguments args)
	{
		EngineSpec spec;
		spec.Name = "Kargono Editor";
		spec.CommandLineArgs = args;
		spec.WorkingDirectory = std::filesystem::current_path();
		spec.DefaultWindowWidth = 1600;
		spec.DefaultWindowHeight = 900;

		Application* editorApp = new EditorApp();
		KG_VERIFY(editorApp, "Editor App Init");
		return new Kargono::Engine(spec, editorApp);
	}
}

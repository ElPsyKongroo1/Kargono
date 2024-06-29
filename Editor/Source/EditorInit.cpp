#include "Kargono.h"
#ifndef KG_TESTING
#include "Kargono/Core/EntryPoint.h"
#endif

#include "EditorApp.h"

namespace Kargono
{
	EngineCore* InitEngineAndCreateApp(CommandLineArgs args)
	{
		AppSpec spec;
		spec.Name = "Kargono Editor";
		spec.CommandLineArgs = args;
		spec.WorkingDirectory = std::filesystem::current_path();
		spec.Width = 1600;
		spec.Height = 900;

		Application* editorApp = new EditorApp();
		KG_VERIFY(editorApp, "Editor App Init");
		return new Kargono::EngineCore(spec, editorApp);
	}
}

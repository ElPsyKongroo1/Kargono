#include <Kargono.h>
#include "Kargono/Core/EntryPoint.h"

#include "RuntimeApp.h"

namespace Kargono
{
	//============================================================
	// CreateApplication Function
	//============================================================
	// This function is defined in the engine in Core/Application.h.
	//		This function is linked by the linker and provides an external
	//		method for starting the application.
	EngineCore* InitEngineAndCreateApp(CommandLineArgs args)
	{
		AppSpec spec;
		spec.Name = "Runtime";
		spec.CommandLineArgs = args;
		spec.WorkingDirectory = std::filesystem::current_path();
		spec.Width = 1600;
		spec.Height = 900;

		Application* editorApp = new RuntimeApp();
		return new Kargono::EngineCore(spec, editorApp);
	}
}


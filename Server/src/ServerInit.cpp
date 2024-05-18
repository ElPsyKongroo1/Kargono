#include <Kargono.h>
#include "Kargono/Core/EntryPoint.h"

#include "ServerApp.h"

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
		spec.Name = "Server";
		spec.CommandLineArgs = args;
		spec.WorkingDirectory = std::filesystem::current_path();
		spec.Width = 0;
		spec.Height = 0;

		Application* editorApp = new ServerApp();
		return new Kargono::EngineCore(spec, editorApp);
	}
}

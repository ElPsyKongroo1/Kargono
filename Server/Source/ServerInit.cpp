#include <Kargono.h>
#ifndef KG_TESTING
#include "Kargono/Core/EntryPoint.h"
#endif

#include "ServerApp.h"

namespace Kargono
{
	//============================================================
	// CreateApplication Function
	//============================================================
	// This function is defined in the engine in Core/Application.h.
	//		This function is linked by the linker and provides an external
	//		method for starting the application.
	void InitEngineAndCreateApp(CommandLineArguments args)
	{
		EngineSpec spec;
		spec.Name = "Server";
		spec.CommandLineArgs = args;
		spec.WorkingDirectory = std::filesystem::current_path();
		spec.DefaultWindowWidth = 0;
		spec.DefaultWindowHeight = 0;

		Application* serverApp = new ServerApp();
		KG_VERIFY(serverApp, "Server App Init");
		EngineService::Init(spec, serverApp);
	}
}

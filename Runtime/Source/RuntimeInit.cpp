#include <Kargono.h>
#ifndef KG_TESTING
#include "Kargono/Core/EntryPoint.h"
#endif

#include "RuntimeApp.h"

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
		spec.Name = "Runtime";
		spec.CommandLineArgs = args;
		spec.WorkingDirectory = std::filesystem::current_path();
		spec.DefaultWindowWidth = 1600;
		spec.DefaultWindowHeight = 900;

		Application* runtimeApp = new RuntimeApp();
		KG_VERIFY(runtimeApp, "Runtime App Init");
		EngineService::Init(spec, runtimeApp);
	}
}


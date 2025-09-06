#include <Kargono.h>
#ifndef KG_TESTING
#include "Modules/Core/EntryPoint.h"
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
	void ExecutableEntryPoint(Engine& engine, CommandLineArguments args)
	{
		// Init runtime
		EngineConfig spec;
		spec.m_ExecutableName = "Runtime";
		spec.m_CmlArgs = args;
		spec.m_WorkingDirectory = std::filesystem::current_path();
		spec.m_DefaultWindowDimensions = { 1600, 900 };
		Application* runtimeApp = new RuntimeApp();
		KG_VERIFY(runtimeApp, "Runtime App Init");
		if (!engine.Init(spec, runtimeApp))
		{
			// Close runtime
			Kargono::Utility::AsyncBusyTimerService::GetActiveBusyTimerContext().CloseAllTimers();
			engine.Terminate();
			return;
		}

		// Run runtime
		engine.GetThread().RunThread();

		// Close runtime
		Kargono::Utility::AsyncBusyTimerService::GetActiveBusyTimerContext().CloseAllTimers();
		engine.Terminate();
	}
}


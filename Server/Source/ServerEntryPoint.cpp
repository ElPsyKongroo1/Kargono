#include <Kargono.h>
#ifndef KG_TESTING
#include "Modules/Core/EntryPoint.h"
#endif

#include "ServerApp.h"
#include "Modules/Network/Server.h"

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
		// Init server
		EngineConfig spec;
		spec.m_ExecutableName = "Server";
		spec.m_CmlArgs = args;
		spec.m_WorkingDirectory = std::filesystem::current_path();
		spec.m_DefaultWindowDimensions = {0, 0};
		Application* serverApp = new ServerApp();
		KG_VERIFY(serverApp, "Server App Init");
		if (!engine.Init(spec, serverApp))
		{
			// Close server
			Kargono::Utility::AsyncBusyTimer::CloseAllTimers();
			engine.Terminate();
		}

		// Wait on the server to close
		Network::Server& server = Network::ServerService::GetActiveServer();
		server.WaitOnThreads();

		// Close server
		Kargono::Utility::AsyncBusyTimer::CloseAllTimers();
		engine.Terminate();
	}
}

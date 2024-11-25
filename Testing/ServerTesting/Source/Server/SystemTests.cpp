#include "doctest.h"

#include "Kargono/Utility/Timers.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Projects/Project.h"
#include "ServerApp.h"

bool InitializeEngine()
{
	Kargono::EngineSpec spec;
	spec.Name = "Server";
	spec.CommandLineArgs = { 0, nullptr };
	spec.WorkingDirectory = std::filesystem::current_path();
	spec.DefaultWindowWidth = 0;
	spec.DefaultWindowHeight = 0;

	Kargono::Application* serverApp = new Kargono::ServerApp("../Projects/TestProject/TestProject.kproj");
	if (!serverApp)
	{
		return false;
	}
	Kargono::EngineService::Init(spec, serverApp);
	return true;
}


TEST_CASE("Initialization and Termination")
{
	CHECK(InitializeEngine());
	CHECK(Kargono::Utility::AsyncBusyTimer::CloseAllTimers());
	CHECK(Kargono::EngineService::Terminate());
}

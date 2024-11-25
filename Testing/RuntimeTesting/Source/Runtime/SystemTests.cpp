#include "doctest.h"

#include "Kargono/Utility/Timers.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Core/Engine.h"
#include "RuntimeApp.h"

bool InitializeEngine()
{
	Kargono::EngineSpec spec;
	spec.Name = "Runtime";
	spec.CommandLineArgs = { 0, nullptr };
	spec.WorkingDirectory = std::filesystem::current_path();
	spec.DefaultWindowWidth = 1600;
	spec.DefaultWindowHeight = 900;

	Kargono::Application* runtimeApp = new Kargono::RuntimeApp("../Projects/TestProject/TestProject.kproj", true);
	if (!runtimeApp)
	{
		return false;
	}
	Kargono::EngineService::Init(spec, runtimeApp);
	return true;
}


TEST_CASE("Initialization and Termination")
{
	CHECK(InitializeEngine());
	CHECK(Kargono::Utility::AsyncBusyTimer::CloseAllTimers());
	CHECK(Kargono::EngineService::Terminate());
}

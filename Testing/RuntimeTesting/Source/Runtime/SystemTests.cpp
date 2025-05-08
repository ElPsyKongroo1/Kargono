#include "doctest.h"

#include "Kargono/Utility/Timers.h"
#include "Kargono/Projects/Project.h"
#include "CoreModule/Engine.h"
#include "RuntimeApp.h"

bool InitializeEngine(Kargono::Engine& engine)
{
	Kargono::EngineConfig spec;
	spec.m_ExecutableName = "Runtime";
	spec.m_CmlArgs = { 0, nullptr };
	spec.m_WorkingDirectory = std::filesystem::current_path();
	spec.m_DefaultWindowDimensions = {1600, 900};

	Kargono::Application* runtimeApp = new Kargono::RuntimeApp("../Projects/TestProject/TestProject.kproj", true);
	if (!runtimeApp)
	{
		return false;
	}
	engine.Init(spec, runtimeApp);
	return true;
}


TEST_CASE("Initialization and Termination")
{
	Kargono::Engine engine;
	Kargono::EngineService::SetActiveEngine(&engine);
	CHECK(InitializeEngine(engine));
	CHECK(Kargono::Utility::AsyncBusyTimer::CloseAllTimers());
	CHECK(engine.Terminate());
}

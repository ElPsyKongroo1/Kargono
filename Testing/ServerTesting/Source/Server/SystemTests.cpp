#include "doctest.h"

#include "Kargono/Utility/Timers.h"
#include "Modules/Core/Engine.h"
#include "Kargono/Projects/Project.h"
#include "ServerApp.h"

bool InitializeEngine(Kargono::Engine& engine)
{
	Kargono::EngineConfig spec;
	spec.m_ExecutableName = "Server";
	spec.m_CmlArgs = { 0, nullptr };
	spec.m_WorkingDirectory = std::filesystem::current_path();
	spec.m_DefaultWindowDimensions = {0, 0};

	Kargono::Application* serverApp = new Kargono::ServerApp("../Projects/TestProject/TestProject.kproj");
	if (!serverApp)
	{
		return false;
	}
	engine.Init(spec, serverApp);
	return true;
}


TEST_CASE("Initialization and Termination")
{
	Kargono::Engine engine;
	Kargono::EngineService::SetActiveEngine(&engine);
	CHECK(InitializeEngine(engine));
	CHECK(Kargono::Utility::AsyncBusyTimerService::GetActiveBusyTimerContext().CloseAllTimers());
	CHECK(engine.Terminate());
}

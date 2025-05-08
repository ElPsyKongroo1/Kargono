#include "doctest.h"

#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/Timers.h"
#include "CoreModule/Engine.h"
#include "EditorApp.h"


bool InitializeEngine(Kargono::Engine& engine)
{
	std::filesystem::path projectPath{};
	
	projectPath = Kargono::Projects::ProjectService::CreateNewProject("TestProject", "./../Projects");
	if (projectPath.empty())
	{
		return false;
	}
	projectPath = "../Projects/TestProject/TestProject.kproj";

	Kargono::EngineConfig editorSpec;
	editorSpec.m_ExecutableName = "Kargono Editor";
	editorSpec.m_CmlArgs = { 0, nullptr };
	editorSpec.m_WorkingDirectory = std::filesystem::current_path();
	editorSpec.m_DefaultWindowDimensions = { 1600, 900};
	Kargono::Application* editorApp = new Kargono::EditorApp(projectPath);
	engine.Init(editorSpec, editorApp);
	KG_VERIFY(editorApp, "Editor App Init");
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

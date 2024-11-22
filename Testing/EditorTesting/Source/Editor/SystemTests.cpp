#include "doctest.h"

#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/Timers.h"
#include "Kargono/Core/Engine.h"
#include "EditorApp.h"


bool InitializeEngine()
{
	std::filesystem::path projectPath{};
	
	projectPath = Kargono::Projects::ProjectService::CreateNewProject("TestProject", "./../Projects");
	if (projectPath.empty())
	{
		return false;
	}
	projectPath = "../Projects/TestProject/TestProject.kproj";

	Kargono::EngineSpec editorSpec;
	editorSpec.Name = "Kargono Editor";
	editorSpec.CommandLineArgs = { 0, nullptr };
	editorSpec.WorkingDirectory = std::filesystem::current_path();
	editorSpec.DefaultWindowWidth = 1600;
	editorSpec.DefaultWindowHeight = 900;
	Kargono::Application* editorApp = new Kargono::EditorApp(projectPath);
	Kargono::EngineService::Init(editorSpec, editorApp);
	KG_VERIFY(editorApp, "Editor App Init");
	return true;
}

TEST_CASE("Initialization and Termination")
{
	CHECK(InitializeEngine());
	CHECK(Kargono::Utility::AsyncBusyTimer::CloseAllTimers());
	CHECK(Kargono::EngineService::Terminate());
}

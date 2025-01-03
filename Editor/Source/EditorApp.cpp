#include "EditorApp.h"

#include "Kargono/Scripting/ScriptCompilerService.h"
#include "Kargono/Utility/FileDialogs.h"
#include "Kargono/Audio/Audio.h"
#include "Kargono/Scenes/Scene.h"
#include "Kargono/EditorUI/EditorUI.h"
#include "Kargono/RuntimeUI/Font.h"
#include "Kargono/RuntimeUI/RuntimeUI.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/AI/AIService.h"
#include "Kargono/Rendering/RenderingService.h"
#include "Kargono/Events/PhysicsEvent.h"
#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Particles/ParticleService.h"

#include "API/EditorUI/ImGuiBackendAPI.h"

namespace Kargono
{

	EditorApp::EditorApp(const std::filesystem::path& projectPath)
		: Application("EditorLayer"), m_InitProjectPath(projectPath)
	{
		// Ensure that the project path is valid and ensure that the editor app is the only instance
		KG_ASSERT(!m_InitProjectPath.empty(), "Attempt to open editor without valid project path!");
		KG_ASSERT(!s_EditorApp);

		// Set static editor app pointer
		s_EditorApp = this;
	}


	void EditorApp::Init()
	{
		// Initialize engine services
		Scripting::ScriptService::Init();
		Audio::AudioService::Init();
		Scenes::SceneService::Init();

		// Create editor app windows
		m_MainWindow = CreateScope<Windows::MainWindow>();
		m_ActiveEditorWindow = ActiveEditorUIWindow::MainWindow;
		
		// Open project provided by launcher app
		OpenProject(m_InitProjectPath);
		
		// Initialize other various engine services
		EditorUI::EditorUIService::Init();
		AI::AIService::Init();
		Rendering::RenderingService::Init();
		Rendering::RenderingService::SetLineWidth(1.0f);
		RuntimeUI::FontService::Init();
		RuntimeUI::RuntimeUIService::Init();
		Particles::ParticleService::Init();

		// Initialize panels
		m_MainWindow->InitPanels();
		m_UIEditorWindow = CreateScope<Windows::UIEditorWindow>();
		m_UIEditorWindow->InitPanels();

		// Open operating system window
		EngineService::GetActiveWindow().SetVisible(true);
	}

	void EditorApp::Terminate()
	{
		// Terminate engine services
		EditorUI::EditorUIService::Terminate();
		RuntimeUI::RuntimeUIService::Terminate();
		Particles::ParticleService::Terminate();
		Audio::AudioService::Terminate();
		Scripting::ScriptService::Terminate();
		AI::AIService::Terminate();
		Scripting::ScriptCompilerService::Terminate();
		Assets::AssetService::ClearAll();
		RuntimeUI::FontService::Terminate();
		Scenes::SceneService::Terminate();
		Rendering::RenderingService::Shutdown();

		m_MainWindow.reset();
		m_UIEditorWindow.reset();
	}

	void EditorApp::OnUpdate(Timestep ts)
	{
		KG_PROFILE_FUNCTION();

		// Call on update for all windows
		m_MainWindow->OnUpdate(ts);

		// Handle rendering editor UI
		switch (m_ActiveEditorWindow)
		{
		case ActiveEditorUIWindow::MainWindow:
			m_MainWindow->OnEditorUIRender();
			break;
		case ActiveEditorUIWindow::UIEditorWindow:
			m_UIEditorWindow->OnUpdate(ts);
			m_UIEditorWindow->OnEditorUIRender();
			break;
		case ActiveEditorUIWindow::None:
		default:
			KG_ERROR("Invalid window type provided to OnUpdate(ts) function");
			break;
		}
	}

	bool EditorApp::OnApplicationEvent(Events::Event* event)
	{
		return m_MainWindow->OnApplicationEvent(event);
	}

	bool EditorApp::OnNetworkEvent(Events::Event* event)
	{
		return m_MainWindow->OnNetworkEvent(event);
	}

	bool EditorApp::OnInputEvent(Events::Event* event)
	{
		// Handle editor UI input events
		bool handled = false;
		handled = EditorUI::EditorUIService::OnInputEvent(event);
		if (handled)
		{
			return true;
		}
		// Handle main window specific input events
		switch (m_ActiveEditorWindow)
		{
		case ActiveEditorUIWindow::MainWindow:
			return m_MainWindow->OnInputEvent(event);
		case ActiveEditorUIWindow::UIEditorWindow:
			return m_UIEditorWindow->OnInputEvent(event);
		case ActiveEditorUIWindow::None:
		default:
			KG_ERROR("Invalid window type provided to OnInputEvent(event) function");
			break;
		}


		return m_MainWindow->OnInputEvent(event);
	}

	bool EditorApp::OnPhysicsEvent(Events::Event* event)
	{
		bool handled = false;
		switch (event->GetEventType())
		{
		case Events::EventType::PhysicsCollisionStart:
			handled = OnPhysicsCollisionStart(*(Events::PhysicsCollisionStart*)event);
			break;
		case Events::EventType::PhysicsCollisionEnd:
			handled = OnPhysicsCollisionEnd(*(Events::PhysicsCollisionEnd*)event);
			break;
		}
		return handled;
	}

	bool EditorApp::OnSceneEvent(Events::Event* event)
	{
		return m_MainWindow->OnSceneEvent(event);
	}

	bool EditorApp::OnEditorEvent(Events::Event* event)
	{
		return m_MainWindow->OnEditorEvent(event);
	}

	bool EditorApp::OnAssetEvent(Events::Event* event)
	{
		return m_MainWindow->OnAssetEvent(event);
	}

	bool EditorApp::OnLogEvent(Events::Event* event)
	{
		Events::LogEvent* logEvent = (Events::LogEvent*)event;

		switch (logEvent->GetEventLevel())
		{
		case Events::LogEventLevel::Info:
			EditorUI::EditorUIService::CreateInfoNotification(logEvent->GetEventText().c_str(), 7000);
			break;
		case Events::LogEventLevel::Warning:
			EditorUI::EditorUIService::CreateWarningNotification(logEvent->GetEventText().c_str(), 7000);
			break;
		case Events::LogEventLevel::Critical:
			EditorUI::EditorUIService::CreateCriticalNotification(logEvent->GetEventText().c_str(), 7000);
			break;
		case Events::LogEventLevel::None:
		default:
			// TODO: Yee, I realize this is a potential circular issue. ehh...
			KG_ERROR("Invalid log event type provided to OnLogEvent()")
			break;
		}
		return false;
	}


	bool EditorApp::OnPhysicsCollisionStart(Events::PhysicsCollisionStart event)
	{
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
		UUID entityOneID = event.GetEntityOne();
		ECS::Entity entityOne = activeScene->GetEntityByUUID(entityOneID);
		UUID entityTwoID = event.GetEntityTwo();
		ECS::Entity entityTwo = activeScene->GetEntityByUUID(entityTwoID);

		KG_ASSERT(entityOne);
		KG_ASSERT(entityTwo);

		bool collisionHandled = false;
		if (entityOne.HasComponent<ECS::Rigidbody2DComponent>())
		{
			ECS::Rigidbody2DComponent& component = entityOne.GetComponent<ECS::Rigidbody2DComponent>();
			Assets::AssetHandle scriptHandle = component.OnCollisionStartScriptHandle;
			Scripting::Script* script = component.OnCollisionStartScript.get();
			if (scriptHandle != Assets::EmptyHandle)
			{
				collisionHandled = Utility::CallWrappedBoolEntityEntity(script->m_Function, entityOneID, entityTwoID);
			}
		}

		if (!collisionHandled && entityTwo.HasComponent<ECS::Rigidbody2DComponent>())
		{
			ECS::Rigidbody2DComponent& component = entityTwo.GetComponent<ECS::Rigidbody2DComponent>();
			Assets::AssetHandle scriptHandle = component.OnCollisionStartScriptHandle;
			Scripting::Script* script = component.OnCollisionStartScript.get();
			if (scriptHandle != Assets::EmptyHandle)
			{
				collisionHandled = Utility::CallWrappedBoolEntityEntity(script->m_Function, entityTwoID, entityOneID);
			}
		}
		return false;
	}

	bool EditorApp::OnPhysicsCollisionEnd(Events::PhysicsCollisionEnd event)
	{
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
		UUID entityOneID = event.GetEntityOne();
		ECS::Entity entityOne = activeScene->GetEntityByUUID(entityOneID);
		UUID entityTwoID = event.GetEntityTwo();
		ECS::Entity entityTwo = activeScene->GetEntityByUUID(entityTwoID);

		KG_ASSERT(entityOne);
		KG_ASSERT(entityTwo);

		bool collisionHandled = false;
		if (entityOne.HasComponent<ECS::Rigidbody2DComponent>())
		{
			ECS::Rigidbody2DComponent& component = entityOne.GetComponent<ECS::Rigidbody2DComponent>();
			Assets::AssetHandle scriptHandle = component.OnCollisionEndScriptHandle;
			Scripting::Script* script = component.OnCollisionEndScript.get();
			if (scriptHandle != Assets::EmptyHandle)
			{
				Utility::CallWrappedBoolEntityEntity(script->m_Function, entityOneID, entityTwoID);
			}
		}

		if (!collisionHandled && entityOne.HasComponent<ECS::Rigidbody2DComponent>())
		{
			ECS::Rigidbody2DComponent& component = entityTwo.GetComponent<ECS::Rigidbody2DComponent>();
			Assets::AssetHandle scriptHandle =  component.OnCollisionEndScriptHandle;
			Scripting::Script* script = component.OnCollisionEndScript.get();
			if (scriptHandle != Assets::EmptyHandle)
			{
			 	collisionHandled = Utility::CallWrappedBoolEntityEntity(script->m_Function, entityTwoID, entityOneID);
			}
		}
		return false;
	}


	bool EditorApp::OpenProject()
	{
		*Scenes::SceneService::GetActiveScene()->GetHoveredEntity() = {};
		std::filesystem::path initialDirectory = std::filesystem::current_path().parent_path() / "Projects";
		if (!Utility::FileSystem::PathExists(initialDirectory))
		{
			initialDirectory = "";
		}
		std::filesystem::path filepath = Utility::FileDialogs::OpenFile("Kargono Project (*.kproj)\0*.kproj\0", initialDirectory.string().c_str());
		if (filepath.empty())
		{
			return false;
		}

		OpenProject(filepath);

		// Reset all resources
		m_MainWindow->m_ContentBrowserPanel->ResetPanelResources();
		m_MainWindow->m_TextEditorPanel->ResetPanelResources();
		m_MainWindow->m_InputMapPanel->ResetPanelResources();
		m_MainWindow->m_GameStatePanel->ResetPanelResources();
		m_MainWindow->m_ScriptEditorPanel->ResetPanelResources();
		m_MainWindow->m_ProjectPanel->ResetPanelResources();
		Scenes::GameStateService::ClearActiveGameState();
		Input::InputMapService::ClearActiveInputMap();

		return true;
	}

	void EditorApp::OpenProject(const std::filesystem::path& path)
	{
		if (Projects::ProjectService::OpenProject(path))
		{
			if (!EngineService::GetActiveWindow().GetNativeWindow())
			{
				EngineService::GetActiveWindow().Init();
				Rendering::RendererAPI::Init();
			}
			Assets::AssetHandle startSceneHandle = Projects::ProjectService::GetActiveStartSceneHandle();

			Scripting::ScriptService::LoadActiveScriptModule();

			if (m_MainWindow->m_EditorScene)
			{
				m_MainWindow->m_EditorScene->DestroyAllEntities();
			}
			Assets::AssetService::ClearAll();
			Assets::AssetService::DeserializeAll();
			if (startSceneHandle == Assets::EmptyHandle)
			{
				m_MainWindow->NewScene("NewScene");
				Projects::ProjectService::SetActiveStartingSceneHandle(m_MainWindow->m_EditorSceneHandle);
				SaveProject();
			}
			else
			{
				m_MainWindow->OpenScene(startSceneHandle);
			}
		}
	}

	void EditorApp::SaveProject()
	{
		Projects::ProjectService::SaveActiveProject();
	}

}

#include "EditorApp.h"

#include "Modules/Scripting/ScriptCompilerService.h"
#include "Kargono/Utility/FileDialogs.h"
#include "Modules/Audio/Audio.h"
#include "Kargono/Scenes/Scene.h"
#include "Modules/EditorUI/EditorUI.h"
#include "Modules/RuntimeUI/Font.h"
#include "Modules/RuntimeUI/RuntimeUI.h"
#include "Modules/Core/Engine.h"
#include "Modules/AI/AIService.h"
#include "Modules/Rendering/RenderingService.h"
#include "Modules/Events/PhysicsEvent.h"
#include "Modules/Events/ApplicationEvent.h"
#include "Kargono/Projects/Project.h"
#include "Modules/Particles/ParticleService.h"
#include "Modules/Input/InputService.h"
#include "Modules/Network/Client.h"

#include "Modules/EditorUI/ExternalAPI/ImGuiBackendAPI.h"

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


	bool EditorApp::Init()
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
		Particles::ParticleService::CreateParticleContext();
		Particles::ParticleService::GetActiveContext().Init();
		EditorUI::EditorUIService::Init();
		AI::AIService::CreateAIContext();
		AI::AIService::GetActiveContext().Init();
		Rendering::RenderingService::Init();
		Rendering::RenderingService::SetLineWidth(1.0f);
		RuntimeUI::FontService::Init();
		RuntimeUI::RuntimeUIService::Init();

		// Initialize panels
		m_MainWindow->InitPanels();
		m_UIEditorWindow = CreateScope<Windows::UIEditorWindow>();
		m_UIEditorWindow->InitPanels();
		m_EmitterConfigEditorWindow = CreateScope<Windows::EmitterConfigWindow>();
		m_EmitterConfigEditorWindow->InitPanels();

		// Open operating system window
		EngineService::GetActiveEngine().GetWindow().SetVisible(true);

		return true;
	}

	bool EditorApp::Terminate()
	{
		// Close all network threads
		if (Network::ClientService::IsClientActive())
		{
			Network::ClientService::Terminate();
		}

		if (Network::ServerService::IsServerActive())
		{
			Network::ServerService::Terminate();
		}

		// Terminate engine services
		EditorUI::EditorUIService::Terminate();
		RuntimeUI::RuntimeUIService::Terminate();
		Particles::ParticleService::GetActiveContext().Terminate();
		Particles::ParticleService::RemoveParticleContext();
		Audio::AudioService::Terminate();
		Scripting::ScriptService::Terminate();
		AI::AIService::GetActiveContext().Terminate();
		AI::AIService::RemoveAIContext();
		Scripting::ScriptCompilerService::Terminate();
		Assets::AssetService::ClearAll();
		RuntimeUI::FontService::Terminate();
		Scenes::SceneService::Terminate();
		Rendering::RenderingService::Shutdown();

		m_MainWindow.reset();
		m_UIEditorWindow.reset();
		m_EmitterConfigEditorWindow.reset();

		return true;
	}

	void EditorApp::OnUpdate(Timestep ts)
	{
		KG_PROFILE_FUNCTION();

		// Call on update for all windows

		// Handle rendering editor UI
		switch (m_ActiveEditorWindow)
		{
		case ActiveEditorUIWindow::MainWindow:
			m_MainWindow->OnUpdate(ts);
			m_MainWindow->OnEditorUIRender();
			break;
		case ActiveEditorUIWindow::UIEditorWindow:
			m_UIEditorWindow->OnUpdate(ts);
			m_UIEditorWindow->OnEditorUIRender();
			break;
		case ActiveEditorUIWindow::EmitterConfigWindow:
			m_EmitterConfigEditorWindow->OnUpdate(ts);
			m_EmitterConfigEditorWindow->OnEditorUIRender();
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

		// Handle editor UI mouse capture
		handled = EditorUI::EditorUIService::OnInputEvent(event);
		if (handled)
		{
			return true;
		}
		
		// Handle application-wide key pressed
		if (event->GetEventType() == Events::EventType::KeyPressed)
		{
			handled = OnKeyPressedEvent(*(Events::KeyPressedEvent*)event);
		}

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
		case ActiveEditorUIWindow::EmitterConfigWindow:
			return m_EmitterConfigEditorWindow->OnInputEvent(event);
		case ActiveEditorUIWindow::None:
		default:
			KG_ERROR("Invalid window type provided to OnInputEvent(event) function");
			break;
		}

		return false;
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
		Particles::ParticleService::GetActiveContext().OnSceneEvent(event);
		m_MainWindow->OnSceneEvent(event);
		return false;
	}

	bool EditorApp::OnEditorEvent(Events::Event* event)
	{
		return m_MainWindow->OnEditorEvent(event);
	}

	bool EditorApp::OnAssetEvent(Events::Event* event)
	{
		m_MainWindow->OnAssetEvent(event);
		m_UIEditorWindow->OnAssetEvent(event);
		m_EmitterConfigEditorWindow->OnAssetEvent(event);
		return false;
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

	bool EditorApp::OnKeyPressedEvent(Events::KeyPressedEvent event)
	{
		// Handle general keyboard input chords
		bool control = Input::InputService::IsKeyPressed(Key::LeftControl) || Input::InputService::IsKeyPressed(Key::RightControl);

		if (event.GetKeyCode() == Key::Z && control)
		{
			EditorUI::EditorUIService::Undo();
			return true;
		}

		return false;
	}

	Math::vec2 EditorApp::GetMouseViewportPosition()
	{
		switch (m_ActiveEditorWindow)
		{
		case ActiveEditorUIWindow::MainWindow: 
			return m_MainWindow->m_ViewportPanel->GetMouseViewportPosition();
		case ActiveEditorUIWindow::UIEditorWindow:
			return m_UIEditorWindow->m_ViewportPanel->GetMouseViewportPosition();
		default:
			KG_WARN("Getting mouse position when no active viewport is available");
			return { 0.0f, 0.0f };
		}
	}

	ViewportData* EditorApp::GetViewportData()
	{
		switch (m_ActiveEditorWindow)
		{
		case ActiveEditorUIWindow::MainWindow:
			return m_MainWindow->m_ViewportPanel->GetViewportData();
		case ActiveEditorUIWindow::UIEditorWindow:
			return m_UIEditorWindow->m_ViewportPanel->GetViewportData();
		default:
			KG_WARN("Getting viewport data when no active viewport is available");
			return nullptr;
		}
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
			if (!EngineService::GetActiveEngine().GetWindow().GetNativeWindow())
			{
				EngineService::GetActiveEngine().GetWindow().Init();
				Rendering::RendererAPI::Init();
			}
			Assets::AssetHandle startSceneHandle = Projects::ProjectService::GetActiveStartSceneHandle();

			// Load in the script shared library
			Scripting::ScriptService::LoadActiveScriptModule();

			if (m_MainWindow->m_EditorScene)
			{
				m_MainWindow->m_EditorScene->DestroyAllEntities();
			}
			Assets::AssetService::ClearAll();
			Assets::AssetService::DeserializeAll();

			// Ensure all script assets are properly loaded in
			Assets::AssetService::LoadAllScriptIntoCache();

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

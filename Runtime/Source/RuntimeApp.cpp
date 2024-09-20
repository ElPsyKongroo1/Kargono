#include "kgpch.h"
#include "RuntimeApp.h"

#include <filesystem>

namespace Kargono
{
	// Final Export Values
	const std::filesystem::path logoPath = "./ProjectIcon.png";

	RuntimeApp::RuntimeApp()
		: Application("RuntimeLayer")
	{
	}

	void RuntimeApp::Init()
	{
		Scripting::ScriptService::Init();
		Audio::AudioService::Init();
		Scenes::SceneService::Init();

#ifdef KG_TESTING
		Audio::AudioService::SetMute(true);
#endif
		auto& currentWindow = EngineService::GetActiveWindow();

		Scenes::SceneService::SetActiveScene(CreateRef<Scenes::Scene>(), Assets::EmptyHandle);

#ifdef KG_TESTING
		OpenProject("../Projects/Pong/Pong.kproj");
#elif defined KG_EXPORT
		std::filesystem::path pathToProject = Utility::FileSystem::FindFileWithExtension(
			std::filesystem::current_path(),
			".kproj");
		if (pathToProject.empty())
		{
			KG_CRITICAL("Could not locate a .kproj file in local directory!");
			EngineService::EndRun();
			return;
		}
		OpenProject(pathToProject);
		if (!Projects::ProjectService::GetActive())
		{
			KG_CRITICAL("Failed to open project!");
			EngineService::EndRun();
			return;
		}
#else
		if (!OpenProject())
		{
			EngineService::EndRun();
			return;
		}
#endif

		
		
#ifndef KG_TESTING
		Projects::ProjectService::GetActiveIsFullscreen() ? currentWindow.SetFullscreen(true) : currentWindow.SetFullscreen(false);
		currentWindow.ResizeWindow(Utility::ScreenResolutionToVec2(Projects::ProjectService::GetActiveTargetResolution()));
		currentWindow.SetResizable(false);
#endif

		Rendering::RenderingService::Init();
		Rendering::RenderingService::SetLineWidth(4.0f);
		RuntimeUI::FontService::Init();
		RuntimeUI::RuntimeUIService::Init();

		OnPlay();
		currentWindow.SetVisible(true);
	}

	void RuntimeApp::Terminate()
	{
		OnStop();

		Audio::AudioService::Terminate();

	}

	void RuntimeApp::OnUpdate(Timestep ts)
	{
		// Render
		Rendering::RenderingService::ResetStats();
		Rendering::RendererAPI::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Rendering::RendererAPI::Clear();

		OnUpdateRuntime(ts);
		Scenes::Entity cameraEntity = Scenes::SceneService::GetActiveScene()->GetPrimaryCameraEntity();
		if (!cameraEntity)
		{
			return;
		}
		Rendering::Camera* mainCamera = &cameraEntity.GetComponent<Scenes::CameraComponent>().Camera;
		Math::mat4 cameraTransform = cameraEntity.GetComponent<Scenes::TransformComponent>().GetTransform();

		if (mainCamera)
		{
			RuntimeUI::RuntimeUIService::PushRenderData(glm::inverse(cameraTransform), 
				EngineService::GetActiveWindow().GetWidth(), EngineService::GetActiveWindow().GetHeight());
		}
	}

	bool RuntimeApp::OnApplicationEvent(Events::Event* event)
	{
		bool handled = false;
		switch (event->GetEventType())
		{
		case Events::EventType::WindowResize:
			handled = OnWindowResize(*(Events::WindowResizeEvent*)event);
			break;
		case Events::EventType::AppClose:
			handled = OnApplicationClose(*(Events::ApplicationCloseEvent*)event);
			break;
		}
		return handled;
	}

	bool RuntimeApp::OnNetworkEvent(Events::Event* event)
	{
		bool handled = false;
		switch (event->GetEventType())
		{
		case Events::EventType::UpdateOnlineUsers:
			handled = OnUpdateUserCount(*(Events::UpdateOnlineUsers*)event);
			break;
		case Events::EventType::ApproveJoinSession:
			handled = OnApproveJoinSession(*(Events::ApproveJoinSession*)event);
			break;
		case Events::EventType::UserLeftSession:
			handled = OnUserLeftSession(*(Events::UserLeftSession*)event);
			break;
		case Events::EventType::CurrentSessionInit:
			handled = OnCurrentSessionInit(*(Events::CurrentSessionInit*)event);
			break;
		case Events::EventType::ConnectionTerminated:
			handled = OnConnectionTerminated(*(Events::ConnectionTerminated*)event);
			break;
		case Events::EventType::UpdateSessionUserSlot:
			handled = OnUpdateSessionUserSlot(*(Events::UpdateSessionUserSlot*)event);
			break;
		case Events::EventType::StartSession:
			handled = OnStartSession(*(Events::StartSession*)event);
			break;
		case Events::EventType::SendReadyCheckConfirm:
			handled = OnSessionReadyCheckConfirm(*(Events::SessionReadyCheckConfirm*)event);
			break;
		case Events::EventType::ReceiveSignal:
			handled = OnReceiveSignal(*(Events::ReceiveSignal*)event);
			break;
		}
		return handled;
	}

	bool RuntimeApp::OnInputEvent(Events::Event* event)
	{
		bool handled = false;
		if (event->GetEventType() == Events::EventType::KeyPressed)
		{
			handled = OnKeyPressed(*(Events::KeyPressedEvent*)event);
		}
		return handled;
	}

	bool RuntimeApp::OnPhysicsEvent(Events::Event* event)
	{
		bool handled = false;
		switch (event->GetEventType())
		{
		case Events::EventType::PhysicsCollisionStart:
			handled = OnPhysicsCollision(*(Events::PhysicsCollisionStart*)event);
			break;
		case Events::EventType::PhysicsCollisionEnd:
			handled = OnPhysicsCollisionEnd(*(Events::PhysicsCollisionEnd*)event);
			break;
		}
		return handled;
	}

	bool RuntimeApp::OnApplicationClose(Events::ApplicationCloseEvent event)
	{
		Events::WindowCloseEvent windowEvent {};
		Events::EventCallbackFn eventCallback = EngineService::GetActiveWindow().GetEventCallback();
		eventCallback(&windowEvent);
		return false;
	}

	bool RuntimeApp::OnWindowResize(Events::WindowResizeEvent event)
	{
		EngineService::GetActiveWindow().SetViewportWidth(event.GetWidth());
		EngineService::GetActiveWindow().SetViewportHeight(event.GetHeight());
		Scenes::SceneService::GetActiveScene()->OnViewportResize((uint32_t)event.GetWidth(), (uint32_t)event.GetHeight());
		return false;
	}

	bool RuntimeApp::OnPhysicsCollision(Events::PhysicsCollisionStart event)
	{
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
		UUID entityOneID = event.GetEntityOne();
		Scenes::Entity entityOne = activeScene->GetEntityByUUID(entityOneID);
		UUID entityTwoID = event.GetEntityTwo();
		Scenes::Entity entityTwo = activeScene->GetEntityByUUID(entityTwoID);

		KG_ASSERT(entityOne);
		KG_ASSERT(entityTwo);

		bool collisionHandled = false;
		if (entityOne.HasComponent<Scenes::ClassInstanceComponent>())
		{
			Scenes::ClassInstanceComponent& component = entityOne.GetComponent<Scenes::ClassInstanceComponent>();
			Assets::AssetHandle scriptHandle = component.ClassReference->GetScripts().OnPhysicsCollisionStartHandle;
			Scripting::Script* script = component.ClassReference->GetScripts().OnPhysicsCollisionStart;
			if (scriptHandle != Assets::EmptyHandle)
			{
				collisionHandled = ((WrappedBoolUInt64UInt64*)script->m_Function.get())->m_Value(entityOneID, entityTwoID);
			}
		}

		if (!collisionHandled && entityTwo.HasComponent<Scenes::ClassInstanceComponent>())
		{
			Scenes::ClassInstanceComponent& component = entityTwo.GetComponent<Scenes::ClassInstanceComponent>();
			Assets::AssetHandle scriptHandle = component.ClassReference->GetScripts().OnPhysicsCollisionStartHandle;
			Scripting::Script* script = component.ClassReference->GetScripts().OnPhysicsCollisionStart;
			if (scriptHandle != Assets::EmptyHandle)
			{
				collisionHandled = ((WrappedBoolUInt64UInt64*)script->m_Function.get())->m_Value(entityTwoID, entityOneID);
			}
		}
		return false;
	}

	bool RuntimeApp::OnPhysicsCollisionEnd(Events::PhysicsCollisionEnd event)
	{
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
		UUID entityOneID = event.GetEntityOne();
		Scenes::Entity entityOne = activeScene->GetEntityByUUID(entityOneID);
		UUID entityTwoID = event.GetEntityTwo();
		Scenes::Entity entityTwo = activeScene->GetEntityByUUID(entityTwoID);

		KG_ASSERT(entityOne);
		KG_ASSERT(entityTwo);

		bool collisionHandled = false;
		if (entityOne.HasComponent<Scenes::ClassInstanceComponent>())
		{
			Scenes::ClassInstanceComponent& component = entityOne.GetComponent<Scenes::ClassInstanceComponent>();
			Assets::AssetHandle scriptHandle = component.ClassReference->GetScripts().OnPhysicsCollisionEndHandle;
			Scripting::Script* script = component.ClassReference->GetScripts().OnPhysicsCollisionEnd;
			if (scriptHandle != Assets::EmptyHandle)
			{
				collisionHandled = ((WrappedBoolUInt64UInt64*)script->m_Function.get())->m_Value(entityOneID, entityTwoID);
			}
		}

		if (!collisionHandled && entityOne.HasComponent<Scenes::ClassInstanceComponent>())
		{
			Scenes::ClassInstanceComponent& component = entityTwo.GetComponent<Scenes::ClassInstanceComponent>();
			Assets::AssetHandle scriptHandle = component.ClassReference->GetScripts().OnPhysicsCollisionEndHandle;
			Scripting::Script* script = component.ClassReference->GetScripts().OnPhysicsCollisionEnd;
			if (scriptHandle != Assets::EmptyHandle)
			{
				collisionHandled = ((WrappedBoolUInt64UInt64*)script->m_Function.get())->m_Value(entityTwoID, entityOneID);
			}
		}
		return false;
	}

	bool RuntimeApp::OnKeyPressed(Events::KeyPressedEvent event)
	{
		Scenes::SceneService::GetActiveScene()->OnKeyPressed(event);
		return false;
	}

	void RuntimeApp::OnUpdateRuntime(Timestep ts)
	{
		// Update
		Scenes::SceneService::GetActiveScene()->OnUpdateInputMode(ts);
		Scenes::SceneService::GetActiveScene()->OnUpdateEntities(ts);
		Scenes::SceneService::GetActiveScene()->OnUpdatePhysics(ts);

		// Render 2D
		Scenes::Entity cameraEntity = Scenes::SceneService::GetActiveScene()->GetPrimaryCameraEntity();
		if (!cameraEntity)
		{
			return;
		}
		Rendering::Camera* mainCamera = &cameraEntity.GetComponent<Scenes::CameraComponent>().Camera;
		Math::mat4 cameraTransform = cameraEntity.GetComponent<Scenes::TransformComponent>().GetTransform();

		if (mainCamera)
		{
			// Transform Matrix needs to be inversed so that final view is from the perspective of the camera
			Scenes::SceneService::GetActiveScene()->RenderScene(*mainCamera, glm::inverse(cameraTransform));
		}
	}

	bool RuntimeApp::OnUpdateUserCount(Events::UpdateOnlineUsers event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnUpdateUserCount();
		if (scriptHandle != 0)
		{
			((WrappedVoidUInt32*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserCount());
		}
		return false;
	}

	bool RuntimeApp::OnApproveJoinSession(Events::ApproveJoinSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnApproveJoinSession();
		if (scriptHandle != 0)
		{
			((WrappedVoidUInt16*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserSlot());
		}
		return false;
	}

	bool RuntimeApp::OnUpdateSessionUserSlot(Events::UpdateSessionUserSlot event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnUpdateSessionUserSlot();
		if (scriptHandle != 0)
		{
			((WrappedVoidUInt16*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserSlot());
		}
		return false;
	}

	bool RuntimeApp::OnUserLeftSession(Events::UserLeftSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnUserLeftSession();
		if (scriptHandle != 0)
		{
			((WrappedVoidUInt16*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserSlot());
		}
		return false;
	}

	bool RuntimeApp::OnCurrentSessionInit(Events::CurrentSessionInit event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnCurrentSessionInit();
		if (scriptHandle != 0)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}
		return false;
	}

	bool RuntimeApp::OnConnectionTerminated(Events::ConnectionTerminated event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnConnectionTerminated();
		if (scriptHandle != 0)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}
		return false;
	}

	bool RuntimeApp::OnStartSession(Events::StartSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnStartSession();
		if (scriptHandle != 0)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}
		return false;
	}

	bool RuntimeApp::OnSessionReadyCheckConfirm(Events::SessionReadyCheckConfirm event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnSessionReadyCheckConfirm();
		if (scriptHandle != Assets::EmptyHandle)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}
		return false;
	}

	bool RuntimeApp::OnReceiveSignal(Events::ReceiveSignal event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnReceiveSignal();
		if (scriptHandle != Assets::EmptyHandle)
		{
			((WrappedVoidUInt16*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetSignal());
		}
		return false;
	}


	bool RuntimeApp::OpenProject()
	{
		std::filesystem::path initialDirectory = std::filesystem::current_path().parent_path() / "Projects";
		if (!std::filesystem::exists(initialDirectory))
		{
			initialDirectory = "";
		}
		std::filesystem::path filepath = Utility::FileDialogs::OpenFile("Kargono Project (*.kproj)\0*.kproj\0", initialDirectory.string().c_str());
		if (filepath.empty()) { return false; }

		OpenProject(filepath);
		return true;
	}

	void RuntimeApp::OpenProject(const std::filesystem::path& path)
	{
		if (Assets::AssetManager::OpenProject(path))
		{
			if (!EngineService::GetActiveWindow().GetNativeWindow())
			{
				Math::vec2 screenSize = Utility::ScreenResolutionToVec2(Projects::ProjectService::GetActiveTargetResolution());
				WindowProps projectProps =
				{
					Projects::ProjectService::GetActiveProjectName(),
					static_cast<uint32_t>(screenSize.x),
					static_cast<uint32_t>(screenSize.y)
				};
				if (std::filesystem::exists(logoPath))
				{
					EngineService::GetActiveWindow().Init(projectProps, logoPath);
				}
				else
				{
					EngineService::GetActiveWindow().Init(projectProps);
				}
				Rendering::RendererAPI::Init();
			}

			Assets::AssetHandle startSceneHandle = Projects::ProjectService::GetActiveStartSceneHandle();
			Scripting::ScriptService::LoadActiveScriptModule();

			if (Scenes::SceneService::GetActiveScene())
			{
				Scenes::SceneService::GetActiveScene()->DestroyAllEntities();
			}
			Assets::AssetManager::ClearAll();
			Assets::AssetManager::DeserializeAll();
			OpenScene(startSceneHandle);

		}
	}


	void RuntimeApp::OpenScene(Assets::AssetHandle sceneHandle)
	{
		Ref<Scenes::Scene> newScene = Assets::AssetManager::GetScene(sceneHandle);
		if (!newScene) { newScene = CreateRef<Scenes::Scene>(); }
		Scenes::SceneService::SetActiveScene(newScene, sceneHandle);
	}


	void RuntimeApp::OnPlay()
	{
		Scenes::SceneService::GetActiveScene()->OnRuntimeStart();
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnRuntimeStart();
		if (scriptHandle != 0)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}

		// Load Default Game State
		if (Projects::ProjectService::GetActiveStartGameState() == 0)
		{
			Scenes::GameStateService::ClearActiveGameState();
		}
		else
		{
			Scenes::GameStateService::SetActiveGameState(Assets::AssetManager::GetGameState(
				Projects::ProjectService::GetActiveStartGameState()), 
				Projects::ProjectService::GetActiveStartGameState());
		}

		if (Projects::ProjectService::GetActiveAppIsNetworked())
		{
			Network::ClientService::Init();
		}
	}

	void RuntimeApp::OnStop()
	{
		Scenes::SceneService::GetActiveScene()->OnRuntimeStop();
		Scenes::SceneService::GetActiveScene()->DestroyAllEntities();
		if (Projects::ProjectService::GetActiveAppIsNetworked())
		{
			Network::ClientService::Terminate();
		}
	}

}

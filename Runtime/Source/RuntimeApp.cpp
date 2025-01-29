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

	RuntimeApp::RuntimeApp(std::filesystem::path projectPath, bool headless) : Application("RuntimeLayer"), m_Headless(headless), m_ProjectPath(projectPath)
	{
	}

	void RuntimeApp::Init()
	{
		Scripting::ScriptService::Init();
		Audio::AudioService::Init();
		Scenes::SceneService::Init();

		if (m_Headless)
		{
			Audio::AudioService::SetMute(true);
		}

		Window& currentWindow = EngineService::GetActiveWindow();

		Scenes::SceneService::SetActiveScene(CreateRef<Scenes::Scene>(), Assets::EmptyHandle);


		
#if defined(KG_EXPORT_RUNTIME) || defined(KG_EXPORT_SERVER)
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
		if (m_ProjectPath.empty())
		{
			if (!OpenProject())
			{
				EngineService::EndRun();
				return;
			}
		}
		else
		{
			OpenProject(m_ProjectPath);
		}
#endif

		
		if (!m_Headless)
		{
			Projects::ProjectService::GetActiveIsFullscreen() ? currentWindow.SetFullscreen(true) : currentWindow.SetFullscreen(false);
			currentWindow.ResizeWindow(Utility::ScreenResolutionToVec2(Projects::ProjectService::GetActiveTargetResolution()));
			currentWindow.SetResizable(false);
		}

		AI::AIService::Init();
		Rendering::RenderingService::Init();
		Rendering::RenderingService::SetLineWidth(4.0f);
		RuntimeUI::FontService::Init();
		RuntimeUI::RuntimeUIService::Init();
		Particles::ParticleService::Init();

		if (!m_Headless)
		{
			InitializeFrameBuffer();;
		}

		OnPlay();
		currentWindow.SetVisible(true);
	}

	void RuntimeApp::Terminate()
	{
		OnStop();

		// Terminate engine services
		RuntimeUI::RuntimeUIService::Terminate();
		Particles::ParticleService::Terminate();
		Audio::AudioService::Terminate();
		Scripting::ScriptService::Terminate();
		AI::AIService::Terminate();
		Assets::AssetService::ClearAll();
		RuntimeUI::FontService::Terminate();
		Scenes::SceneService::Terminate();
		Rendering::RenderingService::Shutdown();
	}

	void RuntimeApp::InitializeFrameBuffer()
	{
		Rendering::FramebufferSpecification fbSpec;
		fbSpec.Attachments = { Rendering::FramebufferDataFormat::RGBA8, Rendering::FramebufferDataFormat::RED_INTEGER, Rendering::FramebufferDataFormat::Depth };
		fbSpec.Width = EngineService::GetActiveWindow().GetWidth();
		fbSpec.Height = EngineService::GetActiveWindow().GetHeight();
		m_ViewportFramebuffer = Rendering::Framebuffer::Create(fbSpec);
		//m_ViewportFramebuffer->Bind();
	}

	void RuntimeApp::OnUpdate(Timestep ts)
	{
		// Render
		Ref<Scenes::Scene> activeScene{ Scenes::SceneService::GetActiveScene() };
		Rendering::RenderingService::ResetStats();
		Rendering::RendererAPI::SetClearColor(activeScene->m_BackgroundColor);
		Rendering::RendererAPI::Clear();

		// Draw all scene entities
 		OnUpdateRuntime(ts);

		// Get primary scene camera for render UI and particles over
		ECS::Entity cameraEntity = Scenes::SceneService::GetActiveScene()->GetPrimaryCameraEntity();
		if (!cameraEntity)
		{
			return;
		}
		Rendering::Camera* mainCamera = &cameraEntity.GetComponent<ECS::CameraComponent>().Camera;
		
		// Only handle UI and particles if a main camera exists
		if (mainCamera)
		{
			// Get camera transform
			Math::mat4 cameraTransform = cameraEntity.GetComponent<ECS::TransformComponent>().GetTransform();

			// Draw particles
			Particles::ParticleService::OnRender(mainCamera->GetProjection() * glm::inverse(cameraTransform));

			// Draw runtimeUI
			RuntimeUI::RuntimeUIService::OnRender(EngineService::GetActiveWindow().GetWidth(), 
				EngineService::GetActiveWindow().GetHeight());
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
		case Events::EventType::AppResize:
			handled = OnApplicationResize(*(Events::ApplicationResizeEvent*)event);
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
			handled = OnPhysicsCollisionStart(*(Events::PhysicsCollisionStart*)event);
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

	bool RuntimeApp::OnApplicationResize(Events::ApplicationResizeEvent event)
	{
		// Resize the window
		EngineService::GetActiveWindow().ResizeWindow({ event.GetWidth(), event.GetHeight() });
		m_ViewportFramebuffer->Resize(event.GetWidth(), event.GetHeight());
		return false;
	}

	bool RuntimeApp::OnWindowResize(Events::WindowResizeEvent event)
	{
		ViewportData& viewportData = EngineService::GetActiveWindow().GetActiveViewport();
		viewportData.m_Width = event.GetWidth();
		viewportData.m_Height = event.GetHeight();
		Scenes::SceneService::GetActiveScene()->OnViewportResize((uint32_t)event.GetWidth(), (uint32_t)event.GetHeight());
		return false;
	}

	bool RuntimeApp::OnPhysicsCollisionStart(Events::PhysicsCollisionStart event)
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

	bool RuntimeApp::OnPhysicsCollisionEnd(Events::PhysicsCollisionEnd event)
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
				collisionHandled = Utility::CallWrappedBoolEntityEntity(script->m_Function, entityOneID, entityTwoID);
			}
		}

		if (!collisionHandled && entityOne.HasComponent<ECS::Rigidbody2DComponent>())
		{
			ECS::Rigidbody2DComponent& component = entityTwo.GetComponent<ECS::Rigidbody2DComponent>();
			Assets::AssetHandle scriptHandle = component.OnCollisionEndScriptHandle;
			Scripting::Script* script = component.OnCollisionEndScript.get();
			if (scriptHandle != Assets::EmptyHandle)
			{
				collisionHandled = Utility::CallWrappedBoolEntityEntity(script->m_Function, entityTwoID, entityOneID);
			}
		}
		return false;
	}

	bool RuntimeApp::OnKeyPressed(Events::KeyPressedEvent event)
	{
		Input::InputMapService::OnKeyPressed(event);
		return false;
	}

	void RuntimeApp::OnUpdateRuntime(Timestep ts)
	{
		// Process AI
		AI::AIService::OnUpdate(ts);
		Particles::ParticleService::OnUpdate(ts);

		// Update
		Input::InputMapService::OnUpdate(ts);
		Scenes::SceneService::GetActiveScene()->OnUpdateEntities(ts);
		Physics::Physics2DService::OnUpdate(ts);

		// Render 2D
		ECS::Entity cameraEntity = Scenes::SceneService::GetActiveScene()->GetPrimaryCameraEntity();
		if (!cameraEntity)
		{
			return;
		}
		Rendering::Camera* mainCamera = &cameraEntity.GetComponent<ECS::CameraComponent>().Camera;
		Math::mat4 cameraTransform = cameraEntity.GetComponent<ECS::TransformComponent>().GetTransform();

		if (mainCamera)
		{
			// Transform Matrix needs to be inversed so that final view is from the perspective of the camera
			Scenes::SceneService::GetActiveScene()->RenderScene(*mainCamera, glm::inverse(cameraTransform));
		}
	}

	bool RuntimeApp::OnUpdateUserCount(Events::UpdateOnlineUsers event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnUpdateUserCountHandle();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidUInt32(Assets::AssetService::GetScript(scriptHandle)->m_Function, event.GetUserCount());
		}
		return false;
	}

	bool RuntimeApp::OnApproveJoinSession(Events::ApproveJoinSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnApproveJoinSessionHandle();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidUInt16(Assets::AssetService::GetScript(scriptHandle)->m_Function, event.GetUserSlot());
		}
		return false;
	}

	bool RuntimeApp::OnUpdateSessionUserSlot(Events::UpdateSessionUserSlot event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnUpdateSessionUserSlotHandle();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidUInt16(Assets::AssetService::GetScript(scriptHandle)->m_Function, event.GetUserSlot());
		}
		return false;
	}

	bool RuntimeApp::OnUserLeftSession(Events::UserLeftSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnUserLeftSessionHandle();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidUInt16(Assets::AssetService::GetScript(scriptHandle)->m_Function, event.GetUserSlot());
		}
		return false;
	}

	bool RuntimeApp::OnCurrentSessionInit(Events::CurrentSessionInit event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnCurrentSessionInitHandle();
		if (scriptHandle != 0)
		{
			Utility::CallWrappedVoidNone(Assets::AssetService::GetScript(scriptHandle)->m_Function);
		}
		return false;
	}

	bool RuntimeApp::OnConnectionTerminated(Events::ConnectionTerminated event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnConnectionTerminatedHandle();
		if (scriptHandle != 0)
		{
			Utility::CallWrappedVoidNone(Assets::AssetService::GetScript(scriptHandle)->m_Function);
		}
		return false;
	}

	bool RuntimeApp::OnStartSession(Events::StartSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnStartSessionHandle();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidNone(Assets::AssetService::GetScript(scriptHandle)->m_Function);
		}
		return false;
	}

	bool RuntimeApp::OnSessionReadyCheckConfirm(Events::SessionReadyCheckConfirm event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnSessionReadyCheckConfirmHandle();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidNone(Assets::AssetService::GetScript(scriptHandle)->m_Function);
		}
		return false;
	}

	bool RuntimeApp::OnReceiveSignal(Events::ReceiveSignal event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnReceiveSignalHandle();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidUInt16(Assets::AssetService::GetScript(scriptHandle)->m_Function, event.GetSignal());
		}
		return false;
	}

	void RuntimeApp::HandleUIMouseHovering()
	{
#if 0
		ImVec2 mousePos = ImGui::GetMousePos();
		mousePos.x -= m_ScreenViewportBounds[0].x;
		mousePos.y -= m_ScreenViewportBounds[0].y;
		Math::vec2 viewportSize = m_ScreenViewportBounds[1] - m_ScreenViewportBounds[0];
		mousePos.y = viewportSize.y - mousePos.y;

		if ((int)mousePos.x >= 0 && (int)mousePos.y >= 0 && (int)mousePos.x < (int)viewportSize.x && (int)mousePos.y < (int)viewportSize.y)
		{
			int pixelData = m_ViewportFramebuffer->ReadPixel(1, (int)mousePos.x, (int)mousePos.y);

			// Extract lower 16 bits
			m_HoveredWidgetID = (uint16_t)(pixelData & 0xFFFF);

			// Extract upper 16 bits
			m_HoveredWindowID = (uint16_t)((pixelData >> 16) & 0xFFFF);
		}

		// Exit early if no valid widget/window is available
		if (m_HoveredWidgetID == RuntimeUI::k_InvalidWidgetID || m_HoveredWindowID == RuntimeUI::k_InvalidWindowID)
		{
			return;
		}

		// Select the widget if applicable
		RuntimeUI::RuntimeUIService::SetSelectedWidgetByIndex({ RuntimeUI::RuntimeUIService::GetActiveUIHandle(),
			m_HoveredWindowID, m_HoveredWidgetID });
#endif
	}


	bool RuntimeApp::OpenProject()
	{
		std::filesystem::path initialDirectory = std::filesystem::current_path().parent_path() / "Projects";
		if (!Utility::FileSystem::PathExists(initialDirectory))
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
		if (Projects::ProjectService::OpenProject(path))
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
				if (Utility::FileSystem::PathExists(logoPath))
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
			Assets::AssetService::ClearAll();
			Assets::AssetService::DeserializeAll();
			OpenScene(startSceneHandle);

		}
	}


	void RuntimeApp::OpenScene(Assets::AssetHandle sceneHandle)
	{
		Ref<Scenes::Scene> newScene = Assets::AssetService::GetScene(sceneHandle);
		if (!newScene) { newScene = CreateRef<Scenes::Scene>(); }
		Scenes::SceneService::SetActiveScene(newScene, sceneHandle);
	}


	void RuntimeApp::OnPlay()
	{
		// Handle initializing core services
		Particles::ParticleService::ClearEmitters();
		Physics::Physics2DService::Init(Scenes::SceneService::GetActiveScene().get(), Scenes::SceneService::GetActiveScene()->m_PhysicsSpecification);
		Scenes::SceneService::GetActiveScene()->OnRuntimeStart();
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnRuntimeStartHandle();
		if (scriptHandle != 0)
		{
			Utility::CallWrappedVoidNone(Assets::AssetService::GetScript(scriptHandle)->m_Function);
		}

		// Load Default Game State
		if (Projects::ProjectService::GetActiveStartGameStateHandle() == 0)
		{
			Scenes::GameStateService::ClearActiveGameState();
		}
		else
		{
			Scenes::GameStateService::SetActiveGameState(Assets::AssetService::GetGameState(
				Projects::ProjectService::GetActiveStartGameStateHandle()), 
				Projects::ProjectService::GetActiveStartGameStateHandle());
		}

		if (Projects::ProjectService::GetActiveAppIsNetworked())
		{
			Network::ClientService::Init();
		}

		// Load particle emitters
		Particles::ParticleService::LoadSceneEmitters(Scenes::SceneService::GetActiveScene());
	}

	void RuntimeApp::OnStop()
	{
		Physics::Physics2DService::Terminate();
		Scenes::SceneService::GetActiveScene()->OnRuntimeStop();
		Scenes::SceneService::GetActiveScene()->DestroyAllEntities();
		if (Projects::ProjectService::GetActiveAppIsNetworked())
		{
			Network::ClientService::Terminate();
		}
	}

}

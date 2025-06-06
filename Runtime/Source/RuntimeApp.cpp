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

	bool RuntimeApp::Init()
	{
		Scripting::ScriptService::Init();
		Audio::AudioService::CreateAudioContext();
		Audio::AudioService::GetActiveContext().Init();
		Scenes::SceneService::Init();

		if (m_Headless)
		{
			Audio::AudioService::GetActiveContext().SetMute(true);
		}

		Window& currentWindow = EngineService::GetActiveEngine().GetWindow();

		Scenes::SceneService::SetActiveScene(CreateRef<Scenes::Scene>(), Assets::EmptyHandle);


		
#if defined(KG_EXPORT_RUNTIME) || defined(KG_EXPORT_SERVER)
		std::filesystem::path pathToProject = Utility::FileSystem::FindFileWithExtension(
			std::filesystem::current_path(),
			".kproj");
		if (pathToProject.empty())
		{
			KG_CRITICAL("Could not locate a .kproj file in local directory!");
			Scripting::ScriptService::Terminate();
			Audio::AudioService::GetActiveContext().Terminate();
			Audio::AudioService::RemoveAudioContext();
			Scenes::SceneService::Terminate();
			return false;
		}
		OpenProject(pathToProject);
		if (!Projects::ProjectService::IsActive())
		{
			KG_CRITICAL("Failed to open project!");
			Scripting::ScriptService::Terminate();
			Audio::AudioService::GetActiveContext().Terminate();
			Audio::AudioService::RemoveAudioContext();
			Scenes::SceneService::Terminate();
			return false;
		}
#else
		if (m_ProjectPath.empty())
		{
			if (!OpenProject())
			{
				Scripting::ScriptService::Terminate();
				Audio::AudioService::GetActiveContext().Terminate();
				Audio::AudioService::RemoveAudioContext();
				Scenes::SceneService::Terminate();
				return false;
			}
		}
		else
		{
			OpenProject(m_ProjectPath);
		}
#endif

		
		if (!m_Headless)
		{
			Projects::ProjectService::GetActiveContext().GetIsFullscreen() ? currentWindow.SetFullscreen(true) : currentWindow.SetFullscreen(false);
			currentWindow.ResizeWindow(Utility::ScreenResolutionToVec2(Projects::ProjectService::GetActiveContext().GetTargetResolution()));
			currentWindow.SetResizable(false);
		}

		AI::AIService::CreateAIContext();
		AI::AIService::GetActiveContext().Init();
		Rendering::RenderingService::Init();
		Rendering::RenderingService::SetLineWidth(4.0f);
		RuntimeUI::FontService::Init();
		RuntimeUI::RuntimeUIService::CreateRuntimeUIContext();
		RuntimeUI::RuntimeUIService::GetActiveContext().Init();
		Particles::ParticleService::CreateParticleContext();
		Particles::ParticleService::GetActiveContext().Init();
		Input::InputMapService::CreateInputMapContext();
		Input::InputMapService::GetActiveContext().Init();

		if (!m_Headless)
		{
			InitializeFrameBuffer();;
		}

		OnPlay();
		currentWindow.SetVisible(true);

		return true;
	}

	bool RuntimeApp::Terminate()
	{
		OnStop();

		// Terminate engine services
		RuntimeUI::RuntimeUIService::GetActiveContext().Terminate();
		RuntimeUI::RuntimeUIService::RemoveRuntimeUIContext();
		Input::InputMapService::GetActiveContext().Terminate();
		Input::InputMapService::RemoveInputMapContext();
		Particles::ParticleService::GetActiveContext().Terminate();
		Particles::ParticleService::RemoveParticleContext();
		Audio::AudioService::GetActiveContext().Terminate();
		Audio::AudioService::RemoveAudioContext();
		Scripting::ScriptService::Terminate();
		AI::AIService::GetActiveContext().Terminate();
		AI::AIService::RemoveAIContext();
		Assets::AssetService::ClearAll();
		RuntimeUI::FontService::Terminate();
		Scenes::SceneService::Terminate();
		Rendering::RenderingService::Shutdown();

		return true;
	}

	void RuntimeApp::InitializeFrameBuffer()
	{
		Rendering::FramebufferSpecification fbSpec;
		fbSpec.Attachments = 
		{
			Rendering::FramebufferDataFormat::RGBA8, 
			Rendering::FramebufferDataFormat::RED_INTEGER, 
			Rendering::FramebufferDataFormat::Depth 
		};
		fbSpec.Width = EngineService::GetActiveEngine().GetWindow().GetWidth();
		fbSpec.Height = EngineService::GetActiveEngine().GetWindow().GetHeight();
		m_ViewportFramebuffer = Rendering::Framebuffer::Create(fbSpec);
		m_ViewportFramebuffer->Bind();
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
			Window& engineWindow{ EngineService::GetActiveEngine().GetWindow()};

			// Get camera transform
			Math::mat4 cameraTransform = cameraEntity.GetComponent<ECS::TransformComponent>().GetTransform();

			// Draw particles
			Particles::ParticleService::GetActiveContext().OnRender(mainCamera->GetProjection() * glm::inverse(cameraTransform));

			// Clear mouse picking buffer
			m_ViewportFramebuffer->SetAttachment(1, -1);

			// Handle runtime UI's OnUpdate()
			Kargono::ViewportData& activeViewport = engineWindow.GetActiveViewport();
			Math::vec2 mousePos = Input::InputService::GetAbsoluteMousePosition();
			// Make sure the y-position is oriented correctly
			mousePos.y = (float)activeViewport.m_Height - mousePos.y;
			RuntimeUI::RuntimeUIService::GetActiveContext().OnUpdate(ts);

			// Draw runtimeUI
			RuntimeUI::RuntimeUIService::GetActiveContext().OnRender(engineWindow.GetWidth(),
				engineWindow.GetHeight());

			// Handle mouse picking for the UI
			HandleUIMouseHovering();

		}

		// Display the framebuffer inside the default
		Rendering::RendererAPI::SetDepthTesting(false);
		m_ViewportFramebuffer->DisplayToDefaultFrameBuffer();
		Rendering::RendererAPI::SetDepthTesting(true);
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
		case Events::EventType::ReceiveOnlineUsers:
			handled = OnUpdateUserCount(*(Events::ReceiveOnlineUsers*)event);
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
		if (event->GetEventType() == Events::EventType::MouseButtonPressed)
		{
			handled = OnMousePressed(*(Events::MouseButtonPressedEvent*)event);
		}

		if (event->GetEventType() == Events::EventType::MouseButtonReleased)
		{
			handled = OnMouseButtonReleased(*(Events::MouseButtonReleasedEvent*)event);
		}

		if (event->GetEventType() == Events::EventType::KeyTyped)
		{
			handled = OnKeyTyped(*(Events::KeyTypedEvent*)event);
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
		Events::EventCallbackFn eventCallback = EngineService::GetActiveEngine().GetWindow().GetEventCallback();
		eventCallback(&windowEvent);
		return false;
	}

	bool RuntimeApp::OnApplicationResize(Events::ApplicationResizeEvent event)
	{
		// Resize the window
		EngineService::GetActiveEngine().GetWindow().ResizeWindow({ event.GetWidth(), event.GetHeight() });
		m_ViewportFramebuffer->Resize(event.GetWidth(), event.GetHeight());
		return false;
	}

	bool RuntimeApp::OnWindowResize(Events::WindowResizeEvent event)
	{
		ViewportData& viewportData = EngineService::GetActiveEngine().GetWindow().GetActiveViewport();
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
				collisionHandled = Utility::CallWrapped<WrappedBoolEntityEntity>(script->m_Function, entityOneID, entityTwoID);
			}
		}

		if (!collisionHandled && entityTwo.HasComponent<ECS::Rigidbody2DComponent>())
		{
			ECS::Rigidbody2DComponent& component = entityTwo.GetComponent<ECS::Rigidbody2DComponent>();
			Assets::AssetHandle scriptHandle = component.OnCollisionStartScriptHandle;
			Scripting::Script* script = component.OnCollisionStartScript.get();
			if (scriptHandle != Assets::EmptyHandle)
			{
				collisionHandled = Utility::CallWrapped<WrappedBoolEntityEntity>(script->m_Function, entityTwoID, entityOneID);
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
				collisionHandled = Utility::CallWrapped<WrappedBoolEntityEntity>(script->m_Function, entityOneID, entityTwoID);
			}
		}

		if (!collisionHandled && entityOne.HasComponent<ECS::Rigidbody2DComponent>())
		{
			ECS::Rigidbody2DComponent& component = entityTwo.GetComponent<ECS::Rigidbody2DComponent>();
			Assets::AssetHandle scriptHandle = component.OnCollisionEndScriptHandle;
			Scripting::Script* script = component.OnCollisionEndScript.get();
			if (scriptHandle != Assets::EmptyHandle)
			{
				collisionHandled = Utility::CallWrapped<WrappedBoolEntityEntity>(script->m_Function, entityTwoID, entityOneID);
			}
		}
		return false;
	}

	bool RuntimeApp::OnKeyTyped(Events::KeyTypedEvent event)
	{
		RuntimeUI::RuntimeUIService::GetActiveContext().OnKeyTypedEvent(event);
		return false;
	}

	bool RuntimeApp::OnKeyPressed(Events::KeyPressedEvent event)
	{
		KG_PROFILE_FUNCTION();
		bool handled = RuntimeUI::RuntimeUIService::GetActiveContext().OnKeyPressedEvent(event);

		if (!handled)
		{
			Input::InputMapService::GetActiveContext().OnKeyPressed(event);
		}

		return handled;
	}

	bool RuntimeApp::OnMousePressed(Events::MouseButtonPressedEvent event)
	{
		// Ensure we are handling a left-click
		if (event.GetMouseButton() != Mouse::ButtonLeft)
		{
			return false;
		}
		RuntimeUI::RuntimeUIContext& uiContext{ RuntimeUI::RuntimeUIService::GetActiveContext()};
		Assets::AssetHandle currentUI = uiContext.GetActiveUIHandle();

		RuntimeUI::IDType idType = uiContext.CheckIDType(m_HoveredWidgetID);

		// Handle on press for the active user interface if applicable
		if (idType == RuntimeUI::IDType::Widget)
		{
			uiContext.OnPressByIndex({ uiContext.GetActiveUIHandle(),
				m_HoveredWidgetID });

			// Handle case where active UI is changed
			if (currentUI != uiContext.GetActiveUIHandle())
			{
				return false;
			}

			// Handle start editing
			uiContext.SetEditingWidgetByIndex({ uiContext.GetActiveUIHandle(),
				m_HoveredWidgetID });

			// Handle specific widget on click's
			Kargono::ViewportData& activeViewport = EngineService::GetActiveEngine().GetWindow().GetActiveViewport();
			Math::vec2 mousePos = Input::InputService::GetAbsoluteMousePosition();
			// Make sure the y-position is oriented correctly
			mousePos.y = (float)activeViewport.m_Height - mousePos.y;
			Events::MouseButtonPressedEvent mouseEvent{ Mouse::ButtonLeft };
			uiContext.OnMouseButtonPressedEvent(mouseEvent);

			// Handle case where active UI is changed
			if (currentUI != uiContext.GetActiveUIHandle())
			{
				return false;
			}
		}
		return false;
	}

	bool RuntimeApp::OnMouseButtonReleased(const Events::MouseButtonReleasedEvent& event)
	{
		RuntimeUI::RuntimeUIContext& uiContext{ RuntimeUI::RuntimeUIService::GetActiveContext() };

		if (uiContext.GetActiveUI())
		{
			uiContext.OnMouseButtonReleasedEvent(event);
		}
		return false;
	}

	void RuntimeApp::OnUpdateRuntime(Timestep ts)
	{
		// Process AI
		AI::AIService::GetActiveContext().OnUpdate(ts);
		Particles::ParticleService::GetActiveContext().OnUpdate(ts);

		// Update
		Input::InputMapService::GetActiveContext().OnUpdate(ts);
		Scenes::SceneService::GetActiveScene()->OnUpdateEntities(ts);
		Physics::Physics2DService::GetActiveContext().OnUpdate(ts);

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

	bool RuntimeApp::OnUpdateUserCount(Events::ReceiveOnlineUsers event)
	{
		Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts()};

		Assets::AssetHandle scriptHandle = clientScripts.m_OnUpdateUserCount;
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrapped<WrappedVoidUInt32>(Assets::AssetService::GetScript(scriptHandle)->m_Function, event.GetUserCount());
		}
		return false;
	}

	bool RuntimeApp::OnApproveJoinSession(Events::ApproveJoinSession event)
	{
		Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

		Assets::AssetHandle scriptHandle{ clientScripts.m_OnApproveJoinSession };
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrapped<WrappedVoidUInt16>(Assets::AssetService::GetScript(scriptHandle)->m_Function, event.GetUserSlot());
		}
		return false;
	}

	bool RuntimeApp::OnUpdateSessionUserSlot(Events::UpdateSessionUserSlot event)
	{
		Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

		Assets::AssetHandle scriptHandle = clientScripts.m_OnUpdateSessionUserSlot;
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrapped<WrappedVoidUInt16>(Assets::AssetService::GetScript(scriptHandle)->m_Function, event.GetUserSlot());
		}
		return false;
	}

	bool RuntimeApp::OnUserLeftSession(Events::UserLeftSession event)
	{
		Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

		Assets::AssetHandle scriptHandle = clientScripts.m_OnUserLeftSession;
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrapped<WrappedVoidUInt16>(Assets::AssetService::GetScript(scriptHandle)->m_Function, event.GetUserSlot());
		}
		return false;
	}

	bool RuntimeApp::OnCurrentSessionInit(Events::CurrentSessionInit event)
	{
		Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

		Assets::AssetHandle scriptHandle = clientScripts.m_OnCurrentSessionInit;
		if (scriptHandle != 0)
		{
			Utility::CallWrapped<WrappedVoidNone>(Assets::AssetService::GetScript(scriptHandle)->m_Function);
		}
		return false;
	}

	bool RuntimeApp::OnConnectionTerminated(Events::ConnectionTerminated event)
	{
		Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

		Assets::AssetHandle scriptHandle = clientScripts.m_OnConnectionTerminated;
		if (scriptHandle != 0)
		{
			Utility::CallWrapped<WrappedVoidNone>(Assets::AssetService::GetScript(scriptHandle)->m_Function);
		}
		return false;
	}

	bool RuntimeApp::OnStartSession(Events::StartSession event)
	{
		Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

		Assets::AssetHandle scriptHandle = clientScripts.m_OnStartSession;
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrapped<WrappedVoidNone>(Assets::AssetService::GetScript(scriptHandle)->m_Function);
		}
		return false;
	}

	bool RuntimeApp::OnSessionReadyCheckConfirm(Events::SessionReadyCheckConfirm event)
	{
		Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

		Assets::AssetHandle scriptHandle = clientScripts.m_OnSessionReadyCheckConfirm;
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrapped<WrappedVoidNone>(Assets::AssetService::GetScript(scriptHandle)->m_Function);
		}
		return false;
	}

	bool RuntimeApp::OnReceiveSignal(Events::ReceiveSignal event)
	{
		Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

		Assets::AssetHandle scriptHandle = clientScripts.m_OnReceiveSignal;
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrapped<WrappedVoidUInt16>(Assets::AssetService::GetScript(scriptHandle)->m_Function, event.GetSignal());
		}
		return false;
	}

	void RuntimeApp::HandleUIMouseHovering()
	{
		// Get the active viewport bounds and mouse position
		Kargono::ViewportData& activeViewport = EngineService::GetActiveEngine().GetWindow().GetActiveViewport();
		Math::vec2 mousePos = Input::InputService::GetAbsoluteMousePosition();

		// Make sure the mouse position is within bounds
		if ((int)mousePos.x < 0 || 
			(int)mousePos.y < 0 || 
			(int)mousePos.x > (int)activeViewport.m_Width || 
			(int)mousePos.y > (int)activeViewport.m_Height)
		{
			return;
		}

		RuntimeUI::RuntimeUIContext& uiContext{ RuntimeUI::RuntimeUIService::GetActiveContext() };

		// Make sure the y-position is oriented correctly
		mousePos.y = (float)activeViewport.m_Height - mousePos.y;

		// Extract mouse picking information from the active framebuffer
		m_HoveredWidgetID = m_ViewportFramebuffer->ReadPixel(1, (int)mousePos.x, (int)mousePos.y);

		RuntimeUI::IDType idType = uiContext.CheckIDType(m_HoveredWidgetID);

		// Exit early if no valid widget is available
		if (idType == RuntimeUI::IDType::None || idType == RuntimeUI::IDType::Window)
		{
			uiContext.ClearHoveredWidget();
			return;
		}

		// Select the widget if applicable
		uiContext.SetHoveredWidgetByIndex({ uiContext.GetActiveUIHandle(),
			m_HoveredWidgetID });

	}

	Math::vec2 RuntimeApp::GetMouseViewportPosition()
	{
		Kargono::ViewportData& activeViewport = EngineService::GetActiveEngine().GetWindow().GetActiveViewport();
		Math::vec2 mousePos = Input::InputService::GetAbsoluteMousePosition();
		// Make sure the y-position is oriented correctly
		mousePos.y = (float)activeViewport.m_Height - mousePos.y;
		return mousePos;
	}

	ViewportData* RuntimeApp::GetViewportData()
	{
		return &EngineService::GetActiveEngine().GetWindow().GetActiveViewport();
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
		if (Projects::ProjectService::GetActiveContext().OpenProject(path))
		{
			if (!EngineService::GetActiveEngine().GetWindow().GetNativeWindow())
			{
				Math::vec2 screenSize = Utility::ScreenResolutionToVec2(Projects::ProjectService::GetActiveContext().GetTargetResolution());
				WindowProps projectProps =
				{
					Projects::ProjectService::GetActiveContext().GetProjectName().c_str(),
					static_cast<uint32_t>(screenSize.x),
					static_cast<uint32_t>(screenSize.y)
				};
				if (Utility::FileSystem::PathExists(logoPath))
				{
					EngineService::GetActiveEngine().GetWindow().Init(projectProps, logoPath);
				}
				else
				{
					EngineService::GetActiveEngine().GetWindow().Init(projectProps);
				}
				Rendering::RendererAPI::Init();
			}

			Assets::AssetHandle startSceneHandle = Projects::ProjectService::GetActiveContext().GetStartSceneHandle();
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
		Projects::Project& activeProject{ Projects::ProjectService::GetActiveContext()};

		// Handle initializing core services
		Particles::ParticleService::GetActiveContext().ClearEmitters();
		Physics::Physics2DService::CreatePhysics2DWorld();
		Physics::Physics2DService::GetActiveContext().Init(Scenes::SceneService::GetActiveScene().get(), Scenes::SceneService::GetActiveScene()->m_PhysicsSpecification);
		Scenes::SceneService::GetActiveScene()->OnRuntimeStart();
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveContext().GetOnRuntimeStartHandle();
		if (scriptHandle != 0)
		{
			Utility::CallWrapped<WrappedVoidNone>(Assets::AssetService::GetScript(scriptHandle)->m_Function);
		}

		// Load Default Game State
		if (activeProject.GetStartGameStateHandle() == 0)
		{
			Scenes::GameStateService::ClearActiveGameState();
		}
		else
		{
			Scenes::GameStateService::SetActiveGameState(Assets::AssetService::GetGameState(
				activeProject.GetStartGameStateHandle()),
				activeProject.GetStartGameStateHandle());
		}

		if (activeProject.GetAppIsNetworked())
		{
			Network::ClientService::GetActiveContext().Init(activeProject.GetServerConfig());
		}

		// Load particle emitters
		Particles::ParticleService::GetActiveContext().LoadSceneEmitters(Scenes::SceneService::GetActiveScene());
	}

	void RuntimeApp::OnStop()
	{
		Physics::Physics2DService::GetActiveContext().Terminate();
		Physics::Physics2DService::RemovePhysics2DWorld();
		Scenes::SceneService::GetActiveScene()->OnRuntimeStop();
		Scenes::SceneService::GetActiveScene()->DestroyAllEntities();
		if (Projects::ProjectService::GetActiveContext().GetAppIsNetworked())
		{
			Network::ClientService::GetActiveContext().Terminate(false);
		}
	}

}

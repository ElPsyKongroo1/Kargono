#include "kgpch.h"
#include "RuntimeLayer.h"

#include <filesystem>

namespace Kargono
{
	// Final Export Values
	//const std::filesystem::path runtimePath = "../Projects/Pong/Pong.kproj";
	//const std::filesystem::path logoPath = "../Projects/Pong/pong_logo.png";

	const std::filesystem::path runtimePath = "./Pong/Pong.kproj";
	const std::filesystem::path logoPath = "./Pong/pong_logo.png";

	RuntimeLayer::RuntimeLayer()
		: Layer("RuntimeLayer")
	{
	}

	void RuntimeLayer::OnAttach()
	{
		Script::ScriptEngine::Init();
		Audio::AudioEngine::Init();

		auto& currentWindow = Application::GetCurrentApp().GetWindow();

		Scene::SetActiveScene(CreateRef<Scene>());
		#if KG_EXPORT == 0
		if (!OpenProject())
		{
			Application::GetCurrentApp().Close();
			return;
		}
		#else
		OpenProject(runtimePath);
		if (!Projects::Project::GetActive())
		{
			Application::GetCurrentApp().Close();
			return;
		}
		#endif
		

		Projects::Project::GetIsFullscreen() ? currentWindow.SetFullscreen(true) : currentWindow.SetFullscreen(false);
		currentWindow.ResizeWindow(Utility::ScreenResolutionToVec2(Projects::Project::GetTargetResolution()));
		currentWindow.SetResizable(false);

		Renderer::Init();
		Renderer::SetLineWidth(4.0f);
		UI::Text::Init();
		UI::Runtime::Init();

		OnPlay();
	}

	void RuntimeLayer::OnDetach()
	{
		
		auto view = Scene::GetActiveScene()->GetAllEntitiesWith<AudioComponent>();
		for (auto& entity : view)
		{
			Entity e = { entity, Scene::GetActiveScene().get() };
			auto& audioComponent = e.GetComponent<AudioComponent>();
			audioComponent.Audio.reset();
		}
		OnStop();

	}

	void RuntimeLayer::OnUpdate(Timestep ts)
	{
		
		// Render
		Renderer::ResetStats();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();

		OnUpdateRuntime(ts);
		Entity cameraEntity = Scene::GetActiveScene()->GetPrimaryCameraEntity();
		Camera* mainCamera = &cameraEntity.GetComponent<CameraComponent>().Camera;
		Math::mat4 cameraTransform = cameraEntity.GetComponent<TransformComponent>().GetTransform();

		if (mainCamera)
		{
			UI::Runtime::PushRenderData(glm::inverse(cameraTransform), 
				Application::GetCurrentApp().GetWindow().GetWidth(), Application::GetCurrentApp().GetWindow().GetHeight());
		}
	}

	void RuntimeLayer::OnEvent(Events::Event& event)
	{
		Events::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Events::KeyPressedEvent>(KG_BIND_EVENT_FN(RuntimeLayer::OnKeyPressed));
		dispatcher.Dispatch<Events::PhysicsCollisionEvent>(KG_BIND_EVENT_FN(RuntimeLayer::OnPhysicsCollision));
		dispatcher.Dispatch<Events::PhysicsCollisionEnd>(KG_BIND_EVENT_FN(RuntimeLayer::OnPhysicsCollisionEnd));
		dispatcher.Dispatch<Events::WindowResizeEvent>(KG_BIND_EVENT_FN(RuntimeLayer::OnWindowResize));
		dispatcher.Dispatch<Events::ApplicationCloseEvent>(KG_BIND_EVENT_FN(RuntimeLayer::OnApplicationClose));
		dispatcher.Dispatch<Events::UpdateOnlineUsers>(KG_BIND_EVENT_FN(RuntimeLayer::OnUpdateUserCount));
		dispatcher.Dispatch<Events::ApproveJoinSession>(KG_BIND_EVENT_FN(RuntimeLayer::OnApproveJoinSession));
		dispatcher.Dispatch<Events::UserLeftSession>(KG_BIND_EVENT_FN(RuntimeLayer::OnUserLeftSession));
		dispatcher.Dispatch<Events::CurrentSessionInit>(KG_BIND_EVENT_FN(RuntimeLayer::OnCurrentSessionInit));
		dispatcher.Dispatch<Events::ConnectionTerminated>(KG_BIND_EVENT_FN(RuntimeLayer::OnConnectionTerminated));
		dispatcher.Dispatch<Events::UpdateSessionUserSlot>(KG_BIND_EVENT_FN(RuntimeLayer::OnUpdateSessionUserSlot));
		dispatcher.Dispatch<Events::StartSession>(KG_BIND_EVENT_FN(RuntimeLayer::OnStartSession));
		dispatcher.Dispatch<Events::SessionReadyCheckConfirm>(KG_BIND_EVENT_FN(RuntimeLayer::OnSessionReadyCheckConfirm));
		dispatcher.Dispatch<Events::ReceiveSignal>(KG_BIND_EVENT_FN(RuntimeLayer::OnReceiveSignal));
	}

	bool RuntimeLayer::OnApplicationClose(Events::ApplicationCloseEvent event)
	{
		Events::WindowCloseEvent windowEvent {};
		Events::EventCallbackFn eventCallback = Application::GetCurrentApp().GetWindow().GetEventCallback();
		eventCallback(windowEvent);
		return false;
	}

	bool RuntimeLayer::OnWindowResize(Events::WindowResizeEvent event)
	{
		Application::GetCurrentApp().GetWindow().SetViewportWidth(event.GetWidth());
		Application::GetCurrentApp().GetWindow().SetViewportHeight(event.GetHeight());
		Scene::GetActiveScene()->OnViewportResize((uint32_t)event.GetWidth(), (uint32_t)event.GetHeight());
		return false;
	}

	bool RuntimeLayer::OnPhysicsCollision(Events::PhysicsCollisionEvent event)
	{
		Script::ScriptEngine::OnPhysicsCollision(event);
		return false;
	}

	bool RuntimeLayer::OnPhysicsCollisionEnd(Events::PhysicsCollisionEnd event)
	{
		Script::ScriptEngine::OnPhysicsCollisionEnd(event);
		return false;
	}

	bool RuntimeLayer::OnKeyPressed(Events::KeyPressedEvent event)
	{
		Script::ScriptEngine::OnKeyPressed(event);
		return false;
	}

	void RuntimeLayer::OnUpdateRuntime(Timestep ts)
	{
		// Update Scripts
		Script::ScriptEngine::OnUpdate(ts);

		Scene::GetActiveScene()->OnUpdatePhysics(ts);

		// Render 2D
		Entity cameraEntity = Scene::GetActiveScene()->GetPrimaryCameraEntity();
		Camera* mainCamera = &cameraEntity.GetComponent<CameraComponent>().Camera;
		Math::mat4 cameraTransform = cameraEntity.GetComponent<TransformComponent>().GetTransform();

		if (mainCamera)
		{
			// Transform Matrix needs to be inversed so that final view is from the perspective of the camera
			Scene::GetActiveScene()->RenderScene(*mainCamera, glm::inverse(cameraTransform));
		}
	}

	bool RuntimeLayer::OnUpdateUserCount(Events::UpdateOnlineUsers event)
	{
		uint32_t userCount = event.GetUserCount();
		void* param = &userCount;
		Script::ScriptEngine::RunCustomCallsFunction(Projects::Project::GetProjectOnUpdateUserCount(), &param);
		return false;
	}

	bool RuntimeLayer::OnApproveJoinSession(Events::ApproveJoinSession event)
	{
		uint16_t userSlot = event.GetUserSlot();
		void* param = &userSlot;
		Script::ScriptEngine::RunCustomCallsFunction(Projects::Project::GetProjectOnApproveJoinSession(), &param);
		return false;
	}

	bool RuntimeLayer::OnUpdateSessionUserSlot(Events::UpdateSessionUserSlot event)
	{
		uint16_t userSlot = event.GetUserSlot();
		void* param = &userSlot;
		Script::ScriptEngine::RunCustomCallsFunction(Projects::Project::GetProjectOnUpdateSessionUserSlot(), &param);
		return false;
	}

	bool RuntimeLayer::OnUserLeftSession(Events::UserLeftSession event)
	{
		uint16_t userSlot = event.GetUserSlot();
		void* param = &userSlot;
		Script::ScriptEngine::RunCustomCallsFunction(Projects::Project::GetProjectOnUserLeftSession(), &param);
		return false;
	}

	bool RuntimeLayer::OnCurrentSessionInit(Events::CurrentSessionInit event)
	{
		Script::ScriptEngine::RunCustomCallsFunction(Projects::Project::GetProjectOnCurrentSessionInit());
		return false;
	}

	bool RuntimeLayer::OnConnectionTerminated(Events::ConnectionTerminated event)
	{
		Script::ScriptEngine::RunCustomCallsFunction(Projects::Project::GetProjectOnConnectionTerminated());
		return false;
	}

	bool RuntimeLayer::OnStartSession(Events::StartSession event)
	{
		Script::ScriptEngine::RunCustomCallsFunction(Projects::Project::GetProjectOnStartSession());
		return false;
	}

	bool RuntimeLayer::OnSessionReadyCheckConfirm(Events::SessionReadyCheckConfirm event)
	{
		Script::ScriptEngine::RunCustomCallsFunction(Projects::Project::GetProjectOnSessionReadyCheckConfirm());
		return false;
	}

	bool RuntimeLayer::OnReceiveSignal(Events::ReceiveSignal event)
	{
		uint16_t signal = event.GetSignal();
		void* param = &signal;
		Script::ScriptEngine::RunCustomCallsFunction(Projects::Project::GetProjectOnReceiveSignal(), &param);
		return false;
	}


	bool RuntimeLayer::OpenProject()
	{
		std::filesystem::path filepath = Utility::FileDialogs::OpenFile("Kargono Project (*.kproj)\0*.kproj\0");
		if (filepath.empty()) { return false; }

		OpenProject(filepath);
		return true;
	}

	void RuntimeLayer::OpenProject(const std::filesystem::path& path)
	{
		if (Assets::AssetManager::OpenProject(path))
		{
			if (!Application::GetCurrentApp().GetWindow().GetNativeWindow())
			{
				Math::vec2 screenSize = Utility::ScreenResolutionToVec2(Projects::Project::GetTargetResolution());
				WindowProps projectProps =
				{
					Projects::Project::GetProjectName(),
					static_cast<uint32_t>(screenSize.x),
					static_cast<uint32_t>(screenSize.y)
				};
				#if KG_EXPORT == 0
				Application::GetCurrentApp().GetWindow().Init(projectProps);
				#else
				Application::GetCurrentApp().GetWindow().Init(projectProps, logoPath);
				#endif
				RenderCommand::Init();
			}
			Assets::AssetHandle startSceneHandle = Projects::Project::GetStartSceneHandle();

			if (Script::ScriptEngine::AppDomainExists()){ Script::ScriptEngine::ReloadAssembly(); }
			else { Script::ScriptEngine::InitialAssemblyLoad(); }
			if (Scene::GetActiveScene())
			{
				Scene::GetActiveScene()->DestroyAllEntities();
			}
			Assets::AssetManager::ClearAll();
			Assets::AssetManager::DeserializeAll();
			OpenScene(startSceneHandle);

		}
	}


	void RuntimeLayer::OpenScene(Assets::AssetHandle sceneHandle)
	{
		Ref<Scene> newScene = Assets::AssetManager::GetScene(sceneHandle);
		if (!newScene) { newScene = CreateRef<Scene>(); }
		Scene::SetActiveScene(newScene);
	}


	void RuntimeLayer::OnPlay()
	{
		Scene::GetActiveScene()->OnRuntimeStart();
		Script::ScriptEngine::RunCustomCallsFunction(Projects::Project::GetProjectOnRuntimeStart());
		if (Projects::Project::GetAppIsNetworked())
		{
			Network::Client::SetActiveClient(CreateRef<Network::Client>());
			Network::Client::SetActiveNetworkThread(CreateRef<std::thread>(&Network::Client::RunClient, Network::Client::GetActiveClient().get()));
		}
	}

	void RuntimeLayer::OnStop()
	{
		Scene::GetActiveScene()->OnRuntimeStop();
		Scene::GetActiveScene()->DestroyAllEntities();
		if (Projects::Project::GetAppIsNetworked())
		{
			Network::Client::GetActiveClient()->StopClient();
			Network::Client::GetActiveNetworkThread()->join();
			Network::Client::GetActiveNetworkThread().reset();
			Network::Client::GetActiveClient().reset();
		}
	}

}

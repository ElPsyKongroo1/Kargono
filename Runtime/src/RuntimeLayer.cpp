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
		: Application("RuntimeLayer")
	{
	}

	void RuntimeLayer::OnAttach()
	{
		Script::ScriptEngine::Init();
		Audio::AudioEngine::Init();

		auto& currentWindow = Core::GetCurrentApp().GetWindow();

		Scene::SetActiveScene(CreateRef<Scene>());
		#if KG_EXPORT == 0
		if (!OpenProject())
		{
			Core::GetCurrentApp().Close();
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
		RuntimeUI::Text::Init();
		RuntimeUI::Runtime::Init();

		OnPlay();
		currentWindow.SetVisible(true);
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
			RuntimeUI::Runtime::PushRenderData(glm::inverse(cameraTransform), 
				Core::GetCurrentApp().GetWindow().GetWidth(), Core::GetCurrentApp().GetWindow().GetHeight());
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
		Events::EventCallbackFn eventCallback = Core::GetCurrentApp().GetWindow().GetEventCallback();
		eventCallback(windowEvent);
		return false;
	}

	bool RuntimeLayer::OnWindowResize(Events::WindowResizeEvent event)
	{
		Core::GetCurrentApp().GetWindow().SetViewportWidth(event.GetWidth());
		Core::GetCurrentApp().GetWindow().SetViewportHeight(event.GetHeight());
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
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnUpdateUserCount();
		if (scriptHandle != 0)
		{
			((WrappedVoidUInt32*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserCount());
		}
		return false;
	}

	bool RuntimeLayer::OnApproveJoinSession(Events::ApproveJoinSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnApproveJoinSession();
		if (scriptHandle != 0)
		{
			((WrappedVoidUInt16*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserSlot());
		}
		return false;
	}

	bool RuntimeLayer::OnUpdateSessionUserSlot(Events::UpdateSessionUserSlot event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnUpdateSessionUserSlot();
		if (scriptHandle != 0)
		{
			((WrappedVoidUInt16*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserSlot());
		}
		return false;
	}

	bool RuntimeLayer::OnUserLeftSession(Events::UserLeftSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnUserLeftSession();
		if (scriptHandle != 0)
		{
			((WrappedVoidUInt16*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserSlot());
		}
		return false;
	}

	bool RuntimeLayer::OnCurrentSessionInit(Events::CurrentSessionInit event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnCurrentSessionInit();
		if (scriptHandle != 0)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}
		return false;
	}

	bool RuntimeLayer::OnConnectionTerminated(Events::ConnectionTerminated event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnConnectionTerminated();
		if (scriptHandle != 0)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}
		return false;
	}

	bool RuntimeLayer::OnStartSession(Events::StartSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnStartSession();
		if (scriptHandle != 0)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}
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

	void RuntimeLayer::OpenProject(const std::filesystem::path& path)
	{
		if (Assets::AssetManager::OpenProject(path))
		{
			if (!Core::GetCurrentApp().GetWindow().GetNativeWindow())
			{
				Math::vec2 screenSize = Utility::ScreenResolutionToVec2(Projects::Project::GetTargetResolution());
				WindowProps projectProps =
				{
					Projects::Project::GetProjectName(),
					static_cast<uint32_t>(screenSize.x),
					static_cast<uint32_t>(screenSize.y)
				};
				#if KG_EXPORT == 0
				Core::GetCurrentApp().GetWindow().Init(projectProps);
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
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnRuntimeStart();
		if (scriptHandle != 0)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}
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

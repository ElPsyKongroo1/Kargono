#include "kgpch.h"
#include "RuntimeApp.h"

#include <filesystem>

namespace Kargono
{
	// Final Export Values
	//const std::filesystem::path runtimePath = "../Projects/Pong/Pong.kproj";
	//const std::filesystem::path logoPath = "../Projects/Pong/pong_logo.png";

	const std::filesystem::path runtimePath = "./Pong/Pong.kproj";
	const std::filesystem::path logoPath = "./Pong/pong_logo.png";

	RuntimeApp::RuntimeApp()
		: Application("RuntimeLayer")
	{
	}

	void RuntimeApp::OnAttach()
	{
		Script::ScriptEngine::Init();
		Audio::AudioEngine::Init();

		auto& currentWindow = EngineCore::GetCurrentEngineCore().GetWindow();

		Scenes::Scene::SetActiveScene(CreateRef<Scenes::Scene>(), Assets::EmptyHandle);
		#if KG_EXPORT == 0
		if (!OpenProject())
		{
			EngineCore::GetCurrentEngineCore().Close();
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

		Rendering::RenderingEngine::Init();
		Rendering::RenderingEngine::SetLineWidth(4.0f);
		RuntimeUI::Text::Init();
		RuntimeUI::Runtime::Init();

		OnPlay();
		currentWindow.SetVisible(true);
	}

	void RuntimeApp::OnDetach()
	{
		
		auto view = Scenes::Scene::GetActiveScene()->GetAllEntitiesWith<Scenes::AudioComponent>();
		for (auto& entity : view)
		{
			Scenes::Entity e = { entity, Scenes::Scene::GetActiveScene().get() };
			auto& audioComponent = e.GetComponent<Scenes::AudioComponent>();
			audioComponent.Audio.reset();
		}
		OnStop();

	}

	void RuntimeApp::OnUpdate(Timestep ts)
	{
		
		// Render
		Rendering::RenderingEngine::ResetStats();
		Rendering::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Rendering::RenderCommand::Clear();

		OnUpdateRuntime(ts);
		Scenes::Entity cameraEntity = Scenes::Scene::GetActiveScene()->GetPrimaryCameraEntity();
		Rendering::Camera* mainCamera = &cameraEntity.GetComponent<Scenes::CameraComponent>().Camera;
		Math::mat4 cameraTransform = cameraEntity.GetComponent<Scenes::TransformComponent>().GetTransform();

		if (mainCamera)
		{
			RuntimeUI::Runtime::PushRenderData(glm::inverse(cameraTransform), 
				EngineCore::GetCurrentEngineCore().GetWindow().GetWidth(), EngineCore::GetCurrentEngineCore().GetWindow().GetHeight());
		}
	}

	void RuntimeApp::OnEvent(Events::Event& event)
	{
		Events::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Events::KeyPressedEvent>(KG_BIND_CLASS_FN(RuntimeApp::OnKeyPressed));
		dispatcher.Dispatch<Events::PhysicsCollisionEvent>(KG_BIND_CLASS_FN(RuntimeApp::OnPhysicsCollision));
		dispatcher.Dispatch<Events::PhysicsCollisionEnd>(KG_BIND_CLASS_FN(RuntimeApp::OnPhysicsCollisionEnd));
		dispatcher.Dispatch<Events::WindowResizeEvent>(KG_BIND_CLASS_FN(RuntimeApp::OnWindowResize));
		dispatcher.Dispatch<Events::ApplicationCloseEvent>(KG_BIND_CLASS_FN(RuntimeApp::OnApplicationClose));
		dispatcher.Dispatch<Events::UpdateOnlineUsers>(KG_BIND_CLASS_FN(RuntimeApp::OnUpdateUserCount));
		dispatcher.Dispatch<Events::ApproveJoinSession>(KG_BIND_CLASS_FN(RuntimeApp::OnApproveJoinSession));
		dispatcher.Dispatch<Events::UserLeftSession>(KG_BIND_CLASS_FN(RuntimeApp::OnUserLeftSession));
		dispatcher.Dispatch<Events::CurrentSessionInit>(KG_BIND_CLASS_FN(RuntimeApp::OnCurrentSessionInit));
		dispatcher.Dispatch<Events::ConnectionTerminated>(KG_BIND_CLASS_FN(RuntimeApp::OnConnectionTerminated));
		dispatcher.Dispatch<Events::UpdateSessionUserSlot>(KG_BIND_CLASS_FN(RuntimeApp::OnUpdateSessionUserSlot));
		dispatcher.Dispatch<Events::StartSession>(KG_BIND_CLASS_FN(RuntimeApp::OnStartSession));
		dispatcher.Dispatch<Events::SessionReadyCheckConfirm>(KG_BIND_CLASS_FN(RuntimeApp::OnSessionReadyCheckConfirm));
		dispatcher.Dispatch<Events::ReceiveSignal>(KG_BIND_CLASS_FN(RuntimeApp::OnReceiveSignal));
	}

	bool RuntimeApp::OnApplicationClose(Events::ApplicationCloseEvent event)
	{
		Events::WindowCloseEvent windowEvent {};
		Events::EventCallbackFn eventCallback = EngineCore::GetCurrentEngineCore().GetWindow().GetEventCallback();
		eventCallback(windowEvent);
		return false;
	}

	bool RuntimeApp::OnWindowResize(Events::WindowResizeEvent event)
	{
		EngineCore::GetCurrentEngineCore().GetWindow().SetViewportWidth(event.GetWidth());
		EngineCore::GetCurrentEngineCore().GetWindow().SetViewportHeight(event.GetHeight());
		Scenes::Scene::GetActiveScene()->OnViewportResize((uint32_t)event.GetWidth(), (uint32_t)event.GetHeight());
		return false;
	}

	bool RuntimeApp::OnPhysicsCollision(Events::PhysicsCollisionEvent event)
	{
		Script::ScriptEngine::OnPhysicsCollision(event);
		return false;
	}

	bool RuntimeApp::OnPhysicsCollisionEnd(Events::PhysicsCollisionEnd event)
	{
		Script::ScriptEngine::OnPhysicsCollisionEnd(event);
		return false;
	}

	bool RuntimeApp::OnKeyPressed(Events::KeyPressedEvent event)
	{
		Script::ScriptEngine::OnKeyPressed(event);
		return false;
	}

	void RuntimeApp::OnUpdateRuntime(Timestep ts)
	{
		// Update Scripts
		Script::ScriptEngine::OnUpdate(ts);

		Scenes::Scene::GetActiveScene()->OnUpdatePhysics(ts);

		// Render 2D
		Scenes::Entity cameraEntity = Scenes::Scene::GetActiveScene()->GetPrimaryCameraEntity();
		Rendering::Camera* mainCamera = &cameraEntity.GetComponent<Scenes::CameraComponent>().Camera;
		Math::mat4 cameraTransform = cameraEntity.GetComponent<Scenes::TransformComponent>().GetTransform();

		if (mainCamera)
		{
			// Transform Matrix needs to be inversed so that final view is from the perspective of the camera
			Scenes::Scene::GetActiveScene()->RenderScene(*mainCamera, glm::inverse(cameraTransform));
		}
	}

	bool RuntimeApp::OnUpdateUserCount(Events::UpdateOnlineUsers event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnUpdateUserCount();
		if (scriptHandle != 0)
		{
			((WrappedVoidUInt32*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserCount());
		}
		return false;
	}

	bool RuntimeApp::OnApproveJoinSession(Events::ApproveJoinSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnApproveJoinSession();
		if (scriptHandle != 0)
		{
			((WrappedVoidUInt16*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserSlot());
		}
		return false;
	}

	bool RuntimeApp::OnUpdateSessionUserSlot(Events::UpdateSessionUserSlot event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnUpdateSessionUserSlot();
		if (scriptHandle != 0)
		{
			((WrappedVoidUInt16*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserSlot());
		}
		return false;
	}

	bool RuntimeApp::OnUserLeftSession(Events::UserLeftSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnUserLeftSession();
		if (scriptHandle != 0)
		{
			((WrappedVoidUInt16*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserSlot());
		}
		return false;
	}

	bool RuntimeApp::OnCurrentSessionInit(Events::CurrentSessionInit event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnCurrentSessionInit();
		if (scriptHandle != 0)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}
		return false;
	}

	bool RuntimeApp::OnConnectionTerminated(Events::ConnectionTerminated event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnConnectionTerminated();
		if (scriptHandle != 0)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}
		return false;
	}

	bool RuntimeApp::OnStartSession(Events::StartSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnStartSession();
		if (scriptHandle != 0)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}
		return false;
	}

	bool RuntimeApp::OnSessionReadyCheckConfirm(Events::SessionReadyCheckConfirm event)
	{
		Script::ScriptEngine::RunCustomCallsFunction(Projects::Project::GetProjectOnSessionReadyCheckConfirm());
		return false;
	}

	bool RuntimeApp::OnReceiveSignal(Events::ReceiveSignal event)
	{
		uint16_t signal = event.GetSignal();
		void* param = &signal;
		Script::ScriptEngine::RunCustomCallsFunction(Projects::Project::GetProjectOnReceiveSignal(), &param);
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
			if (!EngineCore::GetCurrentEngineCore().GetWindow().GetNativeWindow())
			{
				Math::vec2 screenSize = Utility::ScreenResolutionToVec2(Projects::Project::GetTargetResolution());
				WindowProps projectProps =
				{
					Projects::Project::GetProjectName(),
					static_cast<uint32_t>(screenSize.x),
					static_cast<uint32_t>(screenSize.y)
				};
				#if KG_EXPORT == 0
				EngineCore::GetCurrentEngineCore().GetWindow().Init(projectProps);
				#else
				Application::GetCurrentApp().GetWindow().Init(projectProps, logoPath);
				#endif
				Rendering::RenderCommand::Init();
			}
			Assets::AssetHandle startSceneHandle = Projects::Project::GetStartSceneHandle();

			if (Script::ScriptEngine::AppDomainExists()){ Script::ScriptEngine::ReloadAssembly(); }
			else { Script::ScriptEngine::InitialAssemblyLoad(); }
			if (Scenes::Scene::GetActiveScene())
			{
				Scenes::Scene::GetActiveScene()->DestroyAllEntities();
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
		Scenes::Scene::SetActiveScene(newScene, sceneHandle);
	}


	void RuntimeApp::OnPlay()
	{
		Scenes::Scene::GetActiveScene()->OnRuntimeStart();
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

	void RuntimeApp::OnStop()
	{
		Scenes::Scene::GetActiveScene()->OnRuntimeStop();
		Scenes::Scene::GetActiveScene()->DestroyAllEntities();
		if (Projects::Project::GetAppIsNetworked())
		{
			Network::Client::GetActiveClient()->StopClient();
			Network::Client::GetActiveNetworkThread()->join();
			Network::Client::GetActiveNetworkThread().reset();
			Network::Client::GetActiveClient().reset();
		}
	}

}

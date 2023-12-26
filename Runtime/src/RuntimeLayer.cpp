#include "Kargono/kgpch.h"
#include "RuntimeLayer.h"

namespace Kargono
{

	RuntimeLayer::RuntimeLayer()
		: Layer("RuntimeLayer")
	{
	}

	void RuntimeLayer::OnAttach()
	{
		auto& currentWindow = Application::GetCurrentApp().GetWindow();

		Scene::SetActiveScene(CreateRef<Scene>());

		auto commandLineArgs = Application::GetCurrentApp().GetSpecification().CommandLineArgs;
		if (commandLineArgs.Count > 1)
		{
			auto projectFilePath = commandLineArgs[1];
			OpenProject(projectFilePath);
		}
		else
		{
			if (!OpenProject())
			{
				Application::GetCurrentApp().Close();
				return;
			}

		}

		Projects::Project::GetIsFullscreen() ? currentWindow.SetFullscreen(true) : currentWindow.SetFullscreen(false);
		currentWindow.ResizeWindow(Utility::ScreenResolutionToVec2(Projects::Project::GetTargetResolution()));
		currentWindow.SetResizable(false);

		Renderer::Init();
		Renderer::SetLineWidth(4.0f);
		UI::TextEngine::Init();
		UI::RuntimeEngine::Init();

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
			UI::RuntimeEngine::PushRenderData(glm::inverse(cameraTransform), 
				Application::GetCurrentApp().GetWindow().GetWidth(), Application::GetCurrentApp().GetWindow().GetHeight());
		}
	}

	void RuntimeLayer::OnEvent(Events::Event& event)
	{
		Events::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Events::KeyPressedEvent>(KG_BIND_EVENT_FN(RuntimeLayer::OnKeyPressed));
		dispatcher.Dispatch<Events::PhysicsCollisionEvent>(KG_BIND_EVENT_FN(RuntimeLayer::OnPhysicsCollision));
		dispatcher.Dispatch<Events::WindowResizeEvent>(KG_BIND_EVENT_FN(RuntimeLayer::OnWindowResize));
		dispatcher.Dispatch<Events::ApplicationCloseEvent>(KG_BIND_EVENT_FN(RuntimeLayer::OnApplicationClose));

	}

	bool RuntimeLayer::OnApplicationClose(Events::ApplicationCloseEvent event)
	{
		Events::WindowCloseEvent windowEvent {};
		Window::EventCallbackFn eventCallback = Application::GetCurrentApp().GetWindow().GetEventCallback();
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
	}

	void RuntimeLayer::OnStop()
	{
		Scene::GetActiveScene()->OnRuntimeStop();
		Scene::GetActiveScene()->DestroyAllEntities();
	}

}

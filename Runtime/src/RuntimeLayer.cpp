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

		OnScenePlay();
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
		OnSceneStop();

	}

	void RuntimeLayer::OnUpdate(Timestep ts)
	{
		
		// Render
		Renderer::ResetStats();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();

		OnUpdateRuntime(ts);

	}

	void RuntimeLayer::OnEvent(Events::Event& event)
	{
		Events::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Events::WindowResizeEvent>(KG_BIND_EVENT_FN(RuntimeLayer::OnWindowResize));
		dispatcher.Dispatch<Events::PhysicsCollisionEvent>(KG_BIND_EVENT_FN(RuntimeLayer::OnPhysicsCollision));
	}

	bool RuntimeLayer::OnWindowResize(Events::WindowResizeEvent event)
	{
		Scene::GetActiveScene()->OnViewportResize((uint32_t)event.GetWidth(), (uint32_t)event.GetHeight());
		return false;
	}

	bool RuntimeLayer::OnPhysicsCollision(Events::PhysicsCollisionEvent event)
	{
		Script::ScriptEngine::OnPhysicsCollision(event);
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


	void RuntimeLayer::OnScenePlay()
	{
		Scene::GetActiveScene()->OnRuntimeStart();
	}

	void RuntimeLayer::OnSceneStop()
	{
		Scene::GetActiveScene()->OnRuntimeStop();
		Scene::GetActiveScene()->DestroyAllEntities();
	}

}

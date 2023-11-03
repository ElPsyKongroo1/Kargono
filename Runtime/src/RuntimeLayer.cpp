#include "RuntimeLayer.h"

#include "Kargono/Scene/SceneSerializer.h"
#include "Kargono/Math/Math.h"
#include "Kargono/Utils/PlatformUtils.h"
#include "Kargono/Scripting/ScriptEngine.h"
#include "Kargono/Assets/AssetManager.h"


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "box2d/b2_body.h"
#include <iostream>
#include <chrono>

namespace Kargono {

	RuntimeLayer::RuntimeLayer()
		: Layer("RuntimeLayer")
	{
	}

	void RuntimeLayer::OnAttach()
	{
		m_EditorAudio = new AudioContext("resources/audio/mechanist-theme.wav");
		m_PopSound = new AudioBuffer("resources/audio/pop-sound.wav");
		m_EditorAudio->allAudioBuffers.push_back(m_PopSound);
		m_PopSource = new AudioSource(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
			1.0f, 1.0f, AL_FALSE, m_PopSound);
		m_EditorAudio->allAudioSources.push_back(m_PopSource);
		m_LowPopSource = new AudioSource(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
			0.5f, 0.4f, AL_FALSE, m_PopSound);
		m_EditorAudio->allAudioSources.push_back(m_LowPopSource);
		m_EditorAudio->m_DefaultStereoSource->play();

		auto& currentWindow = Application::GetCurrentApp().GetWindow();

		m_ViewportSize = { currentWindow.GetWidth(), currentWindow.GetHeight()};

		m_ActiveScene = CreateRef<Scene>();

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

		Project::GetIsFullscreen() ? currentWindow.SetFullscreen() : currentWindow.DisableFullscreen();

		Renderer::Init();
		Renderer::SetLineWidth(4.0f);

		OnScenePlay();
	}

	void RuntimeLayer::OnDetach()
	{
		OnSceneStop();
		if (m_EditorAudio)
		{
			m_EditorAudio->terminate();
			delete m_EditorAudio;
			m_EditorAudio = nullptr;
		}
		
	}

	void RuntimeLayer::OnUpdate(Timestep ts)
	{

		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		
		// Render
		Renderer::ResetStats();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();

		OnUpdateRuntime(ts);

	}

	void RuntimeLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<PhysicsCollisionEvent>(KG_BIND_EVENT_FN(RuntimeLayer::OnPhysicsCollision));
	}

	bool RuntimeLayer::OnPhysicsCollision(PhysicsCollisionEvent event)
	{
		ScriptEngine::OnPhysicsCollision(event);
		m_PopSource->play();
		return false;
	}

	void RuntimeLayer::OnUpdateRuntime(Timestep ts)
	{
		// Update Scripts
		ScriptEngine::OnUpdate(ts);

		m_ActiveScene->OnUpdatePhysics(ts);

		// Render 2D
		Entity cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
		Camera* mainCamera = &cameraEntity.GetComponent<CameraComponent>().Camera;
		glm::mat4 cameraTransform = cameraEntity.GetComponent<TransformComponent>().GetTransform();

		if (mainCamera)
		{
			// Transform Matrix needs to be inversed so that final view is from the perspective of the camera
			m_ActiveScene->RenderScene(*mainCamera, glm::inverse(cameraTransform));
		}
	}


	bool RuntimeLayer::OpenProject()
	{
		std::string filepath = FileDialogs::OpenFile("Kargono Project (*.kproj)\0*.kproj\0");
		if (filepath.empty()) { return false; }

		OpenProject(filepath);
		return true;
	}

	void RuntimeLayer::OpenProject(const std::filesystem::path& path)
	{
		if (Project::Load(path))
		{
			auto startScenePath = Project::AppendToAssetDirPath(Project::GetActive()->GetConfig().StartScene);

			if (ScriptEngine::AppDomainExists()){ ScriptEngine::ReloadAssembly(); }
			else { ScriptEngine::InitialAssemblyLoad(); }
			if (m_ActiveScene)
			{
				m_ActiveScene->DestroyAllEntities();
			}
			AssetManager::ClearAll();
			AssetManager::DeserializeAll();
			OpenScene(startScenePath);

		}
	}

	void RuntimeLayer::OpenScene()
	{
		std::string filepath = FileDialogs::OpenFile("Kargono Scene (*.kargono)\0*.kargono\0");
		if (!filepath.empty())
		{
			OpenScene(filepath);
		}
	}

	void RuntimeLayer::OpenScene(const std::filesystem::path& path)
	{
		OnSceneStop();

		if (path.extension().string() != ".kargono")
		{
			KG_WARN("Could not load {0} - not a scene file", path.filename().string());
			return;
		}

		Ref<Scene> newScene = CreateRef<Scene>();
		SceneSerializer serializer(newScene);
		if (serializer.Deserialize(path.string()))
		{
			m_ActiveScene = newScene;
			m_ScenePath = path;
		}
	}


	void RuntimeLayer::OnScenePlay()
	{

		m_ActiveScene->OnRuntimeStart();
	}

	void RuntimeLayer::OnSceneStop()
	{

		m_ActiveScene->OnRuntimeStop();

		m_ActiveScene->DestroyAllEntities();
	}

}

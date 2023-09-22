#include "RuntimeLayer.h"
#include "Kargono/Scene/SceneSerializer.h"
#include "Kargono/Math/Math.h"
#include "Kargono/Utils/PlatformUtils.h"
#include "Kargono/Scripting/ScriptEngine.h"

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

		Renderer2D::SetLineWidth(4.0f);

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
		Renderer2D::ResetStats();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();

		m_ActiveScene->OnUpdateRuntime(ts);

	}

	void RuntimeLayer::OnEvent(Event& event)
	{
	}


	bool RuntimeLayer::OpenProject()
	{
		m_HoveredEntity = {};
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
		m_HoveredEntity = {};

		m_ActiveScene->OnRuntimeStart();
	}

	void RuntimeLayer::OnSceneStop()
	{
		m_HoveredEntity = {};

		m_ActiveScene->OnRuntimeStop();

		m_ActiveScene->DestroyAllEntities();
	}

}

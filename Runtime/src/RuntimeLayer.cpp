#include "RuntimeLayer.h"
#include "Kargono/Scene/SceneSerializer.h"
#include "Kargono/Math/Math.h"
#include "Kargono/Utils/PlatformUtils.h"
#include "Kargono/Scripting/ScriptEngine.h"

#include "imgui.h"
#include "ImGuizmo.h"
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
		m_EditorAudio->stereoSource->play();

		m_IconPlay = Texture2D::Create( (Application::Get().GetWorkingDirectory() / "resources/icons/play_icon.png").string());
		m_IconPause = Texture2D::Create((Application::Get().GetWorkingDirectory() / "resources/icons/pause_icon.png").string());
		m_IconSimulate = Texture2D::Create((Application::Get().GetWorkingDirectory() / "resources/icons/simulate_icon.png").string());
		m_IconStop = Texture2D::Create((Application::Get().GetWorkingDirectory() / "resources/icons/stop_icon.png").string());
		m_IconStep = Texture2D::Create((Application::Get().GetWorkingDirectory() / "resources/icons/step_icon.png").string());

		FramebufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER,  FramebufferTextureFormat::Depth };
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		m_ActiveScene = CreateRef<Scene>();

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_LogPanel = CreateScope<LogPanel>();

		auto commandLineArgs = Application::Get().GetSpecification().CommandLineArgs;
		if (commandLineArgs.Count > 1)
		{
			auto projectFilePath = commandLineArgs[1];
			OpenProject(projectFilePath);
		}
		else
		{
			if (!OpenProject())
			{
				Application::Get().Close();
				return;
			}

		}

		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);

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

		// Resize

		if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
		}
		
		//Render
		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();

		// Clear our entity ID attachment to -1
		m_Framebuffer->ClearAttachment(1, -1);



		m_ActiveScene->OnUpdateRuntime(ts);


		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		my = viewportSize.y - my;

		int mouseX = (int)mx;
		int mouseY = (int)my;

		if (mouseX >= 0 && mouseY >= 0 && mouseX <(int)viewportSize.x && mouseY < (int)viewportSize.y)
		{
			int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
			m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_ActiveScene.get());
		}

		OnOverlayRender();

		m_Framebuffer->Unbind();

	}

	void RuntimeLayer::OnImGuiRender()
	{
		static bool dockspaceOpen = true;
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace", &dockspaceOpen, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = minWinSizeX;

		UI_Viewport();

		ImGui::End();
	}


	void RuntimeLayer::UI_Viewport()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

		ImGui::Begin("Viewport");
		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportHovered);

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
		uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 },
			ImVec2{ 1, 0 });

		ImGui::End();
		ImGui::PopStyleVar();
	}
	

	void RuntimeLayer::OnEvent(Event& event)
	{
	}


	void RuntimeLayer::OnOverlayRender()
	{

		Entity camera = m_ActiveScene->GetPrimaryCameraEntity();
		if (!camera) { return; }
		Renderer2D::BeginScene(camera.GetComponent<CameraComponent>().Camera, camera.GetComponent<TransformComponent>().GetTransform());
		Renderer2D::EndScene();
	}

	void RuntimeLayer::NewProject()
	{
		Project::New();
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
			m_ContentBrowserPanel = CreateScope<ContentBrowserPanel>();

		}
	}

	void RuntimeLayer::NewScene()
	{
		m_HoveredEntity = {};
		m_ActiveScene = CreateRef<Scene>();
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_ScenePath = std::filesystem::path();
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
			m_SceneHierarchyPanel.SetContext(m_ActiveScene);
			m_ScenePath = path;
		}
	}


	void RuntimeLayer::OnScenePlay()
	{
		m_HoveredEntity = {};

		m_ActiveScene->OnRuntimeStart();

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void RuntimeLayer::OnSceneStop()
	{
		m_HoveredEntity = {};

		m_ActiveScene->OnRuntimeStop();

		m_ActiveScene->DestroyAllEntities();

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

}

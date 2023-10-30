#include "EditorLayer.h"
#include "Kargono/Scene/SceneSerializer.h"
#include "Kargono/Math/Math.h"
#include "Kargono/Utils/PlatformUtils.h"
#include "Kargono/Scripting/ScriptEngine.h"
#include "Kargono/Core/FileSystem.h"
#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Renderer/Shader.h"

#include "imgui.h"
#include "ImGuizmo.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "box2d/b2_body.h"
#include <iostream>
#include <chrono>
#include <cmath>

namespace Kargono {

	EditorLayer* EditorLayer::s_EditorLayer = nullptr;

	EditorLayer::EditorLayer()
		: Layer("EditorLayer")
	{
		Application::GetCurrentApp().AddImGuiLayer();
		s_EditorLayer = this;
	}

	void EditorLayer::OnAttach()
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

		m_IconPlay = Texture2D::CreateEditorTexture( (Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/play_icon.png").string());
		m_IconPause = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/pause_icon.png").string());
		m_IconSimulate = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/simulate_icon.png").string());
		m_IconStop = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/stop_icon.png").string());
		m_IconStep = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/step_icon.png").string());

		FramebufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER,  FramebufferTextureFormat::Depth };
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		m_EditorScene = CreateRef<Scene>();
		m_ActiveScene = m_EditorScene;

		m_SceneState = SceneState::Edit;

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_LogPanel = CreateScope<LogPanel>();

		auto commandLineArgs = Application::GetCurrentApp().GetSpecification().CommandLineArgs;
		if (commandLineArgs.Count > 1)
		{
			auto projectFilePath = commandLineArgs[1];
			OpenProject(projectFilePath);
		}
		else
		{
			// TODO: prompt the user to select a directory
			//NewProject();

			// If no project is opened, close Editor
			// TODO: this is while we don't have a new project path
			if (!OpenProject())
			{
				Application::GetCurrentApp().Close();
				return;
			}

		}

		Renderer::Init();
		Renderer::SetLineWidth(4.0f);

		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);

		m_EditorAudio->m_DefaultStereoSource->play();

		m_ArialText.Init();
		InitializeOverlayData();
	}

	void EditorLayer::OnDetach()
	{
		if (m_EditorAudio)
		{
			m_EditorAudio->terminate();
			delete m_EditorAudio;
			m_EditorAudio = nullptr;
		}
		
	}

	void EditorLayer::OnUpdate(Timestep ts)
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
		Renderer::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();

		// Clear our entity ID attachment to -1
		m_Framebuffer->ClearAttachment(1, -1);

		// Update Scene
		switch (m_SceneState)
		{
			case SceneState::Edit:
			{
				m_EditorCamera.OnUpdate(ts);

				m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);
				break;
			}
			case SceneState::Simulate:
			{
				m_EditorCamera.OnUpdate(ts);

				m_ActiveScene->OnUpdateSimulation(ts, m_EditorCamera);
				break;
			}
			case SceneState::Play:
			{
				m_ActiveScene->OnUpdateRuntime(ts);
				break;
			}
		}
		

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
			m_HoveredEntity = m_ActiveScene->CheckEntityExists((entt::entity)pixelData) ? Entity((entt::entity)pixelData, m_ActiveScene.get()) : Entity();
		}

		OnOverlayRender();

		m_Framebuffer->Unbind();

	}

	void EditorLayer::OnImGuiRender()
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

		if (m_RuntimeFullscreen && (m_SceneState == SceneState::Simulate || m_SceneState == SceneState::Play))
		{
			if (m_ShowViewport) { UI_Viewport(); }
			if (m_ShowToolbar) { UI_Toolbar(); }
			ImGui::End();
			return;
		}

		// Set up Menu Toolbar
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open Project ...", "Ctrl+O")) { OpenProject(); }

				ImGui::Separator();

				if (ImGui::MenuItem("New Scene", "Ctrl+N")){ NewScene();}

				if (ImGui::MenuItem("Save Scene", "Ctrl+S")) { SaveScene(); }

				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S")) { SaveSceneAs(); }

				if (ImGui::MenuItem("Exit")) { Application::GetCurrentApp().Close(); }
				ImGui::EndMenu();

			}

			if (ImGui::BeginMenu("Script"))
			{
				if (ImGui::MenuItem("Reload Assembly", "Ctrl+R"))
				{
					if (m_SceneState != SceneState::Edit) { OnSceneStop(); }
					ScriptEngine::ReloadAssembly();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Panels"))
			{
				if (ImGui::MenuItem("Scene Hierarchy", NULL, &m_ShowSceneHierarchy)){}
				if (ImGui::MenuItem("Content Browser", NULL, &m_ShowContentBrowser)){}
				if (ImGui::MenuItem("Log", NULL, &m_ShowLog)){}
				if (ImGui::MenuItem("Stats", NULL, &m_ShowStats)){}
				if (ImGui::MenuItem("Settings", NULL, &m_ShowSettings)){}
				if (ImGui::MenuItem("Viewport", NULL, &m_ShowViewport)){}
				if (ImGui::MenuItem("Toolbar", NULL, &m_ShowToolbar)){}
				if (ImGui::MenuItem("Demo Window", NULL, &m_ShowDemoWindow)){}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		if (m_ShowSceneHierarchy) { m_SceneHierarchyPanel.OnImGuiRender(); }
		if (m_ShowContentBrowser) { m_ContentBrowserPanel->OnImGuiRender(); }
		if (m_ShowLog) { m_LogPanel->OnImGuiRender(); }
		if (m_ShowStats) { UI_Stats(); }
		if (m_ShowSettings) { UI_Settings(); }
		if (m_ShowViewport) { UI_Viewport(); }
		if (m_ShowToolbar) { UI_Toolbar(); }
		if (m_ShowDemoWindow) { ImGui::ShowDemoWindow(); }

		ImGui::End();
	}

	void EditorLayer::UI_Toolbar()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

		ImGui::Begin("##toolbar", nullptr, 0);

		bool toolbarEnabled = (bool)m_ActiveScene;

		ImVec4 tintColor = ImVec4(1, 1, 1, 1);

		if (!toolbarEnabled) { tintColor.w = 0.5f; }

		float size = ImGui::GetWindowHeight() - 4.0f;
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));

		bool hasPlayButton = m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play;
		bool hasSimulateButton = m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate;
		bool hasPauseButton = m_SceneState != SceneState::Edit;

		if (hasPlayButton)
		{
			Ref<Texture2D> icon = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate) ? m_IconPlay : m_IconStop;
			if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor )
				&& toolbarEnabled)
			{
				if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate) { OnScenePlay(); }
				else if (m_SceneState == SceneState::Play) { OnSceneStop(); }
			}
		}
		if (hasSimulateButton)
		{
			if (hasPlayButton) { ImGui::SameLine(); }
			
			Ref<Texture2D> icon = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play) ? m_IconSimulate : m_IconStop;
			//ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
			if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor)
				&& toolbarEnabled)
			{
				if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play) { OnSceneSimulate(); }
				else if (m_SceneState == SceneState::Simulate) { OnSceneStop(); }
			}
			
		}

		if (hasPauseButton)
		{
			bool isPaused = m_ActiveScene->IsPaused();
			ImGui::SameLine();
			{
				Ref<Texture2D> icon = m_IconPause;
				//ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
				if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor)
					&& toolbarEnabled)
				{
					m_ActiveScene->SetPaused(!isPaused);
				}
			}
			if (isPaused)
			{
				ImGui::SameLine();
				{
					Ref<Texture2D> icon = m_IconStep;
					bool isPaused = m_ActiveScene->IsPaused();
					//ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
					if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor)
						&& toolbarEnabled)
					{
						m_ActiveScene->Step(1);
					}
				}
			}
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		ImGui::End();
	}

	void EditorLayer::UI_Settings()
	{
		ImGui::Begin("Settings");
		ImGui::Checkbox("Show Physics Colliders", &m_ShowPhysicsColliders);
		ImGui::Checkbox("Show Camera Frustrums", &m_ShowCameraFrustrums);
		ImGui::Checkbox("Fullscreen While Running or Simulating", &m_RuntimeFullscreen);
		static float musicVolume = 10.0f;
		ImGui::Separator();
		if(ImGui::DragFloat("Editor Music", &musicVolume, 0.5f, 0.0f, 100.0f))
		{
			m_EditorAudio->m_DefaultStereoSource->SetGain(musicVolume);
		}
		static int32_t* choice = (int32_t*)&m_EditorCamera.GetMovementType();
		ImGui::Text("Editor Camera Movement:");
		if (ImGui::RadioButton("Model Viewer", choice, (int32_t)EditorCamera::MovementType::ModelView))
		{
			m_EditorCamera.SetMovementType(EditorCamera::MovementType::ModelView);
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("FreeFly", choice, (int32_t)EditorCamera::MovementType::FreeFly))
		{
			m_EditorCamera.SetMovementType(EditorCamera::MovementType::FreeFly);
		}
		ImGui::SameLine();
		ImGui::TextDisabled("(Tab)");

		if (*choice == (int32_t)EditorCamera::MovementType::FreeFly)
		{
			ImGui::DragFloat("Speed", &m_EditorCamera.GetMovementSpeed(), 0.5f, 
				m_EditorCamera.GetMinMovementSpeed(), m_EditorCamera.GetMaxMovementSpeed());
		}

		ImGui::Text("Physics Settings:");
		if (ImGui::DragFloat2("Gravity", glm::value_ptr(m_EditorScene->GetPhysicsSpecification().Gravity), 0.05f))
		{
			if (m_ActiveScene->GetPhysicsWorld())
			{
				m_ActiveScene->GetPhysicsSpecification().Gravity = m_EditorScene->GetPhysicsSpecification().Gravity;
				m_ActiveScene->GetPhysicsWorld()->SetGravity(m_EditorScene->GetPhysicsSpecification().Gravity);
			}
		}

		ImGui::End();
	}

	void EditorLayer::UI_Stats()
	{
		ImGui::Begin("Stats");

		std::string name = "None";
		if (m_HoveredEntity)
		{
			name = m_HoveredEntity.GetComponent<TagComponent>().Tag;
		}
		ImGui::Text("Hovered Entity: %s", name.c_str());

		auto stats = Renderer::GetStats();
		ImGui::Text("Renderer Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		/*ImGui::Text("Quads: %d", stats.VertexCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());*/

		ImGui::End();
	}

	void EditorLayer::UI_Viewport()
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
		Application::GetCurrentApp().GetImGuiLayer()->BlockEvents(!m_ViewportHovered);

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
		uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 },
			ImVec2{ 1, 0 });
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				OpenScene(path);
			}
			ImGui::EndDragDropTarget();

		}

		if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate)

		{
			// Gizmos
			Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
			if (selectedEntity && m_GizmoType != -1)
			{
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y,
					m_ViewportBounds[1].x - m_ViewportBounds[0].x,
					m_ViewportBounds[1].y - m_ViewportBounds[0].y);

				// Camera

				// Runtime camera from entity
				/*auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
				const auto& camera = cameraEntity.GetComponent<CameraComponent>();
				const glm::mat4& cameraProjection = camera.Camera.GetProjection();
				glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());*/

				// Editor Camera
				const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
				glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

				// Entity Transform
				auto& tc = selectedEntity.GetComponent<TransformComponent>();
				glm::mat4 transform = tc.GetTransform();

				// Snapping
				bool snap = Input::IsKeyPressed(Key::LeftControl);
				float snapValue = 0.5f;
				if (m_GizmoType == ImGuizmo::OPERATION::ROTATE) { snapValue = 45.0f; }

				float snapValues[3] = { snapValue, snapValue, snapValue };

				ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
					(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
					nullptr, snap ? snapValues : nullptr);
				if (ImGuizmo::IsUsing())
				{
					glm::vec3 translation, rotation, scale;
					Math::DecomposeTransform(transform, translation, rotation, scale);

					glm::vec3 deltaRotation = rotation - tc.Rotation;
					tc.Translation = translation;
					tc.Rotation += deltaRotation;
					tc.Scale = scale;
				}
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}
	

	void EditorLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate)
		{
			m_EditorCamera.OnEvent(event);

			dispatcher.Dispatch<KeyPressedEvent>(KG_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
			dispatcher.Dispatch<MouseButtonPressedEvent>(KG_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));

		}
		dispatcher.Dispatch<PhysicsCollisionEvent>(KG_BIND_EVENT_FN(EditorLayer::OnPhysicsCollision));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent event)
	{
		if (event.IsRepeat() == 1) { return false; }

		bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
		bool alt = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);

		switch (event.GetKeyCode())
		{
		case Key::Escape:
		{
			m_SceneHierarchyPanel.SetSelectedEntity({});
			break;
		}
		case Key::Tab:
		{
			m_EditorCamera.ToggleMovementType();
			break;
		}

		case Key::N:
		{
			if (control) { NewScene(); }
			break;
		}
		case Key::O:
		{
			if (control) { OpenProject(); }
			break;
		}
		case Key::S:
		{
			if (control)
			{
				if (shift){SaveSceneAs();}
				else { SaveScene(); }
			}
			break;
		}

		// Scene Commands

		case Key::D:
		{
			if (control) { OnDuplicateEntity(); }
			break;
		}

		// Gizmos
		case Key::Q:
			{
			if (!ImGuizmo::IsUsing() && !alt) { m_GizmoType = -1; }
				break;
			}
		case Key::W:
		{
			if (!ImGuizmo::IsUsing() && !alt) { m_GizmoType = ImGuizmo::OPERATION::TRANSLATE; }
			break;
		}
		case Key::E:
		{
			if (!ImGuizmo::IsUsing() && !alt) { m_GizmoType = ImGuizmo::OPERATION::ROTATE; }
			break;
		}
		case Key::R:
		{
			if (control)
			{
				if (m_SceneState != SceneState::Edit) { OnSceneStop(); }
				ScriptEngine::ReloadAssembly();
			}

			if (!ImGuizmo::IsUsing()) { m_GizmoType = ImGuizmo::OPERATION::SCALE; }
			break;
		}
		case Key::Delete:
			{
			if (Application::GetCurrentApp().GetImGuiLayer()->GetActiveWidgetID() == 0)
			{
				Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
				if (selectedEntity)
				{
					m_EditorScene->DestroyEntity(selectedEntity);
					m_SceneHierarchyPanel.SetSelectedEntity({});
				}
			}
				break;

			}
			default:
			{
				break;
			}
		}
		return false;
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent event)
	{
		if (event.GetMouseButton() == Mouse::ButtonLeft)
		{
			if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt) && m_HoveredEntity)
			{
				m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
				// Algorithm to enable double clicking for an entity!
				static float previousTime{ 0.0f };
				static Entity previousEntity {};
				float currentTime = Time::GetTime();
				if (std::fabs(currentTime - previousTime) < 0.2f && m_HoveredEntity == previousEntity)
				{
					auto& transformComponent = m_HoveredEntity.GetComponent<TransformComponent>();
					m_EditorCamera.SetFocalPoint(transformComponent.Translation);
					m_EditorCamera.SetDistance(std::max({ transformComponent.Scale.x, transformComponent.Scale.y, transformComponent.Scale.z }) * 2.5f);
					m_EditorCamera.SetMovementType(EditorCamera::MovementType::ModelView);
				}
				previousTime = currentTime;
				previousEntity = m_HoveredEntity;

			}
		}
		return false;
	}

	bool EditorLayer::OnPhysicsCollision(PhysicsCollisionEvent event)
	{
		m_PopSource->play();
		return false;
	}

	void EditorLayer::OnUpdateParticles()
	{
	}

	static Shader::RendererInputSpec s_CircleInputSpec{};
	static Shader::RendererInputSpec s_LineInputSpec{};
	static glm::vec4 s_RectangleVertexPositions[4]
	{
		{ -0.5f, -0.5f, 0.0f, 1.0f },
			{ 0.5f, -0.5f, 0.0f, 1.0f },
			{ 0.5f, 0.5f, 0.0f, 1.0f },
			{ -0.5f, 0.5f, 0.0f, 1.0f }
	};
	static glm::vec4 s_CubeVertexPositions[8]
	{
		{ -0.5f, -0.5f, 0.5f, 1.0f },		// 0
			{ 0.5f, -0.5f, 0.5f, 1.0f },		// 1
			{ 0.5f, 0.5f, 0.5f, 1.0f },		// 2
			{ -0.5f, 0.5f, 0.5f, 1.0f },		// 3
		{ -0.5f, -0.5f, -0.5f, 1.0f },		// 4
			{ 0.5f, -0.5f, -0.5f, 1.0f },	// 5
			{ 0.5f, 0.5f, -0.5f, 1.0f },		// 6
		{ -0.5f, 0.5f, -0.5f, 1.0f },		// 7
	};

	static glm::uvec2 s_CubeIndices[12]
	{
		{0,1},
		{1,2},
		{2,3},
		{3,0},
		{4,5},
		{5,6},
		{6,7},
		{7,4},
		{0,4},
		{1,5},
		{2,6},
		{3,7}
	};

	static glm::vec4 s_FrustrumVertexPositions[8]
	{
		{ -1.0f, -1.0f, 1.0f, 1.0f },		// 0
			{ 1.0f, -1.0f, 1.0f, 1.0f },		// 1
			{ 1.0f, 1.0f, 1.0f, 1.0f },		// 2
			{ -1.0f, 1.0f, 1.0f, 1.0f },		// 3
		{ -1.0f, -1.0f, -1.0f, 1.0f },		// 4
			{ 1.0f, -1.0f, -1.0f, 1.0f },	// 5
			{ 1.0f, 1.0f, -1.0f, 1.0f },		// 6
		{ -1.0f, 1.0f, -1.0f, 1.0f },		// 7
	};

	static glm::uvec2 s_FrustrumIndices[16]
	{
		{0,1},
		{1,2},
		{2,3},
		{3,0},
		{4,5},
		{5,6},
		{6,7},
		{7,4},
		{0,4},
		{1,5},
		{2,6},
		{3,7},
		{4, 8},
		{5, 8},
		{6, 8},
		{7,8}
	};

	static Ref<std::vector<glm::vec3>> s_OutputVector = CreateRef<std::vector<glm::vec3>>();

	void EditorLayer::InitializeOverlayData()
	{

		// Set up Line Input Specifications for Overlay Calls
		{
			Shader::ShaderSpecification lineShaderSpec {Shader::ColorInputType::FlatColor, Shader::TextureInputType::None, false, true, false, Shape::RenderingType::DrawLine, false};
			auto [uuid, localShader] = AssetManager::GetShader(lineShaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			Shader::SetDataAtInputLocation<glm::vec4>({ 0.0f, 1.0f, 0.0f, 1.0f }, "a_Color", localBuffer, localShader);

			ShapeComponent* lineShapeComponent = new ShapeComponent();
			lineShapeComponent->CurrentShape = Shape::ShapeTypes::None;
			lineShapeComponent->Vertices = nullptr;

			s_LineInputSpec.Shader = localShader;
			s_LineInputSpec.Buffer = localBuffer;
			s_LineInputSpec.ShapeComponent = lineShapeComponent;
		}


		// Set up Circle Input Specification for Overlay Calls

		{
			Shader::ShaderSpecification shaderSpec {Shader::ColorInputType::FlatColor,  Shader::TextureInputType::None, true, true, false, Shape::RenderingType::DrawIndex, false};
			auto [uuid, localShader] = AssetManager::GetShader(shaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			Shader::SetDataAtInputLocation<glm::vec4>({ 0.0f, 1.0f, 0.0f, 1.0f }, "a_Color", localBuffer, localShader);
			Shader::SetDataAtInputLocation<float>(0.05f, "a_Thickness", localBuffer, localShader);
			Shader::SetDataAtInputLocation<float>(0.005f, "a_Fade", localBuffer, localShader);

			ShapeComponent* shapeComp = new ShapeComponent();
			shapeComp->CurrentShape = Shape::ShapeTypes::Quad;
			shapeComp->Vertices = CreateRef<std::vector<glm::vec3>>(Shape::s_Quad.GetIndexVertices());
			shapeComp->Indices = CreateRef<std::vector<uint32_t>>(Shape::s_Quad.GetIndices());

			s_CircleInputSpec.Shader = localShader;
			s_CircleInputSpec.Buffer = localBuffer;
			s_CircleInputSpec.ShapeComponent = shapeComp;
		}

		// TODO: Shape Components and Buffers are memory leaks!
	}

	void EditorLayer::OnOverlayRender()
	{
		if (m_SceneState == SceneState::Play)
		{
			Entity camera = m_ActiveScene->GetPrimaryCameraEntity();
			if (!camera) { return; }
			Renderer::BeginScene(camera.GetComponent<CameraComponent>().Camera, camera.GetComponent<TransformComponent>().GetTransform());
		}
		else
		{
			Renderer::BeginScene(m_EditorCamera);
		}

		if (m_ShowPhysicsColliders)
		{
			// Circle Colliders
			{
				auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
				for (auto entity : view)
				{
					auto [tc, cc2d] = view.get<TransformComponent, CircleCollider2DComponent>(entity);

					glm::vec3 translation = tc.Translation + glm::vec3(cc2d.Offset.x, cc2d.Offset.y, 0.001f);

					glm::vec3 scale = tc.Scale * glm::vec3(cc2d.Radius * 2.0f);

					glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
						* glm::scale(glm::mat4(1.0f), scale);

					s_CircleInputSpec.TransformMatrix = transform;
					Renderer::SubmitDataToRenderer(s_CircleInputSpec);
				}
			}
			// Box Colliders
			{
				auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
				for (auto entity : view)
				{
					auto [tc, bc2d] = view.get<TransformComponent, BoxCollider2DComponent>(entity);

					glm::vec3 translation = tc.Translation + glm::vec3(bc2d.Offset.x, bc2d.Offset.y, 0.001f);
					glm::vec3 scale = tc.Scale * glm::vec3(bc2d.Size * 2.0f, 1.0f);

					glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
						* glm::rotate(glm::mat4(1.0f), tc.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
						* glm::scale(glm::mat4(1.0f), scale);

					static glm::vec4 boxColliderColor {0.0f, 1.0f, 0.0f, 1.0f};
					Shader::SetDataAtInputLocation<glm::vec4>(boxColliderColor, "a_Color", s_LineInputSpec.Buffer, s_LineInputSpec.Shader);

					glm::vec3 lineVertices[4];
					for (size_t i = 0; i < 4; i++)
					{
						lineVertices[i] = transform * s_RectangleVertexPositions[i];
					}
					s_OutputVector->clear();
					s_OutputVector->push_back(lineVertices[0]);
					s_OutputVector->push_back(lineVertices[1]);
					s_LineInputSpec.ShapeComponent->Vertices = s_OutputVector;
					Renderer::SubmitDataToRenderer(s_LineInputSpec);
					s_OutputVector->clear();
					s_OutputVector->push_back(lineVertices[1]);
					s_OutputVector->push_back(lineVertices[2]);
					s_LineInputSpec.ShapeComponent->Vertices = s_OutputVector;
					Renderer::SubmitDataToRenderer(s_LineInputSpec);
					s_OutputVector->clear();
					s_OutputVector->push_back(lineVertices[2]);
					s_OutputVector->push_back(lineVertices[3]);
					s_LineInputSpec.ShapeComponent->Vertices = s_OutputVector;
					Renderer::SubmitDataToRenderer(s_LineInputSpec);
					s_OutputVector->clear();
					s_OutputVector->push_back(lineVertices[3]);
					s_OutputVector->push_back(lineVertices[0]);
					s_LineInputSpec.ShapeComponent->Vertices = s_OutputVector;
					Renderer::SubmitDataToRenderer(s_LineInputSpec);
				}
			}
		}

		if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate)
		{
			// Draw selected entity outline 
			if (Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity()) {
				TransformComponent transform = selectedEntity.GetComponent<TransformComponent>();
				static glm::vec4 selectionColor {1.0f, 0.5f, 0.0f, 1.0f};
				Shader::SetDataAtInputLocation<glm::vec4>(selectionColor, "a_Color", s_LineInputSpec.Buffer, s_LineInputSpec.Shader);

				glm::vec3 lineVertices[8];

				for (size_t i = 0; i < 8; i++)
				{
					lineVertices[i] = transform.GetTransform() * s_CubeVertexPositions[i];
				}

				for (auto& indices : s_CubeIndices)
				{
					s_OutputVector->clear();
					s_OutputVector->push_back(lineVertices[indices.x]);
					s_OutputVector->push_back(lineVertices[indices.y]);
					s_LineInputSpec.ShapeComponent->Vertices = s_OutputVector;
					Renderer::SubmitDataToRenderer(s_LineInputSpec);
				}

				if (selectedEntity.HasComponent<CameraComponent>() && m_ShowCameraFrustrums)
				{
					DrawFrustrum(selectedEntity);
				}
			}
		}

		Renderer::EndScene();

		// Render Text!
		//m_ArialText.RenderText(m_EditorCamera , "abcdefghijklmnopqrstuvwxyz123456789", 800, 850, 0.5f, { 0.2f, 0.5f, 0.6f });
	}

	void EditorLayer::DrawFrustrum(Entity& entity)
	{
		auto& transform = entity.GetComponent<TransformComponent>();
		auto& camera = entity.GetComponent<CameraComponent>();
		float windowWidth = (float)Application::GetCurrentApp().GetWindow().GetWidth();
		float windowHeight = (float)Application::GetCurrentApp().GetWindow().GetHeight();
		glm::vec4 viewport = { 0.0f, 0.0f, windowWidth, windowHeight };
		auto cameraProjectionType = camera.Camera.GetProjectionType();
		glm::vec4 selectionColor { 0.5f, 0.3f, 0.85f, 1.0f };
		Shader::SetDataAtInputLocation<glm::vec4>(selectionColor, "a_Color", s_LineInputSpec.Buffer, s_LineInputSpec.Shader);

		glm::vec3 lineVertices[9];

		for (size_t i = 0; i < 8; i++)
		{
			glm::vec4 vertexPosition = s_FrustrumVertexPositions[i];
			glm::vec4 localSpaceCoordinates = glm::inverse(camera.Camera.GetProjection()) * s_FrustrumVertexPositions[i];
			localSpaceCoordinates = localSpaceCoordinates / localSpaceCoordinates.w; // Perspective Division
			lineVertices[i] = transform.GetTranslation() * transform.GetRotation() * localSpaceCoordinates;
			
		}

		lineVertices[8] = (transform.GetTransform() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		bool colorChanged = false;
		uint32_t iteration = 0;
		for (auto& indices : s_FrustrumIndices)
		{
			if (iteration >= 12 && !colorChanged)
			{
				selectionColor = { 0.3f, 0.1f, 0.8f, 1.0f };
				Shader::SetDataAtInputLocation<glm::vec4>(selectionColor, "a_Color", s_LineInputSpec.Buffer, s_LineInputSpec.Shader);
				colorChanged = true;
			}
			s_OutputVector->clear();
			s_OutputVector->push_back(lineVertices[indices.x]);
			s_OutputVector->push_back(lineVertices[indices.y]);
			s_LineInputSpec.ShapeComponent->Vertices = s_OutputVector;
			Renderer::SubmitDataToRenderer(s_LineInputSpec);
			iteration++;
		}

	}

	void EditorLayer::NewProject()
	{
		Project::New();
	}

	bool EditorLayer::OpenProject()
	{
		m_HoveredEntity = {};
		std::string filepath = FileDialogs::OpenFile("Kargono Project (*.kproj)\0*.kproj\0");
		if (filepath.empty()) { return false; }

		OpenProject(filepath);
		return true;
	}

	void EditorLayer::OpenProject(const std::filesystem::path& path)
	{
		if (Project::Load(path))
		{
			auto startScenePath = Project::AppendToAssetDirPath(Project::GetActive()->GetConfig().StartScene);

			if (ScriptEngine::AppDomainExists()){ ScriptEngine::ReloadAssembly(); }
			else { ScriptEngine::InitialAssemblyLoad(); }
			if (m_EditorScene)
			{
				m_EditorScene->DestroyAllEntities();
			}
			AssetManager::ClearAll();
			AssetManager::DeserializeAll();

			OpenScene(startScenePath);
			m_ContentBrowserPanel = CreateScope<ContentBrowserPanel>();

		}
	}

	void EditorLayer::SaveProject()
	{
		//Project::SaveActive();
	}

	void EditorLayer::NewScene()
	{
		m_HoveredEntity = {};
		m_EditorScene = CreateRef<Scene>();
		m_ActiveScene = m_EditorScene;
		m_SceneHierarchyPanel.SetContext(m_EditorScene);
		m_EditorScenePath = std::filesystem::path();
	}

	void EditorLayer::OpenScene()
	{
		std::string filepath = FileDialogs::OpenFile("Kargono Scene (*.kargono)\0*.kargono\0");
		if (!filepath.empty())
		{
			OpenScene(filepath);
		}
	}

	void EditorLayer::OpenScene(const std::filesystem::path& path)
	{
		if (m_SceneState != SceneState::Edit)
		{
			OnSceneStop();
		}

		if (path.extension().string() != ".kargono")
		{
			KG_WARN("Could not load {0} - not a scene file", path.filename().string());
			return;
		}

		Ref<Scene> newScene = CreateRef<Scene>();
		SceneSerializer serializer(newScene);
		if (serializer.Deserialize(path.string()))
		{
			m_EditorScene = newScene;
			//m_EditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_SceneHierarchyPanel.SetContext(m_EditorScene);

			m_ActiveScene = m_EditorScene;
			m_EditorScenePath = path;
		}
	}

	void EditorLayer::SaveScene()
	{
		if (!m_EditorScenePath.empty()){ SerializeScene(m_EditorScene, m_EditorScenePath);}
		else { SaveSceneAs(); }
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filepath = FileDialogs::SaveFile("Kargono Scene (*.kargono)\0*.kargono\0");
		if (!filepath.empty())
		{
			SerializeScene(m_EditorScene, filepath);
			m_EditorScenePath = filepath;
		}
	}

	void EditorLayer::SerializeScene(Ref<Scene> scene, const std::filesystem::path& path)
	{
		KG_WARN("Serializing scene into path {}", path.string());
		SceneSerializer serializer(scene);
		serializer.Serialize(path.string());
	}

	void EditorLayer::OnScenePlay()
	{
		m_HoveredEntity = {};
		if (m_SceneState == SceneState::Simulate) { OnSceneStop(); }

		m_SceneState = SceneState::Play;

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnRuntimeStart();

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OnSceneSimulate()
	{
		m_HoveredEntity = {};
		if (m_SceneState == SceneState::Play) { OnSceneStop(); }

		m_SceneState = SceneState::Simulate;

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnSimulationStart();

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}
	void EditorLayer::OnSceneStop()
	{
		m_HoveredEntity = {};
		KG_CORE_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate, "Unknown Scene State Given to OnSceneStop")

		if (m_SceneState == SceneState::Play) { m_ActiveScene->OnRuntimeStop(); }
		else if (m_SceneState == SceneState::Simulate) { m_ActiveScene->OnSimulationStop(); }

		m_SceneState = SceneState::Edit;

		m_ActiveScene->DestroyAllEntities();
		m_ActiveScene = m_EditorScene;

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OnScenePause()
	{
		if (m_SceneState == SceneState::Edit) { return; }

		m_ActiveScene->SetPaused(true);
	}

	void EditorLayer::OnDuplicateEntity()
	{
		if (m_SceneState != SceneState::Edit)
			return;

		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		//KG_CORE_WARN("Trying to duplicate Entity: {} ({})", selectedEntity.GetName(), selectedEntity.GetUUID());
		if (selectedEntity)
		{
			Entity newEntity = m_EditorScene->DuplicateEntity(selectedEntity);
			m_SceneHierarchyPanel.SetSelectedEntity(newEntity);
		}
	}

}

#include "Panels/ViewportPanel.h"

#include "Kargono.h"

#include "EditorApp.h"


static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{

	ViewportPanel::ViewportPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName,
			KG_BIND_CLASS_FN(ViewportPanel::OnKeyPressedEditor));
	}
	void ViewportPanel::OnUpdate(Timestep ts)
	{
		// Adjust Framebuffer Size Based on Viewport
		auto& currentWindow = EngineService::GetActiveWindow();
		if (Rendering::FramebufferSpecification spec = m_ViewportFramebuffer->GetSpecification();
			static_cast<float>(currentWindow.GetViewportWidth()) > 0.0f && static_cast<float>(currentWindow.GetViewportHeight()) > 0.0f &&
			(spec.Width != currentWindow.GetViewportWidth() || spec.Height != currentWindow.GetViewportHeight()))
		{
			m_ViewportFramebuffer->Resize((uint32_t)currentWindow.GetViewportWidth(), (uint32_t)currentWindow.GetViewportHeight());
			m_EditorCamera.SetViewportSize(static_cast<float>(currentWindow.GetViewportWidth()), static_cast<float>(currentWindow.GetViewportHeight()));
		}

		// Reset Framebuffer
		Rendering::RenderingService::ResetStats();
		m_ViewportFramebuffer->Bind();
		Rendering::RendererAPI::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Rendering::RendererAPI::Clear();

		// Clear our entity ID attachment to -1
		m_ViewportFramebuffer->ClearAttachment(1, -1);
		std::string focusedWindow = EditorUI::EditorUIService::GetFocusedWindowName();
		// Update Scene
		switch (s_EditorApp->m_SceneState)
		{
		case SceneState::Edit:
		{
			if (focusedWindow == m_PanelName)
			{
				m_EditorCamera.OnUpdate(ts);
			}
			OnUpdateEditor(ts, m_EditorCamera);
			break;
		}
		case SceneState::Simulate:
		{
			if (focusedWindow == m_PanelName)
			{
				m_EditorCamera.OnUpdate(ts);
			}
			OnUpdateSimulation(ts, m_EditorCamera);
			break;
		}
		case SceneState::Play:
		{
			if (!s_EditorApp->m_IsPaused || s_EditorApp->m_StepFrames-- > 0)
			{
				// Update Scripts
				Scenes::Scene::GetActiveScene()->OnUpdateInputMode(ts);
				Scenes::Scene::GetActiveScene()->OnUpdateEntities(ts);
				Scenes::Scene::GetActiveScene()->OnUpdatePhysics(ts);
			}
			OnUpdateRuntime(ts);
			break;
		}
		}

		ProcessMousePicking();

		OnOverlayRender();

		if (s_EditorApp->m_ShowUserInterface)
		{
			auto& currentApplication = EngineService::GetActiveWindow();
			if (s_EditorApp->m_SceneState == SceneState::Play)
			{
				Scenes::Entity cameraEntity = Scenes::Scene::GetActiveScene()->GetPrimaryCameraEntity();
				Rendering::Camera* mainCamera = &cameraEntity.GetComponent<Scenes::CameraComponent>().Camera;
				Math::mat4 cameraTransform = cameraEntity.GetComponent<Scenes::TransformComponent>().GetTransform();

				if (mainCamera)
				{
					RuntimeUI::RuntimeUIService::PushRenderData(glm::inverse(cameraTransform), currentApplication.GetViewportWidth(), currentApplication.GetViewportHeight());
				}
			}
			else
			{
				Math::mat4 cameraViewMatrix = glm::inverse(m_EditorCamera.GetViewMatrix());
				RuntimeUI::RuntimeUIService::PushRenderData(cameraViewMatrix, currentApplication.GetViewportWidth(), currentApplication.GetViewportHeight());
			}

		}

		m_ViewportFramebuffer->Unbind();
	}
	void ViewportPanel::InitializeFrameBuffer()
	{
		Rendering::FramebufferSpecification fbSpec;
		fbSpec.Attachments = {Rendering::FramebufferDataFormat::RGBA8, Rendering::FramebufferDataFormat::RED_INTEGER, Rendering::FramebufferDataFormat::Depth };
		fbSpec.Width = EngineService::GetActiveWindow().GetWidth();
		fbSpec.Height = EngineService::GetActiveWindow().GetHeight();
		m_ViewportFramebuffer = Rendering::Framebuffer::Create(fbSpec);
	}
	void ViewportPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		auto& currentWindow = EngineService::GetActiveWindow();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_NoDecoration;

		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowViewport, window_flags);
		ImGui::PopStyleVar();
		auto viewportOffset = ImGui::GetWindowPos();
		static Math::uvec2 oldViewportSize = { currentWindow.GetViewportWidth(), currentWindow.GetViewportHeight() };
		Math::vec2 localViewportBounds[2];

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		Math::uvec2 aspectRatio = Utility::ScreenResolutionToAspectRatio(Projects::ProjectService::GetTargetResolution());
		if (aspectRatio.x > aspectRatio.y && ((viewportPanelSize.x / aspectRatio.x) * aspectRatio.y) < viewportPanelSize.y)
		{
			currentWindow.SetViewportWidth(static_cast<uint32_t>(viewportPanelSize.x));
			currentWindow.SetViewportHeight(static_cast<uint32_t>((viewportPanelSize.x / aspectRatio.x) * aspectRatio.y));
		}
		else
		{
			currentWindow.SetViewportWidth(static_cast<uint32_t>((viewportPanelSize.y / aspectRatio.y) * aspectRatio.x));
			currentWindow.SetViewportHeight(static_cast<uint32_t>(viewportPanelSize.y));
		}

		localViewportBounds[0] = { (viewportPanelSize.x - static_cast<float>(currentWindow.GetViewportWidth())) * 0.5f, (viewportPanelSize.y - static_cast<float>(currentWindow.GetViewportHeight())) * 0.5f };
		localViewportBounds[1] = { m_ViewportBounds[0].x + static_cast<float>(currentWindow.GetViewportWidth()),  m_ViewportBounds[0].y + static_cast<float>(currentWindow.GetViewportHeight()) };
		m_ViewportBounds[0] = { localViewportBounds[0].x + viewportOffset.x, localViewportBounds[0].y + viewportOffset.y };
		m_ViewportBounds[1] = { m_ViewportBounds[0].x + static_cast<float>(currentWindow.GetViewportWidth()), m_ViewportBounds[0].y + static_cast<float>(currentWindow.GetViewportHeight()) };
		ImGui::SetCursorPos(ImVec2(localViewportBounds[0].x, localViewportBounds[0].y));
		uint64_t textureID = m_ViewportFramebuffer->GetColorAttachmentRendererID();
		ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ static_cast<float>(currentWindow.GetViewportWidth()), static_cast<float>(currentWindow.GetViewportHeight()) }, ImVec2{ 0, 1 },
			ImVec2{ 1, 0 });
		if ((s_EditorApp->m_SceneState == SceneState::Edit || s_EditorApp->m_SceneState == SceneState::Simulate) ||
			(s_EditorApp->m_SceneState == SceneState::Play && s_EditorApp->m_IsPaused))
		{
			if (ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::GetIO().WantCaptureMouse)
			{
				s_EditorApp->OnMouseButtonPressed({ Mouse::ButtonLeft });
			}
		}

		Math::uvec2 viewportSize = { currentWindow.GetViewportWidth(), currentWindow.GetViewportHeight() };
		if (oldViewportSize != viewportSize)
		{
			Scenes::Scene::GetActiveScene()->OnViewportResize((uint32_t)currentWindow.GetViewportWidth(), (uint32_t)currentWindow.GetViewportHeight());
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_SCENE"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				s_EditorApp->OpenScene(path);
			}
			ImGui::EndDragDropTarget();
		}

		if (s_EditorApp->m_SceneState == SceneState::Edit || s_EditorApp->m_SceneState == SceneState::Simulate)
		{
			// Gizmos
			Scenes::Entity selectedEntity = *Scenes::Scene::GetActiveScene()->GetSelectedEntity();
			if (selectedEntity && m_GizmoType != -1)
			{
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y,
					m_ViewportBounds[1].x - m_ViewportBounds[0].x,
					m_ViewportBounds[1].y - m_ViewportBounds[0].y);

				// Editor Camera
				const Math::mat4& cameraProjection = m_EditorCamera.GetProjection();
				Math::mat4 cameraView = m_EditorCamera.GetViewMatrix();

				// Entity Transform
				auto& tc = selectedEntity.GetComponent<Scenes::TransformComponent>();
				Math::mat4 transform = tc.GetTransform();

				// Snapping
				bool snap = Input::InputPolling::IsKeyPressed(Key::LeftControl);
				float snapValue = 0.5f;
				if (m_GizmoType == ImGuizmo::OPERATION::ROTATE) { snapValue = 45.0f; }

				float snapValues[3] = { snapValue, snapValue, snapValue };

				ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
					(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
					nullptr, snap ? snapValues : nullptr);
				if (ImGuizmo::IsUsing())
				{
					Math::vec3 translation, rotation, scale;
					Math::DecomposeTransform(transform, translation, rotation, scale);

					Math::vec3 deltaRotation = rotation - tc.Rotation;
					tc.Translation = translation;
					tc.Rotation += deltaRotation;
					tc.Scale = scale;
				}
			}
		}

		static bool toolbarEnabled{ true };
		constexpr float iconSize{ 36.0f };
		constexpr ImVec4 topBarBackgroundColor{0.0f, 0.0f, 0.0f, 0.93f};

		ImGui::PushStyleColor(ImGuiCol_Button, EditorUI::EditorUIService::s_PureEmpty);
		// {0.2f, 0.2f, 0.2f, 0.63f} // OLD BACKGROUND COLOR
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 windowPos = ImGui::GetWindowPos();
		ImVec2 windowSize = ImGui::GetWindowSize();
		Ref<Rendering::Texture2D> icon {nullptr};

		if (toolbarEnabled)
		{
			// Draw Play/Simulate/Step Background
			draw_list->AddRectFilled(ImVec2(windowPos.x + (windowSize.x / 2) - 90.0f, windowPos.y),
				ImVec2(windowPos.x + (windowSize.x / 2) + 90.0f, windowPos.y + 43.0f),
				ImColor(topBarBackgroundColor), 12.0f, ImDrawFlags_RoundCornersBottom);

			// Draw Display Options Background
			draw_list->AddRectFilled(ImVec2(windowPos.x + windowSize.x - 80.0f, windowPos.y),
				ImVec2(windowPos.x + (windowSize.x) - 48.0f, windowPos.y + 30.0f),
				ImColor(topBarBackgroundColor), 12.0f, ImDrawFlags_RoundCornersBottom);

			// Draw Camera Options Background
			draw_list->AddRectFilled(ImVec2(windowPos.x + windowSize.x - 170.0f, windowPos.y),
				ImVec2(windowPos.x + (windowSize.x) - 100.0f, windowPos.y + 30.0f),
				ImColor(topBarBackgroundColor), 12.0f, ImDrawFlags_RoundCornersBottom);

			// Draw Toggle Top Bar Background
			draw_list->AddRectFilled(ImVec2(windowPos.x + windowSize.x - 30.0f, windowPos.y),
				ImVec2(windowPos.x + (windowSize.x), windowPos.y + 30.0f),
				ImColor(topBarBackgroundColor), 12.0f, ImDrawFlags_RoundCornersBottomLeft);

			bool hasPlayButton = s_EditorApp->m_SceneState == SceneState::Edit || s_EditorApp->m_SceneState == SceneState::Simulate;
			bool hasSimulateButton = s_EditorApp->m_SceneState == SceneState::Edit || s_EditorApp->m_SceneState == SceneState::Play;
			bool hasPauseButton = s_EditorApp->m_SceneState != SceneState::Edit;
			bool hasStepButton = hasPauseButton && s_EditorApp->m_IsPaused;

			// Play/Stop Button
			if (!hasSimulateButton)
			{
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorUI::EditorUIService::s_PureEmpty);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, EditorUI::EditorUIService::s_PureEmpty);
			}
			ImGui::SetCursorPos(ImVec2((windowSize.x / 2) - 77.0f, 4));
			icon = hasPlayButton ? EditorUI::EditorUIService::s_IconPlayActive : EditorUI::EditorUIService::s_IconStopActive;
			if (ImGui::ImageButton((ImTextureID)(uint64_t)(hasSimulateButton ? icon : EditorUI::EditorUIService::s_IconPlay)->GetRendererID(), ImVec2(iconSize, iconSize), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f))
				&& toolbarEnabled)
			{
				if (hasSimulateButton)
				{
					if (s_EditorApp->m_SceneState == SceneState::Edit || s_EditorApp->m_SceneState == SceneState::Simulate)
					{
						s_EditorApp->OnPlay();
					}
					else if (s_EditorApp->m_SceneState == SceneState::Play)
					{
						s_EditorApp->OnStop();
					}
				}
				
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				if (hasSimulateButton)
				{
					ImGui::BeginTooltip();
					ImGui::TextColored(EditorUI::EditorUIService::s_PearlBlue, hasPlayButton ?
						"Run Application" : "Stop Application");
					ImGui::EndTooltip();
				}
			}
			if (!hasSimulateButton)
			{
				ImGui::PopStyleColor(2);
			}

			// Simulate/Stop Simulate
			if (!hasPlayButton)
			{
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorUI::EditorUIService::s_PureEmpty);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, EditorUI::EditorUIService::s_PureEmpty);
			}
			icon = hasSimulateButton ? EditorUI::EditorUIService::s_IconSimulateActive : EditorUI::EditorUIService::s_IconStopActive;
			ImGui::SetCursorPos(ImVec2((windowSize.x / 2) - 37.0f, 4));
			if (ImGui::ImageButton((ImTextureID)(uint64_t)(hasPlayButton ? icon : EditorUI::EditorUIService::s_IconSimulate)->GetRendererID(), ImVec2(iconSize, iconSize), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f))
				&& toolbarEnabled)
			{
				if (hasPlayButton)
				{
					if (s_EditorApp->m_SceneState == SceneState::Edit || s_EditorApp->m_SceneState == SceneState::Play)
					{
						s_EditorApp->OnSimulate();
					}
					else if (s_EditorApp->m_SceneState == SceneState::Simulate)
					{
						s_EditorApp->OnStop();
					}
				}
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				if (hasPlayButton)
				{
					ImGui::BeginTooltip();
					ImGui::TextColored(EditorUI::EditorUIService::s_PearlBlue, hasSimulateButton ?
						"Simulate Physics" : "Stop Physics Simulation");
					ImGui::EndTooltip();
				}
			}
			if (!hasPlayButton)
			{
				ImGui::PopStyleColor(2);
			}

			// Pause Icon
			if (!hasPauseButton)
			{
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorUI::EditorUIService::s_PureEmpty);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, EditorUI::EditorUIService::s_PureEmpty);
			}
			icon = hasPauseButton ? EditorUI::EditorUIService::s_IconPauseActive: EditorUI::EditorUIService::s_IconPause;
			ImGui::SetCursorPos(ImVec2((windowSize.x / 2) + 3.0f, 4));
			if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(iconSize, iconSize), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f))
				&& toolbarEnabled)
			{
				if (hasPauseButton)
				{
					s_EditorApp->m_IsPaused = !s_EditorApp->m_IsPaused;
				}
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				if (hasPauseButton)
				{
					ImGui::BeginTooltip();
					ImGui::TextColored(EditorUI::EditorUIService::s_PearlBlue, s_EditorApp->m_IsPaused ? "Resume Application" : "Pause Application");
					ImGui::EndTooltip();
				}
			}
			if (!hasPauseButton)
			{
				ImGui::PopStyleColor(2);
			}
			// Step Icon
			if (!hasStepButton)
			{
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorUI::EditorUIService::s_PureEmpty);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, EditorUI::EditorUIService::s_PureEmpty);
			}
			icon = hasStepButton ? EditorUI::EditorUIService::s_IconStepActive : EditorUI::EditorUIService::s_IconStep;
			ImGui::SetCursorPos(ImVec2((windowSize.x / 2) + 43.0f, 4));
			if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(iconSize, iconSize), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f))
				&& toolbarEnabled)
			{
				if (hasStepButton)
				{
					s_EditorApp->Step(1);
				}
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				if (hasStepButton)
				{
					ImGui::BeginTooltip();
					ImGui::TextColored(EditorUI::EditorUIService::s_PearlBlue, "Step Application");
					ImGui::EndTooltip();
				}
			}
			if (!hasStepButton)
			{
				ImGui::PopStyleColor(2);
			}

			// Camera Options Button
			icon = EditorUI::EditorUIService::s_IconCameraActive;
			ImGui::SetCursorPos(ImVec2(windowSize.x - 163, 7));
			if (ImGui::ImageButton("Camera Options",
				(ImTextureID)(uint64_t)icon->GetRendererID(),
				ImVec2(16, 14), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
				EditorUI::EditorUIService::s_PureEmpty,
				EditorUI::EditorUIService::s_PureWhite))
			{
				ImGui::OpenPopup("Toggle Viewport Camera Options");
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUI::EditorUIService::s_PearlBlue, "Camera Movement Types");
				ImGui::EndTooltip();
			}

			if (ImGui::BeginPopup("Toggle Viewport Camera Options"))
			{
				if (ImGui::MenuItem("Model Viewer", 0,
					m_EditorCamera.GetMovementType() == Rendering::EditorCamera::MovementType::ModelView))
				{
					m_EditorCamera.SetMovementType(Rendering::EditorCamera::MovementType::ModelView);
				}
				if (ImGui::MenuItem("FreeFly", 0,
					m_EditorCamera.GetMovementType() == Rendering::EditorCamera::MovementType::FreeFly))
				{
					m_EditorCamera.SetMovementType(Rendering::EditorCamera::MovementType::FreeFly);
				}
				ImGui::EndPopup();
			}

			// Camera Speed
			ImGui::SetNextItemWidth(30.0f);
			ImGui::SetCursorPos(ImVec2(windowSize.x - 136, 4));
			ImGui::DragFloat("##CameraSpeed", &m_EditorCamera.GetMovementSpeed(), 0.5f,
				s_EditorApp->m_ViewportPanel->m_EditorCamera.GetMinMovementSpeed(), m_EditorCamera.GetMaxMovementSpeed(),
				"%.0f", ImGuiSliderFlags_NoInput);
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUI::EditorUIService::s_PearlBlue, "Camera Speed");
				ImGui::EndTooltip();
			}

			// Viewport Display Options Button
			icon = EditorUI::EditorUIService::s_IconDisplayActive;
			ImGui::SetCursorPos(ImVec2(windowSize.x - 75, 4));
			if (ImGui::ImageButton("Display Toggle",
				(ImTextureID)(uint64_t)icon->GetRendererID(),
				ImVec2(14, 14), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
				EditorUI::EditorUIService::s_PureEmpty,
				EditorUI::EditorUIService::s_PureWhite))
			{
				ImGui::OpenPopup("Toggle Display Options");
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUI::EditorUIService::s_PearlBlue, "Display Options");
				ImGui::EndTooltip();
			}

			if (ImGui::BeginPopup("Toggle Display Options"))
			{
				if (ImGui::MenuItem("Display Physics Colliders", 0, s_EditorApp->m_ShowPhysicsColliders))
				{
					Utility::Operations::ToggleBoolean(s_EditorApp->m_ShowPhysicsColliders);
				}
				if (ImGui::MenuItem("Display Camera Frustums", 0, s_EditorApp->m_ShowCameraFrustums))
				{
					Utility::Operations::ToggleBoolean(s_EditorApp->m_ShowCameraFrustums);
				}
				if (ImGui::MenuItem("Display Runtime UI", 0, s_EditorApp->m_ShowUserInterface))
				{
					Utility::Operations::ToggleBoolean(s_EditorApp->m_ShowUserInterface);
				}
				if (ImGui::MenuItem("Fullscreen While Running", 0, s_EditorApp->m_RuntimeFullscreen))
				{
					Utility::Operations::ToggleBoolean(s_EditorApp->m_RuntimeFullscreen);
				}
				ImGui::EndPopup();
			}
		}

		// Toggle Top Bar Button
		icon = toolbarEnabled ? EditorUI::EditorUIService::s_IconCheckbox_Check_Enabled :
		EditorUI::EditorUIService::s_IconCheckbox_Empty_Disabled;
		ImGui::SetCursorPos(ImVec2(windowSize.x - 25, 4));
		if (ImGui::ImageButton("Toggle Top Bar",
			(ImTextureID)(uint64_t)icon->GetRendererID(),
			ImVec2(14, 14), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
			EditorUI::EditorUIService::s_PureEmpty,
			EditorUI::EditorUIService::s_PureWhite))
		{
			Utility::Operations::ToggleBoolean(toolbarEnabled);
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetNextFrameWantCaptureMouse(false);
			ImGui::BeginTooltip();
			ImGui::TextColored(EditorUI::EditorUIService::s_PearlBlue, toolbarEnabled ? "Close Toolbar" : "Open Toolbar");
			ImGui::EndTooltip();
		}

		ImGui::PopStyleColor();

		EditorUI::EditorUIService::EndWindow();
	}
	void ViewportPanel::OnEvent(Events::Event& event)
	{
		if (s_EditorApp->m_SceneState == SceneState::Edit || s_EditorApp->m_SceneState == SceneState::Simulate)
		{
			m_EditorCamera.OnEvent(event);
		}
	}
	bool ViewportPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		bool control = Input::InputPolling::IsKeyPressed(Key::LeftControl) || Input::InputPolling::IsKeyPressed(Key::RightControl);
		bool shift = Input::InputPolling::IsKeyPressed(Key::LeftShift) || Input::InputPolling::IsKeyPressed(Key::RightShift);
		bool alt = Input::InputPolling::IsKeyPressed(Key::LeftAlt) || Input::InputPolling::IsKeyPressed(Key::RightAlt);

		switch (event.GetKeyCode())
		{
			case Key::Escape:
			{
				s_EditorApp->m_SceneHierarchyPanel->SetSelectedEntity({});
				return true;
			}
			case Key::Tab:
			{
				s_EditorApp->m_ViewportPanel->m_EditorCamera.ToggleMovementType();
				return true;
			}

			// Gizmos
			case Key::Q:
			{
				if (!ImGuizmo::IsUsing() && !alt)
				{
					s_EditorApp->m_ViewportPanel->m_GizmoType = -1;
					return true;
				}
				return false;
			}
			case Key::W:
			{
				if (!ImGuizmo::IsUsing() && !alt)
				{
					s_EditorApp->m_ViewportPanel->m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
					return true;
				}
				return false;
			}
			case Key::E:
			{
				if (!ImGuizmo::IsUsing() && !alt)
				{
					s_EditorApp->m_ViewportPanel->m_GizmoType = ImGuizmo::OPERATION::ROTATE;
					return true;
				}
				return false;
			}
			default:
			{
				return false;
			}
		}
	}
	void ViewportPanel::ProcessMousePicking()
	{
		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		Math::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		my = viewportSize.y - my;

		int mouseX = (int)mx;
		int mouseY = (int)my;

		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
		{
			int pixelData = m_ViewportFramebuffer->ReadPixel(1, mouseX, mouseY);
			*Scenes::Scene::GetActiveScene()->GetHoveredEntity() = 
				Scenes::Scene::GetActiveScene()->CheckEntityExists((entt::entity)pixelData) ? 
				Scenes::Entity((entt::entity)pixelData, Scenes::Scene::GetActiveScene().get()) : Scenes::Entity();
		}
	}

	void ViewportPanel::OnUpdateEditor(Timestep ts, Rendering::EditorCamera& camera)
	{
		Scenes::Scene::GetActiveScene()->RenderScene(camera, camera.GetViewMatrix());
	}

	void ViewportPanel::OnUpdateRuntime(Timestep ts)
	{

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

	void ViewportPanel::OnUpdateSimulation(Timestep ts, Rendering::EditorCamera& camera)
	{
		EditorApp* editorLayer = EditorApp::GetCurrentApp();

		if (!editorLayer->m_IsPaused || editorLayer->m_StepFrames-- > 0)
		{
			Scenes::Scene::GetActiveScene()->OnUpdatePhysics(ts);
		}

		// Render
		Scenes::Scene::GetActiveScene()->RenderScene(camera, camera.GetViewMatrix());
	}

	static Rendering::RendererInputSpec s_CircleInputSpec{};
	static Rendering::RendererInputSpec s_LineInputSpec{};
	static Math::vec4 s_RectangleVertexPositions[4]
	{
		{ -0.5f, -0.5f, 0.0f, 1.0f },
			{ 0.5f, -0.5f, 0.0f, 1.0f },
			{ 0.5f, 0.5f, 0.0f, 1.0f },
			{ -0.5f, 0.5f, 0.0f, 1.0f }
	};
	static Math::vec4 s_CubeVertexPositions[8]
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

	static Math::uvec2 s_CubeIndices[12]
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

	static Math::vec4 s_FrustrumVertexPositions[8]
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

	static Math::uvec2 s_FrustrumIndices[16]
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

	static Ref<std::vector<Math::vec3>> s_OutputVector = CreateRef<std::vector<Math::vec3>>();

	void ViewportPanel::InitializeOverlayData()
	{
		// Set up Line Input Specifications for Overlay Calls
		{
			Rendering::ShaderSpecification lineShaderSpec{Rendering::ColorInputType::FlatColor, Rendering::TextureInputType::None, false, true, false, Rendering::RenderingType::DrawLine, false };
			auto [uuid, localShader] = Assets::AssetManager::GetShader(lineShaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			Rendering::Shader::SetDataAtInputLocation<Math::vec4>({ 0.0f, 1.0f, 0.0f, 1.0f }, "a_Color", localBuffer, localShader);

			Scenes::ShapeComponent* lineShapeComponent = new Scenes::ShapeComponent();
			lineShapeComponent->CurrentShape = Rendering::ShapeTypes::None;
			lineShapeComponent->Vertices = nullptr;

			s_LineInputSpec.Shader = localShader;
			s_LineInputSpec.Buffer = localBuffer;
			s_LineInputSpec.ShapeComponent = lineShapeComponent;
		}
		// Set up Circle Input Specification for Overlay Calls
		{
			Rendering::ShaderSpecification shaderSpec{Rendering::ColorInputType::FlatColor, Rendering::TextureInputType::None, true, true, false, Rendering::RenderingType::DrawIndex, false };
			auto [uuid, localShader] = Assets::AssetManager::GetShader(shaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			Rendering::Shader::SetDataAtInputLocation<Math::vec4>({ 0.0f, 1.0f, 0.0f, 1.0f }, "a_Color", localBuffer, localShader);
			Rendering::Shader::SetDataAtInputLocation<float>(0.05f, "a_Thickness", localBuffer, localShader);
			Rendering::Shader::SetDataAtInputLocation<float>(0.005f, "a_Fade", localBuffer, localShader);

			Scenes::ShapeComponent* shapeComp = new Scenes::ShapeComponent();
			shapeComp->CurrentShape = Rendering::ShapeTypes::Quad;
			shapeComp->Vertices = CreateRef<std::vector<Math::vec3>>(Rendering::Shape::s_Quad.GetIndexVertices());
			shapeComp->Indices = CreateRef<std::vector<uint32_t>>(Rendering::Shape::s_Quad.GetIndices());

			s_CircleInputSpec.Shader = localShader;
			s_CircleInputSpec.Buffer = localBuffer;
			s_CircleInputSpec.ShapeComponent = shapeComp;
		}

		// TODO: Shape Components and Buffers are memory leaks!
	}

	void ViewportPanel::OnOverlayRender()
	{
		if (s_EditorApp->m_SceneState == SceneState::Play)
		{
			Scenes::Entity camera = Scenes::Scene::GetActiveScene()->GetPrimaryCameraEntity();
			if (!camera) { return; }
			Rendering::RenderingService::BeginScene(camera.GetComponent<Scenes::CameraComponent>().Camera, glm::inverse(camera.GetComponent<Scenes::TransformComponent>().GetTransform()));
		}
		else
		{
			Rendering::RenderingService::BeginScene(m_EditorCamera);
		}

		if (s_EditorApp->m_ShowPhysicsColliders)
		{
			// Circle Colliders
			{
				auto view = Scenes::Scene::GetActiveScene()->GetAllEntitiesWith<Scenes::TransformComponent, Scenes::CircleCollider2DComponent>();
				for (auto entity : view)
				{
					auto [tc, cc2d] = view.get<Scenes::TransformComponent, Scenes::CircleCollider2DComponent>(entity);

					Math::vec3 translation = tc.Translation + Math::vec3(cc2d.Offset.x, cc2d.Offset.y, 0.001f);

					Math::vec3 scale = tc.Scale * Math::vec3(cc2d.Radius * 2.0f);

					Math::mat4 transform = glm::translate(Math::mat4(1.0f), translation)
						* glm::scale(Math::mat4(1.0f), scale);

					s_CircleInputSpec.TransformMatrix = transform;
					Rendering::RenderingService::SubmitDataToRenderer(s_CircleInputSpec);
				}
			}
			// Box Colliders
			{
				auto view = Scenes::Scene::GetActiveScene()->GetAllEntitiesWith<Scenes::TransformComponent, Scenes::BoxCollider2DComponent>();
				for (auto entity : view)
				{
					auto [tc, bc2d] = view.get<Scenes::TransformComponent, Scenes::BoxCollider2DComponent>(entity);

					Math::vec3 translation = tc.Translation + Math::vec3(bc2d.Offset.x, bc2d.Offset.y, 0.001f);
					Math::vec3 scale = tc.Scale * Math::vec3(bc2d.Size * 2.0f, 1.0f);

					Math::mat4 transform = glm::translate(Math::mat4(1.0f), translation)
						* glm::rotate(Math::mat4(1.0f), tc.Rotation.z, Math::vec3(0.0f, 0.0f, 1.0f))
						* glm::scale(Math::mat4(1.0f), scale);

					static Math::vec4 boxColliderColor {0.0f, 1.0f, 0.0f, 1.0f};
					Rendering::Shader::SetDataAtInputLocation<Math::vec4>(boxColliderColor, "a_Color", s_LineInputSpec.Buffer, s_LineInputSpec.Shader);

					Math::vec3 lineVertices[4];
					for (size_t i = 0; i < 4; i++)
					{
						lineVertices[i] = transform * s_RectangleVertexPositions[i];
					}
					s_OutputVector->clear();
					s_OutputVector->push_back(lineVertices[0]);
					s_OutputVector->push_back(lineVertices[1]);
					s_LineInputSpec.ShapeComponent->Vertices = s_OutputVector;
					Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
					s_OutputVector->clear();
					s_OutputVector->push_back(lineVertices[1]);
					s_OutputVector->push_back(lineVertices[2]);
					s_LineInputSpec.ShapeComponent->Vertices = s_OutputVector;
					Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
					s_OutputVector->clear();
					s_OutputVector->push_back(lineVertices[2]);
					s_OutputVector->push_back(lineVertices[3]);
					s_LineInputSpec.ShapeComponent->Vertices = s_OutputVector;
					Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
					s_OutputVector->clear();
					s_OutputVector->push_back(lineVertices[3]);
					s_OutputVector->push_back(lineVertices[0]);
					s_LineInputSpec.ShapeComponent->Vertices = s_OutputVector;
					Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				}
			}
		}

		if (s_EditorApp->m_SceneState == SceneState::Edit || s_EditorApp->m_SceneState == SceneState::Simulate || (s_EditorApp->m_SceneState == SceneState::Play && s_EditorApp->m_IsPaused))
		{
			// Draw selected entity outline 
			if (Scenes::Entity selectedEntity = *Scenes::Scene::GetActiveScene()->GetSelectedEntity()) {
				Scenes::TransformComponent transform = selectedEntity.GetComponent<Scenes::TransformComponent>();
				static Math::vec4 selectionColor {1.0f, 0.5f, 0.0f, 1.0f};
				Rendering::Shader::SetDataAtInputLocation<Math::vec4>(selectionColor, "a_Color", s_LineInputSpec.Buffer, s_LineInputSpec.Shader);

				Math::vec3 lineVertices[8];

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
					Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				}

				if (selectedEntity.HasComponent<Scenes::CameraComponent>() && s_EditorApp->m_ShowCameraFrustums)
				{
					DrawFrustrum(selectedEntity);
				}
			}
		}

		Rendering::RenderingService::EndScene();
	}

	void ViewportPanel::DrawFrustrum(Scenes::Entity& entity)
	{
		auto& transform = entity.GetComponent<Scenes::TransformComponent>();
		auto& camera = entity.GetComponent<Scenes::CameraComponent>();
		float windowWidth = (float)EngineService::GetActiveWindow().GetWidth();
		float windowHeight = (float)EngineService::GetActiveWindow().GetHeight();
		Math::vec4 viewport = { 0.0f, 0.0f, windowWidth, windowHeight };
		auto cameraProjectionType = camera.Camera.GetProjectionType();
		Math::vec4 selectionColor { 0.5f, 0.3f, 0.85f, 1.0f };
		Rendering::Shader::SetDataAtInputLocation<Math::vec4>(selectionColor, "a_Color", s_LineInputSpec.Buffer, s_LineInputSpec.Shader);

		Math::vec3 lineVertices[9];

		for (size_t i = 0; i < 8; i++)
		{
			Math::vec4 vertexPosition = s_FrustrumVertexPositions[i];
			Math::vec4 localSpaceCoordinates = glm::inverse(camera.Camera.GetProjection()) * s_FrustrumVertexPositions[i];
			localSpaceCoordinates = localSpaceCoordinates / localSpaceCoordinates.w; // Perspective Division
			lineVertices[i] = transform.GetTranslation() * transform.GetRotation() * localSpaceCoordinates;

		}

		lineVertices[8] = (transform.GetTransform() * Math::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		bool colorChanged = false;
		uint32_t iteration = 0;
		for (auto& indices : s_FrustrumIndices)
		{
			if (iteration >= 12 && !colorChanged)
			{
				selectionColor = { 0.3f, 0.1f, 0.8f, 1.0f };
				Rendering::Shader::SetDataAtInputLocation<Math::vec4>(selectionColor, "a_Color", s_LineInputSpec.Buffer, s_LineInputSpec.Shader);
				colorChanged = true;
			}
			s_OutputVector->clear();
			s_OutputVector->push_back(lineVertices[indices.x]);
			s_OutputVector->push_back(lineVertices[indices.y]);
			s_LineInputSpec.ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
			iteration++;
		}

	}

}

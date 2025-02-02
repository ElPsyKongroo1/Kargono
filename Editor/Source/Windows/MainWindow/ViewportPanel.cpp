#include "Windows/MainWindow/ViewportPanel.h"

#include "EditorApp.h"

#include "Kargono/Rendering/RenderingService.h"
#include "Kargono/Rendering/Texture.h"
#include "Kargono/Scenes/Scene.h"
#include "Kargono/Input/InputService.h"
#include "Kargono/Utility/Time.h"
#include "Kargono/Events/EditorEvent.h"
#include "Kargono/Utility/Operations.h"
#include "Kargono/Particles/ParticleService.h"

static Kargono::EditorApp* s_EditorApp { nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };

namespace Kargono::Panels
{

	ViewportPanel::ViewportPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
		s_MainWindow->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(ViewportPanel::OnKeyPressedEditor));

		InitializeFrameBuffer();

		m_EditorCamera = Rendering::EditorPerspectiveCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		m_EditorCamera.SetFocalPoint({ 58.0f, 27.0f, 93.0f });
		m_EditorCamera.SetDistance(1.0f);
		m_EditorCamera.SetPitch(0.195f);
		m_EditorCamera.SetYaw(-0.372f);

		InitializeOverlayData();

		KG_ASSERT(Projects::ProjectService::GetActive());

		SetViewportAspectRatio(Utility::ScreenResolutionToAspectRatio(Projects::ProjectService::GetActiveTargetResolution()));
		
	}


	ViewportPanel::~ViewportPanel()
	{
		ClearOverlayData();
	}
	void ViewportPanel::OnUpdate(Timestep ts)
	{
		KG_PROFILE_FUNCTION();

		// Adjust framebuffer & camera viewport size if necessary
		Window& currentWindow = EngineService::GetActiveWindow();
		currentWindow.SetActiveViewport(&m_ViewportData);
		if (Rendering::FramebufferSpecification spec = m_ViewportFramebuffer->GetSpecification();
			(float)m_ViewportData.m_Width > 0.0f && (float)m_ViewportData.m_Height > 0.0f &&
			(spec.Width != m_ViewportData.m_Width || spec.Height != m_ViewportData.m_Height))
		{
			// Update framebuffer and camera viewport size
			m_ViewportFramebuffer->Resize(m_ViewportData.m_Width, m_ViewportData.m_Height);
			m_EditorCamera.SetViewportSize((float)m_ViewportData.m_Width, (float)m_ViewportData.m_Height);
		}

		// Prepare for rendering
		Ref<Scenes::Scene> activeScene{ Scenes::SceneService::GetActiveScene() };
		Rendering::RenderingService::ResetStats();
		m_ViewportFramebuffer->Bind();
		Rendering::RendererAPI::SetClearColor(activeScene->m_BackgroundColor);
		Rendering::RendererAPI::Clear();

		// Clear mouse picking buffer
		m_ViewportFramebuffer->SetAttachment(1, -1);
		FixedString32 focusedWindow{ EditorUI::EditorUIService::GetFocusedWindowName() };

		// Update Scene
		switch (s_MainWindow->m_SceneState)
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
			if (!s_MainWindow->m_IsPaused || s_MainWindow->m_StepFrames-- > 0)
			{
				// Process AI
				AI::AIService::OnUpdate(ts);
				// Process Input Mode
				Input::InputMapService::OnUpdate(ts);
				// Process entity OnUpdate
				Scenes::SceneService::GetActiveScene()->OnUpdateEntities(ts);
				// Process physics
				Physics::Physics2DService::OnUpdate(ts);
			}
			OnUpdateRuntime(ts);
			break;
		}
		}

		// Process Particles
		Particles::ParticleService::OnUpdate(ts);

		// Use mouse picking buffer to handle scene mouse picking
		HandleSceneMouseHovering();

		OnOverlayRender();

		// Handle drawing user interface
		Window& currentApplication = EngineService::GetActiveWindow();
		if (s_MainWindow->m_SceneState == SceneState::Play)
		{
			ECS::Entity cameraEntity = Scenes::SceneService::GetActiveScene()->GetPrimaryCameraEntity();
			if (cameraEntity)
			{
				Rendering::Camera* mainCamera = &cameraEntity.GetComponent<ECS::CameraComponent>().Camera;

				if (mainCamera)
				{
					// Get camera transform
					Math::mat4 cameraTransform = cameraEntity.GetComponent<ECS::TransformComponent>().GetTransform();
					Math::mat4 cameraViewProjection = mainCamera->GetProjection() * glm::inverse(cameraTransform);

					// Render particles
					Particles::ParticleService::OnRender(cameraViewProjection);

				}
			}
		}
		else
		{
			// Render particles
			Particles::ParticleService::OnRender(m_EditorCamera.GetViewProjection());
		}

		if (s_MainWindow->m_SceneState == SceneState::Play)
		{
			// Clear mouse picking buffer again
			m_ViewportFramebuffer->SetAttachment(1, -1);

			// Render RuntimeUI directory to viewport bounds
			RuntimeUI::RuntimeUIService::OnRender(m_ViewportData.m_Width, m_ViewportData.m_Height);

			// Use mouse picking buffer to handle runtime UI mouse picking
			HandleUIMouseHovering();
		}
		
		m_ViewportFramebuffer->Unbind();
	}
	void ViewportPanel::AddDebugLine(Math::vec3 startPoint, Math::vec3 endPoint)
	{
		m_DebugLines.push_back({startPoint, endPoint});
	}
	void ViewportPanel::AddDebugPoint(Math::vec3 startPoint)
	{
		m_DebugPoints.push_back({ startPoint });
	}
	void ViewportPanel::ClearDebugLines()
	{
		m_DebugLines.clear();
	}
	void ViewportPanel::ClearDebugPoints()
	{
		m_DebugPoints.clear();
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

		// Create Window
		Window& currentWindow = EngineService::GetActiveWindow();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		EditorUI::EditorUIService::StartWindow(m_PanelName.CString(), &s_MainWindow->m_ShowViewport, NULL);
		ImGui::PopStyleVar();

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}
		Math::uvec2 oldViewportSize = { m_ViewportData.m_Width, m_ViewportData.m_Height };

		EditorUI::EditorUIService::AutoCalcViewportSize(m_ScreenViewportBounds, m_ViewportData, m_ViewportFocused, m_ViewportHovered,
			m_ViewportAspectRatio);
		
		uint64_t textureID = m_ViewportFramebuffer->GetColorAttachmentRendererID();
		ImGui::Image((ImTextureID)textureID, ImVec2{ (float)m_ViewportData.m_Width, (float)m_ViewportData.m_Height }, ImVec2{ 0, 1 },
			ImVec2{ 1, 0 });
		if ((s_MainWindow->m_SceneState == SceneState::Edit || s_MainWindow->m_SceneState == SceneState::Simulate) ||
			(s_MainWindow->m_SceneState == SceneState::Play && s_MainWindow->m_IsPaused))
		{
			if (ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::GetIO().WantCaptureMouse)
			{
				// Handle selecting entities inside of the viewport panel
				if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::InputService::IsKeyPressed(Key::LeftAlt))
				{
					
					if (*Scenes::SceneService::GetActiveScene()->GetHoveredEntity())
					{
						s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(*Scenes::SceneService::GetActiveScene()->GetHoveredEntity());
						s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::None);

						// Algorithm to enable double clicking for an entity!
						static float previousTime{ 0.0f };
						static ECS::Entity previousEntity{};
						float currentTime = Utility::Time::GetTime();
						if (std::fabs(currentTime - previousTime) < 0.2f && *Scenes::SceneService::GetActiveScene()->GetHoveredEntity() == previousEntity)
						{
							ECS::TransformComponent& transformComponent = Scenes::SceneService::GetActiveScene()->GetHoveredEntity()->GetComponent<ECS::TransformComponent>();
							m_EditorCamera.SetFocalPoint(transformComponent.Translation);
							m_EditorCamera.SetDistance(std::max({ transformComponent.Scale.x, transformComponent.Scale.y, transformComponent.Scale.z }) * 2.5f);
							m_EditorCamera.SetMovementType(Rendering::EditorPerspectiveCamera::MovementType::ModelView);
						}
						previousTime = currentTime;
						previousEntity = *Scenes::SceneService::GetActiveScene()->GetHoveredEntity();
					}
				}
				
			}
		}
		else
		{
			if (ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::GetIO().WantCaptureMouse)
			{
				// Handle selecting entities inside of the viewport panel
				if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::InputService::IsKeyPressed(Key::LeftAlt))
				{

					if (m_HoveredWindowID != RuntimeUI::k_InvalidWindowID && m_HoveredWidgetID != RuntimeUI::k_InvalidWidgetID)
					{
						RuntimeUI::RuntimeUIService::OnPressByIndex({ RuntimeUI::RuntimeUIService::GetActiveUIHandle(),
							m_HoveredWindowID, m_HoveredWidgetID });
					}
				}
			}
		}

		// Check if the viewport size has changed and update the scene's viewport
		if (oldViewportSize.x != m_ViewportData.m_Width || oldViewportSize.y != m_ViewportData.m_Height)
		{
			Scenes::SceneService::GetActiveScene()->OnViewportResize((uint32_t)m_ViewportData.m_Width, (uint32_t)m_ViewportData.m_Height);
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_SCENE"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				s_MainWindow->OpenScene(path);
			}
			ImGui::EndDragDropTarget();
		}

		if (s_MainWindow->m_SceneState == SceneState::Edit || s_MainWindow->m_SceneState == SceneState::Simulate)
		{
			// Gizmos
			ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (selectedEntity && m_GizmoType != -1)
			{
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(m_ScreenViewportBounds[0].x, m_ScreenViewportBounds[0].y,
					m_ScreenViewportBounds[1].x - m_ScreenViewportBounds[0].x,
					m_ScreenViewportBounds[1].y - m_ScreenViewportBounds[0].y);

				// Editor Camera
				const Math::mat4& cameraProjection = m_EditorCamera.GetProjection();
				Math::mat4 cameraView = m_EditorCamera.GetViewMatrix();

				// Entity Transform
				ECS::TransformComponent& transformComponent = selectedEntity.GetComponent<ECS::TransformComponent>();
				Math::mat4 transform = transformComponent.GetTransform();

				// Snapping
				bool snap = Input::InputService::IsKeyPressed(Key::LeftControl);
				float snapValue = 0.5f;
				if (m_GizmoType == ImGuizmo::OPERATION::ROTATE) { snapValue = 45.0f; }

				float snapValues[3] = { snapValue, snapValue, snapValue };

				ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
					(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::WORLD, glm::value_ptr(transform),
					nullptr, snap ? snapValues : nullptr);
				if (ImGuizmo::IsUsing())
				{
					Math::vec3 translation, rotation, scale;
					Math::DecomposeTransform(transform, translation, rotation, scale);

					Math::vec3 deltaRotation = rotation - transformComponent.Rotation;
					transformComponent.Translation = translation;
					transformComponent.Rotation += deltaRotation;
					transformComponent.Scale = scale;
				}
			}
		}

		DrawToolbarOverlay();

		EditorUI::EditorUIService::EndWindow();
	}
	void ViewportPanel::OnInputEvent(Events::Event* event)
	{
		if (s_MainWindow->m_SceneState == SceneState::Edit || s_MainWindow->m_SceneState == SceneState::Simulate)
		{
			m_EditorCamera.OnInputEvent(event);
		}
	}
	void ViewportPanel::OnEditorEvent(Events::Event* event)
	{
		if (event->GetEventType() == Events::EventType::ManageEditor)
		{
			Events::ManageEditor* manageEditor = (Events::ManageEditor*)event;
			static Events::DebugLineData lineData;
			static Events::DebugPointData pointData;

			// Handle editor events
			switch (manageEditor->GetAction())
			{
			case Events::ManageEditorAction::AddDebugLine:
				lineData = manageEditor->GetDebugLineData();
				AddDebugLine(lineData.m_StartPoint, lineData.m_EndPoint);
				break;
			case Events::ManageEditorAction::AddDebugPoint:
				pointData = manageEditor->GetDebugPointData();
				AddDebugPoint(pointData.m_Point);
				break;
			case Events::ManageEditorAction::ClearDebugLines:
				ClearDebugLines();
				break;
			case Events::ManageEditorAction::ClearDebugPoints:
				ClearDebugPoints();
				break;
			default:
				KG_ERROR("Invalid editor event type provided to viewport panel!");
				break;
			}
		}
	}
	bool ViewportPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		bool control = Input::InputService::IsKeyPressed(Key::LeftControl) || Input::InputService::IsKeyPressed(Key::RightControl);
		bool shift = Input::InputService::IsKeyPressed(Key::LeftShift) || Input::InputService::IsKeyPressed(Key::RightShift);
		bool alt = Input::InputService::IsKeyPressed(Key::LeftAlt) || Input::InputService::IsKeyPressed(Key::RightAlt);

		switch (event.GetKeyCode())
		{
			case Key::Escape:
			{
				s_MainWindow->m_SceneEditorPanel->SetSelectedEntity({});
				s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::None);
				return true;
			}
			case Key::Tab:
			{
				s_MainWindow->m_ViewportPanel->m_EditorCamera.ToggleMovementType();
				return true;
			}

			// Gizmos
			case Key::Q:
			{
				if (!ImGuizmo::IsUsing() && !alt)
				{
					s_MainWindow->m_ViewportPanel->m_GizmoType = -1;
					return true;
				}
				return false;
			}
			case Key::W:
			{
				if (!ImGuizmo::IsUsing() && !alt)
				{
					s_MainWindow->m_ViewportPanel->m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
					return true;
				}
				return false;
			}
			case Key::E:
			{
				if (!ImGuizmo::IsUsing() && !alt)
				{
					s_MainWindow->m_ViewportPanel->m_GizmoType = ImGuizmo::OPERATION::ROTATE;
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
	void ViewportPanel::HandleSceneMouseHovering()
	{
		ImVec2 mousePosition = ImGui::GetMousePos();
		mousePosition.x -= m_ScreenViewportBounds[0].x;
		mousePosition.y -= m_ScreenViewportBounds[0].y;
		Math::vec2 viewportSize = m_ScreenViewportBounds[1] - m_ScreenViewportBounds[0];
		mousePosition.y = viewportSize.y - mousePosition.y;

		int mouseX = (int)mousePosition.x;
		int mouseY = (int)mousePosition.y;

		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
		{
			int pixelData = m_ViewportFramebuffer->ReadPixel(1, mouseX, mouseY);
			*Scenes::SceneService::GetActiveScene()->GetHoveredEntity() = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID((entt::entity)pixelData);
		}
	}

	void ViewportPanel::HandleUIMouseHovering()
	{
		ImVec2 mousePos = ImGui::GetMousePos();
		mousePos.x -= m_ScreenViewportBounds[0].x;
		mousePos.y -= m_ScreenViewportBounds[0].y;
		Math::vec2 viewportSize = m_ScreenViewportBounds[1] - m_ScreenViewportBounds[0];
		mousePos.y = viewportSize.y - mousePos.y;

		if ((int)mousePos.x >= 0 && (int)mousePos.y >= 0 && (int)mousePos.x < (int)viewportSize.x && (int)mousePos.y < (int)viewportSize.y)
		{
			int pixelData = m_ViewportFramebuffer->ReadPixel(1, (int)mousePos.x, (int)mousePos.y);

			// Extract lower 16 bits
			m_HoveredWidgetID = (uint16_t)(pixelData & 0xFFFF);

			// Extract upper 16 bits
			m_HoveredWindowID = (uint16_t)((pixelData >> 16) & 0xFFFF);
		}

		// Exit early if no valid widget/window is available
		if (m_HoveredWidgetID == RuntimeUI::k_InvalidWidgetID || m_HoveredWindowID == RuntimeUI::k_InvalidWindowID)
		{
			RuntimeUI::RuntimeUIService::ClearHoveredWidget();
			return;
		}

		// Select the widget if applicable
		RuntimeUI::RuntimeUIService::SetHoveredWidgetByIndex({ RuntimeUI::RuntimeUIService::GetActiveUIHandle(),
			m_HoveredWindowID, m_HoveredWidgetID });

	}

	void ViewportPanel::OnUpdateEditor(Timestep ts, Rendering::EditorPerspectiveCamera& camera)
	{
		Scenes::SceneService::GetActiveScene()->RenderScene(camera, camera.GetViewMatrix());
	}

	void ViewportPanel::OnUpdateRuntime(Timestep ts)
	{
		// Render 
		ECS::Entity cameraEntity = Scenes::SceneService::GetActiveScene()->GetPrimaryCameraEntity();
		if (!cameraEntity)
		{
			return;
		}
		Rendering::Camera* mainCamera = &cameraEntity.GetComponent<ECS::CameraComponent>().Camera;
		Math::mat4 cameraTransform = cameraEntity.GetComponent<ECS::TransformComponent>().GetTransform();

		if (mainCamera)
		{
			// Transform Matrix needs to be inversed so that final view is from the perspective of the camera
			Scenes::SceneService::GetActiveScene()->RenderScene(*mainCamera, glm::inverse(cameraTransform));
		}

	}

	void ViewportPanel::OnUpdateSimulation(Timestep ts, Rendering::EditorPerspectiveCamera& camera)
	{

		if (!s_MainWindow->m_IsPaused || s_MainWindow->m_StepFrames-- > 0)
		{
			Physics::Physics2DService::OnUpdate(ts); 
		}

		// Render
		Scenes::SceneService::GetActiveScene()->RenderScene(camera, camera.GetViewMatrix());
	}

	static Rendering::RendererInputSpec s_CircleInputSpec{};
	static Rendering::RendererInputSpec s_PointInputSpec{};
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

	static Math::vec4 s_DefaultFrustumVertexPositions[8]
	{
		{ -1.0f, -1.0f, 1.0f, 1.0f },			// 0
			{ 1.0f, -1.0f, 1.0f, 1.0f },		// 1
			{ 1.0f, 1.0f, 1.0f, 1.0f },		// 2
			{ -1.0f, 1.0f, 1.0f, 1.0f },		// 3
		{ -1.0f, -1.0f, -1.0f, 1.0f },		// 4
			{ 1.0f, -1.0f, -1.0f, 1.0f },		// 5
			{ 1.0f, 1.0f, -1.0f, 1.0f },		// 6
		{ -1.0f, 1.0f, -1.0f, 1.0f },			// 7
	};

	static Math::uvec2 s_FrustumIndices[16]
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
			auto [uuid, localShader] = Assets::AssetService::GetShader(lineShaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			Rendering::Shader::SetDataAtInputLocation<Math::vec4>({ 0.0f, 1.0f, 0.0f, 1.0f }, "a_Color", localBuffer, localShader);

			ECS::ShapeComponent* lineShapeComponent = new ECS::ShapeComponent();
			lineShapeComponent->CurrentShape = Rendering::ShapeTypes::None;
			lineShapeComponent->Vertices = nullptr;

			s_LineInputSpec.m_Shader = localShader;
			s_LineInputSpec.m_Buffer = localBuffer;
			s_LineInputSpec.m_ShapeComponent = lineShapeComponent;
		}
		// Set up Circle Input Specification for Overlay Calls
		{
			Rendering::ShaderSpecification shaderSpec{Rendering::ColorInputType::FlatColor, Rendering::TextureInputType::None, true, true, false, Rendering::RenderingType::DrawIndex, false };
			auto [uuid, localShader] = Assets::AssetService::GetShader(shaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			Rendering::Shader::SetDataAtInputLocation<Math::vec4>({ 0.0f, 1.0f, 0.0f, 1.0f }, "a_Color", localBuffer, localShader);
			Rendering::Shader::SetDataAtInputLocation<float>(0.05f, "a_Thickness", localBuffer, localShader);
			Rendering::Shader::SetDataAtInputLocation<float>(0.005f, "a_Fade", localBuffer, localShader);

			ECS::ShapeComponent* shapeComp = new ECS::ShapeComponent();
			shapeComp->CurrentShape = Rendering::ShapeTypes::Quad;
			shapeComp->Vertices = CreateRef<std::vector<Math::vec3>>(Rendering::Shape::s_Quad.GetIndexVertices());
			shapeComp->Indices = CreateRef<std::vector<uint32_t>>(Rendering::Shape::s_Quad.GetIndices());

			s_CircleInputSpec.m_Shader = localShader;
			s_CircleInputSpec.m_Buffer = localBuffer;
			s_CircleInputSpec.m_ShapeComponent = shapeComp;
		}

		// Set up Point Input Specifications for Overlay Calls
		{
			Rendering::ShaderSpecification pointShaderSpec{Rendering::ColorInputType::FlatColor, Rendering::TextureInputType::None, false, true, false, Rendering::RenderingType::DrawPoint, false };
			auto [uuid, localShader] = Assets::AssetService::GetShader(pointShaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			Rendering::Shader::SetDataAtInputLocation<Math::vec4>({ 0.0f, 1.0f, 0.0f, 1.0f }, "a_Color", localBuffer, localShader);

			ECS::ShapeComponent* pointShapeComponent = new ECS::ShapeComponent();
			pointShapeComponent->CurrentShape = Rendering::ShapeTypes::None;
			pointShapeComponent->Vertices = nullptr;

			s_PointInputSpec.m_Shader = localShader;
			s_PointInputSpec.m_Buffer = localBuffer;
			s_PointInputSpec.m_ShapeComponent = pointShapeComponent;
		}

		// TODO: Shape Components and Buffers are memory leaks!
	}

	void ViewportPanel::ClearOverlayData()
	{
		s_LineInputSpec.ClearData();
		s_PointInputSpec.ClearData();
		s_CircleInputSpec.ClearData();
	}

	void ViewportPanel::SetViewportAspectRatio(const Math::uvec2& newAspectRatio)
	{
		m_ViewportAspectRatio = newAspectRatio;
	}

	void ViewportPanel::OnOverlayRender()
	{
		if (s_MainWindow->m_SceneState == SceneState::Play)
		{
			ECS::Entity cameraEntity = Scenes::SceneService::GetActiveScene()->GetPrimaryCameraEntity();
			if (!cameraEntity)
			{
				return;
			}
			Rendering::RenderingService::BeginScene(cameraEntity.GetComponent<ECS::CameraComponent>().Camera, glm::inverse(cameraEntity.GetComponent<ECS::TransformComponent>().GetTransform()));
		}
		else
		{
			Rendering::RenderingService::BeginScene(m_EditorCamera);
		}

		if (s_MainWindow->m_ShowPhysicsColliders)
		{
			// Circle Colliders
			{
				auto view = Scenes::SceneService::GetActiveScene()->GetAllEntitiesWith<ECS::TransformComponent, ECS::CircleCollider2DComponent>();
				for (auto entity : view)
				{
					auto [tc, cc2d] = view.get<ECS::TransformComponent, ECS::CircleCollider2DComponent>(entity);

					Math::vec3 translation = tc.Translation + Math::vec3(cc2d.Offset.x, cc2d.Offset.y, 0.001f);

					Math::vec3 scale = tc.Scale * Math::vec3(cc2d.Radius * 2.0f);

					Math::mat4 transform = glm::translate(Math::mat4(1.0f), translation)
						* glm::scale(Math::mat4(1.0f), scale);

					s_CircleInputSpec.m_TransformMatrix = transform;
					Rendering::RenderingService::SubmitDataToRenderer(s_CircleInputSpec);
				}
			}
			// Box Colliders
			{
				auto view = Scenes::SceneService::GetActiveScene()->GetAllEntitiesWith<ECS::TransformComponent, ECS::BoxCollider2DComponent>();
				for (entt::entity entity : view)
				{
					auto [tc, bc2d] = view.get<ECS::TransformComponent, ECS::BoxCollider2DComponent>(entity);

					Math::vec3 translation = tc.Translation + Math::vec3(bc2d.Offset.x, bc2d.Offset.y, 0.001f);
					Math::vec3 scale = tc.Scale * Math::vec3(bc2d.Size * 2.0f, 1.0f);

					Math::mat4 transform = glm::translate(Math::mat4(1.0f), translation)
						* glm::rotate(Math::mat4(1.0f), tc.Rotation.z, Math::vec3(0.0f, 0.0f, 1.0f))
						* glm::scale(Math::mat4(1.0f), scale);

					static Math::vec4 boxColliderColor {0.0f, 1.0f, 0.0f, 1.0f};
					Rendering::Shader::SetDataAtInputLocation<Math::vec4>(boxColliderColor, "a_Color", s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);

					Math::vec3 lineVertices[4];
					for (size_t i = 0; i < 4; i++)
					{
						lineVertices[i] = transform * s_RectangleVertexPositions[i];
					}
					s_OutputVector->clear();
					s_OutputVector->push_back(lineVertices[0]);
					s_OutputVector->push_back(lineVertices[1]);
					s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
					Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
					s_OutputVector->clear();
					s_OutputVector->push_back(lineVertices[1]);
					s_OutputVector->push_back(lineVertices[2]);
					s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
					Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
					s_OutputVector->clear();
					s_OutputVector->push_back(lineVertices[2]);
					s_OutputVector->push_back(lineVertices[3]);
					s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
					Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
					s_OutputVector->clear();
					s_OutputVector->push_back(lineVertices[3]);
					s_OutputVector->push_back(lineVertices[0]);
					s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
					Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				}
			}
		}

		if (s_MainWindow->m_SceneState == SceneState::Edit || 
			s_MainWindow->m_SceneState == SceneState::Simulate || 
			(s_MainWindow->m_SceneState == SceneState::Play && s_MainWindow->m_IsPaused))
		{
			// Draw selected entity outline 
			if (ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity()) 
			{
				ECS::TransformComponent transform = selectedEntity.GetComponent<ECS::TransformComponent>();
				static Math::vec4 selectionColor {1.0f, 0.5f, 0.0f, 1.0f};
				Rendering::Shader::SetDataAtInputLocation<Math::vec4>(selectionColor, "a_Color", s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);

				Math::vec3 lineVertices[8];

				// Create all vertices
				for (size_t i = 0; i < 8; i++)
				{
					lineVertices[i] = transform.GetTransform() * s_CubeVertexPositions[i];
				}
				// Create and submit lines to renderer
				for (Math::uvec2& indices : s_CubeIndices)
				{
					s_OutputVector->clear();
					s_OutputVector->push_back(lineVertices[indices.x]);
					s_OutputVector->push_back(lineVertices[indices.y]);
					s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
					Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				}

				if (selectedEntity.HasComponent<ECS::CameraComponent>() && s_MainWindow->m_ShowCameraFrustums)
				{
					DrawFrustrum(selectedEntity);
				}
			}
			DrawGridLines(); 
		}

		DrawDebugLines();

		DrawSplines();


		Rendering::RenderingService::EndScene();
	}

	void ViewportPanel::DrawFrustrum(ECS::Entity& entity)
	{
		Math::vec3 lineVertices[9];
		Math::vec4 selectionColor { 0.5f, 0.3f, 0.85f, 1.0f };

		// Get entity transform and entity camera
		auto& transform = entity.GetComponent<ECS::TransformComponent>();
		auto& camera = entity.GetComponent<ECS::CameraComponent>();
		// Submit frustrum cube color to renderer input
		Rendering::Shader::SetDataAtInputLocation<Math::vec4>(selectionColor, "a_Color", s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);


		// Set lineVertices 0 - 7 with vertices from camera frustum
		for (size_t i = 0; i < 8; i++)
		{
			Math::vec4 localSpaceCoordinates = glm::inverse(camera.Camera.GetProjection()) * s_DefaultFrustumVertexPositions[i];
			localSpaceCoordinates = localSpaceCoordinates / localSpaceCoordinates.w; // Perspective Division
			lineVertices[i] = transform.GetTranslation() * transform.GetRotation() * localSpaceCoordinates;
		}
		// Set lineVertices 8 with the entity location
		lineVertices[8] = (transform.GetTransform() * Math::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		bool colorChanged = false;
		uint32_t iteration = 0;
		for (auto& index : s_FrustumIndices)
		{
			if (iteration >= 12 && !colorChanged)
			{
				selectionColor = { 0.3f, 0.1f, 0.8f, 1.0f };
				Rendering::Shader::SetDataAtInputLocation<Math::vec4>(selectionColor, "a_Color", s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);
				colorChanged = true;
			}
			s_OutputVector->clear();
			s_OutputVector->push_back(lineVertices[index.x]);
			s_OutputVector->push_back(lineVertices[index.y]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
			iteration++;
		}

	}

	void ViewportPanel::DrawGridLines()
	{
		Math::vec3 cameraPosition = m_EditorCamera.GetPosition();
		Math::vec3 fineGridStart =
		{
			/*Utility::Operations::RoundDown((int32_t)(cameraPosition.x - (m_LargeGridSpacing / 2)), (int32_t)m_FineGridSpacing),
			Utility::Operations::RoundDown((int32_t)(cameraPosition.y - (m_LargeGridSpacing / 2)), (int32_t)m_FineGridSpacing),
			Utility::Operations::RoundDown((int32_t)(cameraPosition.z - (m_LargeGridSpacing / 2)), (int32_t)m_FineGridSpacing),*/
			Utility::Operations::RoundDown((int32_t)cameraPosition.x, (int32_t)m_LargeGridSpacing),
			Utility::Operations::RoundDown((int32_t)cameraPosition.y, (int32_t)m_LargeGridSpacing),
			Utility::Operations::RoundDown((int32_t)cameraPosition.z, (int32_t)m_LargeGridSpacing)
		};

		// Set cameraFrustrumVertices 0 - 7 with vertices from camera frustum
		Math::vec3 currentVertex;
		Math::vec3 minimumValues {std::numeric_limits<float>().max()};
		Math::vec3 maximumValues {-std::numeric_limits<float>().max()};
		for (size_t i = 0; i < 8; i++)
		{
			Math::vec4 localSpaceCoordinates = glm::inverse(m_EditorCamera.GetProjection()) * s_DefaultFrustumVertexPositions[i];
			localSpaceCoordinates = localSpaceCoordinates / localSpaceCoordinates.w; // Perspective Division
			currentVertex = glm::inverse(m_EditorCamera.GetViewMatrix()) * localSpaceCoordinates;
			if (currentVertex.x > maximumValues.x)
			{
				maximumValues.x = currentVertex.x;
			}
			if (currentVertex.y > maximumValues.y)
			{
				maximumValues.y = currentVertex.y;
			}
			if (currentVertex.z > maximumValues.z)
			{
				maximumValues.z = currentVertex.z;
			}
			if (currentVertex.x < minimumValues.x)
			{
				minimumValues.x = currentVertex.x;
			}
			if (currentVertex.y < minimumValues.y)
			{
				minimumValues.y = currentVertex.y;
			}
			if (currentVertex.z < minimumValues.z)
			{
				minimumValues.z = currentVertex.z;
			}
		}

		// Start Grids
		int32_t currentLine;
		s_OutputVector->clear();
		Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_GridMajor),
			"a_Color", s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);
		// X-Y Grid
		if (m_DisplayXYMajorGrid)
		{
			// Create Y Lines
			currentLine = Utility::Operations::RoundDown((int32_t)minimumValues.x, (int32_t)m_LargeGridSpacing);
			while ((float)currentLine < maximumValues.x)
			{
				if (currentLine == 0)
				{
					currentLine += (int32_t)m_LargeGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->push_back({ currentLine, minimumValues.y, 0.0f });
				s_OutputVector->push_back({ currentLine, maximumValues.y, 0.0f });
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_LargeGridSpacing;
			}

			// Create X Lines
			currentLine = Utility::Operations::RoundDown((int32_t)minimumValues.y, (int32_t)m_LargeGridSpacing);
			while ((float)currentLine < maximumValues.y)
			{
				if (currentLine == 0)
				{
					currentLine += (int32_t)m_LargeGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->push_back({ minimumValues.x, currentLine, 0.0f });
				s_OutputVector->push_back({ maximumValues.x, currentLine, 0.0f });
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_LargeGridSpacing;
			}
		}

		// Y-Z Grid
		if (m_DisplayYZMajorGrid)
		{ 
			// Create Y Lines
			currentLine = Utility::Operations::RoundDown((int32_t)minimumValues.y, (int32_t)m_LargeGridSpacing);
			while ((float)currentLine < maximumValues.y)
			{
				if (currentLine == 0)
				{
					currentLine += (int32_t)m_LargeGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->push_back({ 0.0f, currentLine, minimumValues.z });
				s_OutputVector->push_back({ 0.0f, currentLine, maximumValues.z });
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_LargeGridSpacing;
			}

			// Create Z Lines
			currentLine = Utility::Operations::RoundDown((int32_t)minimumValues.z, (int32_t)m_LargeGridSpacing);
			while ((float)currentLine < maximumValues.z)
			{
				if (currentLine == 0)
				{
					currentLine += (int32_t)m_LargeGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->push_back({ 0.0f, minimumValues.y, currentLine});
				s_OutputVector->push_back({ 0.0f, maximumValues.y, currentLine });
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_LargeGridSpacing;
			}
		}

		// X-Z Grid
		if (m_DisplayXZMajorGrid)
		{ 
			// Create Large X Lines
			currentLine = Utility::Operations::RoundDown((int32_t)minimumValues.x, (int32_t)m_LargeGridSpacing);
			while ((float)currentLine < maximumValues.x)
			{
				if (currentLine == 0)
				{
					currentLine += (int32_t)m_LargeGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->push_back({ currentLine, 0.0f, minimumValues.z });
				s_OutputVector->push_back({ currentLine, 0.0f, maximumValues.z });
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_LargeGridSpacing;
			}

			// Create Large Z Lines
			currentLine = Utility::Operations::RoundDown((int32_t)minimumValues.z, (int32_t)m_LargeGridSpacing);
			while ((float)currentLine < maximumValues.z)
			{
				if (currentLine == 0)
				{
					currentLine += (int32_t)m_LargeGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->push_back({ minimumValues.x, 0.0f, currentLine });
				s_OutputVector->push_back({ maximumValues.x, 0.0f, currentLine });
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_LargeGridSpacing;
			}
		}

		// Set Color for minor grid lines
		Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_GridMinor),
			"a_Color", s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);

		if (m_DisplayXYMinorGrid)
		{
			// Create X Minor Grid lines
			currentLine = (int32_t)(fineGridStart.x - m_LargeGridSpacing);
			while ((float)currentLine < (fineGridStart.x + 2 * m_LargeGridSpacing))
			{
				if (currentLine % (int32_t)m_LargeGridSpacing == 0)
				{
					currentLine += (int32_t)m_FineGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->push_back({ currentLine,   fineGridStart.y - m_LargeGridSpacing, 0.0f });
				s_OutputVector->push_back({ currentLine,fineGridStart.y + 2 * m_LargeGridSpacing , 0.0f });
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_FineGridSpacing;
			}

			// Create Y Minor Grid lines
			currentLine = (int32_t)(fineGridStart.y - m_LargeGridSpacing);
			while ((float)currentLine < (fineGridStart.y + 2 * m_LargeGridSpacing))
			{
				if (currentLine % (int32_t)m_LargeGridSpacing == 0)
				{
					currentLine += (int32_t)m_FineGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->push_back({ fineGridStart.x - m_LargeGridSpacing, currentLine, 0.0f });
				s_OutputVector->push_back({ fineGridStart.x + 2 * m_LargeGridSpacing , currentLine, 0.0f });
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_FineGridSpacing;
			}
		}

		if (m_DisplayXZMinorGrid)
		{
			// Create X Minor Grid lines
			currentLine = (int32_t)(fineGridStart.x - m_LargeGridSpacing);
			while ((float)currentLine < (fineGridStart.x + 2 * m_LargeGridSpacing))
			{
				if (currentLine % (int32_t)m_LargeGridSpacing == 0)
				{
					currentLine += (int32_t)m_FineGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->push_back({ currentLine, 0.0f, fineGridStart.z - m_LargeGridSpacing });
				s_OutputVector->push_back({ currentLine, 0.0f, fineGridStart.z + 2 * m_LargeGridSpacing });
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_FineGridSpacing;
			}

			// Create Z Minor Grid lines
			currentLine = (int32_t)(fineGridStart.z - m_LargeGridSpacing);
			while ((float)currentLine < (fineGridStart.z + 2 * m_LargeGridSpacing))
			{
				if (currentLine % (int32_t)m_LargeGridSpacing == 0)
				{
					currentLine += (int32_t)m_FineGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->push_back({ fineGridStart.x - m_LargeGridSpacing, 0.0f, currentLine });
				s_OutputVector->push_back({ fineGridStart.x + 2 * m_LargeGridSpacing, 0.0f, currentLine });
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_FineGridSpacing;
			}
		}

		if (m_DisplayYZMinorGrid)
		{
			// Create Y Minor Grid lines
			currentLine = (int32_t)(fineGridStart.y - m_LargeGridSpacing);
			while ((float)currentLine < (fineGridStart.y + 2 * m_LargeGridSpacing))
			{
				if (currentLine % (int32_t)m_LargeGridSpacing == 0)
				{
					currentLine += (int32_t)m_FineGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->push_back({ 0.0f, currentLine,  fineGridStart.z - m_LargeGridSpacing });
				s_OutputVector->push_back({ 0.0f , currentLine,  fineGridStart.z + (2 * m_LargeGridSpacing) });
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_FineGridSpacing;
			}

			// Create Z Minor Grid lines
			currentLine = (int32_t)(fineGridStart.z - m_LargeGridSpacing);
			while ((float)currentLine < (fineGridStart.z + 2 * m_LargeGridSpacing))
			{
				if (currentLine % (int32_t)m_LargeGridSpacing == 0)
				{
					currentLine += (int32_t)m_FineGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->push_back({ 0.0f, fineGridStart.y - m_LargeGridSpacing, currentLine });
				s_OutputVector->push_back({ 0.0f , fineGridStart.y + 2 * m_LargeGridSpacing, currentLine });
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_FineGridSpacing;
			}
		}

		if (m_DisplayXZMajorGrid || m_DisplayXYMajorGrid || m_DisplayYZMajorGrid)
		{
			// X Axis
			s_OutputVector->clear();
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_HighlightColor1),
				"a_Color", s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);
			s_OutputVector->push_back({ minimumValues.x, 0.0f, 0.0f });
			s_OutputVector->push_back({ maximumValues.x, 0.0f, 0.0f });
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);


			// Y Axis
			s_OutputVector->clear();
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_HighlightColor2),
				"a_Color", s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);
			s_OutputVector->push_back({ 0.0f, minimumValues.y, 0.0f });
			s_OutputVector->push_back({ 0.0f, maximumValues.y, 0.0f });
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);


			// Z Axis
			s_OutputVector->clear();
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_HighlightColor3),
				"a_Color", s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);
			s_OutputVector->push_back({ 0.0f, 0.0f, minimumValues.z });
			s_OutputVector->push_back({ 0.0f, 0.0f, maximumValues.z });
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
		}
	}

	void ViewportPanel::DrawDebugLines()
	{
		// Draw debug line
		for (DebugLine& line : m_DebugLines)
		{
			s_OutputVector->clear();
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_Red),
				"a_Color", s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);
			s_OutputVector->push_back(line.m_StartPoint);
			s_OutputVector->push_back(line.m_EndPoint);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
		}

		// Draw debug points
		for (DebugPoint& point : m_DebugPoints)
		{
			s_OutputVector->clear();
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_Red),
				"a_Color", s_PointInputSpec.m_Buffer, s_PointInputSpec.m_Shader);
			s_OutputVector->push_back(point.m_Point);
			s_PointInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_PointInputSpec);
		}
		
	}

	void ViewportPanel::DrawSplines()
	{
		const float stepValue{ 0.05f };

		for (const Math::Spline& spline : m_DebugSplines)
		{
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_PureWhite),
				"a_Color", s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);
			float adjustmentFactor{ spline.m_Looped ? 0.0f : 3.0f };
			for (float time{ 0.0f }; time < (float)spline.m_Points.size() - adjustmentFactor; time += stepValue) // Note we subtract 3.0f to account for edge control points
			{
				// Get the point from the spline
				Math::vec3 point = Math::SplineService::GetSplinePoint(spline, time);
				Math::vec3 point2 = Math::SplineService::GetSplinePoint(spline, time + stepValue);
				
				s_OutputVector->clear();
				
				s_OutputVector->push_back(point);
				s_OutputVector->push_back(point2);
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
			}
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_Red),
				"a_Color", s_PointInputSpec.m_Buffer, s_PointInputSpec.m_Shader);
			for (const Math::vec3& point : spline.m_Points)
			{
				// Draw the point
				s_OutputVector->clear();
				s_OutputVector->push_back(point);
				s_PointInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_PointInputSpec);
			}
		}

	}

	void ViewportPanel::DrawToolbarOverlay()
	{
		constexpr float k_IconSize{ 36.0f };
		ImGui::PushStyleColor(ImGuiCol_Button, EditorUI::EditorUIService::s_PureEmpty);
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 initialScreenCursorPos = ImGui::GetWindowPos() + ImGui::GetCursorStartPos();
		ImVec2 initialCursorPos = ImGui::GetCursorStartPos();

		ImVec2 windowSize = ImGui::GetWindowSize();
		Ref<Rendering::Texture2D> icon{ nullptr };
		if (m_ToolbarEnabled)
		{
			// Draw Play/Simulate/Step Background
			draw_list->AddRectFilled(ImVec2(initialScreenCursorPos.x + (windowSize.x / 2) - 90.0f, initialScreenCursorPos.y),
				ImVec2(initialScreenCursorPos.x + (windowSize.x / 2) + 90.0f, initialScreenCursorPos.y + 43.0f),
				ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 12.0f, ImDrawFlags_RoundCornersBottom);

			// Draw Display Options Background
			draw_list->AddRectFilled(ImVec2(initialScreenCursorPos.x + windowSize.x - 80.0f, initialScreenCursorPos.y),
				ImVec2(initialScreenCursorPos.x + (windowSize.x) - 48.0f, initialScreenCursorPos.y + 30.0f),
				ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 12.0f, ImDrawFlags_RoundCornersBottom);

			// Draw Grid Options Background
			draw_list->AddRectFilled(ImVec2(initialScreenCursorPos.x + windowSize.x - 257.0f, initialScreenCursorPos.y),
				ImVec2(initialScreenCursorPos.x + (windowSize.x) - 187.0f, initialScreenCursorPos.y + 30.0f),
				ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 12.0f, ImDrawFlags_RoundCornersBottom);

			// Draw Camera Options Background
			draw_list->AddRectFilled(ImVec2(initialScreenCursorPos.x + windowSize.x - 170.0f, initialScreenCursorPos.y),
				ImVec2(initialScreenCursorPos.x + (windowSize.x) - 100.0f, initialScreenCursorPos.y + 30.0f),
				ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 12.0f, ImDrawFlags_RoundCornersBottom);

			// Draw Toggle Top Bar Background
			draw_list->AddRectFilled(ImVec2(initialScreenCursorPos.x + windowSize.x - 30.0f, initialScreenCursorPos.y),
				ImVec2(initialScreenCursorPos.x + (windowSize.x), initialScreenCursorPos.y + 30.0f),
				ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 12.0f, ImDrawFlags_RoundCornersBottomLeft);

			bool hasPlayButton = s_MainWindow->m_SceneState == SceneState::Edit || s_MainWindow->m_SceneState == SceneState::Simulate;
			bool hasSimulateButton = s_MainWindow->m_SceneState == SceneState::Edit || s_MainWindow->m_SceneState == SceneState::Play;
			bool hasPauseButton = s_MainWindow->m_SceneState != SceneState::Edit;
			bool hasStepButton = hasPauseButton && s_MainWindow->m_IsPaused;

			// Play/Stop Button
			if (!hasSimulateButton)
			{
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorUI::EditorUIService::s_PureEmpty);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, EditorUI::EditorUIService::s_PureEmpty);
			}
			ImGui::SetCursorPos(ImVec2(initialCursorPos.x + (windowSize.x / 2) - 77.0f, initialCursorPos.y + 4));
			icon = hasPlayButton ? EditorUI::EditorUIService::s_IconPlay : EditorUI::EditorUIService::s_IconStop;
			if (ImGui::ImageButton((ImTextureID)(uint64_t)(hasSimulateButton ? icon : EditorUI::EditorUIService::s_IconPlay)->GetRendererID(),
				ImVec2(k_IconSize, k_IconSize), ImVec2(0, 0),
				ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f),
				EditorUI::EditorUIService::s_HighlightColor1)
				&& m_ToolbarEnabled)
			{
				if (hasSimulateButton)
				{
					if (s_MainWindow->m_SceneState == SceneState::Edit || s_MainWindow->m_SceneState == SceneState::Simulate)
					{
						s_MainWindow->OnPlay();
					}
					else if (s_MainWindow->m_SceneState == SceneState::Play)
					{
						s_MainWindow->OnStop();
					}
				}

			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				if (hasSimulateButton)
				{
					ImGui::BeginTooltip();
					ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, hasPlayButton ?
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
			icon = hasSimulateButton ? EditorUI::EditorUIService::s_IconSimulate : EditorUI::EditorUIService::s_IconStop;
			ImGui::SetCursorPos(ImVec2(initialCursorPos.x + (windowSize.x / 2) - 37.0f, initialCursorPos.y + 4));
			if (ImGui::ImageButton((ImTextureID)(uint64_t)(hasPlayButton ? icon : EditorUI::EditorUIService::s_IconSimulate)->GetRendererID(),
				ImVec2(k_IconSize, k_IconSize), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
				0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f),
				EditorUI::EditorUIService::s_HighlightColor1)
				&& m_ToolbarEnabled)
			{
				if (hasPlayButton)
				{
					if (s_MainWindow->m_SceneState == SceneState::Edit || s_MainWindow->m_SceneState == SceneState::Play)
					{
						s_MainWindow->OnSimulate();
					}
					else if (s_MainWindow->m_SceneState == SceneState::Simulate)
					{
						s_MainWindow->OnStop();
					}
				}
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				if (hasPlayButton)
				{
					ImGui::BeginTooltip();
					ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, hasSimulateButton ?
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
			icon = EditorUI::EditorUIService::s_IconPause;
			ImGui::SetCursorPos(ImVec2(initialCursorPos.x + (windowSize.x / 2) + 3.0f, initialCursorPos.y + 4));
			if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(k_IconSize, k_IconSize),
				ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
				0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f),
				hasPauseButton ? EditorUI::EditorUIService::s_HighlightColor1 : EditorUI::EditorUIService::s_DisabledColor)
				&& m_ToolbarEnabled)
			{
				if (hasPauseButton)
				{
					s_MainWindow->m_IsPaused = !s_MainWindow->m_IsPaused;
				}
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				if (hasPauseButton)
				{
					ImGui::BeginTooltip();
					ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, s_MainWindow->m_IsPaused ? "Resume Application" : "Pause Application");
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
			icon = EditorUI::EditorUIService::s_IconStep;
			ImGui::SetCursorPos(ImVec2(initialCursorPos.x + (windowSize.x / 2) + 43.0f, initialCursorPos.y + 4));
			if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(),
				ImVec2(k_IconSize, k_IconSize), ImVec2{ 0, 1 },
				ImVec2{ 1, 0 }, 0,
				ImVec4(0.0f, 0.0f, 0.0f, 0.0f),
				hasStepButton ? EditorUI::EditorUIService::s_HighlightColor1 : EditorUI::EditorUIService::s_DisabledColor)
				&& m_ToolbarEnabled)
			{
				if (hasStepButton)
				{
					s_MainWindow->Step(1);
				}
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				if (hasStepButton)
				{
					ImGui::BeginTooltip();
					ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, "Step Application");
					ImGui::EndTooltip();
				}
			}
			if (!hasStepButton)
			{
				ImGui::PopStyleColor(2);
			}

			// Camera Options Button
			icon = EditorUI::EditorUIService::s_IconCamera;
			ImGui::SetCursorPos(ImVec2(initialCursorPos.x + windowSize.x - 163, initialCursorPos.y + 5));
			if (ImGui::ImageButton("Camera Options",
				(ImTextureID)(uint64_t)icon->GetRendererID(),
				ImVec2(14, 14), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
				EditorUI::EditorUIService::s_PureEmpty,
				EditorUI::EditorUIService::s_HighlightColor1))
			{
				ImGui::OpenPopup("Toggle Viewport Camera Options");
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, "Camera Movement Types");
				ImGui::EndTooltip();
			}

			if (ImGui::BeginPopup("Toggle Viewport Camera Options"))
			{
				if (ImGui::MenuItem("Model Viewer", 0,
					m_EditorCamera.GetMovementType() == Rendering::EditorPerspectiveCamera::MovementType::ModelView))
				{
					m_EditorCamera.SetMovementType(Rendering::EditorPerspectiveCamera::MovementType::ModelView);
				}
				if (ImGui::MenuItem("FreeFly", 0,
					m_EditorCamera.GetMovementType() == Rendering::EditorPerspectiveCamera::MovementType::FreeFly))
				{
					m_EditorCamera.SetMovementType(Rendering::EditorPerspectiveCamera::MovementType::FreeFly);
				}
				ImGui::EndPopup();
			}

			// Camera Speed
			ImGui::SetNextItemWidth(30.0f);
			ImGui::SetCursorPos(ImVec2(initialCursorPos.x + windowSize.x - 138, initialCursorPos.y + 6));
			ImGui::DragFloat("##CameraSpeed", &m_EditorCamera.GetMovementSpeed(), 0.5f,
				s_MainWindow->m_ViewportPanel->m_EditorCamera.GetMinMovementSpeed(), m_EditorCamera.GetMaxMovementSpeed(),
				"%.0f", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_CenterText);
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, "Camera Speed");
				ImGui::EndTooltip();
			}

			// Viewport Display Options Button
			icon = EditorUI::EditorUIService::s_IconDisplay;
			ImGui::SetCursorPos(ImVec2(initialCursorPos.x + windowSize.x - 75, initialCursorPos.y + 4));
			if (ImGui::ImageButton("Display Toggle",
				(ImTextureID)(uint64_t)icon->GetRendererID(),
				ImVec2(14, 14), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
				EditorUI::EditorUIService::s_PureEmpty,
				EditorUI::EditorUIService::s_HighlightColor1))
			{
				ImGui::OpenPopup("Toggle Display Options");
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, "Display Options");
				ImGui::EndTooltip();
			}

			if (ImGui::BeginPopup("Toggle Display Options"))
			{
				if (ImGui::MenuItem("Display Physics Colliders", 0, s_MainWindow->m_ShowPhysicsColliders))
				{
					Utility::Operations::ToggleBoolean(s_MainWindow->m_ShowPhysicsColliders);
				}
				if (ImGui::MenuItem("Display Camera Frustums", 0, s_MainWindow->m_ShowCameraFrustums))
				{
					Utility::Operations::ToggleBoolean(s_MainWindow->m_ShowCameraFrustums);
				}
				if (ImGui::MenuItem("Fullscreen While Running", 0, s_MainWindow->m_RuntimeFullscreen))
				{
					Utility::Operations::ToggleBoolean(s_MainWindow->m_RuntimeFullscreen);
				}
				ImGui::EndPopup();
			}

			// Grid Options Button
			icon = EditorUI::EditorUIService::s_IconGrid;
			ImGui::SetCursorPos(ImVec2(initialCursorPos.x + windowSize.x - 252, initialCursorPos.y + 4));
			if (ImGui::ImageButton("Grid Toggle",
				(ImTextureID)(uint64_t)icon->GetRendererID(),
				ImVec2(14, 14), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
				EditorUI::EditorUIService::s_PureEmpty,
				EditorUI::EditorUIService::s_HighlightColor1))
			{
				ImGui::OpenPopup("Grid Options");
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, "Grid Options");
				ImGui::EndTooltip();
			}

			if (ImGui::BeginPopup("Grid Options"))
			{
				if (ImGui::BeginMenu("X-Y Grid"))
				{
					if (ImGui::MenuItem("Display Infinite Grid", 0, m_DisplayXYMajorGrid))
					{
						Utility::Operations::ToggleBoolean(m_DisplayXYMajorGrid);

						if (!m_DisplayXYMajorGrid && m_DisplayXYMinorGrid)
						{
							Utility::Operations::ToggleBoolean(m_DisplayXYMinorGrid);
						}
					}
					if (m_DisplayXYMajorGrid)
					{
						if (ImGui::MenuItem("Display Local Grid", 0, m_DisplayXYMinorGrid))
						{
							Utility::Operations::ToggleBoolean(m_DisplayXYMinorGrid);
						}
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("X-Z Grid"))
				{
					if (ImGui::MenuItem("Display Infinite Grid", 0, m_DisplayXZMajorGrid))
					{
						Utility::Operations::ToggleBoolean(m_DisplayXZMajorGrid);

						if (!m_DisplayXZMajorGrid && m_DisplayXZMinorGrid)
						{
							Utility::Operations::ToggleBoolean(m_DisplayXZMinorGrid);
						}
					}

					if (m_DisplayXZMajorGrid)
					{
						if (ImGui::MenuItem("Display Local Grid", 0, m_DisplayXZMinorGrid))
						{
							Utility::Operations::ToggleBoolean(m_DisplayXZMinorGrid);
						}
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Y-Z Grid"))
				{
					if (ImGui::MenuItem("Display Infinite Grid", 0, m_DisplayYZMajorGrid))
					{
						Utility::Operations::ToggleBoolean(m_DisplayYZMajorGrid);
						if (!m_DisplayYZMajorGrid && m_DisplayYZMinorGrid)
						{
							Utility::Operations::ToggleBoolean(m_DisplayYZMinorGrid);
						}
					}
					if (m_DisplayYZMajorGrid)
					{
						if (ImGui::MenuItem("Display Local Grid", 0, m_DisplayYZMinorGrid))
						{
							Utility::Operations::ToggleBoolean(m_DisplayYZMinorGrid);
						}
					}

					ImGui::EndMenu();
				}
				ImGui::EndPopup();
			}

			// Grid Spacing
			ImGui::SetNextItemWidth(30.0f);
			ImGui::SetCursorPos(ImVec2(initialCursorPos.x + windowSize.x - 227, initialCursorPos.y + 6));
			ImGui::DragFloat("##GridSpacing", &m_FineGridSpacing, 1.0f,
				1.0f, 50.0f,
				"%.0f", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_CenterText);
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, "Local Grid Spacing");
				ImGui::EndTooltip();
			}
		}

		// Toggle Top Bar Button
		icon = m_ToolbarEnabled ? EditorUI::EditorUIService::s_IconCheckbox_Enabled :
			EditorUI::EditorUIService::s_IconCheckbox_Disabled;
		ImGui::SetCursorPos(ImVec2(initialCursorPos.x + windowSize.x - 25, initialCursorPos.y + 4));
		if (ImGui::ImageButton("Toggle Top Bar",
			(ImTextureID)(uint64_t)icon->GetRendererID(),
			ImVec2(14, 14), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
			EditorUI::EditorUIService::s_PureEmpty,
			m_ToolbarEnabled ? EditorUI::EditorUIService::s_HighlightColor1 : EditorUI::EditorUIService::s_DisabledColor))
		{
			Utility::Operations::ToggleBoolean(m_ToolbarEnabled);
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetNextFrameWantCaptureMouse(false);
			ImGui::BeginTooltip();
			ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, m_ToolbarEnabled ? "Close Toolbar" : "Open Toolbar");
			ImGui::EndTooltip();
		}

		ImGui::PopStyleColor();

		if (Scenes::SceneService::GetActiveScene()->IsRunning() && !Scenes::SceneService::GetActiveScene()->GetPrimaryCameraEntity())
		{
			ImGui::PushFont(EditorUI::EditorUIService::s_FontAntaLarge);
			ImVec2 cursorStart = ImGui::GetCursorStartPos();
			windowSize = ImGui::GetContentRegionAvail();
			ImVec2 textSize = ImGui::CalcTextSize("No Primary Camera Set");
			ImGui::SetCursorPos({ cursorStart.x + (windowSize.x / 2) - (textSize.x / 2), cursorStart.y + (windowSize.y / 2) - (textSize.y / 2) });
			ImGui::Text("No Primary Camera Set");
			ImGui::PopFont();
		}
	}

}

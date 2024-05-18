#include "Panels/ViewportPanel.h"

#include "Kargono.h"

#include "EditorLayer.h"


namespace Kargono
{
	static EditorLayer* s_EditorLayer { nullptr };

	ViewportPanel::ViewportPanel()
	{
		s_EditorLayer = EditorLayer::GetCurrentLayer();
	}
	void ViewportPanel::OnUpdate(Timestep ts)
	{
		// Adjust Framebuffer Size Based on Viewport
		auto& currentWindow = Core::GetCurrentApp().GetWindow();
		if (FramebufferSpecification spec = m_ViewportFramebuffer->GetSpecification();
			static_cast<float>(currentWindow.GetViewportWidth()) > 0.0f && static_cast<float>(currentWindow.GetViewportHeight()) > 0.0f &&
			(spec.Width != currentWindow.GetViewportWidth() || spec.Height != currentWindow.GetViewportHeight()))
		{
			m_ViewportFramebuffer->Resize((uint32_t)currentWindow.GetViewportWidth(), (uint32_t)currentWindow.GetViewportHeight());
			m_EditorCamera.SetViewportSize(static_cast<float>(currentWindow.GetViewportWidth()), static_cast<float>(currentWindow.GetViewportHeight()));
		}

		// Reset Framebuffer
		Renderer::ResetStats();
		m_ViewportFramebuffer->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();

		// Clear our entity ID attachment to -1
		m_ViewportFramebuffer->ClearAttachment(1, -1);

		// Update Scene
		switch (s_EditorLayer->m_SceneState)
		{
		case SceneState::Edit:
		{
			m_EditorCamera.OnUpdate(ts);
			OnUpdateEditor(ts, m_EditorCamera);
			break;
		}
		case SceneState::Simulate:
		{
			m_EditorCamera.OnUpdate(ts);

			OnUpdateSimulation(ts, m_EditorCamera);
			break;
		}
		case SceneState::Play:
		{
			OnUpdateRuntime(ts);
			break;
		}
		}

		ProcessMousePicking();

		OnOverlayRender();

		if (s_EditorLayer->m_ShowUserInterface)
		{
			auto& currentApplication = Core::GetCurrentApp().GetWindow();
			if (s_EditorLayer->m_SceneState == SceneState::Play)
			{
				Entity cameraEntity = Scene::GetActiveScene()->GetPrimaryCameraEntity();
				Camera* mainCamera = &cameraEntity.GetComponent<CameraComponent>().Camera;
				Math::mat4 cameraTransform = cameraEntity.GetComponent<TransformComponent>().GetTransform();

				if (mainCamera)
				{
					RuntimeUI::Runtime::PushRenderData(glm::inverse(cameraTransform), currentApplication.GetViewportWidth(), currentApplication.GetViewportHeight());
				}
			}
			else
			{
				Math::mat4 cameraViewMatrix = glm::inverse(m_EditorCamera.GetViewMatrix());
				RuntimeUI::Runtime::PushRenderData(cameraViewMatrix, currentApplication.GetViewportWidth(), currentApplication.GetViewportHeight());
			}

		}

		m_ViewportFramebuffer->Unbind();
	}
	void ViewportPanel::InitializeFrameBuffer()
	{
		FramebufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferDataFormat::RGBA8, FramebufferDataFormat::RED_INTEGER,  FramebufferDataFormat::Depth };
		fbSpec.Width = Core::GetCurrentApp().GetWindow().GetWidth();
		fbSpec.Height = Core::GetCurrentApp().GetWindow().GetHeight();
		m_ViewportFramebuffer = Framebuffer::Create(fbSpec);
	}
	void ViewportPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		auto& currentWindow = Core::GetCurrentApp().GetWindow();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_NoDecoration;

		EditorUI::Editor::StartWindow("Viewport", &s_EditorLayer->m_ShowViewport, window_flags);
		auto viewportOffset = ImGui::GetWindowPos();
		static Math::uvec2 oldViewportSize = { currentWindow.GetViewportWidth(), currentWindow.GetViewportHeight() };
		Math::vec2 localViewportBounds[2];

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		EditorUI::Editor::BlockEvents(!m_ViewportHovered);

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		Math::uvec2 aspectRatio = Utility::ScreenResolutionToAspectRatio(Projects::Project::GetTargetResolution());
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

		Math::uvec2 viewportSize = { currentWindow.GetViewportWidth(), currentWindow.GetViewportHeight() };
		if (oldViewportSize != viewportSize)
		{
			Scene::GetActiveScene()->OnViewportResize((uint32_t)currentWindow.GetViewportWidth(), (uint32_t)currentWindow.GetViewportHeight());
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_SCENE"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				s_EditorLayer->OpenScene(path);
			}
			ImGui::EndDragDropTarget();
		}

		if (s_EditorLayer->m_SceneState == SceneState::Edit || s_EditorLayer->m_SceneState == SceneState::Simulate)

		{
			// Gizmos
			Entity selectedEntity = *Scene::GetActiveScene()->GetSelectedEntity();
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
				auto& tc = selectedEntity.GetComponent<TransformComponent>();
				Math::mat4 transform = tc.GetTransform();

				// Snapping
				bool snap = InputPolling::IsKeyPressed(Key::LeftControl);
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

		EditorUI::Editor::EndWindow();
		ImGui::PopStyleVar();
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
			*Scene::GetActiveScene()->GetHoveredEntity() = Scene::GetActiveScene()->CheckEntityExists((entt::entity)pixelData) ? Entity((entt::entity)pixelData, Scene::GetActiveScene().get()) : Entity();
		}
	}

	void ViewportPanel::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{
		Scene::GetActiveScene()->RenderScene(camera, camera.GetViewMatrix());
	}

	void ViewportPanel::OnUpdateRuntime(Timestep ts)
	{

		if (!s_EditorLayer->m_IsPaused || s_EditorLayer->m_StepFrames-- > 0)
		{
			// Update Scripts

			Script::ScriptEngine::OnUpdate(ts);

			Scene::GetActiveScene()->OnUpdatePhysics(ts);
		}

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

	void ViewportPanel::OnUpdateSimulation(Timestep ts, EditorCamera& camera)
	{
		EditorLayer* editorLayer = EditorLayer::GetCurrentLayer();

		if (!editorLayer->m_IsPaused || editorLayer->m_StepFrames-- > 0)
		{
			Scene::GetActiveScene()->OnUpdatePhysics(ts);
		}

		// Render
		Scene::GetActiveScene()->RenderScene(camera, camera.GetViewMatrix());
	}

	static RendererInputSpec s_CircleInputSpec{};
	static RendererInputSpec s_LineInputSpec{};
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
			ShaderSpecification lineShaderSpec{ ColorInputType::FlatColor, TextureInputType::None, false, true, false, RenderingType::DrawLine, false };
			auto [uuid, localShader] = Assets::AssetManager::GetShader(lineShaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			Shader::SetDataAtInputLocation<Math::vec4>({ 0.0f, 1.0f, 0.0f, 1.0f }, "a_Color", localBuffer, localShader);

			ShapeComponent* lineShapeComponent = new ShapeComponent();
			lineShapeComponent->CurrentShape = ShapeTypes::None;
			lineShapeComponent->Vertices = nullptr;

			s_LineInputSpec.Shader = localShader;
			s_LineInputSpec.Buffer = localBuffer;
			s_LineInputSpec.ShapeComponent = lineShapeComponent;
		}


		// Set up Circle Input Specification for Overlay Calls

		{
			ShaderSpecification shaderSpec{ ColorInputType::FlatColor,  TextureInputType::None, true, true, false, RenderingType::DrawIndex, false };
			auto [uuid, localShader] = Assets::AssetManager::GetShader(shaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			Shader::SetDataAtInputLocation<Math::vec4>({ 0.0f, 1.0f, 0.0f, 1.0f }, "a_Color", localBuffer, localShader);
			Shader::SetDataAtInputLocation<float>(0.05f, "a_Thickness", localBuffer, localShader);
			Shader::SetDataAtInputLocation<float>(0.005f, "a_Fade", localBuffer, localShader);

			ShapeComponent* shapeComp = new ShapeComponent();
			shapeComp->CurrentShape = ShapeTypes::Quad;
			shapeComp->Vertices = CreateRef<std::vector<Math::vec3>>(Shape::s_Quad.GetIndexVertices());
			shapeComp->Indices = CreateRef<std::vector<uint32_t>>(Shape::s_Quad.GetIndices());

			s_CircleInputSpec.Shader = localShader;
			s_CircleInputSpec.Buffer = localBuffer;
			s_CircleInputSpec.ShapeComponent = shapeComp;
		}

		// TODO: Shape Components and Buffers are memory leaks!
	}

	void ViewportPanel::OnOverlayRender()
	{
		if (s_EditorLayer->m_SceneState == SceneState::Play)
		{
			Entity camera = Scene::GetActiveScene()->GetPrimaryCameraEntity();
			if (!camera) { return; }
			Renderer::BeginScene(camera.GetComponent<CameraComponent>().Camera, glm::inverse(camera.GetComponent<TransformComponent>().GetTransform()));
		}
		else
		{
			Renderer::BeginScene(m_EditorCamera);
		}

		if (s_EditorLayer->m_ShowPhysicsColliders)
		{
			// Circle Colliders
			{
				auto view = Scene::GetActiveScene()->GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
				for (auto entity : view)
				{
					auto [tc, cc2d] = view.get<TransformComponent, CircleCollider2DComponent>(entity);

					Math::vec3 translation = tc.Translation + Math::vec3(cc2d.Offset.x, cc2d.Offset.y, 0.001f);

					Math::vec3 scale = tc.Scale * Math::vec3(cc2d.Radius * 2.0f);

					Math::mat4 transform = glm::translate(Math::mat4(1.0f), translation)
						* glm::scale(Math::mat4(1.0f), scale);

					s_CircleInputSpec.TransformMatrix = transform;
					Renderer::SubmitDataToRenderer(s_CircleInputSpec);
				}
			}
			// Box Colliders
			{
				auto view = Scene::GetActiveScene()->GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
				for (auto entity : view)
				{
					auto [tc, bc2d] = view.get<TransformComponent, BoxCollider2DComponent>(entity);

					Math::vec3 translation = tc.Translation + Math::vec3(bc2d.Offset.x, bc2d.Offset.y, 0.001f);
					Math::vec3 scale = tc.Scale * Math::vec3(bc2d.Size * 2.0f, 1.0f);

					Math::mat4 transform = glm::translate(Math::mat4(1.0f), translation)
						* glm::rotate(Math::mat4(1.0f), tc.Rotation.z, Math::vec3(0.0f, 0.0f, 1.0f))
						* glm::scale(Math::mat4(1.0f), scale);

					static Math::vec4 boxColliderColor {0.0f, 1.0f, 0.0f, 1.0f};
					Shader::SetDataAtInputLocation<Math::vec4>(boxColliderColor, "a_Color", s_LineInputSpec.Buffer, s_LineInputSpec.Shader);

					Math::vec3 lineVertices[4];
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

		if (s_EditorLayer->m_SceneState == SceneState::Edit || s_EditorLayer->m_SceneState == SceneState::Simulate || (s_EditorLayer->m_SceneState == SceneState::Play && s_EditorLayer->m_IsPaused))
		{
			// Draw selected entity outline 
			if (Entity selectedEntity = *Scene::GetActiveScene()->GetSelectedEntity()) {
				TransformComponent transform = selectedEntity.GetComponent<TransformComponent>();
				static Math::vec4 selectionColor {1.0f, 0.5f, 0.0f, 1.0f};
				Shader::SetDataAtInputLocation<Math::vec4>(selectionColor, "a_Color", s_LineInputSpec.Buffer, s_LineInputSpec.Shader);

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
					Renderer::SubmitDataToRenderer(s_LineInputSpec);
				}

				if (selectedEntity.HasComponent<CameraComponent>() && s_EditorLayer->m_ShowCameraFrustrums)
				{
					DrawFrustrum(selectedEntity);
				}
			}
		}

		Renderer::EndScene();
	}

	void ViewportPanel::DrawFrustrum(Entity& entity)
	{
		auto& transform = entity.GetComponent<TransformComponent>();
		auto& camera = entity.GetComponent<CameraComponent>();
		float windowWidth = (float)Core::GetCurrentApp().GetWindow().GetWidth();
		float windowHeight = (float)Core::GetCurrentApp().GetWindow().GetHeight();
		Math::vec4 viewport = { 0.0f, 0.0f, windowWidth, windowHeight };
		auto cameraProjectionType = camera.Camera.GetProjectionType();
		Math::vec4 selectionColor { 0.5f, 0.3f, 0.85f, 1.0f };
		Shader::SetDataAtInputLocation<Math::vec4>(selectionColor, "a_Color", s_LineInputSpec.Buffer, s_LineInputSpec.Shader);

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
				Shader::SetDataAtInputLocation<Math::vec4>(selectionColor, "a_Color", s_LineInputSpec.Buffer, s_LineInputSpec.Shader);
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

}

#include "Windows/UserInterface/UIEditorViewportPanel.h"

#include "EditorApp.h"
#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp{ nullptr };
static Kargono::Windows::UIEditorWindow* s_UIWindow{ nullptr };

namespace Kargono::Panels
{
	UIEditorViewportPanel::UIEditorViewportPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_UIWindow = s_EditorApp->m_UIEditorWindow.get();

		InitializeFrameBuffer();
		InitializeOverlayData();

		m_EditorCamera = Rendering::EditorOrthographicCamera(Math::vec2(100.0f, 100.0f), -3.0f, 3.0f);
		m_EditorCamera.SetPosition(Math::vec3(0.0f, 0.0f, 0.0f));
		m_EditorCamera.SetRotation(Math::vec3(0.0f, 0.0f, 0.0f));
		m_EditorCamera.SetKeyboardSpeed(200.0f);
		m_EditorCamera.SetKeyboardMinSpeed(50.0f);
		m_EditorCamera.SetKeyboardMaxSpeed(500.0f);

		KG_ASSERT(Projects::ProjectService::GetActive());
		m_ViewportAspectRatio = Utility::ScreenResolutionToAspectRatio(Projects::ProjectService::GetActiveTargetResolution());

	}
	void UIEditorViewportPanel::InitializeFrameBuffer()
	{
		Rendering::FramebufferSpecification fbSpec;
		fbSpec.Attachments = { Rendering::FramebufferDataFormat::RGBA8, Rendering::FramebufferDataFormat::RED_INTEGER, Rendering::FramebufferDataFormat::Depth };
		fbSpec.Width = EngineService::GetActiveWindow().GetWidth();
		fbSpec.Height = EngineService::GetActiveWindow().GetHeight();
		m_ViewportFramebuffer = Rendering::Framebuffer::Create(fbSpec);
	}

	void UIEditorViewportPanel::OnUpdate(Timestep ts)
	{
		KG_PROFILE_FUNCTION();

		// Handle editor camera movement
		FixedString32 focusedWindow{ EditorUI::EditorUIService::GetFocusedWindowName() };
		if (focusedWindow == m_PanelName)
		{
			m_EditorCamera.OnUpdate(ts);
		}

		// Adjust framebuffer & camera viewport size if necessary
		Window& currentWindow = EngineService::GetActiveWindow();
		currentWindow.SetActiveViewport(&m_ViewportData);
		if (Rendering::FramebufferSpecification spec = m_ViewportFramebuffer->GetSpecification();
			(float)m_ViewportData.m_Width > 0.0f && (float)m_ViewportData.m_Height > 0.0f &&
			(spec.Width != m_ViewportData.m_Width || spec.Height != m_ViewportData.m_Height))
		{
			// Update framebuffer and camera viewport size
			m_ViewportFramebuffer->Resize(m_ViewportData.m_Width, m_ViewportData.m_Height);
			m_EditorCamera.SetCameraFixedSize(Math::vec2((float)m_ViewportData.m_Width, (float)m_ViewportData.m_Height));
			m_EditorCamera.OnViewportResize();
		}

		// Prepare for rendering
		Rendering::RenderingService::ResetStats();
		m_ViewportFramebuffer->Bind();
		Rendering::RendererAPI::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Rendering::RendererAPI::Clear();

		// Clear mouse picking attachment value
		m_ViewportFramebuffer->SetAttachment(1, -1);

		// TODO: Add background image to viewport


		DrawUnderlay();

		// Handle drawing user interface
		Window& currentApplication = EngineService::GetActiveWindow();
		RuntimeUI::RuntimeUIService::PushRenderData(m_EditorCamera.GetViewProjection(), m_ViewportData.m_Width, m_ViewportData.m_Height);

		HandleMouseHovering();

		// Draw window/widget overlay
		DrawOverlay();

		// Complete rendering
		m_ViewportFramebuffer->Unbind();

	}
	void UIEditorViewportPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_UIWindow->m_ShowViewport);
		ImGui::PopStyleVar();

		// Early out if the window is not visible
		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		EditorUI::EditorUIService::AutoCalcViewportSize(m_ScreenViewportBounds, m_ViewportData, m_ViewportFocused, m_ViewportHovered,
			m_ViewportAspectRatio);

		uint64_t textureID = m_ViewportFramebuffer->GetColorAttachmentRendererID();
		ImGui::Image((ImTextureID)textureID, ImVec2{ (float)m_ViewportData.m_Width, (float)m_ViewportData.m_Height }, ImVec2{ 0, 1 },
			ImVec2{ 1, 0 });
		if (ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::GetIO().WantCaptureMouse)
		{
			if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::InputService::IsKeyPressed(Key::LeftAlt))
			{
				if (m_HoveredWindowID != k_InvalidWindowID)
				{
					EditorUI::TreePath path;
					bool success{ false };
					path.AddNode(m_HoveredWindowID);
					if (m_HoveredWidgetID == k_InvalidWidgetID)
					{
						success = s_UIWindow->m_TreePanel->m_UITree.SelectEntry(path);
					}
					else
					{
						s_UIWindow->m_TreePanel->m_UITree.ExpandNodePath(path);
						path.AddNode(m_HoveredWidgetID);
						success = s_UIWindow->m_TreePanel->m_UITree.SelectEntry(path);
					}

					if (!success)
					{
						KG_WARN("Failed to select window/widget with ID {} and {}", m_HoveredWindowID, m_HoveredWidgetID);
					}
				}
			}
			
		}

		DrawGizmo();

		DrawToolbarOverlay();

		// End the window
		EditorUI::EditorUIService::EndWindow();
	}

	void UIEditorViewportPanel::OnInputEvent(Events::Event* event)
	{
		m_EditorCamera.OnInputEvent(event);
	}

	bool UIEditorViewportPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		// Check if alt key is pressed
		bool alt = Input::InputService::IsKeyPressed(Key::LeftAlt) || Input::InputService::IsKeyPressed(Key::RightAlt);

		// Handle various key presses for the user interface editor panel
		switch (event.GetKeyCode())
		{
			
			case Key::Q:
			{
				// Reset gizmo type
				if (!ImGuizmo::IsUsing() && !alt)
				{
					m_GizmoType = -1;
					return true;
				}
			}
			case Key::W:
			{
				// Set gizmo type to translate
				if (!ImGuizmo::IsUsing() && !alt)
				{
					m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
					return true;
				}
			}
			default:
			{
				return false;
			}
		}

		return false;
	}

	// Overlay Data
	static Rendering::RendererInputSpec s_LineInputSpec{};
	static std::vector<Math::vec4> s_RectangleVertexPositions
	{
			{ -0.5f, -0.5f, 0.0f, 1.0f },
			{ 0.5f, -0.5f, 0.0f, 1.0f },
			{ 0.5f, 0.5f, 0.0f, 1.0f },
			{ -0.5f, 0.5f, 0.0f, 1.0f }
	};

	static std::vector<uint32_t> s_RectangleIndices
	{
			0, 1,
			1, 2,
			2, 3,
			3, 0
	};
	static Ref<std::vector<Math::vec3>> s_OutputVector = CreateRef<std::vector<Math::vec3>>();


	void UIEditorViewportPanel::InitializeOverlayData()
	{
		// Set up Line Input Specifications for Overlay Calls
		{
			Rendering::ShaderSpecification lineShaderSpec{ Rendering::ColorInputType::FlatColor, Rendering::TextureInputType::None, false, true, false, Rendering::RenderingType::DrawLine, false };
			auto [uuid, localShader] = Assets::AssetService::GetShader(lineShaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			// Add red color to shader
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_Red), "a_Color", localBuffer, localShader);

			// Create the shape component
			ECS::ShapeComponent* lineShapeComponent = new ECS::ShapeComponent();
			lineShapeComponent->CurrentShape = Rendering::ShapeTypes::Quad;

			s_LineInputSpec.m_Shader = localShader;
			s_LineInputSpec.m_Buffer = localBuffer;
			s_LineInputSpec.m_ShapeComponent = lineShapeComponent;
		}
	}

	void UIEditorViewportPanel::DrawOverlay()
	{
		if (s_UIWindow->m_PropertiesPanel->m_ActiveWindow && s_UIWindow->m_PropertiesPanel->m_ActiveWindow->GetWindowDisplayed())
		{

			// Reset the rendering context
			Rendering::RendererAPI::ClearDepthBuffer();

			//// Calculate orthographic projection matrix for user interface
			//Math::mat4 orthographicProjection = glm::ortho(0.0f, (float)m_ViewportData.m_Width,
			//	0.0f, (float)m_ViewportData.m_Height, -1.0f, 1.0f);

			// Start rendering context
			Rendering::RenderingService::BeginScene(m_EditorCamera.GetViewProjection());

			// Draw window/widget bounding boxes
			DrawDebugLines();
			Rendering::RenderingService::EndScene();
		}
	}
	void UIEditorViewportPanel::DrawUnderlay()
	{
		// Start rendering context
		Rendering::RenderingService::BeginScene(m_EditorCamera.GetViewProjection());

		// Draw grid if enabled
		if (m_DisplayGrid)
		{
			DrawProportionalGrid();
		}

		// Draw viewport outline
		DrawViewportOutline();

		Rendering::RenderingService::EndScene();

	}
	void UIEditorViewportPanel::DrawViewportOutline()
	{

		// Draw viewport bounds
		Math::vec3 selectionBoxVertices[4]
		{
		{ 0.0f, 0.0f, -1.0f },
		{ (float)m_ViewportData.m_Width, 0.0f, -1.0f },
		{ (float)m_ViewportData.m_Width, (float)m_ViewportData.m_Height, -1.0f },
		{ 0.0f, (float)m_ViewportData.m_Height, -1.0f }
		};
		Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_HighlightColor1), "a_Color", s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);
		s_OutputVector->clear();
		s_OutputVector->push_back(selectionBoxVertices[0]);
		s_OutputVector->push_back(selectionBoxVertices[1]);
		s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
		Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
		s_OutputVector->clear();
		s_OutputVector->push_back(selectionBoxVertices[1]);
		s_OutputVector->push_back(selectionBoxVertices[2]);
		s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
		Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
		s_OutputVector->clear();
		s_OutputVector->push_back(selectionBoxVertices[2]);
		s_OutputVector->push_back(selectionBoxVertices[3]);
		s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
		Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
		s_OutputVector->clear();
		s_OutputVector->push_back(selectionBoxVertices[3]);
		s_OutputVector->push_back(selectionBoxVertices[0]);
		s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
		Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
		
	}
	void UIEditorViewportPanel::HandleMouseHovering()
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
	}
	void UIEditorViewportPanel::DrawToolbarOverlay()
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


			// Camera Options Button
			icon = EditorUI::EditorUIService::s_IconCamera;
			ImGui::SetCursorPos(ImVec2(initialCursorPos.x + windowSize.x - 163, initialCursorPos.y + 5));
			if (ImGui::ImageButton("Camera Options",
				(ImTextureID)(uint64_t)icon->GetRendererID(),
				ImVec2(14, 14), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
				EditorUI::EditorUIService::s_PureEmpty,
				EditorUI::EditorUIService::s_HighlightColor1))
			{
				ImGui::OpenPopup("UI Camera Options");
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, "Camera Options");
				ImGui::EndTooltip();
			}

			if (ImGui::BeginPopup("UI Camera Options"))
			{
				if (ImGui::MenuItem("Reset Camera Position"))
				{
					ResetCamera();
				}
				ImGui::EndPopup();
			}

			// Camera Speed
			ImGui::SetNextItemWidth(30.0f);
			ImGui::SetCursorPos(ImVec2(initialCursorPos.x + windowSize.x - 138, initialCursorPos.y + 6));
			ImGui::DragFloat("##CameraSpeed", &m_EditorCamera.GetKeyboardSpeed(), 0.5f,
				m_EditorCamera.GetKeyboardMinSpeed(), m_EditorCamera.GetKeyboardMaxSpeed(),
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
				if (ImGui::BeginMenu("Resolution Options"))
				{
					if (ImGui::MenuItem("Wide Screen (16:9)"))
					{
						m_ViewportAspectRatio = Math::uvec2(16, 9);
					}
					if (ImGui::MenuItem("Standard SD (4:3)"))
					{
						m_ViewportAspectRatio = Math::uvec2(4, 3);
					}
					if (ImGui::MenuItem("Box (1:1)"))
					{
						m_ViewportAspectRatio = Math::uvec2(1, 1);
					}

					ImGui::Separator();

					if (ImGui::MenuItem("Match Current Monitor"))
					{
						m_ViewportAspectRatio = Utility::ScreenResolutionToAspectRatio(
							Projects::ScreenResolutionOptions::MatchDevice);
					}
					if (ImGui::MenuItem("Set to Project Resolution"))
					{
						m_ViewportAspectRatio = Utility::ScreenResolutionToAspectRatio(
							Projects::ProjectService::GetActiveTargetResolution());
					}

					ImGui::EndMenu();
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
				if (ImGui::MenuItem("Display Grid", 0, m_DisplayGrid))
				{
					Utility::Operations::ToggleBoolean(m_DisplayGrid);
				}
				ImGui::EndPopup();
			}

			// Grid Spacing
			ImGui::SetNextItemWidth(30.0f);
			ImGui::SetCursorPos(ImVec2(initialCursorPos.x + windowSize.x - 227, initialCursorPos.y + 6));
			ImGui::DragFloat("##GridSpacing", &m_PropertionalGridSections, 1.0f,
				1.0f, 50.0f,
				"%.0f", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_CenterText);
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, "Section Count");
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
	}
	void UIEditorViewportPanel::DrawProportionalGrid()
	{
		// Draw grid lines
		if (m_PropertionalGridSections == 1)
		{
			return;
		}

		// Calculate grid line values and set up vertex data
		size_t lineCount = {((size_t)m_PropertionalGridSections - 1) * 2};
		size_t vertexCount = lineCount * 2;
		std::vector<Math::vec3> gridVertices;
		gridVertices.reserve(vertexCount);

		// Fill vertex data
		for (size_t iteration{ 1 }; iteration <= lineCount; iteration++)
		{
			float x = ((float)m_ViewportData.m_Width / m_PropertionalGridSections) * iteration;
			float y = ((float)m_ViewportData.m_Height / m_PropertionalGridSections) * iteration;

			// Horizontal line
			gridVertices.push_back({ 0.0f, y, -1.0f });
			gridVertices.push_back({ (float)m_ViewportData.m_Width, y, -1.0f });

			// Vertical line
			gridVertices.push_back({ x, 0.0f, -1.0f });
			gridVertices.push_back({ x, (float)m_ViewportData.m_Height, -1.0f });
		}

		Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_GridMinor), "a_Color", s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);

		for (size_t i = 0; i < vertexCount; i += 2)
		{
			s_OutputVector->clear();
			s_OutputVector->push_back(gridVertices[i]);
			s_OutputVector->push_back(gridVertices[i + 1]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
		}
	}
	void UIEditorViewportPanel::DrawGizmo()
	{
		// Retrieve the active widget and window
		RuntimeUI::Widget* widget{ s_UIWindow->m_PropertiesPanel->m_ActiveWidget };
		RuntimeUI::Window* window{ s_UIWindow->m_PropertiesPanel->m_ActiveWindow };

		// Ensure that the window is valid
		if (!window || m_GizmoType == -1)
		{
			return;
		}
		
		
		if (!widget)
		{
			// Set up the gizmo for the window
			ImGuizmo::SetOrthographic(true);
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(m_ScreenViewportBounds[0].x, m_ScreenViewportBounds[0].y,
				m_ScreenViewportBounds[1].x - m_ScreenViewportBounds[0].x,
				m_ScreenViewportBounds[1].y - m_ScreenViewportBounds[0].y);

			// Editor Camera
			const Math::mat4& cameraProjection = m_EditorCamera.GetProjection();
			Math::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			// Get window transform
			Math::vec3 windowPosition = window->CalculateWorldPosition(m_ViewportData.m_Width, m_ViewportData.m_Height);
			Math::mat4 transform{ glm::translate(Math::mat4(1.0), windowPosition)};

			// Snapping
			bool snap = Input::InputService::IsKeyPressed(Key::LeftControl);
			float snapValue = 0.5f;
			if (m_GizmoType == ImGuizmo::OPERATION::ROTATE) { snapValue = 45.0f; }

			float snapValues[3] = { snapValue, snapValue, snapValue };

			// Manipulate and draw the gizmo
			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::WORLD, glm::value_ptr(transform),
				nullptr, snap ? snapValues : nullptr);
			if (ImGuizmo::IsUsing())
			{
				// Get the new transform
				Math::vec3 translation, rotation, scale;
				Math::DecomposeTransform(transform, translation, rotation, scale);

				// Update the window's position
				window->m_ScreenPosition = window->CalculateScreenPosition(
					Math::vec2(translation.x, translation.y), 
					m_ViewportData.m_Width, 
					m_ViewportData.m_Height);
			}
		}
		else
		{
			ImGuizmo::SetOrthographic(true);
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(m_ScreenViewportBounds[0].x, m_ScreenViewportBounds[0].y,
				m_ScreenViewportBounds[1].x - m_ScreenViewportBounds[0].x,
				m_ScreenViewportBounds[1].y - m_ScreenViewportBounds[0].y);

			// Editor Camera
			const Math::mat4& cameraProjection = m_EditorCamera.GetProjection();
			Math::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			// Get widget transform
			Math::vec3 windowPosition = window->CalculateWorldPosition(m_ViewportData.m_Width, m_ViewportData.m_Height);
			Math::vec3 windowSize = window->CalculateSize(m_ViewportData.m_Width, m_ViewportData.m_Height);
			Math::vec3 widgetPosition = widget->CalculateWorldPosition(windowPosition, windowSize);
			Math::mat4 transform{ glm::translate(Math::mat4(1.0), widgetPosition) };

			// Snapping
			bool snap = Input::InputService::IsKeyPressed(Key::LeftControl);
			float snapValue = 0.5f;
			if (m_GizmoType == ImGuizmo::OPERATION::ROTATE) { snapValue = 45.0f; }

			float snapValues[3] = { snapValue, snapValue, snapValue };

			// Manipulate and draw the gizmo
			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::WORLD, glm::value_ptr(transform),
				nullptr, snap ? snapValues : nullptr);
			if (ImGuizmo::IsUsing())
			{
				// Get the new transform
				Math::vec3 translation, rotation, scale;
				Math::DecomposeTransform(transform, translation, rotation, scale);

				// Update the widget's position
				Math::vec3 screenPosition = widget->CalculateWindowPosition(
					Math::vec2(translation.x, translation.y),
					windowPosition,
					windowSize);

				// Update the widget's x position
				if (widget->m_XPositionType == RuntimeUI::PixelOrPercent::Percent)
				{
					widget->m_PercentPosition.x = screenPosition.x;
				}
				else
				{
					widget->m_PixelPosition.x = (int)screenPosition.x;
				}

				// Update the widget's y position
				if (widget->m_YPositionType == RuntimeUI::PixelOrPercent::Percent)
				{
					widget->m_PercentPosition.y = screenPosition.y;
				}
				else
				{
					widget->m_PixelPosition.y = (int)screenPosition.y;
				}
				
			}
		}
	}
	void UIEditorViewportPanel::OnOpenUI()
	{
		ResetCamera();
	}
	void UIEditorViewportPanel::DrawDebugLines()
	{
		// Retrieve the active widget and window
		RuntimeUI::Widget* widget{ s_UIWindow->m_PropertiesPanel->m_ActiveWidget };
		RuntimeUI::Window* window{ s_UIWindow->m_PropertiesPanel->m_ActiveWindow };

		// Get position data for rendering window
		Math::vec3 windowScale = window->CalculateSize(m_ViewportData.m_Width, m_ViewportData.m_Height);
		Math::vec3 initialWindowTranslation = window->CalculateWorldPosition(m_ViewportData.m_Width, m_ViewportData.m_Height);
		Math::vec3 finalWindowTranslation = Math::vec3(initialWindowTranslation.x + (windowScale.x / 2), initialWindowTranslation.y + (windowScale.y / 2), initialWindowTranslation.z);

		// Handle either active widget specific debug lines or active window debug lines
		if (widget)
		{
			// Calculate the widget's rendering data
			Math::vec3 widgetSize = widget->CalculateSize(windowScale);
			Math::vec3 widgetTranslation = widget->CalculateWorldPosition(initialWindowTranslation, windowScale);
			Math::vec3 modifiedOriginTranslation = Math::vec3(
				widgetTranslation.x + (widgetSize.x / 2), 
				widgetTranslation.y + (widgetSize.y / 2), 
				widgetTranslation.z);
			// Create the widget's background rendering data
			Math::mat4 widgetTransform = glm::translate(Math::mat4(1.0f), modifiedOriginTranslation)
				* glm::scale(Math::mat4(1.0f), widgetSize);

			// Draw the widget's selection box
			Math::vec3 selectionBoxVertices[4];
			for (size_t i = 0; i < 4; i++)
			{
				selectionBoxVertices[i] = widgetTransform * s_RectangleVertexPositions[i];
			}
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_HighlightColor4), "a_Color", s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);
			s_OutputVector->clear();
			s_OutputVector->push_back(selectionBoxVertices[0]);
			s_OutputVector->push_back(selectionBoxVertices[1]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
			s_OutputVector->clear();
			s_OutputVector->push_back(selectionBoxVertices[1]);
			s_OutputVector->push_back(selectionBoxVertices[2]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
			s_OutputVector->clear();
			s_OutputVector->push_back(selectionBoxVertices[2]);
			s_OutputVector->push_back(selectionBoxVertices[3]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
			s_OutputVector->clear();
			s_OutputVector->push_back(selectionBoxVertices[3]);
			s_OutputVector->push_back(selectionBoxVertices[0]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);

			DrawWidgetConstraintDistanceLines(window, widget, widgetTransform, modifiedOriginTranslation);

		}
		if (window)
		{
			// Create background rendering data
			Math::mat4 windowTransform = glm::translate(Math::mat4(1.0f), finalWindowTranslation)
				* glm::scale(Math::mat4(1.0f), windowScale);

			Math::vec3 selectionBoxVertices[4];
			for (size_t i = 0; i < 4; i++)
			{
				selectionBoxVertices[i] = windowTransform * s_RectangleVertexPositions[i];
			}
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_Red), "a_Color", s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);
			s_OutputVector->clear();
			s_OutputVector->push_back(selectionBoxVertices[0]);
			s_OutputVector->push_back(selectionBoxVertices[1]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
			s_OutputVector->clear();
			s_OutputVector->push_back(selectionBoxVertices[1]);
			s_OutputVector->push_back(selectionBoxVertices[2]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
			s_OutputVector->clear();
			s_OutputVector->push_back(selectionBoxVertices[2]);
			s_OutputVector->push_back(selectionBoxVertices[3]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
			s_OutputVector->clear();
			s_OutputVector->push_back(selectionBoxVertices[3]);
			s_OutputVector->push_back(selectionBoxVertices[0]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);

			if (!widget)
			{
				DrawWindowConstraintDistanceLines(window);
			}
		}

	}
	void UIEditorViewportPanel::DrawWidgetConstraintDistanceLines(RuntimeUI::Window* window, RuntimeUI::Widget* widget, const Math::mat4& widgetTransform, const Math::vec3& widgetTranslation)
	{
		Math::vec3 constraintDistanceVerts[6];
		constexpr float k_ConstraintPadding{ 5.0f };
		constexpr float k_VanityPaddingSize{ 15.0f };

		Math::vec3 windowScale = window->CalculateSize(m_ViewportData.m_Width, m_ViewportData.m_Height);
		Math::vec3 initialWindowTranslation = window->CalculateWorldPosition(m_ViewportData.m_Width, m_ViewportData.m_Height);
		Math::vec3 widgetSize = widget->CalculateSize(windowScale);


		if (widget->m_XRelativeOrAbsolute == RuntimeUI::RelativeOrAbsolute::Absolute ||
			widget->m_XConstraint == RuntimeUI::Constraint::Left ||
			widget->m_XConstraint == RuntimeUI::Constraint::None ||
			widget->m_XConstraint == RuntimeUI::Constraint::Center)
		{
			// Create widget's constraint distance lines
			constraintDistanceVerts[0] = { initialWindowTranslation.x, widgetTranslation.y, widgetTranslation.z };
			constraintDistanceVerts[1] = widgetTransform * Math::vec4(-0.5f, 0.0f, 0.0f, 1.0f); // Transform center of widget to world space

			float widgetLeftEdge = widgetTranslation.x - (widgetSize.x / 2.0f);
			float windowLeftEdge = initialWindowTranslation.x;

			// Check if the widget's edge is outside the window
			if (widgetLeftEdge > windowLeftEdge + k_ConstraintPadding)
			{
				// Apply padding when the widget's edge is outside the window
				constraintDistanceVerts[0].x += k_ConstraintPadding;
				constraintDistanceVerts[1].x -= k_ConstraintPadding;
			}
			else if (widgetLeftEdge < windowLeftEdge - k_ConstraintPadding)
			{
				// Apply padding when the widget's edge is inside the window
				constraintDistanceVerts[0].x -= k_ConstraintPadding;
				constraintDistanceVerts[1].x += k_ConstraintPadding;
			}
			// Note, otherwise just leave the lines as is

			// Add vanity lines
			constraintDistanceVerts[2] = { constraintDistanceVerts[0].x, constraintDistanceVerts[0].y - k_VanityPaddingSize, widgetTranslation.z };
			constraintDistanceVerts[3] = { constraintDistanceVerts[0].x, constraintDistanceVerts[0].y + k_VanityPaddingSize, widgetTranslation.z };
			constraintDistanceVerts[4] = { constraintDistanceVerts[1].x, constraintDistanceVerts[0].y - k_VanityPaddingSize, widgetTranslation.z };
			constraintDistanceVerts[5] = { constraintDistanceVerts[1].x, constraintDistanceVerts[0].y + k_VanityPaddingSize, widgetTranslation.z };

			// Draw the x-axis constraint distance lines
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_HighlightColor1), "a_Color", s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);
			s_OutputVector->clear();
			s_OutputVector->push_back(constraintDistanceVerts[0]);
			s_OutputVector->push_back(constraintDistanceVerts[1]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
			s_OutputVector->clear();
			s_OutputVector->push_back(constraintDistanceVerts[2]);
			s_OutputVector->push_back(constraintDistanceVerts[3]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
			s_OutputVector->clear();
			s_OutputVector->push_back(constraintDistanceVerts[4]);
			s_OutputVector->push_back(constraintDistanceVerts[5]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
		}
		if (widget->m_XConstraint == RuntimeUI::Constraint::Right || widget->m_XConstraint == RuntimeUI::Constraint::Center)
		{
			// Create widget's constraint distance lines
			constraintDistanceVerts[0] = widgetTransform * Math::vec4(0.5f, 0.0f, 0.0f, 1.0f); // Transform center of widget to world space
			constraintDistanceVerts[1] = { initialWindowTranslation.x + windowScale.x, widgetTranslation.y, widgetTranslation.z };

			// Apply padding to distance lines
			
			float widgetRightEdge = widgetTranslation.x + (widgetSize.x / 2.0f);
			float windowRightEdge = initialWindowTranslation.x + windowScale.x;

			// Check if the widget's edge is outside the window
			if (widgetRightEdge > windowRightEdge + k_ConstraintPadding)
			{
				// Apply padding when the widget's edge is outside the window
				constraintDistanceVerts[0].x -= k_ConstraintPadding;
				constraintDistanceVerts[1].x += k_ConstraintPadding;
			}
			else if (widgetRightEdge < windowRightEdge - k_ConstraintPadding)
			{
				// Apply padding when the widget's edge is inside the window
				constraintDistanceVerts[0].x += k_ConstraintPadding;
				constraintDistanceVerts[1].x -= k_ConstraintPadding;
			}
			// Note, otherwise just leave the lines as is
			
			

			// Add vanity lines
			constraintDistanceVerts[2] = { constraintDistanceVerts[0].x, constraintDistanceVerts[0].y - k_VanityPaddingSize, widgetTranslation.z };
			constraintDistanceVerts[3] = { constraintDistanceVerts[0].x, constraintDistanceVerts[0].y + k_VanityPaddingSize, widgetTranslation.z };
			constraintDistanceVerts[4] = { constraintDistanceVerts[1].x, constraintDistanceVerts[0].y - k_VanityPaddingSize, widgetTranslation.z };
			constraintDistanceVerts[5] = { constraintDistanceVerts[1].x, constraintDistanceVerts[0].y + k_VanityPaddingSize, widgetTranslation.z };

			// Draw the x-axis constraint distance lines
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_HighlightColor1), "a_Color", s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);
			s_OutputVector->clear();
			s_OutputVector->push_back(constraintDistanceVerts[0]);
			s_OutputVector->push_back(constraintDistanceVerts[1]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
			s_OutputVector->clear();
			s_OutputVector->push_back(constraintDistanceVerts[2]);
			s_OutputVector->push_back(constraintDistanceVerts[3]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
			s_OutputVector->clear();
			s_OutputVector->push_back(constraintDistanceVerts[4]);
			s_OutputVector->push_back(constraintDistanceVerts[5]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
		}

		if (widget->m_YRelativeOrAbsolute == RuntimeUI::RelativeOrAbsolute::Absolute ||
			widget->m_YConstraint == RuntimeUI::Constraint::Bottom ||
			widget->m_YConstraint == RuntimeUI::Constraint::None ||
			widget->m_YConstraint == RuntimeUI::Constraint::Center)
		{
			// Create widget's constraint distance lines
			constraintDistanceVerts[0] = { widgetTranslation.x , initialWindowTranslation.y, widgetTranslation.z };
			constraintDistanceVerts[1] = widgetTransform * Math::vec4(0.0f, -0.5f, 0.0f, 1.0f); // Transform center of widget to world space

			float widgetBottomEdge = widgetTranslation.y - (widgetSize.y / 2.0f);
			float windowBottomEdge = initialWindowTranslation.y;

			// Check if the widget's edge is outside the window
			if (widgetBottomEdge > windowBottomEdge + k_ConstraintPadding)
			{
				// Apply padding when the widget's edge is outside the window
				constraintDistanceVerts[0].y += k_ConstraintPadding;
				constraintDistanceVerts[1].y -= k_ConstraintPadding;
			}
			else if (widgetBottomEdge < windowBottomEdge - k_ConstraintPadding)
			{
				// Apply padding when the widget's edge is inside the window
				constraintDistanceVerts[0].y -= k_ConstraintPadding;
				constraintDistanceVerts[1].y += k_ConstraintPadding;
			}
			// Note, otherwise just leave the lines as is

			// Add vanity lines
			constraintDistanceVerts[2] = { constraintDistanceVerts[0].x - k_VanityPaddingSize, constraintDistanceVerts[0].y, widgetTranslation.z };
			constraintDistanceVerts[3] = { constraintDistanceVerts[0].x + k_VanityPaddingSize, constraintDistanceVerts[0].y, widgetTranslation.z };
			constraintDistanceVerts[4] = { constraintDistanceVerts[0].x - k_VanityPaddingSize, constraintDistanceVerts[1].y, widgetTranslation.z };
			constraintDistanceVerts[5] = { constraintDistanceVerts[0].x + k_VanityPaddingSize, constraintDistanceVerts[1].y, widgetTranslation.z };

			// Draw the x-axis constraint distance lines
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_HighlightColor2), "a_Color", s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);
			s_OutputVector->clear();
			s_OutputVector->push_back(constraintDistanceVerts[0]);
			s_OutputVector->push_back(constraintDistanceVerts[1]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
			s_OutputVector->clear();
			s_OutputVector->push_back(constraintDistanceVerts[2]);
			s_OutputVector->push_back(constraintDistanceVerts[3]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
			s_OutputVector->clear();
			s_OutputVector->push_back(constraintDistanceVerts[4]);
			s_OutputVector->push_back(constraintDistanceVerts[5]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
		}

		if (widget->m_YConstraint == RuntimeUI::Constraint::Top || widget->m_YConstraint == RuntimeUI::Constraint::Center)
		{
			// Create widget's constraint distance lines
			constraintDistanceVerts[0] = widgetTransform * Math::vec4(0.0f, 0.5f, 0.0f, 1.0f); // Transform center of widget to world space
			constraintDistanceVerts[1] = { widgetTranslation.x, initialWindowTranslation.y + windowScale.y, widgetTranslation.z };

			// Apply padding to distance lines
			float widgetTopEdge = widgetTranslation.y + (widgetSize.y / 2.0f);
			float windowTopEdge = initialWindowTranslation.y + windowScale.y;

			// Check if the widget's edge is outside the window
			if (widgetTopEdge > windowTopEdge + k_ConstraintPadding)
			{
				// Apply padding when the widget's edge is outside the window
				constraintDistanceVerts[0].y -= k_ConstraintPadding;
				constraintDistanceVerts[1].y += k_ConstraintPadding;
			}
			else if (widgetTopEdge < windowTopEdge - k_ConstraintPadding)
			{
				// Apply padding when the widget's edge is inside the window
				constraintDistanceVerts[0].y += k_ConstraintPadding;
				constraintDistanceVerts[1].y -= k_ConstraintPadding;
			}
			// Note, otherwise just leave the lines as is
			


			// Add vanity lines
			constraintDistanceVerts[2] = { constraintDistanceVerts[0].x - k_VanityPaddingSize, constraintDistanceVerts[0].y, widgetTranslation.z };
			constraintDistanceVerts[3] = { constraintDistanceVerts[0].x + k_VanityPaddingSize, constraintDistanceVerts[0].y, widgetTranslation.z };
			constraintDistanceVerts[4] = { constraintDistanceVerts[0].x - k_VanityPaddingSize, constraintDistanceVerts[1].y, widgetTranslation.z };
			constraintDistanceVerts[5] = { constraintDistanceVerts[0].x + k_VanityPaddingSize, constraintDistanceVerts[1].y, widgetTranslation.z };

			// Draw the x-axis constraint distance lines
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_HighlightColor2), "a_Color", s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);
			s_OutputVector->clear();
			s_OutputVector->push_back(constraintDistanceVerts[0]);
			s_OutputVector->push_back(constraintDistanceVerts[1]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
			s_OutputVector->clear();
			s_OutputVector->push_back(constraintDistanceVerts[2]);
			s_OutputVector->push_back(constraintDistanceVerts[3]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
			s_OutputVector->clear();
			s_OutputVector->push_back(constraintDistanceVerts[4]);
			s_OutputVector->push_back(constraintDistanceVerts[5]);
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
		}
	}

	void UIEditorViewportPanel::DrawWindowConstraintDistanceLines(RuntimeUI::Window* window)
	{
		Math::vec3 constraintDistanceVerts[6];
		constexpr float k_ConstraintPadding{ 5.0f };
		constexpr float k_VanityPaddingSize{ 15.0f };

		Math::vec3 windowScale = window->CalculateSize(m_ViewportData.m_Width, m_ViewportData.m_Height);
		Math::vec3 initialWindowTranslation = window->CalculateWorldPosition(m_ViewportData.m_Width, m_ViewportData.m_Height);
		Math::vec3 finalWindowTranslation = Math::vec3(initialWindowTranslation.x + (windowScale.x / 2), initialWindowTranslation.y + (windowScale.y / 2), initialWindowTranslation.z);

		// Create window's constraint distance lines
		constraintDistanceVerts[0] = { 0.0f, finalWindowTranslation.y, finalWindowTranslation.z };
		constraintDistanceVerts[1] = { finalWindowTranslation.x - windowScale.x / 2.0f, finalWindowTranslation.y, finalWindowTranslation.z }; 

		float windowLeftEdge = finalWindowTranslation.x - (windowScale.x / 2.0f);
		float viewportLeftEdge = 0.0f;

		// Check if the window's edge is outside the viewport
		if (windowLeftEdge > viewportLeftEdge + k_ConstraintPadding)
		{
			// Apply padding when the window's edge is outside the viewport
			constraintDistanceVerts[0].x += k_ConstraintPadding;
			constraintDistanceVerts[1].x -= k_ConstraintPadding;
		}
		else if (windowLeftEdge < viewportLeftEdge - k_ConstraintPadding)
		{
			// Apply padding when the window's edge is inside the viewport
			constraintDistanceVerts[0].x -= k_ConstraintPadding;
			constraintDistanceVerts[1].x += k_ConstraintPadding;
		}
		// Note, otherwise just leave the lines as is

		// Add vanity lines
		constraintDistanceVerts[2] = { constraintDistanceVerts[0].x, constraintDistanceVerts[0].y - k_VanityPaddingSize, finalWindowTranslation.z };
		constraintDistanceVerts[3] = { constraintDistanceVerts[0].x, constraintDistanceVerts[0].y + k_VanityPaddingSize, finalWindowTranslation.z };
		constraintDistanceVerts[4] = { constraintDistanceVerts[1].x, constraintDistanceVerts[0].y - k_VanityPaddingSize, finalWindowTranslation.z };
		constraintDistanceVerts[5] = { constraintDistanceVerts[1].x, constraintDistanceVerts[0].y + k_VanityPaddingSize, finalWindowTranslation.z };

		// Draw the x-axis constraint distance lines
		Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_HighlightColor1), "a_Color", s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);
		s_OutputVector->clear();
		s_OutputVector->push_back(constraintDistanceVerts[0]);
		s_OutputVector->push_back(constraintDistanceVerts[1]);
		s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
		Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
		s_OutputVector->clear();
		s_OutputVector->push_back(constraintDistanceVerts[2]);
		s_OutputVector->push_back(constraintDistanceVerts[3]);
		s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
		Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
		s_OutputVector->clear();
		s_OutputVector->push_back(constraintDistanceVerts[4]);
		s_OutputVector->push_back(constraintDistanceVerts[5]);
		s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
		Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
		
		// Create window's constraint distance lines
		constraintDistanceVerts[0] = { finalWindowTranslation.x, 0.0f, finalWindowTranslation.z };
		constraintDistanceVerts[1] = { finalWindowTranslation.x, finalWindowTranslation.y - windowScale.y / 2.0f , finalWindowTranslation.z };
		float windowBottomEdge = finalWindowTranslation.y - (windowScale.y / 2.0f);
		float viewportBottomEdge = 0.0f;

		// Check if the window's edge is outside the viewport
		if (windowBottomEdge > viewportBottomEdge + k_ConstraintPadding)
		{
			// Apply padding when the window's edge is outside the viewport
			constraintDistanceVerts[0].y += k_ConstraintPadding;
			constraintDistanceVerts[1].y -= k_ConstraintPadding;
		}
		else if (windowBottomEdge < viewportBottomEdge - k_ConstraintPadding)
		{
			// Apply padding when the window's edge is inside the viewport
			constraintDistanceVerts[0].y -= k_ConstraintPadding;
			constraintDistanceVerts[1].y += k_ConstraintPadding;
		}
		// Note, otherwise just leave the lines as is

		// Add vanity lines
		constraintDistanceVerts[2] = { constraintDistanceVerts[0].x - k_VanityPaddingSize, constraintDistanceVerts[0].y, finalWindowTranslation.z };
		constraintDistanceVerts[3] = { constraintDistanceVerts[0].x + k_VanityPaddingSize, constraintDistanceVerts[0].y, finalWindowTranslation.z };
		constraintDistanceVerts[4] = { constraintDistanceVerts[0].x - k_VanityPaddingSize, constraintDistanceVerts[1].y, finalWindowTranslation.z };
		constraintDistanceVerts[5] = { constraintDistanceVerts[0].x + k_VanityPaddingSize, constraintDistanceVerts[1].y, finalWindowTranslation.z };

		// Draw the x-axis constraint distance lines
		Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_HighlightColor2), "a_Color", s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);
		s_OutputVector->clear();
		s_OutputVector->push_back(constraintDistanceVerts[0]);
		s_OutputVector->push_back(constraintDistanceVerts[1]);
		s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
		Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
		s_OutputVector->clear();
		s_OutputVector->push_back(constraintDistanceVerts[2]);
		s_OutputVector->push_back(constraintDistanceVerts[3]);
		s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
		Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
		s_OutputVector->clear();
		s_OutputVector->push_back(constraintDistanceVerts[4]);
		s_OutputVector->push_back(constraintDistanceVerts[5]);
		s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
		Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
	}

	void UIEditorViewportPanel::ResetCamera()
	{
		m_EditorCamera.SetOrthographicSize(1.0f);
		m_EditorCamera.SetPosition({ 0.0f, 0.0f, 0.0f });
	}

}

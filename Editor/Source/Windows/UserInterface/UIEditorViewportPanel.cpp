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

		m_EditorCamera = Rendering::EditorOrthographicCamera(1000.0f, -2.0f, 2.0f);
		m_EditorCamera.SetPosition(Math::vec3(0.0f, 0.0f, 0.0f));
		m_EditorCamera.SetOrientation(Math::vec3(0.0f, 0.0f, 0.0f));
		m_EditorCamera.SetKeyboardSpeed(150.0f);
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

		EditorUI::EditorUIService::AutoCalcViewportSize(m_ScreenViewportBounds, m_ViewportData, m_ViewportFocused, m_ViewportHovered);

		uint64_t textureID = m_ViewportFramebuffer->GetColorAttachmentRendererID();
		ImGui::Image((ImTextureID)textureID, ImVec2{ (float)m_ViewportData.m_Width, (float)m_ViewportData.m_Height }, ImVec2{ 0, 1 },
			ImVec2{ 1, 0 });
		if (ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::GetIO().WantCaptureMouse)
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
					path.AddNode(m_HoveredWidgetID);
					success = s_UIWindow->m_TreePanel->m_UITree.SelectEntry(path);
				}

				if (!success)
				{
					KG_WARN("Failed to select window/widget with ID {} and {}", m_HoveredWindowID, m_HoveredWidgetID);
				}
			}
		}

		// End the window
		EditorUI::EditorUIService::EndWindow();
	}

	void UIEditorViewportPanel::OnInputEvent(Events::Event* event)
	{
		FixedString32 focusedWindow{ EditorUI::EditorUIService::GetFocusedWindowName() };
		if (m_ViewportFocused)
		{
			m_EditorCamera.OnInputEvent(event);
		}
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
			DrawWindowWidgetDebugLines();
			Rendering::RenderingService::EndScene();
		}
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
	void UIEditorViewportPanel::DrawWindowWidgetDebugLines()
	{
		// Retrieve the active widget and window
		RuntimeUI::Widget* widget{ s_UIWindow->m_PropertiesPanel->m_ActiveWidget };
		RuntimeUI::Window* window{ s_UIWindow->m_PropertiesPanel->m_ActiveWindow };

		// Get position data for rendering window
		Math::vec3 windowScale = window->CalculateSize(m_ViewportData.m_Width, m_ViewportData.m_Height);
		Math::vec3 initialWindowTranslation = window->CalculatePosition(m_ViewportData.m_Width, m_ViewportData.m_Height);
		Math::vec3 finalWindowTranslation = Math::vec3(initialWindowTranslation.x + (windowScale.x / 2), initialWindowTranslation.y + (windowScale.y / 2), initialWindowTranslation.z);

		// Handle either active widget specific debug lines or active window debug lines
		if (widget)
		{
			// Calculate the widget's rendering data
			Math::vec3 widgetSize = widget->CalculateSize(windowScale);
			Math::vec3 widgetTranslation = widget->CalculatePosition(initialWindowTranslation, windowScale);
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
		Math::vec3 initialWindowTranslation = window->CalculatePosition(m_ViewportData.m_Width, m_ViewportData.m_Height);
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
			if (widget->m_XPositionType == RuntimeUI::PixelOrPercent::Percent)
			{
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
			}
			

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
			if (widget->m_YPositionType == RuntimeUI::PixelOrPercent::Percent)
			{
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
			}


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
		Math::vec3 initialWindowTranslation = window->CalculatePosition(m_ViewportData.m_Width, m_ViewportData.m_Height);
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

}

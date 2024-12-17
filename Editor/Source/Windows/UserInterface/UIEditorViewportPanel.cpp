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

		m_EditorCamera = Rendering::EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);

		m_EditorCamera.SetFocalPoint({ 58.0f, 27.0f, 93.0f });
		m_EditorCamera.SetDistance(1.0f);
		m_EditorCamera.SetPitch(0.195f);
		m_EditorCamera.SetYaw(-0.372f);
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
		Rendering::RenderingService::ResetStats();
		m_ViewportFramebuffer->Bind();
		Rendering::RendererAPI::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Rendering::RendererAPI::Clear();

		// Clear mouse picking attachment value
		m_ViewportFramebuffer->SetAttachment(1, -1);

		// TODO: Add background image to viewport

		// Handle drawing user interface
		Window& currentApplication = EngineService::GetActiveWindow();
		Math::mat4 cameraViewMatrix = glm::inverse(m_EditorCamera.GetViewMatrix());
		RuntimeUI::RuntimeUIService::PushRenderData(cameraViewMatrix, m_ViewportData.m_Width, m_ViewportData.m_Height);

		ProcessMousePicking();

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
			Math::vec4 redColor
			{
				EditorUI::EditorUIService::s_Red.x,
				EditorUI::EditorUIService::s_Red.y,
				EditorUI::EditorUIService::s_Red.z,
				EditorUI::EditorUIService::s_Red.w
			};
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(redColor, "a_Color", localBuffer, localShader);

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
			Rendering::RendererAPI::SetLineWidth(2.0f);

			// Reset the rendering context
			Rendering::RendererAPI::ClearDepthBuffer();

			// Calculate orthographic projection matrix for user interface
			Math::mat4 orthographicProjection = glm::ortho(0.0f, (float)m_ViewportData.m_Width,
				0.0f, (float)m_ViewportData.m_Height, -1.0f, 1.0f);
			Math::mat4 outputMatrix = orthographicProjection;

			// Start rendering context
			Rendering::RenderingService::BeginScene(outputMatrix);

			RuntimeUI::Widget* widget{ s_UIWindow->m_PropertiesPanel->m_ActiveWidget };
			RuntimeUI::Window* window{ s_UIWindow->m_PropertiesPanel->m_ActiveWindow };

			//cameraViewMatrix, m_ViewportData.m_Width, m_ViewportData.m_Height

			// Get position data for rendering window
			Math::vec3 windowScale = window->CalculateSize(m_ViewportData.m_Width, m_ViewportData.m_Height);
			Math::vec3 initialWindowTranslation = window->CalculatePosition(m_ViewportData.m_Width, m_ViewportData.m_Height);
			Math::vec3 finalWindowTranslation = Math::vec3(initialWindowTranslation.x + (windowScale.x / 2), initialWindowTranslation.y + (windowScale.y / 2), initialWindowTranslation.z);

			// Draw outline for either the active widget or the active window
			if (s_UIWindow->m_PropertiesPanel->m_ActiveWidget)
			{
				// Calculate the widget's rendering data
				Math::vec3 widgetSize = widget->CalculateSize(windowScale);
				Math::vec3 widgetTranslation = widget->CalculatePosition(initialWindowTranslation, windowScale);

				// Create the widget's background rendering data
				Math::mat4 transform = glm::translate(Math::mat4(1.0f), Math::vec3(widgetTranslation.x + (widgetSize.x / 2), widgetTranslation.y + (widgetSize.y / 2), widgetTranslation.z))
					* glm::scale(Math::mat4(1.0f), widgetSize);

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
			else
			{
				// Create background rendering data
				Math::mat4 transform = glm::translate(Math::mat4(1.0f), finalWindowTranslation)
				* glm::scale(Math::mat4(1.0f), windowScale);

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
			
			Rendering::RendererAPI::SetLineWidth(1.0f);
			Rendering::RenderingService::EndScene();
		}
	}
	void UIEditorViewportPanel::ProcessMousePicking()
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
}

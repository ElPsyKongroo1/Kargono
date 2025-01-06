

#include "Windows/EmitterConfig/EmitterConfigViewportPanel.h"

#include "Kargono/Rendering/RenderingService.h"
#include "Kargono/Input/InputService.h"
#include "Kargono/Rendering/Texture.h"
#include "Kargono/Utility/Operations.h"

#include "EditorApp.h"

static Kargono::EditorApp* s_EditorApp{ nullptr };
static Kargono::Windows::EmitterConfigWindow* s_EmitterConfigWindow{ nullptr };

namespace Kargono::Panels
{
	EmitterConfigViewportPanel::EmitterConfigViewportPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EmitterConfigWindow = s_EditorApp->m_EmitterConfigEditorWindow.get();

		InitializeFrameBuffer();
		InitializeOverlayData();

		// TODO: Initialize editor camera

		KG_ASSERT(Projects::ProjectService::GetActive());
		m_ViewportAspectRatio = Utility::ScreenResolutionToAspectRatio(Projects::ProjectService::GetActiveTargetResolution());

	}

	EmitterConfigViewportPanel::~EmitterConfigViewportPanel()
	{
		ClearOverlayData();
	}

	void EmitterConfigViewportPanel::InitializeFrameBuffer()
	{
		Rendering::FramebufferSpecification fbSpec;
		fbSpec.Attachments = { Rendering::FramebufferDataFormat::RGBA8, Rendering::FramebufferDataFormat::RED_INTEGER, Rendering::FramebufferDataFormat::Depth };
		fbSpec.Width = EngineService::GetActiveWindow().GetWidth();
		fbSpec.Height = EngineService::GetActiveWindow().GetHeight();
		m_ViewportFramebuffer = Rendering::Framebuffer::Create(fbSpec);
	}

	void EmitterConfigViewportPanel::OnUpdate(Timestep ts)
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
			//m_EditorCamera.SetCameraFixedSize(Math::vec2((float)m_ViewportData.m_Width, (float)m_ViewportData.m_Height));
			//m_EditorCamera.OnViewportResize();
		}

		// Prepare for rendering
		Rendering::RenderingService::ResetStats();
		m_ViewportFramebuffer->Bind();
		Rendering::RendererAPI::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Rendering::RendererAPI::Clear();

		// Clear mouse picking attachment value
		m_ViewportFramebuffer->SetAttachment(1, -1);

		// TODO: Add background image to viewport


		// Handle drawing Emitter Config
		Window& currentApplication = EngineService::GetActiveWindow();
		// TODO: Add back camera matrix
		RuntimeUI::RuntimeUIService::OnRender(m_ViewportData.m_Width, m_ViewportData.m_Height);

		HandleMouseHovering();

		// Complete rendering
		m_ViewportFramebuffer->Unbind();

	}
	void EmitterConfigViewportPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EmitterConfigWindow->m_ShowViewport);
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
		
		// TODO: Handle on mouse hover to select entity

		DrawGizmo();

		DrawToolbarOverlay();

		// End the window
		EditorUI::EditorUIService::EndWindow();
	}

	void EmitterConfigViewportPanel::OnInputEvent(Events::Event* event)
	{
		m_EditorCamera.OnInputEvent(event);
	}

	bool EmitterConfigViewportPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		// Check if alt key is pressed
		bool alt = Input::InputService::IsKeyPressed(Key::LeftAlt) || Input::InputService::IsKeyPressed(Key::RightAlt);

		// Handle various key presses for the Emitter Config editor panel
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
	static Ref<std::vector<Math::vec3>> s_OutputVector {CreateRef<std::vector<Math::vec3>>()};


	void EmitterConfigViewportPanel::InitializeOverlayData()
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

	void EmitterConfigViewportPanel::ClearOverlayData()
	{
		s_LineInputSpec.ClearData();
	}

	void EmitterConfigViewportPanel::HandleMouseHovering()
	{
	}
	void EmitterConfigViewportPanel::DrawToolbarOverlay()
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

			if (ImGui::BeginPopup("Camera Options"))
			{
				if (ImGui::MenuItem("Reset Camera Position"))
				{
					ResetCamera();
				}
				ImGui::EndPopup();
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
	void EmitterConfigViewportPanel::DrawGizmo()
	{
		// TODO: Draw perspective gizmo
	}
	void EmitterConfigViewportPanel::OnOpenEmitterConfig()
	{
		ResetCamera();
	}

	void EmitterConfigViewportPanel::ResetCamera()
	{
		// TODO: Reset the perspective camera
	}

}



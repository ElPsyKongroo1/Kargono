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

		m_EditorCamera = Rendering::EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);

		m_EditorCamera.SetFocalPoint({ 58.0f, 27.0f, 93.0f });
		m_EditorCamera.SetDistance(1.0f);
		m_EditorCamera.SetPitch(0.195f);
		m_EditorCamera.SetYaw(-0.372f);
	}
	void UIEditorViewportPanel::InitializeFrameBuffer()
	{
		Rendering::FramebufferSpecification fbSpec;
		fbSpec.Attachments = { Rendering::FramebufferDataFormat::RGBA8, Rendering::FramebufferDataFormat::Depth };
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

		// TODO: Add background image to viewport

		// Handle drawing user interface
		Window& currentApplication = EngineService::GetActiveWindow();
		Math::mat4 cameraViewMatrix = glm::inverse(m_EditorCamera.GetViewMatrix());
		RuntimeUI::RuntimeUIService::PushRenderData(cameraViewMatrix, m_ViewportData.m_Width, m_ViewportData.m_Height);

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

		// End the window
		EditorUI::EditorUIService::EndWindow();
	}
}

#include "Sandbox2D.h"

#include <iostream>
#include <chrono>
#include <glm/gtc/type_ptr.hpp>

#include "dependencies/imgui/imgui.h"
#include "Platform/OpenGL/OpenGLShader.h"

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true), m_SquareColor({0.2f, 0.3f, 0.8f, 1.0f})
{
	
}


void Sandbox2D::OnAttach()
{
	KG_PROFILE_FUNCTION();

	m_CheckerboardTexture = Kargono::Texture2D::Create("assets/textures/Checkerboard.png");

	Kargono::FramebufferSpecification fbspec;
	fbspec.Width = 1280;
	fbspec.Height = 720;
	m_Framebuffer = Kargono::Framebuffer::Create(fbspec);

}

void Sandbox2D::OnDetach()
{
	KG_PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(Kargono::Timestep ts)
{
	KG_PROFILE_FUNCTION();

	// Update

	m_CameraController.OnUpdate(ts);


	//Render
	Kargono::Renderer2D::ResetStats();

	{
		KG_PROFILE_SCOPE("Renderer Prep::OnUpdate");
		m_Framebuffer->Bind();
		Kargono::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Kargono::RenderCommand::Clear();
	}

	{
		static float rotation = 0.0f;
		rotation += ts * 50.0f;

		KG_PROFILE_SCOPE("Renderer Draw::OnUpdate");
		Kargono::Renderer2D::BeginScene(m_CameraController.GetCamera());
		Kargono::Renderer2D::DrawRotatedQuad({ 1.0f, 0.0f }, { 0.8f, 0.8f },-45.0f, { 0.8f, 0.2f, 0.3f, 1.0f });
		Kargono::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, { 0.8f, 0.2f, 0.3f, 1.0f });
		Kargono::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, m_SquareColor);
		Kargono::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 20.0f, 20.0f }, m_CheckerboardTexture, 10.0f);
		Kargono::Renderer2D::DrawRotatedQuad({ -2.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, rotation, m_CheckerboardTexture, 20.0f);
		Kargono::Renderer2D::EndScene();

		Kargono::Renderer2D::BeginScene(m_CameraController.GetCamera());
		for (float y = -5.0f; y < 5.0f; y += 0.2f)
		{
			for (float x = -5.0f; x < 5.0f; x += 0.2f)
			{
				glm::vec4 color = { x + 5.0f / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.5f};
				Kargono::Renderer2D::DrawQuad({ x, y }, { 0.45f, 0.45f }, color);
			}
		}
		Kargono::Renderer2D::EndScene();
		m_Framebuffer->Unbind();
	}
}

void Sandbox2D::OnImGuiRender()
{
	KG_PROFILE_FUNCTION();
    static bool dockingEnabled = true;
	if (dockingEnabled)
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
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows,
				// which we can't undo at the moment without finer window depth/z control.

				if (ImGui::MenuItem("Exit")) { Kargono::Application::Get().Close(); }
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::Begin("Settings");

		auto stats = Kargono::Renderer2D::GetStats();
		ImGui::Text("Renderer2D Stats:");
		ImGui::Text("Draw Calls: %d:", stats.DrawCalls);
		ImGui::Text("Quads: %d:", stats.QuadCount);
		ImGui::Text("Vertices: %d:", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d:", stats.GetTotalIndexCount());

		ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		ImGui::Image((void*)textureID, ImVec2{ 1280.0f, 720.f});
		ImGui::End();

		ImGui::End();
	}
}

void Sandbox2D::OnEvent(Kargono::Event& event)
{
	m_CameraController.OnEvent(event);
}

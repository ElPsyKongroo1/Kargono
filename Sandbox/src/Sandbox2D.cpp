#include "Sandbox2D.h"

#include <iostream>
#include <chrono>
#include <glm/gtc/type_ptr.hpp>

#include "dependencies/imgui/imgui.h"
#include "Platform/OpenGL/OpenGLShader.h"

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true)
{
	
}


void Sandbox2D::OnAttach()
{
	m_CheckerboardTexture = Kargono::Texture2D::Create("assets/textures/Checkerboard.png");
}

void Sandbox2D::OnDetach()
{
}

void Sandbox2D::OnUpdate(Kargono::Timestep ts)
{
	KG_PROFILE_FUNCTION();

	// Update
	{
		KG_PROFILE_SCOPE("CameraController::OnUpdate");
		m_CameraController.OnUpdate(ts);
	}

	//Render
	{
		KG_PROFILE_SCOPE("Renderer Prep::OnUpdate");
		Kargono::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Kargono::RenderCommand::Clear();
	}

	{
		KG_PROFILE_SCOPE("Renderer Draw::OnUpdate");
		Kargono::Renderer2D::BeginScene(m_CameraController.GetCamera());
		Kargono::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, { 0.8f, 0.2f, 0.3f, 1.0f });
		Kargono::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, { 0.2f, 0.3f, 0.8f, 1.0f });
		Kargono::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_CheckerboardTexture);
		Kargono::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnImGuiRender()
{
	KG_PROFILE_FUNCTION();

	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(Kargono::Event& event)
{
	m_CameraController.OnEvent(event);
}

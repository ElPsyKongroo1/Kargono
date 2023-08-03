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
		for (float y = -5.0f; y < 5.0f; y += 0.5f)
		{
			for (float x = -5.0f; x < 5.0f; x += 0.5f)
			{
				glm::vec4 color = { x + 5.0f / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.5f};
				Kargono::Renderer2D::DrawQuad({ x, y }, { 0.45f, 0.45f }, color);
			}
		}
		Kargono::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnImGuiRender()
{
	KG_PROFILE_FUNCTION();

	ImGui::Begin("Settings");

	auto stats = Kargono::Renderer2D::GetStats();
	ImGui::Text("Renderer2D Stats:");
	ImGui::Text("Draw Calls: %d:", stats.DrawCalls);
	ImGui::Text("Quads: %d:", stats.QuadCount);
	ImGui::Text("Vertices: %d:", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d:", stats.GetTotalIndexCount());

	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(Kargono::Event& event)
{
	m_CameraController.OnEvent(event);
}

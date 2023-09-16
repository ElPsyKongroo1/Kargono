#include "kgpch.h"

#include "Kargono/Renderer/Renderer.h"
#include "Kargono/Renderer/Renderer2D.h"
#include "Kargono/Renderer/RenderCommand.h"


namespace Kargono
{
	void Renderer::Init()
	{
		RenderCommand::Init();
		Renderer2D::Init();
	}
	void Renderer::Shutdown()
	{
		Renderer2D::Shutdown();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	Scope<Renderer::SceneData> Renderer::s_SceneData = CreateScope<Renderer::SceneData>();
	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}
	void Kargono::Renderer::EndScene()
	{
	}
	void Renderer::Submit( const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		shader->Bind();
		shader->SetMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
		shader->SetMat4("u_Transform", transform);
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
}

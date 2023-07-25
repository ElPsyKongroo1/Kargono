#include "Kargono/kgpch.h"
#include "Kargono/Renderer/Renderer.h"
#include "Renderer.h"

#include "RenderCommand.h"

namespace Kargono
{
	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;
	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}
	void Kargono::Renderer::EndScene()
	{
	}
	void Renderer::Submit( const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray )
	{
		shader->Bind();
		shader->UploadUniformMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
}

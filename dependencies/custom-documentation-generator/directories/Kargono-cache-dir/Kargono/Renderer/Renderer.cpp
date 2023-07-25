#include "Kargono/kgpch.h"
#include "Kargono/Renderer/Renderer.h"
#include "Renderer.h"

#include "RenderCommand.h"

namespace Kargono
{
	void Renderer::BeginScene()
	{
	}
	void Kargono::Renderer::EndScene()
	{
	}
	void Renderer::Submit(const std::shared_ptr<VertexArray>& vertexArray)
	{
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
}

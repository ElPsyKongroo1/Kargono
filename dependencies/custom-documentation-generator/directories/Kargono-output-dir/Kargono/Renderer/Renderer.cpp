#include "Kargono/kgpch.h"
#include "Kargono/Renderer/Renderer.h"
#include "Renderer.h"

#include "RenderCommand.h"

namespace Kargono
{
/// @brief Begins a new rendering scene
	void Renderer::BeginScene()
	{
	}
/// @brief Ends the current rendering scene
	void Kargono::Renderer::EndScene()
	{
	}
/// @brief Submits a vertex array for rendering
/// @param vertexArray The vertex array to be rendered
	void Renderer::Submit(const std::shared_ptr<VertexArray>& vertexArray)
	{
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
}

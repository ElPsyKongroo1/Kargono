#pragma once
#include "Kargono/Renderer/RendererAPI.h"
#include "Kargono/Renderer/VertexArray.h"
namespace Kargono {

	class OpenGLRendererAPI : public Kargono::RendererAPI
	{
	public:
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) override;
	};
}

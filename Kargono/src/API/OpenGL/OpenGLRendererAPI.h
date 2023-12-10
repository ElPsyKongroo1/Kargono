#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Renderer/RendererAPI.h"
#include "Kargono/Renderer/VertexArray.h"

namespace Kargono {

	class OpenGLRendererAPI : public Kargono::RendererAPI
	{
	public:
		virtual void Init() override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		virtual void ClearDepthBuffer() override;
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void SetDepthTesting(bool value) override;
		virtual void SetStencilMask(uint32_t value) override;
		virtual void StencilTestFunc(RendererAPI::StencilComparisonType comparisonType, int32_t reference, uint32_t mask) override;
		virtual void StencilTestOptions(RendererAPI::StencilOptions sfail, RendererAPI::StencilOptions dfail, RendererAPI::StencilOptions sdpass) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;
		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t* indexPointer, uint32_t indexCount) override;
		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;
		virtual void DrawTriangles(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;

		virtual void SetLineWidth(float width) override;
	};
}

#pragma once

#include "Kargono/Rendering/VertexArray.h"
#include "Kargono/Math/Math.h"

namespace Kargono::Rendering
{
	enum class RenderAPI
	{
		None = 0, OpenGL = 1
	};

	enum class StencilComparisonType
	{
		None = 0, NEVER, LESS, LEQUAL, GREATER, GEQUAL,
		EQUAL, NOTEQUAL, ALWAYS
	};

	enum class StencilOptions
	{
		None = 0, KEEP, ZERO, REPLACE, INCR, INCR_WRAP, DECR, DECR_WRAP, INVERT
	};

	class RendererAPI
	{
	public:
		virtual ~RendererAPI() = default;
		virtual void Init() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void ClearDepthBuffer() = 0;
		virtual void SetClearColor(const Math::vec4& color) = 0;
		virtual void SetDepthTesting(bool value) = 0;
		virtual void SetStencilMask(uint32_t value) = 0;
		virtual void StencilTestFunc(StencilComparisonType comparisonType, int32_t reference, uint32_t mask) = 0;
		virtual void StencilTestOptions(StencilOptions sfail, StencilOptions dfail, StencilOptions sdpass) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t* indexPointer, uint32_t indexCount) = 0;
		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;
		virtual void DrawTriangles(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;

		virtual void SetLineWidth(float width) = 0;

		static RenderAPI GetAPI() { return s_API; }
		static Scope<RendererAPI> Create();
	private:
		static RenderAPI s_API;
	};
}

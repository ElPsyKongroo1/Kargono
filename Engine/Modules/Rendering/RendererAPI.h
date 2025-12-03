#pragma once

#include "Modules/Rendering/VertexArray.h"
#include "Kargono/Math/Math.h"

namespace Kargono::Rendering
{
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
		//==============================
		// Lifecycle Functions
		//==============================
		static void Init();
	public:
		//==============================
		// Configuration Functions
		//==============================
		static void SetLineWidth(float width);
		static void SetPointWidth(float size);
		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
		static void ClearDepthBuffer();
		static void SetClearColor(const Math::vec4& color);
		static void SetDepthTesting(bool value);
		static void SetStencilMask(uint32_t value);
		static void StencilTestFunc(StencilComparisonType comparisonType, int32_t reference, uint32_t mask);
		static void StencilTestOptions(StencilOptions sfail, StencilOptions dfail, StencilOptions sdpass);
		static void Clear();
	public:
		//==============================
		// Rendering Functions
		//==============================
		static void DrawIndexed(VertexArray& vertexArray, uint32_t indexCount = 0);
		static void DrawIndexed(VertexArray& vertexArray, uint32_t* indexPointer, uint32_t indexCount);
		static void DrawLines(VertexArray& vertexArray, uint32_t vertexCount);
		static void DrawPoints(VertexArray& vertexArray, uint32_t vertexCount);
		static void DrawTriangles(VertexArray& vertexArray, uint32_t vertexCount);

	};
}

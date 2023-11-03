#pragma once

#include "Kargono/Renderer/RendererAPI.h"

namespace Kargono
{
	class RenderCommand
	{

	public:
		static void Init()
		{
			s_RendererAPI->Init();
		}

		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}

		static void EnableDepthTesting(bool value)
		{
			s_RendererAPI->EnableDepthTesting(value);
		}

		static void SetStencilMask(uint32_t value)
		{
			s_RendererAPI->SetStencilMask(value);
		}

		static void StencilTestFunc(RendererAPI::StencilComparisonType comparisonType, int32_t reference, uint32_t mask)
		{
			s_RendererAPI->StencilTestFunc(comparisonType, reference, mask);
		}

		static void StencilTestOptions(RendererAPI::StencilOptions sfail, RendererAPI::StencilOptions dfail, RendererAPI::StencilOptions sdpass)
		{
			s_RendererAPI->StencilTestOptions(sfail, dfail, sdpass);
		}

		static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		};
		static void Clear() { s_RendererAPI->Clear(); }

		static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0)
		{
			s_RendererAPI->DrawIndexed(vertexArray, indexCount);
		}

		static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t* indexPointer, uint32_t indexCount)
		{
			s_RendererAPI->DrawIndexed(vertexArray, indexPointer, indexCount);
		}

		static void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
		{
			s_RendererAPI->DrawLines(vertexArray, vertexCount);
		}
		static void DrawTriangles(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
		{
			s_RendererAPI->DrawTriangles(vertexArray, vertexCount);
		}
		static void SetLineWidth(float width)
		{
			s_RendererAPI->SetLineWidth(width);
		}

	private:
		static Scope<RendererAPI> s_RendererAPI;
	};
}

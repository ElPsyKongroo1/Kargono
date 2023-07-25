#include "Kargono/kgpch.h"
#include "Kargono/Renderer/Buffer.h"
#include "Renderer.h"
#include "Kargono/Core.h"
#include "Kargono/Log.h"
#include "Platform/OpenGL/OpenGLBuffer.h"


namespace Kargono {

/// @brief Creates a new vertex buffer
///
/// @param vertices The array of vertex data
/// @param size The size of the vertex data
	VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size)
	{
/// @brief Switch statement to check the current RendererAPI
		switch (Renderer::GetAPI())
		{
/// @brief Case for RendererAPI::None
///
/// @return nullptr
/// @brief Case for RendererAPI::None
///
/// @return nullptr
		case RendererAPI::API::None:
			KG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
			break;
		
/// @brief Case for RendererAPI::OpenGL
///
/// @param vertices The array of vertex data
/// @param size The size of the vertex data
/// @return new OpenGLVertexBuffer
/// @brief Case for RendererAPI::OpenGL
///
/// @param indices The array of index data
/// @param size The size of the index data
/// @return new OpenGLIndexBuffer
		case RendererAPI::API::OpenGL:
			return new OpenGLVertexBuffer(vertices, size);
			break;
		}
/// @brief Assertion for unknown RendererAPI
///
/// @return nullptr
/// @brief Assertion for unknown RendererAPI
///
/// @return nullptr
		KG_CORE_ASSERT(false, "RendererAPI:: Unknown RendererAPI!");
		return nullptr;
	}

/// @brief Creates a new index buffer
///
/// @param indices The array of index data
/// @param size The size of the index data
	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			KG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
			break;

		case RendererAPI::API::OpenGL:
			return new OpenGLIndexBuffer(indices, size);
			break;
		}
		KG_CORE_ASSERT(false, "RendererAPI:: Unknown RendererAPI!");
		return nullptr;
	}

}

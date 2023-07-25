#include "Kargono/kgpch.h"
#include "Kargono/Renderer/VertexArray.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Kargono {

/// @brief Factory method for creating a new VertexArray object
/// @return A pointer to the newly created VertexArray object
	VertexArray* VertexArray::Create()
	{
/// @brief Switch statement to determine which RendererAPI to use
		switch (Renderer::GetAPI())
		{
/// @brief Case for RendererAPI::None
/// @details This case is currently not supported
/// @param message The error message
		case RendererAPI::API::None:
			KG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
			break;

/// @brief Case for RendererAPI::OpenGL
/// @details This case creates and returns a new OpenGLVertexArray object
/// @return A pointer to the newly created OpenGLVertexArray object
		case RendererAPI::API::OpenGL:
			return new OpenGLVertexArray();
			break;
		}
/// @brief Assertion for unknown RendererAPI
/// @details This assertion occurs when the RendererAPI is unknown
		KG_CORE_ASSERT(false, "RendererAPI:: Unknown RendererAPI!");
		return nullptr;
	}


}

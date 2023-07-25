/// @brief Include the precompiled header file for Kargono engine
#include "Kargono/kgpch.h"
/// @brief Include the RenderCommand header file
#include "RenderCommand.h"
/// @brief Include the OpenGLRendererAPI header file
#include "Platform/OpenGL/OpenGLRendererAPI.h"

/// @brief Define the namespace for the Kargono engine
namespace Kargono
{
/// @brief Initialize the static member variable s_RendererAPI with a new instance of the OpenGLRendererAPI
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;
}
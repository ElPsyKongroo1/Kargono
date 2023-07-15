/// @source Includes the precompiled header file for the Kargono engine.
#include "Kargono/kgpch.h"
/// @source Includes the Renderer header file for the Kargono engine.
#include "Kargono/Renderer/Renderer.h"

namespace Kargono
{
/// @var Renderer::s_RendererAPI
/// @brief Defines the static variable s_RendererAPI of type RendererAPI. It is set to OpenGL.
/// @details Used to determine the current rendering API used in the Renderer class.
	RendererAPI Renderer::s_RendererAPI = RendererAPI::OpenGL;

}
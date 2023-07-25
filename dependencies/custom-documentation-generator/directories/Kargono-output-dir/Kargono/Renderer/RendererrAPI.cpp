#include "Kargono/kgpch.h"
#include "Kargono/Renderer/RendererAPI.h"

namespace Kargono
{
/// @brief The currently active graphics API in the renderer
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;
}
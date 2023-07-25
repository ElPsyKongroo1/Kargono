#include "Kargono/kgpch.h"
#include "RenderCommand.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Kargono
{
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;
}
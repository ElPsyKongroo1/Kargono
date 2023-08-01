#include "Kargono/kgpch.h"
#include "Kargono/Renderer/RenderCommand.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Kargono
{
	Scope<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();
}
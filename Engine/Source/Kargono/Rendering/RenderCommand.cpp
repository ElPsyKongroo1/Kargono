#include "kgpch.h"

#include "Kargono/Rendering/RenderCommand.h"

#include "API/RenderingAPI/OpenGLRendererAPI.h"

namespace Kargono::Rendering
{
	Scope<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();
}

#include "kgpch.h"

#include "Kargono/Renderer/RendererAPI.h"
#include "API/OpenGL/OpenGLRendererAPI.h"

namespace Kargono {

	RenderAPI RendererAPI::s_API = RenderAPI::OpenGL;

	Scope<RendererAPI> RendererAPI::Create()
	{
		return CreateScope<::API::OpenGL::OpenGLRendererAPI>();
	}

}

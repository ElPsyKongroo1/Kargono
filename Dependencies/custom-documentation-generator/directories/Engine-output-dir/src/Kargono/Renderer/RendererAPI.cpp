#include "kgpch.h"

#include "Kargono/Renderer/RendererAPI.h"
#include "API/OpenGL/OpenGLRendererAPI.h"

namespace Kargono {

	RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

	Scope<RendererAPI> RendererAPI::Create()
	{
		return CreateScope<::API::OpenGL::OpenGLRendererAPI>();
	}

}

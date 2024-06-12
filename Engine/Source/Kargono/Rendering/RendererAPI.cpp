#include "kgpch.h"

#include "Kargono/Rendering/RendererAPI.h"

#include "API/RenderingAPI/OpenGLRendererAPI.h"

namespace Kargono::Rendering
{

	RenderAPI RendererAPI::s_API = RenderAPI::OpenGL;

	Scope<RendererAPI> RendererAPI::Create()
	{
		return CreateScope<::API::RenderingAPI::OpenGLRendererAPI>();
	}

}

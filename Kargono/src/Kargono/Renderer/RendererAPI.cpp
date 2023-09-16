#include "kgpch.h"

#include "Kargono/Renderer/RendererAPI.h"
#include "API/OpenGL/OpenGLRendererAPI.h"

namespace Kargono {

	RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

	Scope<RendererAPI> RendererAPI::Create()
	{
		switch (s_API)
		{
		case RendererAPI::API::None:    KG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateScope<OpenGLRendererAPI>();
		}

		KG_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}

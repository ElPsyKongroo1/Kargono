#include "kgpch.h"

#include "Kargono/Renderer/GraphicsContext.h"
#include "Kargono/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLContext.h"

namespace Kargono {
	Scope<Kargono::GraphicsContext> GraphicsContext::Create(void* window)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    KG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return Kargono::CreateScope<OpenGLContext>(static_cast<GLFWwindow*>(window));
		}

		KG_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}

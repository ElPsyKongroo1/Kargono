#include "kgpch.h"

#include "Kargono/Renderer/Framebuffer.h"
#include "Kargono/Renderer/RendererAPI.h"
#include "Kargono/Renderer/Renderer.h"

#include "API/RenderingAPI/OpenGLFramebuffer.h"

namespace Kargono
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		return CreateRef<API::OpenGL::OpenGLFramebuffer>(spec);
	}
}

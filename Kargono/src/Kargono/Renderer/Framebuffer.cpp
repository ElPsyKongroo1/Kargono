#include "kgpch.h"

#include "Kargono/Renderer/Framebuffer.h"
#include "Kargono/Renderer/RendererAPI.h"
#include "Kargono/Renderer/Renderer.h"
#include "API/OpenGL/OpenGLFramebuffer.h"

namespace Kargono
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		return CreateRef<OpenGLFramebuffer>(spec);
	}
}

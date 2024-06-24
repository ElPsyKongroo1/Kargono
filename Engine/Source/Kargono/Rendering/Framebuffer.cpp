#include "kgpch.h"

#include "Kargono/Rendering/Framebuffer.h"
#include "Kargono/Rendering/RendererAPI.h"
#include "Kargono/Rendering/RenderingService.h"

#include "API/RenderingAPI/OpenGLFramebuffer.h"

namespace Kargono::Rendering
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLFramebuffer>(spec);
#endif
	}
}

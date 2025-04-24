#include "kgpch.h"

#include "RenderingPlugin/Framebuffer.h"
#include "RenderingPlugin/RendererAPI.h"
#include "RenderingPlugin/RenderingService.h"

#include "RenderingPlugin/ExternalAPI/OpenGLFramebuffer.h"

namespace Kargono::Rendering
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLFramebuffer>(spec);
#endif
	}
}

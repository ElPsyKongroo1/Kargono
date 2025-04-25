#include "kgpch.h"

#include "RenderModule/Framebuffer.h"
#include "RenderModule/RendererAPI.h"
#include "RenderModule/RenderingService.h"

#include "RenderModule/ExternalAPI/OpenGLFramebuffer.h"

namespace Kargono::Rendering
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLFramebuffer>(spec);
#endif
	}
}

#include "kgpch.h"

#include "Modules/Rendering/Framebuffer.h"
#include "Modules/Rendering/RendererAPI.h"
#include "Modules/Rendering/RenderingService.h"

#include "Modules/Rendering/ExternalAPI/OpenGLFramebuffer.h"

namespace Kargono::Rendering
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLFramebuffer>(spec);
#endif
	}
}

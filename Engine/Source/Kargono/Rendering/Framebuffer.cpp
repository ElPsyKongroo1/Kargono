#include "kgpch.h"

#include "Kargono/Rendering/Framebuffer.h"
#include "Kargono/Rendering/RendererAPI.h"
#include "Kargono/Rendering/RenderingEngine.h"

#include "API/RenderingAPI/OpenGLFramebuffer.h"

namespace Kargono::Rendering
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		return CreateRef<API::RenderingAPI::OpenGLFramebuffer>(spec);
	}
}
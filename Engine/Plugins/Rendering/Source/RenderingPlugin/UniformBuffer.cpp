#include "kgpch.h"

#include "RenderingPlugin/RenderingService.h"
#include "RenderingPlugin/UniformBuffer.h"

#include "RenderingPlugin/ExternalAPI/OpenGLUniformBuffer.h"

namespace Kargono::Rendering
{

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLUniformBuffer>(size, binding);
#endif
	}
}

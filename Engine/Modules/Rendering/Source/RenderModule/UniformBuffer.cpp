#include "kgpch.h"

#include "RenderModule/RenderingService.h"
#include "RenderModule/UniformBuffer.h"

#include "RenderModule/ExternalAPI/OpenGLUniformBuffer.h"

namespace Kargono::Rendering
{

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLUniformBuffer>(size, binding);
#endif
	}
}

#include "kgpch.h"

#include "Kargono/Rendering/RenderingService.h"
#include "Kargono/Rendering/UniformBuffer.h"

#include "API/RenderingAPI/OpenGLUniformBuffer.h"

namespace Kargono::Rendering
{

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLUniformBuffer>(size, binding);
#endif
	}
}

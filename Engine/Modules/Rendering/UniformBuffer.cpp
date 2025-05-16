#include "kgpch.h"

#include "Modules/Rendering/RenderingService.h"
#include "Modules/Rendering/UniformBuffer.h"

#include "Modules/Rendering/ExternalAPI/OpenGLUniformBuffer.h"

namespace Kargono::Rendering
{

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLUniformBuffer>(size, binding);
#endif
	}
}

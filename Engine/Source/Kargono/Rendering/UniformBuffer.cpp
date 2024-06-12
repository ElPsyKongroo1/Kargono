#include "kgpch.h"

#include "Kargono/Rendering/RenderingEngine.h"
#include "Kargono/Rendering/UniformBuffer.h"

#include "API/RenderingAPI/OpenGLUniformBuffer.h"

namespace Kargono::Rendering
{

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		return CreateRef<API::RenderingAPI::OpenGLUniformBuffer>(size, binding);
	}
}

#include "kgpch.h"

#include "Kargono/Renderer/Renderer.h"
#include "Kargono/Renderer/UniformBuffer.h"

#include "API/RenderingAPI/OpenGLUniformBuffer.h"

namespace Kargono
{

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		return CreateRef<API::RenderingAPI::OpenGLUniformBuffer>(size, binding);
	}
}

#include "kgpch.h"

#include "Kargono/Renderer/Renderer.h"
#include "Kargono/Renderer/UniformBuffer.h"
#include "Platform/OpenGL/OpenGLUniformBuffer.h"

namespace Kargono {

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    KG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLUniformBuffer>(size, binding);
		}

		KG_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}

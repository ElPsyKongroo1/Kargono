#include "kgpch.h"

#include "Modules/Rendering/RenderingService.h"
#include "Modules/Rendering/UniformBuffer.h"

#include "API/Platform/gladAPI.h"

namespace Kargono::Rendering
{
	UniformElement::UniformElement(UniformDataType type, std::string_view name)
		 : m_Name(name), m_Type(type) {  }

	uint32_t UniformElement::GetComponentCount() const
	{
		switch (m_Type)
		{
		case UniformDataType::Float:	return 1;
		case UniformDataType::Float2:	return 2;
		case UniformDataType::Float3:	return 3;
		case UniformDataType::Float4:	return 4;
		case UniformDataType::Sampler2D:return 4;
		case UniformDataType::Int:		return 1;
		case UniformDataType::Int2:		return 2;
		case UniformDataType::Int3:		return 3;
		case UniformDataType::Int4:		return 4;
		case UniformDataType::Mat3:		return 3; // 3* float3
		case UniformDataType::Mat4:		return 4; // 4* float4
		case UniformDataType::Bool:		return 1;
		}
		KG_ERROR("Unknown UniformDataType!");
		return 0;
	}

	UniformBufferList::UniformBufferList(std::initializer_list<UniformElement> elements)
	: m_Elements(elements) {}


	UniformBuffer::~UniformBuffer()
	{
		if (m_Registered)
		{
			DeregisterBuffer();
		}
	}


	void UniformBuffer::RegisterBuffer(uint32_t size, uint32_t binding)
	{
		KG_ASSERT(!m_Registered);
		// Instantiate Buffer
		glCreateBuffers(1, &m_RendererID);
		// Create OpenGL buffer storage
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW); // TODO: investigate usage hint
		// Bind buffer to binding location
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
		m_Registered = true;
	}

	void UniformBuffer::DeregisterBuffer()
	{
		KG_ASSERT(m_Registered);
		glDeleteBuffers(1, &m_RendererID);
		m_Registered = false;
	}

	void UniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}
}

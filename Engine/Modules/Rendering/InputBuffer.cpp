#include "kgpch.h"

#include "Kargono/Core/Base.h"
#include "Modules/Rendering/InputBuffer.h"
#include "Modules/Rendering/RenderingService.h"

#include "API/Platform/gladAPI.h"


namespace Kargono::Rendering
{
	InputBufferElement::InputBufferElement(InputDataType type, std::string_view name, bool normalized)
		: m_Name(name), m_Type(type), m_Size(Utility::ShaderDataTypeSize(type)), m_Offset(0), m_Normalized(normalized)
	{
	}

	uint32_t InputBufferElement::GetComponentCount() const
	{
		switch (m_Type)
		{
		case InputDataType::Float:		return 1;
		case InputDataType::Float2:		return 2;
		case InputDataType::Float3:		return 3;
		case InputDataType::Float4:		return 4;
		case InputDataType::Int:		return 1;
		case InputDataType::Int2:		return 2;
		case InputDataType::Int3:		return 3;
		case InputDataType::Int4:		return 4;
		case InputDataType::Mat3:		return 3; // 3* float3
		case InputDataType::Mat4:		return 4; // 4* float4
		case InputDataType::Bool:		return 1;
		}
		KG_ERROR("Unknown ShaderDataType!");
		return 0;
	}

	InputBufferElement::operator bool() const
	{
		if (m_Name.empty()) 
		{ 
			return false;
		}
		return true;
	}

	InputBufferLayout::InputBufferLayout(std::initializer_list<InputBufferElement> elements)
		: m_Elements(elements)
	{
		CalculateOffsetsAndStride();
	}

	InputBufferElement* InputBufferLayout::FindElementByHash(uint32_t nameHash)
	{
		if (!m_ElementLocations.contains(nameHash))
		{
			KG_WARN("Could not locate element inside InputBufferLayout");
			return nullptr;
		}

		return &m_Elements.at(m_ElementLocations.at(nameHash));
	}

	void InputBufferLayout::AddBufferElement(const InputBufferElement& bufferElement)
	{
		m_Elements.push_back(bufferElement);
		CalculateOffsetsAndStride();
	}

	void InputBufferLayout::CalculateOffsetsAndStride()
	{
		m_Stride = 0;
		m_ElementLocations.clear();
		if (m_Elements.empty()) { return; }

		size_t offset = 0;
		uint32_t iteration = 0;
		for (InputBufferElement& element : m_Elements)
		{
			m_ElementLocations.insert({ Utility::FileSystem::CRCFromString(element.m_Name.c_str()), static_cast<std::uint32_t>(iteration) });
			element.m_Offset = offset;
			offset += element.m_Size;
			m_Stride += element.m_Size;
			iteration++;
		}
	}

	void VertexBuffer::RegisterBuffer(uint32_t size)
	{
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		m_Registered = true;
	}
	void VertexBuffer::RegisterBuffer(float* vertices, uint32_t size)
	{
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
		m_Registered = true;
	}
	void VertexBuffer::DeregisterBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
		m_Registered = false;
	}
	void VertexBuffer::Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}
	void VertexBuffer::Unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	void VertexBuffer::SetData(const void* data, uint32_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}

	void IndexBuffer::RegisterBuffer(uint32_t* indices, uint32_t count)
	{
		m_Count = count;

		glCreateBuffers(1, &m_RendererID);

		// GL_ELEMENT_ARRAY_BUFFER is not valid without an actively bound VAO
		// Binding with GL_ARRAY_BUFFER allows the data to be loaded regardless of VAO state. 
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
		m_Registered = true;
	}

	void IndexBuffer::DeregisterBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
		m_Registered = false;
	}

	void IndexBuffer::Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

	void IndexBuffer::Unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

}

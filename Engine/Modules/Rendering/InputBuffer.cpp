#include "kgpch.h"

#include "Kargono/Core/Base.h"
#include "Modules/Rendering/InputBuffer.h"
#include "Modules/Rendering/RenderingService.h"

#include "Modules/Rendering/ExternalAPI/OpenGLBuffer.h"


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

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLVertexBuffer>(size);
#endif
	}
	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLVertexBuffer>(vertices, size);
#endif
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLIndexBuffer>(indices, count);
#endif
	}

}

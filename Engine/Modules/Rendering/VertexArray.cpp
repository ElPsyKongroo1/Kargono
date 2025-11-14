#include "kgpch.h"

#include "Modules/Rendering/VertexArray.h"
#include "Modules/Rendering/RenderingService.h"
#include "Modules/Rendering/InputBuffer.h"

#include "API/Platform/gladAPI.h"


namespace Kargono::Utility
{
	static GLenum ShaderDataTypeToOpenGLBaseType(Rendering::InputDataType type)
	{
		switch (type)
		{
		case Rendering::InputDataType::Float:	return GL_FLOAT;
		case Rendering::InputDataType::Float2:	return GL_FLOAT;
		case Rendering::InputDataType::Float3:	return GL_FLOAT;
		case Rendering::InputDataType::Float4:	return GL_FLOAT;
		case Rendering::InputDataType::Mat3:		return GL_FLOAT;
		case Rendering::InputDataType::Mat4:		return GL_FLOAT;
		case Rendering::InputDataType::Int:		return GL_INT;
		case Rendering::InputDataType::Int2:		return GL_INT;
		case Rendering::InputDataType::Int3:		return GL_INT;
		case Rendering::InputDataType::Int4:		return GL_INT;
		case Rendering::InputDataType::Bool:		return GL_BOOL;
		}
		KG_ERROR("Invalid Conversion at ShaderDataTypeToOpenGLBaseType!");
		return (GLenum)-1;
	}
}

namespace Kargono::Rendering
{
	VertexArray::~VertexArray()
	{
		if (m_Registered)
		{
			DeregisterArray();
		}
	}
	void VertexArray::RegisterArray()
	{
		KG_ASSERT(!m_Registered);
		glCreateVertexArrays(1, &m_RendererID);
		m_Registered = true;
	}
	void VertexArray::DeregisterArray()
	{
		KG_ASSERT(m_Registered);
		glDeleteVertexArrays(1, &m_RendererID);
		m_Registered = false;
	}
	void VertexArray::Bind() const
	{
		glBindVertexArray(m_RendererID);
	}
	void VertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}
	void VertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		KG_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		// Get the Vertex Buffer Layout
		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();
		const InputBufferLayout& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			// Here we bind all of the individual elements in a layout to the VertexBufferArray
			//		A switch is used since different functions are needed for different data types
			switch (element.m_Type)
			{
			case InputDataType::Float:
			case InputDataType::Float2:
			case InputDataType::Float3:
			case InputDataType::Float4:
			{
				glEnableVertexAttribArray(m_VertexBufferIndex);
				glVertexAttribPointer(m_VertexBufferIndex,
					element.GetComponentCount(),
					Utility::ShaderDataTypeToOpenGLBaseType(element.m_Type),
					element.m_Normalized ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					(const void*)element.m_Offset);
				m_VertexBufferIndex++;
				break;
			}
			case InputDataType::Int:
			case InputDataType::Int2:
			case InputDataType::Int3:
			case InputDataType::Int4:
			case InputDataType::Bool:
			{
				glEnableVertexAttribArray(m_VertexBufferIndex);
				glVertexAttribIPointer(m_VertexBufferIndex,
					element.GetComponentCount(),
					Utility::ShaderDataTypeToOpenGLBaseType(element.m_Type),
					layout.GetStride(),
					(const void*)element.m_Offset);
				m_VertexBufferIndex++;
				break;
			}
			case InputDataType::Mat3:
			case InputDataType::Mat4:
			{
				uint8_t count = (uint8_t)element.GetComponentCount();
				for (uint8_t i = 0; i < count; i++)
				{
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribPointer(m_VertexBufferIndex,
						count,
						Utility::ShaderDataTypeToOpenGLBaseType(element.m_Type),
						element.m_Normalized ? GL_TRUE : GL_FALSE,
						layout.GetStride(),
						(const void*)(element.m_Offset + sizeof(float) * count * i));
					glVertexAttribDivisor(m_VertexBufferIndex, 1);
					m_VertexBufferIndex++;
				}
				break;
			}
			default:
				KG_ERROR("Unknown ShaderDataType!");
			}
		}

		// Finally add vertex buffer to instance
		m_VertexBuffers.push_back(vertexBuffer);

	}
	void VertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();
		m_IndexBuffer = indexBuffer;
	}
}

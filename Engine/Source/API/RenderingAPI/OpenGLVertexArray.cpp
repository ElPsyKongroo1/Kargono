#include "kgpch.h"

#include "API/OpenGL/OpenGLVertexArray.h"

#include "API/Windowing/gladAPI.h"

namespace API::Utility
{
	static GLenum ShaderDataTypeToOpenGLBaseType(Kargono::InputDataType type)
	{
		switch (type)
		{
		case Kargono::InputDataType::Float:	return GL_FLOAT;
		case Kargono::InputDataType::Float2:	return GL_FLOAT;
		case Kargono::InputDataType::Float3:	return GL_FLOAT;
		case Kargono::InputDataType::Float4:	return GL_FLOAT;
		case Kargono::InputDataType::Mat3:		return GL_FLOAT;
		case Kargono::InputDataType::Mat4:		return GL_FLOAT;
		case Kargono::InputDataType::Int:		return GL_INT;
		case Kargono::InputDataType::Int2:		return GL_INT;
		case Kargono::InputDataType::Int3:		return GL_INT;
		case Kargono::InputDataType::Int4:		return GL_INT;
		case Kargono::InputDataType::Bool:		return GL_BOOL;
		}
		KG_ASSERT(false, "Invalid Conversion at ShaderDataTypeToOpenGLBaseType!");
		return -1;
	}
}

namespace API::OpenGL
{
	OpenGLVertexArray::OpenGLVertexArray()
	{
		glCreateVertexArrays(1, &m_RendererID);
	}
	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	}
	void OpenGLVertexArray::Bind() const
	{
		glBindVertexArray(m_RendererID);
	}
	void OpenGLVertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}
	void OpenGLVertexArray::AddVertexBuffer(const Kargono::Ref<Kargono::VertexBuffer>& vertexBuffer)
	{
		KG_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		// Get the Vertex Buffer Layout
		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();
		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			// Here we bind all of the individual elements in a layout to the VertexBufferArray
			//		A switch is used since different functions are needed for different data types
			switch (element.Type)
			{
			case Kargono::InputDataType::Float:
			case Kargono::InputDataType::Float2:
			case Kargono::InputDataType::Float3:
			case Kargono::InputDataType::Float4:
			{
				glEnableVertexAttribArray(m_VertexBufferIndex);
				glVertexAttribPointer(m_VertexBufferIndex,
					element.GetComponentCount(),
					Utility::ShaderDataTypeToOpenGLBaseType(element.Type),
					element.Normalized ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					(const void*)element.Offset);
				m_VertexBufferIndex++;
				break;
			}
			case Kargono::InputDataType::Int:
			case Kargono::InputDataType::Int2:
			case Kargono::InputDataType::Int3:
			case Kargono::InputDataType::Int4:
			case Kargono::InputDataType::Bool:
			{
				glEnableVertexAttribArray(m_VertexBufferIndex);
				glVertexAttribIPointer(m_VertexBufferIndex,
					element.GetComponentCount(),
					Utility::ShaderDataTypeToOpenGLBaseType(element.Type),
					layout.GetStride(),
					(const void*)element.Offset);
				m_VertexBufferIndex++;
				break;
			}
			case Kargono::InputDataType::Mat3:
			case Kargono::InputDataType::Mat4:
			{
				uint8_t count = element.GetComponentCount();
				for (uint8_t i = 0; i < count; i++)
				{
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribPointer(m_VertexBufferIndex,
						count,
						Utility::ShaderDataTypeToOpenGLBaseType(element.Type),
						element.Normalized ? GL_TRUE : GL_FALSE,
						layout.GetStride(),
						(const void*)(element.Offset + sizeof(float) * count * i));
					glVertexAttribDivisor(m_VertexBufferIndex, 1);
					m_VertexBufferIndex++;
				}
				break;
			}
			default:
				KG_ASSERT(false, "Unknown ShaderDataType!");
			}
		}

		// Finally add vertex buffer to instance
		m_VertexBuffers.push_back(vertexBuffer);

	}
	void OpenGLVertexArray::SetIndexBuffer(const Kargono::Ref<Kargono::IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();
		m_IndexBuffer = indexBuffer;
	}
}

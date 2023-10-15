#include "kgpch.h"

#include "API/OpenGL/OpenGLVertexArray.h"

#include <glad/glad.h>

namespace Kargono
{
	static GLenum ShaderDataTypeToOpenGLBaseType(InputDataType type)
	{
		switch (type)
		{
			case InputDataType::Float:	return GL_FLOAT;
			case InputDataType::Float2:	return GL_FLOAT;
			case InputDataType::Float3:	return GL_FLOAT;
			case InputDataType::Float4:	return GL_FLOAT;
			case InputDataType::Mat3:		return GL_FLOAT;
			case InputDataType::Mat4:		return GL_FLOAT;
			case InputDataType::Int:		return GL_INT;
			case InputDataType::Int2:		return GL_INT;
			case InputDataType::Int3:		return GL_INT;
			case InputDataType::Int4:		return GL_INT;
			case InputDataType::Bool:		return GL_BOOL;
			}
			KG_CORE_ASSERT(false, "Invalid Conversion at ShaderDataTypeToOpenGLBaseType!");
			return -1;
		}

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
	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		KG_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();
		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			switch (element.Type)
			{
			case InputDataType::Float:
			case InputDataType::Float2:
			case InputDataType::Float3:
			case InputDataType::Float4:
			{
				glEnableVertexAttribArray(m_VertexBufferIndex);
				glVertexAttribPointer(m_VertexBufferIndex,
					element.GetComponentCount(),
					ShaderDataTypeToOpenGLBaseType(element.Type),
					element.Normalized ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					(const void*)element.Offset);
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
					ShaderDataTypeToOpenGLBaseType(element.Type),
					layout.GetStride(),
					(const void*)element.Offset);
				m_VertexBufferIndex++;
				break;
			}
			case InputDataType::Mat3:
			case InputDataType::Mat4:
			{
				uint8_t count = element.GetComponentCount();
				for (uint8_t i = 0; i < count; i++)
				{
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribPointer(m_VertexBufferIndex,
						count,
						ShaderDataTypeToOpenGLBaseType(element.Type),
						element.Normalized ? GL_TRUE : GL_FALSE,
						layout.GetStride(),
						(const void*)(element.Offset + sizeof(float) * count * i));
					glVertexAttribDivisor(m_VertexBufferIndex, 1);
					m_VertexBufferIndex++;
				}
				break;
			}
			default:
				KG_CORE_ASSERT(false, "Unknown ShaderDataType!");
			}
		}

		m_VertexBuffers.push_back(vertexBuffer);

	}
	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}
}

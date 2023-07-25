#include "Kargono/kgpch.h"
#include "OpenGLVertexArray.h"

#include <glad/glad.h>

namespace Kargono
{
	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
			case Kargono::ShaderDataType::Float:	return GL_FLOAT;
			case Kargono::ShaderDataType::Float2:	return GL_FLOAT;
			case Kargono::ShaderDataType::Float3:	return GL_FLOAT;
			case Kargono::ShaderDataType::Float4:	return GL_FLOAT;
			case Kargono::ShaderDataType::Mat3:		return GL_FLOAT;
			case Kargono::ShaderDataType::Mat4:		return GL_FLOAT;
			case Kargono::ShaderDataType::Int:		return GL_INT;
			case Kargono::ShaderDataType::Int2:		return GL_INT;
			case Kargono::ShaderDataType::Int3:		return GL_INT;
			case Kargono::ShaderDataType::Int4:		return GL_INT;
			case Kargono::ShaderDataType::Bool:		return GL_BOOL;
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
	void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
	{
		KG_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();
		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index,
				static_cast<GLint>(element.GetComponentCount()),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				static_cast<GLint>(layout.GetStride()),
				(const void*)element.Offset);
			index++;
		}

		m_VertexBuffers.push_back(vertexBuffer);

	}
	void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}
}
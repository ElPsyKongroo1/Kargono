/// @brief Includes the precompiled header file
#include "kgpch.h"

/// @brief Includes the OpenGlVertexArray API header file
#include "API/OpenGL/OpenGLVertexArray.h"

/// @brief Include GLAD for managing OpenGL function pointers
#include <glad/glad.h>

/// @brief Namespace enclosing API OpenGL functionality
namespace API::OpenGL
{
/// @brief Converts Kargono input data type to OpenGL base data type. 
/// @param type - Kargono input data type 
/// @return GLenum - OpenGL base enumerated data type
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
/// @brief Assertion for handling invalid conversion
			KG_CORE_ASSERT(false, "Invalid Conversion at ShaderDataTypeToOpenGLBaseType!");
			return -1;
		}

/// @brief Constructor for OpenGLVertexArray class
	OpenGLVertexArray::OpenGLVertexArray()
	{
/// @details Creates one OpenGL Vertex Array object and retrieves its ID
		glCreateVertexArrays(1, &m_RendererID);
	}
/// @brief Destructor for OpenGLVertexArray class
	OpenGLVertexArray::~OpenGLVertexArray()
	{
/// @details Deletes the OpenGL Vertex Array object
		glDeleteVertexArrays(1, &m_RendererID);
	}
/// @brief Binds the vertex array object to the context
	void OpenGLVertexArray::Bind() const
	{
/// @details Invokes OpenGL to bind to the vertex array
/// @brief Bind vertex array using its ID
		glBindVertexArray(m_RendererID);
	}
/// @brief Unbinds the vertex array object from the context
	void OpenGLVertexArray::Unbind() const
	{
/// @details Invokes OpenGL to unbind the vertex array
		glBindVertexArray(0);
	}
/// @brief Adds a vertex buffer to the vertex array. 
/// @param vertexBuffer - Vertex buffer to be added
	void OpenGLVertexArray::AddVertexBuffer(const Kargono::Ref<Kargono::VertexBuffer>& vertexBuffer)
	{
/// @brief Assertion handling for missing vertex buffer layout
		KG_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

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
					ShaderDataTypeToOpenGLBaseType(element.Type),
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
					ShaderDataTypeToOpenGLBaseType(element.Type),
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

		// Finally add vertex buffer to instance
		m_VertexBuffers.push_back(vertexBuffer);

	}
/// @brief Set the Index Buffer 
/// @param indexBuffer The index buffer to be set
	void OpenGLVertexArray::SetIndexBuffer(const Kargono::Ref<Kargono::IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_RendererID);
/// @brief Bind the index buffer
		indexBuffer->Bind();
/// @brief Set the index buffer to the specified buffer
		m_IndexBuffer = indexBuffer;
	}
}

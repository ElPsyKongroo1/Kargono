/// @file Includes specific header files for this implementation file
#include "kgpch.h"

/// @file Includes header file for OpenGL uniform buffer
#include "OpenGLUniformBuffer.h"

/// @file Includes header file for OpenGL functions and enumerations
#include <glad/glad.h>

/// @namespace Defines a space for our OpenGL API
namespace API::OpenGL
{

/// @brief Class constructor, instantiates a buffer, provides buffer storage, and binds buffer to binding location
	OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding)
	{
		// Instantiate Buffer
		glCreateBuffers(1, &m_RendererID);
		// Create OpenGL buffer storage
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW); // TODO: investigate usage hint
		// Bind buffer to binding location
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
	}

/// @brief Class destructor, deletes buffer
	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}


/// @brief Function for setting the buffer data with specific offset
/// @param data Data to be set in buffer
/// @param size Size of the data
/// @param offset The starting point to set the data in the buffer
	void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}

}

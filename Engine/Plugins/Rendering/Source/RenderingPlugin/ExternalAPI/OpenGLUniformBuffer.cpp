#include "kgpch.h"

#include "RenderingPlugin/ExternalAPI/OpenGLUniformBuffer.h"
#include "API/Platform/gladAPI.h"

#ifdef KG_RENDERER_OPENGL

namespace API::RenderingAPI
{

	OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding)
	{
		// Instantiate Buffer
		glCreateBuffers(1, &m_RendererID);
		// Create OpenGL buffer storage
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW); // TODO: investigate usage hint
		// Bind buffer to binding location
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}


	void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}

}

#endif

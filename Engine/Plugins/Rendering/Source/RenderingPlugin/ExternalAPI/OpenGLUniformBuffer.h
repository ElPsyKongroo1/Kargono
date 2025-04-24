#pragma once

#include "RenderingPlugin/UniformBuffer.h"

#ifdef KG_RENDERER_OPENGL

namespace API::RenderingAPI
{
	//============================================================
	// OpenGL Uniform Buffer Class
	//============================================================
	// This class represents a Uniform Buffer inside of OpenGL. The class holds
	//		a reference to the actual uniform buffer with m_RendererID. A uniform buffer
	//		simply represents a portion of allocated data (buffer) in the GPU that can
	//		be accessed by any shader program. This is particularly useful for data that
	//		does not change per render pass such as the view projection matrix of the current
	//		camera.
	class OpenGLUniformBuffer : public Kargono::Rendering::UniformBuffer
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This constructor creates the uniform buffer, creates the storage for the buffer in OpenGL,
		//		and binds the buffer to a specific slot that is accessible on the GPU.
		OpenGLUniformBuffer(uint32_t size, uint32_t binding);
		// This constructor simply deletes the current buffer associated with m_RendererID in OpenGL
		virtual ~OpenGLUniformBuffer();

		//==============================
		// Update OpenGL Context
		//==============================
		// This function allows the caller to Set the data inside the uniform buffer either entirely or
		//		only a specific section. Ex: An offset of 0 and a size that contains the entire buffer
		//		will fill the entire buffer. An offset of 1 and a size of a float will only fill the float
		//		at the location of 1 in the buffer
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
	private:
		// m_RendererID represents the OpenGL uniform buffer associated with this object. Any OpenGL calls
		//		will use this ID.
		uint32_t m_RendererID = 0;
	};
}

#endif

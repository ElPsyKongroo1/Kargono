#pragma once

#include "Modules/Rendering/UniformBuffer.h"

#ifdef KG_RENDERER_OPENGL

namespace API::RenderingAPI
{
	class OpenGLUniformBuffer : public Kargono::Rendering::UniformBuffer
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		OpenGLUniformBuffer(uint32_t size, uint32_t binding);
		virtual ~OpenGLUniformBuffer();

		//==============================
		// Update OpenGL Context
		//==============================
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
	private:
		uint32_t m_RendererID{ 0 };
	};
}

#endif

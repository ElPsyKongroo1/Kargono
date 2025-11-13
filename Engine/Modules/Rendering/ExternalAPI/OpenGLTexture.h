#pragma once

#include "Modules/Rendering/Assets/Texture.h"
#include "Modules/Assets/AssetsCommon.h"
#include "Kargono/Core/Buffer.h"

#include "API/Platform/gladAPI.h"

#ifdef KG_RENDERER_OPENGL

namespace API::RenderingAPI
{
	class OpenGLTexture2D : public Kargono::Rendering::Texture2D
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		OpenGLTexture2D(const Kargono::Rendering::TextureSpecification& spec);
		OpenGLTexture2D(uint32_t rendererID, uint32_t width, uint32_t height);
		OpenGLTexture2D(const char* path);
		OpenGLTexture2D(Kargono::Buffer buffer, const Kargono::Rendering::TextureMetaData& metadata);
		virtual ~OpenGLTexture2D();
		//==============================
		// Binding Functionality
		//==============================
		virtual void Bind(uint32_t slot = 0) const override;
	public:
		//==============================
		// Load Function(s)
		//==============================
		virtual void LoadBuffer(Kargono::Buffer buffer, const Kargono::Rendering::TextureMetaData& metadata) override;
		//==============================
		// Update OpenGL Context
		//==============================
		virtual void SetData(void* data, uint32_t size) override;
		//==============================
		// Getters/Setters
		//==============================
		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint32_t GetRendererID() const override { return m_RendererID; }
		//==============================
		// Operator Overloads
		//==============================
		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == other.GetRendererID();
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_RendererID;
		GLenum m_InternalFormat;
		GLenum m_DataFormat;
	};
}

#endif

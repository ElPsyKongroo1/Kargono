#include "kgpch.h"

#include "Kargono/Core/Log.h"

#include "RenderingPlugin/ExternalAPI/OpenGLTexture.h"
#include "API/ImageProcessing/stbAPI.h"
#include "API/Platform/gladAPI.h"

#ifdef KG_RENDERER_OPENGL

namespace API::Utility
{
	static GLenum KargonoFormatToGLDataFormat(Kargono::Rendering::ImageFormat format)
	{
		switch (format)
		{
		case Kargono::Rendering::ImageFormat::RGB8: return GL_RGB;
		case Kargono::Rendering::ImageFormat::RGBA8: return GL_RGBA;
		default:
		{
			KG_ERROR("Invalid ImageFormat in KargonoFormatToGLDataFormat");
			return 0;
		}
		}
	}

	static GLenum KargonoFormatToGLInternalFormat(Kargono::Rendering::ImageFormat format)
	{
		switch (format)
		{
		case Kargono::Rendering::ImageFormat::RGB8: return GL_RGB8;
		case Kargono::Rendering::ImageFormat::RGBA8: return GL_RGBA8;
		default:
		{
			KG_ERROR("Invalid ImageFormat in KargonoFormatToGLInternalFormat");
			return 0;
		}
				
		}
	}
}

namespace API::RenderingAPI
{
	OpenGLTexture2D::OpenGLTexture2D(const Kargono::Rendering::TextureSpecification& spec)
		: m_Width(spec.Width), m_Height(spec.Height)
	{
		m_InternalFormat = Utility::KargonoFormatToGLInternalFormat(spec.Format);
		m_DataFormat = Utility::KargonoFormatToGLDataFormat(spec.Format);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	// Used to Create Texture from previously unmanaged texture
	OpenGLTexture2D::OpenGLTexture2D(uint32_t rendererID, uint32_t width, uint32_t height)
		: m_RendererID(rendererID), m_Width(width), m_Height(height)
	{
		
	}
	OpenGLTexture2D::OpenGLTexture2D(const char* path)
	{
		// Load texture into byte buffer
		int32_t width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = nullptr;
		{
			data = stbi_load(path, &width, &height, &channels, 0);
		}

		// Find internal format of texture
		if (data)
		{
			m_Width = width;
			m_Height = height;

			GLenum internalFormat = 0, dataFormat = 0;
			if (channels == 4)
			{
				internalFormat = GL_RGBA8;
				dataFormat = GL_RGBA;
			}
			else if (channels == 3)
			{
				internalFormat = GL_RGB8;
				dataFormat = GL_RGB;
			}

			m_InternalFormat = internalFormat;
			m_DataFormat = dataFormat;

			KG_ASSERT(internalFormat & dataFormat, "Format not supported!");

			// Instantiate texture into OpenGL
			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
			glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);
		}
	}
	OpenGLTexture2D::OpenGLTexture2D(Kargono::Buffer buffer, const Kargono::Assets::TextureMetaData& metadata)
	{
		KG_ASSERT(buffer.Data, "Buffer does not have any valid data to input into Texture2D!");

		m_Width = metadata.Width;
		m_Height = metadata.Height;

		GLenum internalFormat = 0, dataFormat = 0;
		if (metadata.Channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (metadata.Channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}

		m_InternalFormat = internalFormat;
		m_DataFormat = dataFormat;

		KG_ASSERT(internalFormat & dataFormat, "Format not supported!");

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE,
			buffer.As<unsigned char>());
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}
	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		const uint32_t bytesPerPixel = m_DataFormat == GL_RGBA ? 4 : 3;
		KG_ASSERT(size == m_Width * m_Height * bytesPerPixel, "Data must be entire texture!");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}
	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}
}


#endif

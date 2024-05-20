/// @brief Includes the precompiled header file for the Kargono engine
#include "kgpch.h"

/// @brief Includes the Log class for core logging functionality from the Kargono engine
#include "Kargono/Core/Log.h"
/// @brief Includes the OpenGLTexture header for handling OpenGL texture objects
#include "API/OpenGL/OpenGLTexture.h"

/// @brief Includes the stb_image library for texture manipulation and query
#include "stb_image.h"
/// @brief Includes the glad library for managing OpenGL functions
#include <glad/glad.h>

/// @brief Namespace containing OpenGL related functionalities in the API
namespace API::OpenGL
{
	// Used to Create Texture from previously unmanaged texture
/// @brief Constructs an OpenGLTexture2D object with a rendererID, width and height 
 /// @param rendererID 
 /// @param width 
 /// @param height
	OpenGLTexture2D::OpenGLTexture2D(uint32_t rendererID, uint32_t width, uint32_t height)
		:m_RendererID(rendererID), m_Width(width), m_Height(height)
	{
		
	}
/// @brief Constructs an OpenGLTexture2D object from a resource path 
 /// @param path String representing the input texture resource path
	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
	{
		// Load texture into byte buffer
		int32_t width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = nullptr;
		{
			data = stbi_load(path.c_str(), &width, &height, &channels, 0);
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

			KG_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!");

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
/// @brief Constructs an OpenGLTexture2D object using a Buffer and TextureMetaData 
 /// @param buffer Buffer object containing the texture data 
 /// @param metadata Metadata object containing properties of the texture
	OpenGLTexture2D::OpenGLTexture2D(Kargono::Buffer buffer, const Kargono::Assets::TextureMetaData& metadata)
	{
		KG_CORE_ASSERT(buffer.Data, "Buffer does not have any valid data to input into Texture2D!");

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

		KG_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!");

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE,
			buffer.As<unsigned char>());
	}

/// @brief Destructs the OpenGLTexture2D object and deletes associated OpenGL textures
	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}
/// @brief Sets the data for the OpenGLTexture2D with provided size 
 /// @param data Pointer to the data to be set 
 /// @param size The size of provided data
	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		const uint32_t bytesPerPixel = m_DataFormat == GL_RGBA ? 4 : 3;
		KG_CORE_ASSERT(size == m_Width * m_Height * bytesPerPixel, "Data must be entire texture!");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}
/// @brief Binds the OpenGLTexture2D to a specific slot 
 /// @param slot The slot to bind the texture in
	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}
}

#pragma once

#include "RenderModule/Texture.h"
#include "AssetModule/Asset.h"
#include "Kargono/Core/Buffer.h"

#include "API/Platform/gladAPI.h"

#ifdef KG_RENDERER_OPENGL

namespace API::RenderingAPI
{
	//============================================================
	// OpenGL Texture Class
	//============================================================
	// This class represents an OpenGL instance of a 2D Texture. This class can wrap
	//		an already instantiated texture, create/instantiate a texture from a path (used in editor),
	//		or instantiate a texture through a pre-loaded intermediate. The m_RendererID holds the
	//		reference to the OpenGL implementation of the texture.
	class OpenGLTexture2D : public Kargono::Rendering::Texture2D
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================

		// All of these constructors instantiate the OpenGL version of a texture.

		// TODO: This should be a temporary api for creating a texture!
		OpenGLTexture2D(const Kargono::Rendering::TextureSpecification& spec);
		// This constructor takes an already instantiated texture and wraps it inside
		//		this class. I plan to remove this constructor once I make the
		//		texture creation API more complete.
		OpenGLTexture2D(uint32_t rendererID, uint32_t width, uint32_t height);
		// This constructor instantiates a texture directly from a filepath. This functionality
		//		is mainly used for the editor logos.
		OpenGLTexture2D(const char* path);
		// This constructor takes an intermediate byte buffer and it's associated metadata
		//		to instantiate a texture. This is the main way to create a texture through the
		//		asset system.
		OpenGLTexture2D(Kargono::Buffer buffer, const Kargono::Assets::TextureMetaData& metadata);

		// This destructor simply deletes the texture inside the OpenGL context.
		virtual ~OpenGLTexture2D();

		//==============================
		// Binding Functionality
		//==============================
		// This binding functionality is similar to uploading uniforms in the shader. This function
		//		uploads a reference to the texture to a slot that is accessible inside GLSL shader code
		//		similar to a uniform.
		virtual void Bind(uint32_t slot = 0) const override;

		//==============================
		// Update OpenGL Context
		//==============================

		// This function allows new data to be pushed into the OpenGL texture.
		//		The size of the data buffer must match the size of the texture.
		virtual void SetData(void* data, uint32_t size) override;

		//==============================
		// Getters/Setters
		//==============================
		// This functions are simply accessors for instance data.
		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint32_t GetRendererID() const override { return m_RendererID; }


		//==============================
		// Operator Overloads
		//==============================
		// Allows Textures to be easily compared
		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == other.GetRendererID();
		}
	private:
		// m_Width and m_Height are simply the dimensions of the texture
		uint32_t m_Width, m_Height;
		// m_RendererID represents the OpenGL texture associated with this object. Any OpenGL calls
		//		will use this ID.
		uint32_t m_RendererID;
		// These internal formats are for debugging purposes. This will get refactored later.
		GLenum m_InternalFormat, m_DataFormat;
	};
}

#endif

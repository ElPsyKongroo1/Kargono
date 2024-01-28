#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/Buffer.h"
#include "Kargono/Assets/Asset.h"

#include <string>
#include <filesystem>


namespace Kargono
{

	enum class ImageFormat
	{
		None = 0,
		R8,
		RGB8,
		RGBA8,
		RGBA32F
	};

	struct TextureSpecification
	{
		uint32_t Width = 1;
		uint32_t Height = 1;
		ImageFormat Format = ImageFormat::RGBA8;
		bool GenerateMipMaps = true;

	};

	class Texture
	{
	public:
		virtual ~Texture() = default;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const TextureSpecification& spec);
		static Ref<Texture2D> Create(uint32_t rendererID, uint32_t width, uint32_t height);

		// Create Texture using intermediate format
		static Ref<Texture2D> Create(Buffer buffer, const Assets::TextureMetaData& metadata);

		// Create unmanaged texture outside of AssetManager. Used for Editor Textures only.
		// Runtime related textures should use AssetManager.
		static Ref<Texture2D> CreateEditorTexture(const std::filesystem::path& path);

	};
}

namespace Kargono::Utility
{
	inline uint32_t ImageFormatToBytes(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::R8:		{ return 1;}
		case ImageFormat::RGB8:		{ return 3;}
		case ImageFormat::RGBA8:	{ return 4;}
		case ImageFormat::RGBA32F:	{ return 16;}
		case ImageFormat::None:		{ return 1; }
		default:
			{
				KG_ASSERT(false, "Invalid ImageFormat submitted to ImageFormatToSize");
				return 0;
			}
		}
	}
}


#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/Buffer.h"
#include "Modules/Assets/Metadata.h"
#include "Modules/Rendering/Module/RenderingModule.h"
#include "Modules/Assets/Module/AssetTag.h"

#include <string>
#include <filesystem>


namespace Kargono::Rendering
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
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		TextureSpecification() = default;
		~TextureSpecification() = default;
	public:
		//==============================
		// Public Fields
		//==============================
		Buffer m_Buffer{};
		size_t m_Width{ 1 };
		size_t m_Height{ 1 };
		ImageFormat m_Format{ ImageFormat::RGBA8 };
		bool m_GenerateMipMaps{ true };
	};

	struct TextureMetaData
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		TextureMetaData() = default;
		~TextureMetaData() = default;
	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//==============================
		// Public Fields
		//==============================
		int32_t m_Width{ 0 };
		int32_t m_Height{ 0 };
		int32_t m_Channels{ 0 };
	};

	class Texture
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Texture() = default;
		virtual ~Texture() = default;
	public:
		//==============================
		// Getters/Setters
		//==============================
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;
		virtual void SetData(void* data, uint32_t size) = 0;

		//==============================
		// Interact w/ Graphics API
		//==============================
		virtual void Bind(uint32_t slot = 0) const = 0;
	public:
		//==============================
		// Operator Overloads
		//==============================
		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		//==============================
		// Metaprogramming Info
		//==============================
		using Metadata = TextureMetaData;
		using Spec = TextureSpecification;
		constexpr static std::array<FixedBufStr16, 1> k_ImportExtensions{ ".png" };
		constexpr static std::array<FixedBufStr16, 1> k_IntermediateExtensions{ ".kgbinary" };
	public:
		//==============================
		// Asset Config Info
		//==============================
		constexpr static FixedBufStr32 GetAssetName()
		{
			return "Texture";
		}

		constexpr static Assets::AssetFlags GetAssetFlags()
		{
			Assets::AssetFlags flags{};
			flags.SetFlag(Assets::AssetFlag::HasAssetCache);
			flags.ClearFlag(Assets::AssetFlag::RequireUniqueName);
			flags.SetFlag(Assets::AssetFlag::AllowDefaultUpdateAsset);
			return flags;
		}

		constexpr static FixedBufStr16 GetFileExtension()
		{
			return ".kgtexture";
		}

		constexpr static std::span<const FixedBufStr16> GetImportExtensions()
		{
			return std::span(k_ImportExtensions.data(), k_ImportExtensions.size());
		}

		constexpr static std::span<const FixedBufStr16> GetIntermediateExtensions()
		{
			return std::span(k_IntermediateExtensions.data(), k_IntermediateExtensions.size());
		}

		static void CreateFromName(Assets::Metadata& metadata);
		static void CreateFromFile(Assets::Metadata& metadata, const std::filesystem::path& sourcePath);
		static void CreateFromSpec(Assets::Metadata& metadata, const TextureSpecification& spec);
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Texture2D() = default;
		virtual ~Texture2D() = default;
	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//==============================
		// Load Function(s)
		//==============================
		virtual void LoadBuffer(Buffer buffer, const TextureMetaData& metadata) = 0;
		void ValidateDelete(Assets::Metadata& metadata);
	public:
		//==============================
		// Static Helpers (UGHHHH REMOVE THIS)
		//==============================
		static Ref<Texture2D> Create(const TextureSpecification& spec);
		static Ref<Texture2D> Create(uint32_t rendererID, uint32_t width, uint32_t height);
		// Create Texture using intermediate format
		static Ref<Texture2D> Create(Buffer buffer, const TextureMetaData& metadata);
		// Create unmanaged texture outside of AssetManager. Used for Editor Textures only.
		// Runtime related textures should use AssetManager.
		static Ref<Texture2D> CreateEditorTexture(const std::filesystem::path& path);
	};
}

namespace Kargono::Utility
{
	inline size_t ImageFormatToBytes(Rendering::ImageFormat format)
	{
		switch (format)
		{
		case Rendering::ImageFormat::R8:
			return 1;
		case Rendering::ImageFormat::RGB8:
			return 3;
		case Rendering::ImageFormat::RGBA8:
			return 4;
		case Rendering::ImageFormat::RGBA32F: 
			return 16;
		case Rendering::ImageFormat::None:
			return 1;
		default:
			KG_ERROR("Invalid ImageFormat submitted to ImageFormatToSize");
			return 0;
			
		}
	}
}


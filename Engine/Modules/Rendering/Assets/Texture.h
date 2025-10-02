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
		uint32_t m_Width{ 1 };
		uint32_t m_Height{ 1 };
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
	public:
		//==============================
		// Static Asset Functions
		//==============================
		constexpr static Assets::AssetConfig GetAssetConfig()
		{
			Assets::AssetConfig config{};
			config.m_Identifier = Assets::GetAssetIdentifier<Texture2D>();
			config.m_Name = "Texture";
			config.m_FileExtension = ".kgtexture";
			config.m_ImportExtensions = { ".png" };
			config.m_RegistryPath = "Texture2D/TextureRegistry.kgreg";
			config.m_Flags.SetFlag(Assets::AssetFlags::HasAssetCache);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasIntermediateLocation);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasFileLocation);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasFileImporting);
			config.m_Flags.ClearFlag(Assets::AssetFlags::HasAssetSaving);
			config.m_Flags.ClearFlag(Assets::AssetFlags::HasAssetCreationFromName);
			return config;
		}
		static void CreateAssetFileFromName(std::string_view name,
			Assets::Metadata& metadata, std::filesystem::path& assetPath);
		static void CreateAssetIntermediateFromFile(Assets::Metadata& metadata,
			std::filesystem::path& filePath, std::filesystem::path& intermediatePath);
		static void CreateTextureIntermediateFromBuffer(Buffer buffer, int32_t width, 
			int32_t height, int32_t channels, Assets::Metadata& newAsset);
		static Assets::AssetHandle ImportNewTextureFromData(Buffer buffer, int32_t width, 
			int32_t height, int32_t channels);
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

		void DeleteValidation(Assets::AssetHandle assetHandle);
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
	inline uint32_t ImageFormatToBytes(Rendering::ImageFormat format)
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


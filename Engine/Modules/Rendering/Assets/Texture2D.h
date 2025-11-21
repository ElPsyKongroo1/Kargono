#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/Buffer.h"
#include "Modules/Assets/Metadata.h"
#include "Modules/Rendering/Module/RenderingModule.h"
#include "Modules/Assets/Module/AssetTag.h"

#include "API/Platform/gladAPI.h"

#include <string>
#include <filesystem>
#include <span>


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

	class Texture2D
	{
	public:
		//==============================
		// Asset Config
		//==============================
		// Metaprogramming info
		using Metadata = TextureMetaData;
		using Spec = TextureSpecification;
		constexpr static std::array<FixedBufStr16, 1> k_ImportExtensions{ ".png" };
		constexpr static std::array<FixedBufStr16, 1> k_IntermediateExtensions{ ".kgbinary" };

		// Base asset info
		constexpr static FixedBufStr32 GetAssetName()
		{
			return "Texture";
		}

		constexpr static Assets::AssetFlags GetAssetFlags()
		{
			Assets::AssetFlags flags{};
			flags.SetFlag(Assets::AssetFlag::HasAssetCache);
			flags.ClearFlag(Assets::AssetFlag::RequireUniqueName);
			return flags;
		}

		// Optional asset functions
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
		~Texture2D() = default;
	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//==============================
		// Validate Functions
		//==============================
		void ValidateDelete(Assets::Metadata& metadata);
	public:
		//==============================
		// Interact w/ Renderer
		//==============================
		// Register texture w/ renderer
		void RegisterTexture(const TextureSpecification& spec);
		void RegisterTexture(uint32_t rendererID, uint32_t width, uint32_t height);
		void RegisterTexture(Buffer buffer, const TextureMetaData& metadata);
		void RegisterTexture(const std::filesystem::path& path);
		void DeregisterTexture();
		// Bind w/ OpenGL state machine
		void Bind(uint32_t slot) const;
		// Load data into OpenGL buffer
		void SetData(void* data, uint32_t size);
		void LoadBuffer(Buffer buffer, const TextureMetaData& metadata);
	public:
		//==============================
		// Getters/Setters
		//==============================
		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
		uint32_t GetRendererID() const { return m_RendererID; }
	public:
		//==============================
		// Operator Overloads
		//==============================
		bool operator==(const Texture2D& other) const { return m_RendererID == other.GetRendererID(); }
	private:
		//==============================
		// Internal Fields
		//==============================
		uint32_t m_Width{ 0 };
		uint32_t m_Height{ 0 };
		uint32_t m_RendererID{ 0 };
		GLenum m_InternalFormat {};
		GLenum m_DataFormat{};
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


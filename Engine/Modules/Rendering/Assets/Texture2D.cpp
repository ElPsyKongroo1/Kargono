#include "kgpch.h"

#include "Modules/Rendering/RenderingService.h"
#include "Modules/Rendering/Assets/Texture2D.h"
#include "Kargono/Projects/Project.h"
#include "Modules/Assets/AssetService.h"
#include "Modules/RuntimeUI/Assets/RuntimeUIUserInterface.h"

#include "API/Platform/gladAPI.h"
#include "API/ImageProcessing/stbAPI.h"

namespace Kargono::Utility
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

namespace Kargono::Rendering
{
	void Texture2D::RegisterTexture(const Kargono::Rendering::TextureSpecification& spec)
	{
		m_Width = spec.m_Width;
		m_Height = spec.m_Height;

		m_InternalFormat = Utility::KargonoFormatToGLInternalFormat(spec.m_Format);
		m_DataFormat = Utility::KargonoFormatToGLDataFormat(spec.m_Format);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	void Texture2D::RegisterTexture(uint32_t rendererID, uint32_t width, uint32_t height)
	{
		m_RendererID = rendererID;
		m_Width = width;
		m_Height = height;
	}
	void Texture2D::RegisterTexture(const std::filesystem::path& path)
	{
		// Load texture into byte buffer
		int32_t width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = nullptr;
		{
			data = stbi_load(path.string().c_str(), &width, &height, &channels, 0);
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
	void Texture2D::RegisterTexture(Kargono::Buffer buffer, const Kargono::Rendering::TextureMetaData& metadata)
	{
		LoadBuffer(buffer, metadata);
	}

	void Texture2D::DeregisterTexture()
	{
		glDeleteTextures(1, &m_RendererID);
	}
	void Texture2D::LoadBuffer(Kargono::Buffer buffer, const Kargono::Rendering::TextureMetaData& metadata)
	{
		KG_ASSERT(buffer.m_Data);

		m_Width = metadata.m_Width;
		m_Height = metadata.m_Height;

		GLenum internalFormat = 0, dataFormat = 0;
		if (metadata.m_Channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (metadata.m_Channels == 3)
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
	void Texture2D::SetData(void* data, uint32_t size)
	{
		const uint32_t bytesPerPixel = m_DataFormat == GL_RGBA ? 4 : 3;
		KG_ASSERT(size == m_Width * m_Height * bytesPerPixel, "Data must be entire texture!");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}
	void Texture2D::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}

	void TextureMetaData::Serialize(void* context)
	{
		// Get asset context
		Assets::SerializeMetaDataContext* metadataContext = (Assets::SerializeMetaDataContext*)context;
		KG_ASSERT(metadataContext, "Context cannot be null");

		// Get context fields
		YAML::Emitter& emitter = *metadataContext->m_Serializer;

		// Serialize
		emitter << YAML::Key << "TextureHeight" << YAML::Value << m_Height;
		emitter << YAML::Key << "TextureWidth" << YAML::Value << m_Width;
		emitter << YAML::Key << "TextureChannels" << YAML::Value << m_Channels;
	}

	void TextureMetaData::Deserialize(void* context)
	{
		// Get asset context
		Assets::DeserializeMetaDataContext* assetContext = (Assets::DeserializeMetaDataContext*)context;
		KG_ASSERT(assetContext, "Context cannot be null");

		// Get context fields
		YAML::Node& metadataNode = *assetContext->m_Node;

		// Deserialize
		m_Height = metadataNode["TextureHeight"].as<int32_t>();
		m_Width = metadataNode["TextureWidth"].as<int32_t>();
		m_Channels = metadataNode["TextureChannels"].as<int32_t>();
	}

	void Texture2D::Serialize(void* context)
	{
		KG_ERROR("Texture serialization is not implemented");
	}

	void Texture2D::Deserialize(void* context)
	{
		// Get context
		Assets::DeserializeAssetContext* deserializeContext = (Assets::DeserializeAssetContext*)context;
		KG_ASSERT(deserializeContext, "Context cannot be null");
		Assets::Metadata* metadata = deserializeContext->m_AssetMetadata;
		KG_ASSERT(metadata, "Metadata cannot be null");

		// Get asset path
		std::string_view intermediateExtension
		{
			Texture2D::GetIntermediateExtensions().front().StringView()
		};
		const std::filesystem::path& assetPath = 
			metadata->GetAssetFullIntermediatePath<Texture2D>(intermediateExtension);

		// Load the texture
		TextureMetaData& specificMetadata = *metadata->GetSpecificMetaData<TextureMetaData>();
		Buffer currentResource = Utility::FileSystem::ReadFileBinary(assetPath);
		LoadBuffer(currentResource, specificMetadata);
		currentResource.Release();
	}

	void Texture2D::CreateFromName(Assets::Metadata& metadata)
	{
		const std::filesystem::path& assetPath = metadata.GetAssetFullFilePath<Texture2D>();

		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		out << YAML::Key << "Name" << YAML::Value << metadata.m_Name.CString(); // Output texture name
		out << YAML::EndMap; // End of File Map

		std::ofstream fout(assetPath);
		fout << out.c_str();
		KG_INFO("Successfully created texture inside asset directory at {}", assetPath);
	}
	void Texture2D::CreateFromFile(Assets::Metadata& metadata,
		const std::filesystem::path& sourcePath)
	{
		std::string_view intermediateExtension
		{
			Texture2D::GetIntermediateExtensions().front().StringView()
		};
		const std::filesystem::path intermediatePath = 
			metadata.GetAssetFullIntermediatePath<Texture2D>(intermediateExtension);

		// Create Texture Binary Intermediate
		int32_t width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		Buffer buffer{};
		stbi_uc* data = nullptr;
		{
			data = stbi_load(sourcePath.string().c_str(), &width, &height, &channels, 0);
		}

		buffer.Allocate(static_cast<unsigned long long>(width) * height * channels * sizeof(uint8_t));
		buffer.m_Data = data;

		// Save Binary Intermediate into File
		Utility::FileSystem::WriteFileBinary(intermediatePath, buffer);

		// Check that save was successful
		if (!data)
		{
			KG_ERROR("Failed to load data from file in texture importer!");
			buffer.Release();
			return;
		}

		// Load data into In-Memory Metadata object
		TextureMetaData& specificMetadata{ *metadata.GetSpecificMetaData<TextureMetaData>()};
		specificMetadata.m_Width = width;
		specificMetadata.m_Height = height;
		specificMetadata.m_Channels = channels;
		buffer.Release();
	}

	void Texture2D::CreateFromSpec(Assets::Metadata& metadata, const TextureSpecification& spec)
	{
		// Save Binary Intermediate into File
		std::string_view intermediateExtension
		{
			Texture2D::GetIntermediateExtensions().front().StringView()
		};
		const std::filesystem::path intermediatePath =
			metadata.GetAssetFullIntermediatePath<Texture2D>(intermediateExtension);
		Utility::FileSystem::WriteFileBinary(intermediatePath, spec.m_Buffer);

		// Load data into texture metadata
		TextureMetaData& textureMetadata{ *metadata.GetSpecificMetaData<TextureMetaData>() };
		textureMetadata.m_Width = spec.m_Width;
		textureMetadata.m_Height = spec.m_Height;
		textureMetadata.m_Channels = Utility::ImageFormatToBytes(spec.m_Format);
	}

	void Texture2D::ValidateDelete(Assets::Metadata& metadata)
	{
		// Check user interface assets
		for (auto& [uiHandle, metadata] : Assets::AssetService::GetUserInterfaceRegistry())
		{
			// Handle UI level function pointers
			Ref<RuntimeUI::UserInterface> userInterfaceRef = Assets::AssetService::GetUserInterface(uiHandle);
			bool uiModified = Assets::AssetService::RemoveTextureFromUserInterface(userInterfaceRef, metadata.m_Handle);
			if (uiModified)
			{
				Assets::AssetService::SaveUserInterface(uiHandle, userInterfaceRef);
			}
		}
	}
}

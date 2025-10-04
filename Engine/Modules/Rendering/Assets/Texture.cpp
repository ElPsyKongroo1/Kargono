#include "kgpch.h"

#include "Modules/Rendering/RenderingService.h"
#include "Modules/Rendering/Assets/Texture.h"
#include "Kargono/Projects/Project.h"
#include "Modules/Assets/AssetService.h"
#include "Modules/RuntimeUI/Assets/RuntimeUIUserInterface.h"

#include "Modules/Rendering/ExternalAPI/OpenGLTexture.h"
#include "API/ImageProcessing/stbAPI.h"

namespace Kargono::Rendering
{
	void TextureMetaData::Serialize(void* context)
	{
		// Get asset context
		KG_ASSERT(context, "Context cannot be null");
		Assets::SerializeMetaDataContext& metadataContext = *(Assets::SerializeMetaDataContext*)context;

		// Get context fields
		YAML::Emitter& emitter = *metadataContext.m_Serializer;

		// Serialize
		emitter << YAML::Key << "TextureHeight" << YAML::Value << m_Height;
		emitter << YAML::Key << "TextureWidth" << YAML::Value << m_Width;
		emitter << YAML::Key << "TextureChannels" << YAML::Value << m_Channels;
	}

	void TextureMetaData::Deserialize(void* context)
	{
		// Get asset context
		KG_ASSERT(context, "Context cannot be null");
		Assets::DeserializeMetaDataContext& assetContext = *(Assets::DeserializeMetaDataContext*)context;

		// Get context fields
		YAML::Node& metadataNode = *assetContext.m_Node;

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

		// Get asset path
		const std::filesystem::path& assetPath = deserializeContext->m_AssetPath;
		Assets::Metadata& metadata{ *deserializeContext->m_AssetMetadata };

		// Load the texture
		TextureMetaData& specificMetadata = *metadata.GetSpecificMetaData<TextureMetaData>();
		Buffer currentResource = Utility::FileSystem::ReadFileBinary(assetPath);
		LoadBuffer(currentResource, specificMetadata);
		currentResource.Release();
	}

	void Texture2D::CreateAssetFromName(Assets::Metadata& metadata, std::string_view name,
		std::filesystem::path& assetPath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		out << YAML::Key << "Name" << YAML::Value << std::string(name); // Output texture name
		out << YAML::EndMap; // End of File Map

		std::ofstream fout(assetPath);
		fout << out.c_str();
		KG_INFO("Successfully created texture inside asset directory at {}", assetPath);
	}
	void Texture2D::CreateAssetFromFile(Assets::Metadata& metadata,
		std::filesystem::path& sourcePath, std::filesystem::path& destPath)
	{
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
		Utility::FileSystem::WriteFileBinary(destPath, buffer);

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

	void Texture2D::CreateAssetFromSpec(Assets::Metadata& metadata, 
		TextureSpecification& spec, std::filesystem::path& assetPath)
	{
		// Save Binary Intermediate into File
		Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };
		std::filesystem::path intermediateFullPath = projectPaths.GetIntermediateDirectory() / metadata.m_IntermediateLocation;
		Utility::FileSystem::WriteFileBinary(intermediateFullPath, spec.m_Buffer);

		// Load data into In-Memory Metadata object
		TextureMetaData& textureMetadata{ *metadata.GetSpecificMetaData<TextureMetaData>() };
		textureMetadata.m_Width = spec.m_Width;
		textureMetadata.m_Height = spec.m_Height;
		textureMetadata.m_Channels = Utility::ImageFormatToBytes(spec.m_Format);
	}

	bool Texture2D::CreateSpecValidation(TextureSpecification& spec)
	{
		return false;
	}

	void Texture2D::DeleteValidation(Assets::AssetHandle assetHandle)
	{
		// Check user interface assets
		for (auto& [uiHandle, assetInfo] : Assets::AssetService::GetUserInterfaceRegistry())
		{
			// Handle UI level function pointers
			Ref<RuntimeUI::UserInterface> userInterfaceRef = Assets::AssetService::GetUserInterface(uiHandle);
			bool uiModified = Assets::AssetService::RemoveTextureFromUserInterface(userInterfaceRef, assetHandle);
			if (uiModified)
			{
				Assets::AssetService::SaveUserInterface(uiHandle, userInterfaceRef);
			}
		}
	}

	Ref<Texture2D> Texture2D::Create(const TextureSpecification& spec)
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLTexture2D>(spec);
#endif
	}
	Ref<Texture2D> Texture2D::Create(uint32_t rendererID, uint32_t width, uint32_t height)
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLTexture2D>(rendererID, width, height);
#endif
	}
	Ref<Texture2D> Texture2D::Create(Buffer buffer, const TextureMetaData& metadata)
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLTexture2D>(buffer, metadata);
#endif
	}
	Ref<Texture2D> Texture2D::CreateEditorTexture(const std::filesystem::path& path)
	{
			KG_ASSERT(path.is_absolute(), "Path provided to texture create function is not an absolute path!")
#ifdef KG_RENDERER_OPENGL
			return CreateRef<API::RenderingAPI::OpenGLTexture2D>(path.string().c_str());
#endif
	}
}

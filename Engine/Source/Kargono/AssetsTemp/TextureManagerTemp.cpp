#include "kgpch.h"

#include "Kargono/AssetsTemp/AssetService.h"
#include "Kargono/AssetsTemp/TextureManagerTemp.h"

#include "API/ImageProcessing/stbAPI.h"

namespace Kargono::Assets
{
	Ref<Rendering::Texture2D> Texture2DManager::DeserializeAsset(Assets::Asset& asset, const std::filesystem::path& assetPath)
	{
		Assets::TextureMetaData metadata = *asset.Data.GetSpecificMetaData<TextureMetaData>();
		Buffer currentResource = Utility::FileSystem::ReadFileBinary(assetPath);
		Ref<Rendering::Texture2D> newTexture = Rendering::Texture2D::Create(currentResource, metadata);
		currentResource.Release();
		return newTexture;
	}
	void Texture2DManager::SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::Asset& currentAsset)
	{
		Assets::TextureMetaData* metadata = static_cast<Assets::TextureMetaData*>(currentAsset.Data.SpecificFileData.get());
		serializer << YAML::Key << "TextureHeight" << YAML::Value << metadata->Height;
		serializer << YAML::Key << "TextureWidth" << YAML::Value << metadata->Width;
		serializer << YAML::Key << "TextureChannels" << YAML::Value << metadata->Channels;
	}

	void Texture2DManager::CreateAssetIntermediateFromFile(Asset& newAsset, const std::filesystem::path& fullFileLocation, const std::filesystem::path& fullIntermediateLocation)
	{
		// Create Texture Binary Intermediate
		int32_t width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		Buffer buffer{};
		stbi_uc* data = nullptr;
		{
			data = stbi_load(fullFileLocation.string().c_str(), &width, &height, &channels, 0);
		}

		buffer.Allocate(static_cast<unsigned long long>(width) * height * channels * sizeof(uint8_t));
		buffer.Data = data;

		// Save Binary Intermediate into File
		Utility::FileSystem::WriteFileBinary(fullIntermediateLocation, buffer);

		// Check that save was successful
		if (!data)
		{
			KG_ERROR("Failed to load data from file in texture importer!");
			buffer.Release();
			return;
		}

		// Load data into In-Memory Metadata object
		Ref<Assets::TextureMetaData> metadata = CreateRef<Assets::TextureMetaData>();
		metadata->Width = width;
		metadata->Height = height;
		metadata->Channels = channels;
		newAsset.Data.SpecificFileData = metadata;
		buffer.Release();
	}

	void Texture2DManager::DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::Asset& currentAsset)
	{
		Ref<Assets::TextureMetaData> texMetaData = CreateRef<Assets::TextureMetaData>();

		texMetaData->Height = metadataNode["TextureHeight"].as<int32_t>();
		texMetaData->Width = metadataNode["TextureWidth"].as<int32_t>();
		texMetaData->Channels = metadataNode["TextureChannels"].as<int32_t>();

		currentAsset.Data.SpecificFileData = texMetaData;
	}

	
}

#include "kgpch.h"

#include "Kargono/Assets/AssetService.h"
#include "Kargono/Assets/TextureManager.h"
#include "Kargono/Rendering/Texture.h"

#include "API/ImageProcessing/stbAPI.h"

namespace Kargono::Assets
{

	AssetHandle Texture2DManager::ImportNewTextureFromData(Buffer buffer, int32_t width, int32_t height, int32_t channels)
	{
		// Create Checksum
		std::string currentCheckSum = Utility::FileSystem::ChecksumFromBuffer(buffer);

		// Ensure checksum is valid
		if (currentCheckSum.empty())
		{
			KG_WARN("Generated empty checksum from data buffer");
			return Assets::EmptyHandle;
		}

		// Ensure duplicate asset is not found in registry.
		for (const auto& [handle, asset] : m_AssetRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				//KG_WARN("Attempt to instantiate duplicate {} asset. Returning existing asset.", m_AssetName);
				return handle;
			}
		}

		// Create New Asset/Handle
		AssetHandle newHandle{};
		Assets::Asset newAsset{};
		newAsset.Handle = newHandle;
		newAsset.Data.Type = m_AssetType;
		newAsset.Data.CheckSum = currentCheckSum;
		newAsset.Data.FileLocation = "";
		newAsset.Data.IntermediateLocation = m_RegistryLocation.parent_path() / ((std::string)newAsset.Handle + m_FileExtension.CString());

		// Create Intermediate
		CreateTextureIntermediateFromBuffer(buffer, width, height, channels, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and Create Texture
		m_AssetRegistry.insert({ newHandle, newAsset });
		SerializeAssetRegistry(); // Update Registry File on Disk

		// Fill in-memory cache if appropriate
		if (m_Flags.test(AssetManagerOptions::HasAssetCache))
		{
			std::filesystem::path assetPath = Projects::ProjectService::GetActiveIntermediateDirectory() / newAsset.Data.IntermediateLocation;
			m_AssetCache.insert({ newHandle, DeserializeAsset(newAsset, assetPath) });
		}

		Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>(newHandle, AssetType::Texture, Events::ManageAssetAction::Create);
		EngineService::SubmitToEventQueue(event);
		return newHandle;

	}

	void Texture2DManager::CreateTextureIntermediateFromBuffer(Buffer buffer, int32_t width, int32_t height, int32_t channels, Assets::Asset& newAsset)
	{
		// Save Binary Intermediate into File
		std::filesystem::path intermediateFullPath = Projects::ProjectService::GetActiveIntermediateDirectory() / newAsset.Data.IntermediateLocation;
		Utility::FileSystem::WriteFileBinary(intermediateFullPath, buffer);

		// Load data into In-Memory Metadata object
		Ref<Assets::TextureMetaData> metadata = CreateRef<Assets::TextureMetaData>();
		metadata->Width = width;
		metadata->Height = height;
		metadata->Channels = channels;
		newAsset.Data.SpecificFileData = metadata;
	}

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

	void Texture2DManager::CreateAssetFileFromName(const std::string& name, Asset& asset, const std::filesystem::path& assetPath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		out << YAML::Key << "Name" << YAML::Value << name; // Output texture name
		out << YAML::EndMap; // End of File Map

		std::ofstream fout(assetPath);
		fout << out.c_str();
		KG_INFO("Successfully created texture inside asset directory at {}", assetPath);
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

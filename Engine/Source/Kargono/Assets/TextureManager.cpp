#include "kgpch.h"

#include "Kargono/Assets/Asset.h"
#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/FileSystem.h"
#include "API/Serialization/SerializationAPI.h"

#include "stb_image.h"

namespace Kargono::Assets
{
	std::unordered_map<AssetHandle, Assets::Asset> AssetManager::s_TextureRegistry {};
	std::unordered_map<AssetHandle, Ref<Texture2D>> AssetManager::s_Textures {};

	void AssetManager::DeserializeTextureRegistry()
	{
		// Clear current registry and open registry in current project 
		s_TextureRegistry.clear();
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& textureRegistryLocation = Projects::Project::GetAssetDirectory() / "Textures/Intermediates/TextureRegistry.kgreg";

		if (!std::filesystem::exists(textureRegistryLocation))
		{
			KG_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(textureRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgscene file '{0}'\n     {1}", textureRegistryLocation.string(), e.what());
			return;
		}

		// Opening registry node 
		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_INFO("Deserializing Texture Registry");

		// Opening all assets 
		auto assets = data["Assets"];
		if (assets)
		{
			for (auto asset : assets)
			{
				Assets::Asset newAsset{};
				newAsset.Handle = asset["AssetHandle"].as<uint64_t>();

				// Retrieving metadata for asset 
				auto metadata = asset["MetaData"];
				newAsset.Data.CheckSum = metadata["CheckSum"].as<std::string>();
				newAsset.Data.IntermediateLocation = metadata["IntermediateLocation"].as<std::string>();
				newAsset.Data.Type = Utility::StringToAssetType(metadata["AssetType"].as<std::string>());

				// Retrieving texture specific metadata 
				if (newAsset.Data.Type == Assets::Texture)
				{
					Ref<Assets::TextureMetaData> texMetaData = CreateRef<Assets::TextureMetaData>();

					texMetaData->Height = metadata["TextureHeight"].as<int32_t>();
					texMetaData->Width = metadata["TextureWidth"].as<int32_t>();
					texMetaData->Channels = metadata["TextureChannels"].as<int32_t>();
					texMetaData->InitialFileLocation = metadata["InitialFileLocation"].as<std::string>();

					newAsset.Data.SpecificFileData = texMetaData;
				}

				// Add asset to in memory registry 
				s_TextureRegistry.insert({ newAsset.Handle, newAsset });

			}
		}
	}

	void AssetManager::SerializeTextureRegistry()
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& textureRegistryLocation = Projects::Project::GetAssetDirectory() / "Textures/Intermediates/TextureRegistry.kgreg";
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Registry" << YAML::Value << "Untitled";
		out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;

		// Asset
		for (auto& [handle, asset] : s_TextureRegistry)
		{
			out << YAML::BeginMap; // Asset Map
			out << YAML::Key << "AssetHandle" << YAML::Value << static_cast<uint64_t>(handle);

			out << YAML::Key << "MetaData" << YAML::Value;
			out << YAML::BeginMap; // MetaData Map
			out << YAML::Key << "CheckSum" << YAML::Value << asset.Data.CheckSum;
			out << YAML::Key << "IntermediateLocation" << YAML::Value << asset.Data.IntermediateLocation.string();
			out << YAML::Key << "AssetType" << YAML::Value << Utility::AssetTypeToString(asset.Data.Type);

			if (asset.Data.Type == Assets::AssetType::Texture)
			{
				Assets::TextureMetaData* metadata = static_cast<Assets::TextureMetaData*>(asset.Data.SpecificFileData.get());
				out << YAML::Key << "TextureHeight" << YAML::Value << metadata->Height;
				out << YAML::Key << "TextureWidth" << YAML::Value << metadata->Width;
				out << YAML::Key << "TextureChannels" << YAML::Value << metadata->Channels;
				out << YAML::Key << "InitialFileLocation" << YAML::Value << metadata->InitialFileLocation.string();
			}

			out << YAML::EndMap; // MetaData Map

			out << YAML::EndMap; // Asset Map
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		Utility::FileSystem::CreateNewDirectory(textureRegistryLocation.parent_path());

		std::ofstream fout(textureRegistryLocation);
		fout << out.c_str();
	}

	AssetHandle AssetManager::ImportNewTextureFromFile(const std::filesystem::path& filePath)
	{
		// Create Checksum
		std::string currentCheckSum = Utility::FileSystem::ChecksumFromFile(filePath);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		// Compare currentChecksum to registered assets
		bool isAssetDuplicate = false;
		AssetHandle currentHandle{};
		for (const auto& [handle, asset] : s_TextureRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				isAssetDuplicate = true;
				currentHandle = handle;
				break;
			}
		}

		if (isAssetDuplicate)
		{
			KG_ERROR("THERE IS A DUPLICATE!");
			return currentHandle;
		}

		// Create New Asset/Handle
		AssetHandle newHandle{};

		Assets::Asset newAsset{};
		newAsset.Handle = newHandle;

		// Create Intermediate
		CreateTextureIntermediateFromFile(filePath, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and Create Texture
		s_TextureRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeTextureRegistry(); // Update Registry File on Disk

		s_Textures.insert({ newHandle, InstantiateTextureIntoMemory(newAsset) });

		return newHandle;

	}

	AssetHandle AssetManager::ImportNewTextureFromData(Buffer buffer, int32_t width, int32_t height, int32_t channels)
	{
		// Create Checksum
		std::string currentCheckSum = Utility::FileSystem::ChecksumFromBuffer(buffer);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		// Compare currentChecksum to registered assets
		bool isAssetDuplicate = false;
		AssetHandle currentHandle{};
		for (const auto& [handle, asset] : s_TextureRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				isAssetDuplicate = true;
				currentHandle = handle;
				break;
			}
		}

		if (isAssetDuplicate)
		{
			//KG_ERROR("THERE IS A DUPLICATE!");
			return currentHandle;
		}

		// Create New Asset/Handle
		AssetHandle newHandle{};

		Assets::Asset newAsset{};
		newAsset.Handle = newHandle;

		// Create Intermediate
		CreateTextureIntermediateFromBuffer(buffer, width, height, channels, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and Create Texture
		s_TextureRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeTextureRegistry(); // Update Registry File on Disk

		s_Textures.insert({ newHandle, InstantiateTextureIntoMemory(newAsset) });

		return newHandle;

	}

	void AssetManager::CreateTextureIntermediateFromFile(const std::filesystem::path& filePath, Assets::Asset& newAsset)
	{
		// Create Texture Binary Intermediate
		int32_t width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		Buffer buffer{};
		stbi_uc* data = nullptr;
		{
			data = stbi_load(filePath.string().c_str(), &width, &height, &channels, 0);
		}

		buffer.Allocate(static_cast<unsigned long long>(width) * height * channels * sizeof(uint8_t));
		buffer.Data = data;

		// Save Binary Intermediate into File
		std::string intermediatePath = "Textures/Intermediates/" + (std::string)newAsset.Handle + ".kgtexture";
		std::filesystem::path intermediateFullPath = Projects::Project::GetAssetDirectory() / intermediatePath;
		Utility::FileSystem::WriteFileBinary(intermediateFullPath, buffer);

		// Check that save was successful
		if (!data)
		{
			KG_ERROR("Failed to load data from file in texture importer!");
			buffer.Release();
			return;
		}

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::Texture;
		newAsset.Data.IntermediateLocation = intermediatePath;
		Ref<Assets::TextureMetaData> metadata = CreateRef<Assets::TextureMetaData>();
		metadata->Width = width;
		metadata->Height = height;
		metadata->Channels = channels;
		metadata->InitialFileLocation = Utility::FileSystem::GetRelativePath(Projects::Project::GetAssetDirectory(), filePath);
		newAsset.Data.SpecificFileData = metadata;

		buffer.Release();

	}

	void AssetManager::CreateTextureIntermediateFromBuffer(Buffer buffer, int32_t width, int32_t height, int32_t channels, Assets::Asset& newAsset)
	{
		// Save Binary Intermediate into File
		std::string intermediatePath = "Textures/Intermediates/" + (std::string)newAsset.Handle + ".kgtexture";
		std::filesystem::path intermediateFullPath = Projects::Project::GetAssetDirectory() / intermediatePath;
		Utility::FileSystem::WriteFileBinary(intermediateFullPath, buffer);

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::Texture;
		newAsset.Data.IntermediateLocation = intermediatePath;
		Ref<Assets::TextureMetaData> metadata = CreateRef<Assets::TextureMetaData>();
		metadata->Width = width;
		metadata->Height = height;
		metadata->Channels = channels;
		metadata->InitialFileLocation = "None";
		newAsset.Data.SpecificFileData = metadata;
	}

	Ref<Texture2D> AssetManager::InstantiateTextureIntoMemory(Assets::Asset& asset)
	{
		Assets::TextureMetaData metadata = *static_cast<Assets::TextureMetaData*>(asset.Data.SpecificFileData.get());
		Buffer currentResource{};
		currentResource = Utility::FileSystem::ReadFileBinary(Projects::Project::GetAssetDirectory() / asset.Data.IntermediateLocation);
		Ref<Texture2D> newTexture = Texture2D::Create(currentResource, metadata);

		currentResource.Release();
		return newTexture;
	}

	Ref<Texture2D> AssetManager::GetTexture(const AssetHandle& handle)
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no active project when retreiving texture!");

		if (s_Textures.contains(handle)) { return s_Textures[handle]; }

		if (s_TextureRegistry.contains(handle))
		{
			auto asset = s_TextureRegistry[handle];

			Ref<Texture2D> newTexture = InstantiateTextureIntoMemory(asset);
			s_Textures.insert({ asset.Handle, newTexture });
			return newTexture;
		}

		KG_ERROR("No texture is associated with provided handle!");
		return nullptr;

	}

	void AssetManager::ClearTextureRegistry()
	{
		s_TextureRegistry.clear();
		s_Textures.clear();
	}

}

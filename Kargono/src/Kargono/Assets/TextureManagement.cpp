#include "kgpch.h"

#include "Kargono/Assets/AssetManager.h"

#include "Kargono/Core/FileSystem.h"
#include "Kargono/Project/Project.h"
#include "API/Serialization/SerializationAPI.h"

#include "stb_image.h"

namespace Kargono
{
	std::unordered_map<AssetHandle, Asset> AssetManager::s_TextureRegistry {};
	std::unordered_map<AssetHandle, Ref<Texture2D>> AssetManager::s_Textures {};

	void AssetManager::DeserializeTextureRegistry()
	{
		s_TextureRegistry.clear();
		KG_CORE_ASSERT(Project::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& textureRegistryLocation = Project::GetAssetDirectory() / "Textures/Intermediates/TextureRegistry.kgreg";

		if (!std::filesystem::exists(textureRegistryLocation))
		{
			KG_CORE_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(textureRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_CORE_ERROR("Failed to load .kargono file '{0}'\n     {1}", textureRegistryLocation.string(), e.what());
			return;
		}

		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_CORE_TRACE("Deserializing Registry");

		auto assets = data["Assets"];
		if (assets)
		{
			for (auto asset : assets)
			{
				Asset newAsset{};
				newAsset.Handle = asset["AssetHandle"].as<uint64_t>();

				auto metadata = asset["MetaData"];

				newAsset.Data.CheckSum = metadata["CheckSum"].as<std::string>();
				newAsset.Data.IntermediateLocation = metadata["IntermediateLocation"].as<std::string>();
				newAsset.Data.Type = Assets::StringToAssetType(metadata["AssetType"].as<std::string>());
				if (newAsset.Data.Type == Assets::Texture)
				{
					Ref<TextureMetaData> texMetaData = CreateRef<TextureMetaData>();

					texMetaData->Height = metadata["TextureHeight"].as<int32_t>();
					texMetaData->Width = metadata["TextureWidth"].as<int32_t>();
					texMetaData->Channels = metadata["TextureChannels"].as<int32_t>();
					texMetaData->InitialFileLocation = metadata["InitialFileLocation"].as<std::string>();

					newAsset.Data.SpecificFileData = texMetaData;
				}
				s_TextureRegistry.insert({ newAsset.Handle, newAsset });

			}
		}
	}

	void AssetManager::SerializeTextureRegistry()
	{
		KG_CORE_ASSERT(Project::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& textureRegistryLocation = Project::GetAssetDirectory() / "Textures/Intermediates/TextureRegistry.kgreg";
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
			out << YAML::Key << "AssetType" << YAML::Value << Assets::AssetTypeToString(asset.Data.Type);

			if (asset.Data.Type == Assets::AssetType::Texture)
			{
				TextureMetaData* metadata = static_cast<TextureMetaData*>(asset.Data.SpecificFileData.get());
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

		FileSystem::CreateNewDirectory(textureRegistryLocation.parent_path());

		std::ofstream fout(textureRegistryLocation);
		fout << out.c_str();
	}

	AssetHandle AssetManager::ImportNewTextureFromFile(const std::filesystem::path& filePath)
	{
		// Create Checksum
		std::string currentCheckSum = FileSystem::ChecksumFromFile(filePath);

		if (currentCheckSum.empty())
		{
			KG_CORE_ERROR("Failed to generate checksum from file!");
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
			KG_CORE_ERROR("THERE IS A DUPLICATE!");
			return currentHandle;
		}

		AssetHandle newHandle{};

		Asset newAsset{};
		newAsset.Handle = newHandle;

		CreateTextureIntermediateFromFile(filePath, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		s_TextureRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeTextureRegistry(); // Update Registry File on Disk

		s_Textures.insert({ newHandle, InstantiateTextureIntoMemory(newAsset) });

		return newHandle;

	}

	AssetHandle AssetManager::ImportNewTextureFromData(Buffer buffer, int32_t width, int32_t height, int32_t channels)
	{
		// Create Checksum
		std::string currentCheckSum = FileSystem::ChecksumFromBuffer(buffer);

		if (currentCheckSum.empty())
		{
			KG_CORE_ERROR("Failed to generate checksum from file!");
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
			//KG_CORE_ERROR("THERE IS A DUPLICATE!");
			return currentHandle;
		}

		AssetHandle newHandle{};

		Asset newAsset{};
		newAsset.Handle = newHandle;

		CreateTextureIntermediateFromBuffer(buffer, width, height, channels, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		s_TextureRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeTextureRegistry(); // Update Registry File on Disk

		s_Textures.insert({ newHandle, InstantiateTextureIntoMemory(newAsset) });

		return newHandle;

	}

	void AssetManager::CreateTextureIntermediateFromFile(const std::filesystem::path& filePath, Asset& newAsset)
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
		std::filesystem::path intermediateFullPath = Project::GetAssetDirectory() / intermediatePath;
		FileSystem::WriteFileBinary(intermediateFullPath, buffer);

		// Check that save was successful
		if (!data)
		{
			KG_CORE_ERROR("Failed to load data from file in texture importer!");
			buffer.Release();
			return;
		}

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::Texture;
		newAsset.Data.IntermediateLocation = intermediatePath;
		Ref<TextureMetaData> metadata = CreateRef<TextureMetaData>();
		metadata->Width = width;
		metadata->Height = height;
		metadata->Channels = channels;
		metadata->InitialFileLocation = FileSystem::GetRelativePath(Project::GetAssetDirectory(), filePath);
		newAsset.Data.SpecificFileData = metadata;

		buffer.Release();

	}

	void AssetManager::CreateTextureIntermediateFromBuffer(Buffer buffer, int32_t width, int32_t height, int32_t channels,  Asset& newAsset)
	{
		// Save Binary Intermediate into File
		std::string intermediatePath = "Textures/Intermediates/" + (std::string)newAsset.Handle + ".kgtexture";
		std::filesystem::path intermediateFullPath = Project::GetAssetDirectory() / intermediatePath;
		FileSystem::WriteFileBinary(intermediateFullPath, buffer);

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::Texture;
		newAsset.Data.IntermediateLocation = intermediatePath;
		Ref<TextureMetaData> metadata = CreateRef<TextureMetaData>();
		metadata->Width = width;
		metadata->Height = height;
		metadata->Channels = channels;
		metadata->InitialFileLocation = "None";
		newAsset.Data.SpecificFileData = metadata;
	}

	Ref<Texture2D> AssetManager::InstantiateTextureIntoMemory(Asset& asset)
	{
		TextureMetaData metadata = *static_cast<TextureMetaData*>(asset.Data.SpecificFileData.get());
		Buffer currentResource{};
		currentResource = FileSystem::ReadFileBinary(Project::GetAssetDirectory() / asset.Data.IntermediateLocation);
		Ref<Texture2D> newTexture = Texture2D::Create(currentResource, metadata);

		currentResource.Release();
		return newTexture;
	}

	Ref<Texture2D> AssetManager::GetTexture(const AssetHandle& handle)
	{
		KG_CORE_ASSERT(Project::GetActive(), "There is no active project when retreiving texture!");

		if (s_Textures.contains(handle)) { return s_Textures[handle]; }

		if (s_TextureRegistry.contains(handle))
		{
			auto asset = s_TextureRegistry[handle];

			Ref<Texture2D> newTexture = InstantiateTextureIntoMemory(asset);
			s_Textures.insert({ asset.Handle, newTexture });
			return newTexture;
		}

		KG_CORE_ERROR("No texture is associated with provided handle!");
		return nullptr;

	}

	void AssetManager::ClearTextureRegistry()
	{
		s_TextureRegistry.clear();
		s_Textures.clear();
	}

}

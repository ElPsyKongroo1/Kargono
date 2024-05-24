#include "kgpch.h"

#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/FileSystem.h"
#include "API/Serialization/yamlcppAPI.h"

#include "API/Audio/OpenALAPI.h"
#include "API/Audio/drwavAPI.h"

namespace Kargono::Assets
{

	std::unordered_map<AssetHandle, Assets::Asset> AssetManager::s_AudioRegistry {};
	std::unordered_map<AssetHandle, Ref<Audio::AudioBuffer>> AssetManager::s_Audio {};

	void AssetManager::DeserializeAudioRegistry()
	{
		// Clear current registry and open registry in current project 
		s_AudioRegistry.clear();
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& audioRegistryLocation = Projects::Project::GetAssetDirectory() / "Audio/Intermediates/AudioRegistry.kgreg";

		if (!std::filesystem::exists(audioRegistryLocation))
		{
			KG_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(audioRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgscene file '{0}'\n     {1}", audioRegistryLocation.string(), e.what());
			return;
		}
		// Opening registry node 
		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_INFO("Deserializing Audio Registry");
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

				// Retrieving audio specific metadata 
				if (newAsset.Data.Type == Assets::Audio)
				{
					Ref<Assets::AudioMetaData> audioMetaData = CreateRef<Assets::AudioMetaData>();

					audioMetaData->Channels = metadata["Channels"].as<uint32_t>();
					audioMetaData->SampleRate = metadata["SampleRate"].as<uint32_t>();
					audioMetaData->TotalPcmFrameCount = metadata["TotalPcmFrameCount"].as<uint64_t>();
					audioMetaData->TotalSize = metadata["TotalSize"].as<uint64_t>();
					audioMetaData->InitialFileLocation = metadata["InitialFileLocation"].as<std::string>();

					newAsset.Data.SpecificFileData = audioMetaData;
				}

				// Add asset to in memory registry 
				s_AudioRegistry.insert({ newAsset.Handle, newAsset });

			}
		}
	}

	void AssetManager::SerializeAudioRegistry()
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& audioRegistryLocation = Projects::Project::GetAssetDirectory() / "Audio/Intermediates/AudioRegistry.kgreg";
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Registry" << YAML::Value << "Audio";
		out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;

		// Asset
		for (auto& [handle, asset] : s_AudioRegistry)
		{
			out << YAML::BeginMap; // Asset Map
			out << YAML::Key << "AssetHandle" << YAML::Value << static_cast<uint64_t>(handle);

			out << YAML::Key << "MetaData" << YAML::Value;
			out << YAML::BeginMap; // MetaData Map
			out << YAML::Key << "CheckSum" << YAML::Value << asset.Data.CheckSum;
			out << YAML::Key << "IntermediateLocation" << YAML::Value << asset.Data.IntermediateLocation.string();
			out << YAML::Key << "AssetType" << YAML::Value << Utility::AssetTypeToString(asset.Data.Type);

			if (asset.Data.Type == Assets::AssetType::Audio)
			{
				Assets::AudioMetaData* metadata = static_cast<Assets::AudioMetaData*>(asset.Data.SpecificFileData.get());
				out << YAML::Key << "Channels" << YAML::Value << metadata->Channels;
				out << YAML::Key << "SampleRate" << YAML::Value << metadata->SampleRate;
				out << YAML::Key << "TotalPcmFrameCount" << YAML::Value << metadata->TotalPcmFrameCount;
				out << YAML::Key << "TotalSize" << YAML::Value << metadata->TotalSize;
				out << YAML::Key << "InitialFileLocation" << YAML::Value << metadata->InitialFileLocation.string();
			}

			out << YAML::EndMap; // MetaData Map
			out << YAML::EndMap; // Asset Map
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		Utility::FileSystem::CreateNewDirectory(audioRegistryLocation.parent_path());

		std::ofstream fout(audioRegistryLocation);
		fout << out.c_str();
	}

	AssetHandle AssetManager::ImportNewAudioFromFile(const std::filesystem::path& filePath)
	{
		// Create Checksum
		const std::string currentCheckSum = Utility::FileSystem::ChecksumFromFile(filePath);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		for (const auto& [handle, asset] : s_AudioRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate audio asset");
				return handle;
			}
		}

		// Create New Asset/Handle
		AssetHandle newHandle{};

		Assets::Asset newAsset{};
		newAsset.Handle = newHandle;

		// Create Intermediate
		CreateAudioIntermediateFromFile(filePath, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and Create Texture
		s_AudioRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeAudioRegistry(); // Update Registry File on Disk

		s_Audio.insert({ newHandle, InstantiateAudioIntoMemory(newAsset) });

		return newHandle;
	}

	Ref<Audio::AudioBuffer> AssetManager::InstantiateAudioIntoMemory(Assets::Asset& asset)
	{
		Assets::AudioMetaData metadata = *static_cast<Assets::AudioMetaData*>(asset.Data.SpecificFileData.get());
		Buffer currentResource{};
		currentResource = Utility::FileSystem::ReadFileBinary(Projects::Project::GetAssetDirectory() / asset.Data.IntermediateLocation);
		Ref<Audio::AudioBuffer> newAudio = CreateRef<Audio::AudioBuffer>();
		alec(alBufferData(newAudio->m_BufferID, metadata.Channels > 1 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, currentResource.Data, static_cast<ALsizei>(currentResource.Size), metadata.SampleRate));

		currentResource.Release();
		return newAudio;
	}

	Ref<Audio::AudioBuffer> AssetManager::GetAudio(const AssetHandle& handle)
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no active project when retreiving audio!");

		if (s_Audio.contains(handle)) { return s_Audio[handle]; }

		if (s_AudioRegistry.contains(handle))
		{
			auto asset = s_AudioRegistry[handle];

			Ref<Audio::AudioBuffer> newAudio = InstantiateAudioIntoMemory(asset);
			s_Audio.insert({ asset.Handle, newAudio });
			return newAudio;
		}

		KG_ERROR("No audio is associated with provided handle!");
		return nullptr;
	}

	std::tuple<AssetHandle, Ref<Audio::AudioBuffer>> AssetManager::GetAudio(const std::filesystem::path& filepath)
	{
		KG_ASSERT(Projects::Project::GetActive(), "Attempt to use Project Field without active project!");

		for (auto& [assetHandle, asset] : s_AudioRegistry)
		{
			auto metadata = (Assets::AudioMetaData*)asset.Data.SpecificFileData.get();
			if (metadata->InitialFileLocation.compare(filepath) == 0)
			{
				return std::make_tuple(assetHandle, GetAudio(assetHandle));
			}
		}
		// Return empty audio if audio does not exist
		KG_INFO("Invalid filepath provided to GetAudio {}", filepath.string());
		return std::make_tuple(0, nullptr);
	}

	void AssetManager::ClearAudioRegistry()
	{
		s_AudioRegistry.clear();
		s_Audio.clear();
	}

	void AssetManager::CreateAudioIntermediateFromFile(const std::filesystem::path& filePath, Assets::Asset& newAsset)
	{
		// Create Buffers
		uint32_t channels = 0;
		uint32_t sampleRate = 0;
		uint64_t totalPcmFrameCount = 0;
		uint64_t totalSize = 0;
		Buffer pcmData{};
		drwav_int16* pSampleData = drwav_open_file_and_read_pcm_frames_s16(filePath.string().c_str(), &channels, &sampleRate, &totalPcmFrameCount, nullptr);
		if (!pSampleData)
		{
			KG_ERROR("Failed to load audio file");
			drwav_free(pSampleData, nullptr);
			return;
		}
		totalSize = totalPcmFrameCount * channels * 2;
		if ((totalSize) > drwav_uint64(std::numeric_limits<size_t>::max()))
		{
			KG_ERROR("Too much data in file for 32bit addressed vector");
			drwav_free(pSampleData, nullptr);
			return;
		}
		pcmData.Allocate(size_t(totalSize));
		std::memcpy(pcmData.Data, pSampleData, pcmData.Size /*two bytes in s16*/);
		drwav_free(pSampleData, nullptr);

		// Save Binary Intermediate into File
		std::string intermediatePath = "Audio/Intermediates/" + (std::string)newAsset.Handle + ".kgaudio";
		std::filesystem::path intermediateFullPath = Projects::Project::GetAssetDirectory() / intermediatePath;
		Utility::FileSystem::WriteFileBinary(intermediateFullPath, pcmData);

		// Check that save was successful
		if (!pcmData)
		{
			KG_ERROR("Failed to load data from file in audio importer!");
			return;
		}

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::Audio;
		newAsset.Data.IntermediateLocation = intermediatePath;
		Ref<Assets::AudioMetaData> metadata = CreateRef<Assets::AudioMetaData>();
		metadata->Channels = channels;
		metadata->SampleRate = sampleRate;;
		metadata->TotalPcmFrameCount = totalPcmFrameCount;
		metadata->TotalSize = totalSize;
		metadata->InitialFileLocation = Utility::FileSystem::GetRelativePath(Projects::Project::GetAssetDirectory(), filePath);
		newAsset.Data.SpecificFileData = metadata;
		pcmData.Release();
	}
}

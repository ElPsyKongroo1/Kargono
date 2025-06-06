#include "kgpch.h"

#include "Kargono/Assets/AssetService.h"
#include "Kargono/Assets/AudioManager.h"

#include "API/Audio/OpenALAPI.h"
#include "API/Audio/drwavAPI.h"


namespace Kargono::Assets
{
	Ref<Audio::AudioBuffer> AudioBufferManager::DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		Assets::AudioMetaData metadata = *asset.Data.GetSpecificMetaData<Assets::AudioMetaData>();
		Buffer currentResource{};
		currentResource = Utility::FileSystem::ReadFileBinary(assetPath);
		Ref<Audio::AudioBuffer> newAudio = CreateRef<Audio::AudioBuffer>();
		CallAndCheckALError(alBufferData(newAudio->m_BufferID, metadata.Channels > 1 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, currentResource.Data, static_cast<ALsizei>(currentResource.Size), metadata.SampleRate));
		currentResource.Release();
		return newAudio;
	}

	void AudioBufferManager::SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset)
	{
		Assets::AudioMetaData* metadata = currentAsset.Data.GetSpecificMetaData<AudioMetaData>();
		serializer << YAML::Key << "Channels" << YAML::Value << metadata->Channels;
		serializer << YAML::Key << "SampleRate" << YAML::Value << metadata->SampleRate;
		serializer << YAML::Key << "TotalPcmFrameCount" << YAML::Value << metadata->TotalPcmFrameCount;
		serializer << YAML::Key << "TotalSize" << YAML::Value << metadata->TotalSize;
	}

	void AudioBufferManager::CreateAssetFileFromName(std::string_view name, AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		UNREFERENCED_PARAMETER(asset);

		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		out << YAML::Key << "Name" << YAML::Value << std::string(name); // Output audio name
		out << YAML::EndMap; // End of File Map

		std::ofstream fout(assetPath);
		fout << out.c_str();
		KG_INFO("Successfully created audio inside asset directory at {}", assetPath);
	}

	void AudioBufferManager::CreateAssetIntermediateFromFile(AssetInfo& newAsset, const std::filesystem::path& fullFileLocation, const std::filesystem::path& fullIntermediateLocation)
	{
		// Create Buffers
		uint32_t channels = 0;
		uint32_t sampleRate = 0;
		drwav_uint64 totalPcmFrameCount = 0;
		drwav_uint64 totalSize = 0;
		Buffer pcmData{};
		drwav_int16* pSampleData = drwav_open_file_and_read_pcm_frames_s16(fullFileLocation.string().c_str(), &channels, &sampleRate, &totalPcmFrameCount, nullptr);
		if (!pSampleData)
		{
			KG_WARN("Failed to load audio file");
			drwav_free(pSampleData, nullptr);
			return;
		}
		totalSize = totalPcmFrameCount * channels * 2;
		if ((totalSize) > drwav_uint64(std::numeric_limits<size_t>::max()))
		{
			KG_WARN("Too much data in file for 32bit addressed vector");
			drwav_free(pSampleData, nullptr);
			return;
		}
		pcmData.Allocate(size_t(totalSize));
		std::memcpy(pcmData.Data, pSampleData, pcmData.Size /*two bytes in s16*/);
		drwav_free(pSampleData, nullptr);

		// Save Binary Intermediate into File
		Utility::FileSystem::WriteFileBinary(fullIntermediateLocation, pcmData);

		// Check that save was successful
		if (!pcmData)
		{
			KG_WARN("Failed to load data from file in audio importer!");
			return;
		}

		// Load data into In-Memory Metadata object
		Ref<Assets::AudioMetaData> metadata = CreateRef<Assets::AudioMetaData>();
		metadata->Channels = channels;
		metadata->SampleRate = sampleRate;
		metadata->TotalPcmFrameCount = totalPcmFrameCount;
		metadata->TotalSize = totalSize;
		newAsset.Data.SpecificFileData = metadata;
		pcmData.Release();
	}

	void AudioBufferManager::DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::AssetInfo& currentAsset)
	{
		Ref<Assets::AudioMetaData> audioMetaData = CreateRef<Assets::AudioMetaData>();
		audioMetaData->Channels = metadataNode["Channels"].as<uint32_t>();
		audioMetaData->SampleRate = metadataNode["SampleRate"].as<uint32_t>();
		audioMetaData->TotalPcmFrameCount = metadataNode["TotalPcmFrameCount"].as<uint64_t>();
		audioMetaData->TotalSize = metadataNode["TotalSize"].as<uint64_t>();
		currentAsset.Data.SpecificFileData = audioMetaData;
	}
}

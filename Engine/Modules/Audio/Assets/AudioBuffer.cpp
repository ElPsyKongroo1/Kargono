#include "kgpch.h"

#include "Modules/Audio/Assets/AudioBuffer.h"
#include "Kargono/Core/Buffer.h"

#include "Modules/Audio/ExternalAPI/OpenALAPI.h"
#include "Modules/Audio/ExternalAPI/drwavAPI.h"
#include "API/Serialization/yamlcppAPI.h"

namespace Kargono::Audio
{
	void AudioBufferMetaData::Serialize(void* context)
	{
		// Get asset context
		KG_ASSERT(context, "Context cannot be null");
		Assets::SerializeMetaDataContext<AudioBuffer>& metadataContext = *(Assets::SerializeMetaDataContext<AudioBuffer>*)context;

		// Get context fields
		YAML::Emitter& emitter = *metadataContext.m_Serializer;

		emitter << YAML::Key << "Channels" << YAML::Value << m_Channels;
		emitter << YAML::Key << "SampleRate" << YAML::Value << m_SampleRate;
		emitter << YAML::Key << "TotalPcmFrameCount" << YAML::Value << m_TotalPcmFrameCount;
		emitter << YAML::Key << "TotalSize" << YAML::Value << m_TotalSize;
	}

	void AudioBufferMetaData::Deserialize(void* context)
	{
		// Get asset context
		KG_ASSERT(context, "Context cannot be null");
		Assets::DeserializeMetaDataContext<AudioBuffer>& assetContext = *(Assets::DeserializeMetaDataContext<AudioBuffer>*)context;

		// Get context fields
		YAML::Node& metadataNode = *assetContext.m_Node;

		// Read metadata
		m_Channels = metadataNode["Channels"].as<uint32_t>();
		m_SampleRate = metadataNode["SampleRate"].as<uint32_t>();
		m_TotalPcmFrameCount = metadataNode["TotalPcmFrameCount"].as<uint64_t>();
		m_TotalSize = metadataNode["TotalSize"].as<uint64_t>();
	}

	void AudioBuffer::CreateFromFile(Assets::Metadata<AudioBuffer>& metadata,
		std::filesystem::path& sourceFile)
	{
		// Get intermediate location
		const FixedBufStr16& intermediateExtension
		{
			AudioBuffer::GetIntermediateExtensions().front()
		};
		std::filesystem::path intermediateLocation
		{
			metadata.GetAssetFullIntermediatePath(intermediateExtension.StringView())
		};

		// Create buffers
		uint32_t channels = 0;
		uint32_t sampleRate = 0;
		drwav_uint64 totalPcmFrameCount = 0;
		drwav_uint64 totalSize = 0;
		Buffer pcmData{};
		drwav_int16* pSampleData = drwav_open_file_and_read_pcm_frames_s16(sourceFile.string().c_str(), &channels, &sampleRate, &totalPcmFrameCount, nullptr);
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
		std::memcpy(pcmData.m_Data, pSampleData, pcmData.m_Size /*two bytes in s16*/);
		drwav_free(pSampleData, nullptr);

		// Save binary intermediate into file
		Utility::FileSystem::WriteFileBinary(intermediateLocation, pcmData);

		// Check that save was successful
		if (!pcmData)
		{
			KG_WARN("Failed to load data from file in audio importer!");
			return;
		}

		// Load data into in-memory metadata object
		AudioBufferMetaData& audioMetadata{ metadata.GetMetadataExtension() };
		audioMetadata.m_Channels = channels;
		audioMetadata.m_SampleRate = sampleRate;
		audioMetadata.m_TotalPcmFrameCount = totalPcmFrameCount;
		audioMetadata.m_TotalSize = totalSize;
		pcmData.Release();
	}

	AudioBuffer::AudioBuffer()
	{
		CallAndCheckALError(alGenBuffers(1, &m_BufferID));
	}

	AudioBuffer::~AudioBuffer()
	{
		CallAndCheckALError(alDeleteBuffers(1, &(m_BufferID)));
	}
	void AudioBuffer::Serialize(void* context) 
	{
		KG_ERROR("Serialization not implemented for audio buffer");
	}
	void AudioBuffer::Deserialize(void* context)
	{
		// Get asset context
		KG_ASSERT(context, "Context cannot be null");
		Assets::DeserializeAssetContext<AudioBuffer>& assetContext = *(Assets::DeserializeAssetContext<AudioBuffer>*)context;

		// Get context fields
		KG_ASSERT(assetContext.m_AssetMetadata, "Metadata cannot be null");
		Assets::Metadata<AudioBuffer>& metadata{ *assetContext.m_AssetMetadata };
		// Get intermediate location
		const FixedBufStr16& intermediateExtension
		{
			AudioBuffer::GetIntermediateExtensions().front()
		};
		std::filesystem::path intermediateLocation
		{
			metadata.GetAssetFullIntermediatePath(intermediateExtension.StringView())
		};

		// Get specific metadata
		AudioBufferMetaData& audioBufferMetadata = metadata.GetMetadataExtension();

		// Load audio data from file
		Buffer currentResource{};
		currentResource = Utility::FileSystem::ReadFileBinary(intermediateLocation);

		// Upload audio data to OpenAL
		CallAndCheckALError(alBufferData(m_BufferID, audioBufferMetadata.m_Channels > 1 ?
			AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, currentResource.m_Data, 
			static_cast<ALsizei>(currentResource.m_Size), audioBufferMetadata.m_SampleRate));
		currentResource.Release();
	}
}
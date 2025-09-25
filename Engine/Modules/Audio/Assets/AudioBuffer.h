#pragma once

#include "Modules/Audio/Module/AudioModule.h"
#include "Modules/Assets/Module/AssetTag.h"

#include <cstdint>
#include <limits>

namespace Kargono::Audio
{
	struct AudioMetaData
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		AudioMetaData() = default;
		~AudioMetaData() = default;
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
		uint32_t m_Channels{ 0 };
		uint32_t m_SampleRate{ 0 };
		uint64_t m_TotalPcmFrameCount{ 0 };
		uint64_t m_TotalSize{ 0 };
	};

	using BufferID = uint32_t;
	constexpr BufferID k_InvalidBufferID { 0 }; // TODO: Maybe change to upperbound??

	class AudioBuffer
	{
	public:
		//==============================
		// Metaprogramming Info
		//==============================
		using Metadata = AudioMetaData;
	public:
		//==============================
		// Static Asset Functions
		//==============================
		constexpr static Assets::AssetConfig GetAssetConfig()
		{
			Assets::AssetConfig config{};
			config.m_Identifier = Assets::GetAssetIdentifier<AudioBuffer>();
			config.m_Name = "Audio";
			config.m_FileExtension = ".kgaudio";
			config.m_ImportExtensions = { ".wav" };
			config.m_RegistryPath = "AudioBuffer/AudioRegistry.kgreg";
			config.m_IntermediateExtension = "";
			config.m_Flags.SetFlag(Assets::AssetFlags::HasAssetCache);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasIntermediateLocation);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasFileLocation);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasFileImporting);
			config.m_Flags.ClearFlag(Assets::AssetFlags::HasAssetSaving);
			config.m_Flags.ClearFlag(Assets::AssetFlags::HasAssetCreationFromName);
			return config;
		}
		static void CreateAssetFileFromName(std::string_view name, Assets::Metadata& metadata, std::filesystem::path& path);
		static void CreateAssetIntermediateFromFile(std::string_view name, Assets::Metadata& metadata, std::filesystem::path& path);
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		AudioBuffer();
		~AudioBuffer();
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
		BufferID m_BufferID{ k_InvalidBufferID };
	};

	Register_Module_Tag(AudioBuffer, Assets::AssetTag)
}
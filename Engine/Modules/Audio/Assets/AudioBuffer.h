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
		constexpr static std::array<FixedBufStr16, 1> k_ImportExtensions{ ".wav" };
		constexpr static std::array<FixedBufStr16, 1> k_IntermediateExtensions{ ".kgbinary" };
	public:
		//==============================
		// Asset Config Info
		//==============================
		constexpr static FixedBufStr32 GetAssetName()
		{
			return "Audio Buffer";
		}

		constexpr static Assets::AssetFlags GetAssetFlags()
		{
			Assets::AssetFlags flags{};
			flags.SetFlag(Assets::AssetFlag::HasAssetCache);
			flags.ClearFlag(Assets::AssetFlag::RequireUniqueName);
			return flags;
		}

		constexpr static FixedBufStr16 GetFileExtension()
		{
			return ".kgaudio";
		}

		constexpr static std::span<const FixedBufStr16> GetImportExtensions()
		{
			return std::span(k_ImportExtensions.data(), k_ImportExtensions.size());
		}

		constexpr static std::span<const FixedBufStr16> GetIntermediateExtensions()
		{
			return std::span(k_IntermediateExtensions.data(), k_IntermediateExtensions.size());
		}

		static void CreateAssetFilesFromFile(Assets::Metadata& metadata, std::filesystem::path& sourcePath);
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
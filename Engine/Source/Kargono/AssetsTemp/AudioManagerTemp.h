#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class AudioBufferManager : public AssetManagerTemp<Audio::AudioBuffer>
	{
	public:
		AudioBufferManager() : AssetManagerTemp<Audio::AudioBuffer>()
		{
			m_AssetName = "Audio";
			m_FileExtension = ".kgaudio";
			m_ValidImportFileExtensions = { ".wav" };
			m_Flags.set(AssetManagerOptions::HasAssetCache, true);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, true);
		}
		virtual ~AudioBufferManager() = default;
	public:
		// Functions specific to this manager type
		virtual Ref<Audio::AudioBuffer> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}

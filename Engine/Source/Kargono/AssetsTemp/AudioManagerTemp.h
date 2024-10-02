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
			m_AssetType = AssetType::Audio;
			m_FileExtension = ".kgaudio";
			m_ValidImportFileExtensions = { ".wav" };
			m_RegistryLocation = "Audio/Intermediates/AudioRegistry.kgreg";
			m_Flags.set(AssetManagerOptions::HasAssetCache, true);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, true);
			m_Flags.set(AssetManagerOptions::HasAssetSaving, false);
			m_Flags.set(AssetManagerOptions::HasAssetCreationFromName, false);
		}
		virtual ~AudioBufferManager() = default;
	public:
		// Functions specific to this manager type
		virtual Ref<Audio::AudioBuffer> DeserializeAsset(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::Asset& currentAsset) override;
		virtual void CreateAssetIntermediateFromFile(Asset& newAsset, const std::filesystem::path& fullFileLocation, const std::filesystem::path& fullIntermediateLocation) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::Asset& currentAsset) override;
	};
}

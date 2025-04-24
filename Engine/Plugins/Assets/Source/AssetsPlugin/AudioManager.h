#pragma once
#include "AssetsPlugin/AssetManager.h"

namespace Kargono::Audio { class AudioBuffer; }

namespace Kargono::Assets
{
	class AudioBufferManager : public AssetManager<Audio::AudioBuffer>
	{
	public:
		AudioBufferManager() : AssetManager<Audio::AudioBuffer>()
		{
			m_AssetName = "Audio";
			m_AssetType = AssetType::Audio;
			m_FileExtension = ".kgaudio";
			m_ValidImportFileExtensions = { ".wav" };
			m_RegistryLocation = "AudioBuffer/AudioRegistry.kgreg";
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
		virtual Ref<Audio::AudioBuffer> DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset) override;
		virtual void CreateAssetFileFromName(std::string_view name, AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void CreateAssetIntermediateFromFile(AssetInfo& newAsset, const std::filesystem::path& fullFileLocation, const std::filesystem::path& fullIntermediateLocation) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::AssetInfo& currentAsset) override;
	};
}

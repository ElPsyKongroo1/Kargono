#pragma once
#include "AssetModule/AssetManager.h"

namespace Kargono::Scenes { class GameState; }

namespace Kargono::Assets
{
	class GameStateManager : public AssetManager<Scenes::GameState>
	{
	public:
		GameStateManager() : AssetManager<Scenes::GameState>()
		{
			m_AssetName = "Game State";
			m_AssetType = AssetType::GameState;
			m_FileExtension = ".kgstate";
			m_RegistryLocation = "GameState/GameStateRegistry.kgreg";
			m_Flags.set(AssetManagerOptions::HasAssetCache, false);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
			m_Flags.set(AssetManagerOptions::HasAssetSaving, true);
			m_Flags.set(AssetManagerOptions::HasAssetCreationFromName, true);
		}
		virtual ~GameStateManager() = default;
	public:

		// Class specific functions
		virtual void CreateAssetFileFromName(std::string_view name, AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAsset(Ref<Scenes::GameState> assetReference, const std::filesystem::path& assetPath) override;
		virtual Ref<Scenes::GameState> DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::AssetInfo& currentAsset) override;
	};
}

#pragma once
#include "Modules/Assets/AssetManager.h"

namespace Kargono::AI { struct AIState; }

namespace Kargono::Assets
{
	class AIStateManager : public AssetManager<AI::AIState>
	{
	public:
		AIStateManager() : AssetManager<AI::AIState>()
		{
			m_AssetName = "AI State";
			m_AssetType = AssetType::AIState;
			m_FileExtension = ".kgaistate";
			m_RegistryLocation = "AIState/AIStateRegistry.kgreg";
			m_Flags.set(AssetManagerOptions::HasAssetCache, true);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
			m_Flags.set(AssetManagerOptions::HasAssetSaving, true);
			m_Flags.set(AssetManagerOptions::HasAssetCreationFromName, true);
		}
		virtual ~AIStateManager() = default;
	public:

		// Class specific functions
		virtual void CreateAssetFileFromName(std::string_view name, AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAsset(Ref<AI::AIState> assetReference, const std::filesystem::path& assetPath) override;
		virtual Ref<AI::AIState> DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void DeleteAssetValidation(AssetHandle assetHandle) override;

		bool RemoveScript(Ref<AI::AIState> aiStateRef, Assets::AssetHandle scriptHandle);
	};
}

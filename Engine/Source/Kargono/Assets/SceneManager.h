#pragma once
#include "Kargono/Assets/AssetManager.h"

namespace Kargono::Scenes { class Scene; }

namespace Kargono::Assets
{
	class SceneManager : public AssetManager<Scenes::Scene>
	{
	public:
		SceneManager() : AssetManager<Scenes::Scene>()
		{
			m_AssetName = "Scene";
			m_AssetType = AssetType::Scene;
			m_FileExtension = ".kgscene";
			m_RegistryLocation = "Scene/SceneRegistry.kgreg";
			m_Flags.set(AssetManagerOptions::HasAssetCache, false);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
			m_Flags.set(AssetManagerOptions::HasAssetSaving, true);
			m_Flags.set(AssetManagerOptions::HasAssetCreationFromName, true);
		}
		virtual ~SceneManager() = default;
	public:

		// Class specific functions
		virtual void CreateAssetFileFromName(const std::string& name, AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAsset(Ref<Scenes::Scene> assetReference, const std::filesystem::path& assetPath) override;
		virtual Ref<Scenes::Scene> DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath) override;

		bool RemoveScript(Ref<Scenes::Scene> sceneRef, Assets::AssetHandle scriptHandle);
		bool RemoveAIState(Ref<Scenes::Scene> sceneRef, Assets::AssetHandle aiStateHandle);
		bool RemoveProjectComponent(Ref<Scenes::Scene> sceneRef, Assets::AssetHandle projectCompHandle);
	};
}

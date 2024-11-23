#pragma once
#include "Kargono/Assets/AssetManager.h"

namespace Kargono::Input { class InputMap; }

namespace Kargono::Assets
{
	class InputMapManager : public AssetManager<Input::InputMap>
	{
	public:
		InputMapManager() : AssetManager<Input::InputMap>()
		{
			m_AssetName = "Input Map";
			m_AssetType = AssetType::InputMap;
			m_FileExtension = ".kginput";
			m_RegistryLocation = "InputMap/InputMapRegistry.kgreg";
			m_Flags.set(AssetManagerOptions::HasAssetCache, false);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
			m_Flags.set(AssetManagerOptions::HasAssetSaving, true);
			m_Flags.set(AssetManagerOptions::HasAssetCreationFromName, true);
		}
		virtual ~InputMapManager() = default;
	public:

		// Class specific functions
		virtual void CreateAssetFileFromName(const std::string& name, AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAsset(Ref<Input::InputMap> assetReference, const std::filesystem::path& assetPath) override;
		virtual Ref<Input::InputMap> DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath) override;

		bool RemoveScript(Ref<Input::InputMap> inputMapRef, Assets::AssetHandle scriptHandle);
	};
}

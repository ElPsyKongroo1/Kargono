#pragma once
#include "Kargono/Assets/AssetManager.h"

namespace Kargono::Input { class InputMode; }

namespace Kargono::Assets
{
	class InputModeManager : public AssetManager<Input::InputMode>
	{
	public:
		InputModeManager() : AssetManager<Input::InputMode>()
		{
			m_AssetName = "Input Mode";
			m_AssetType = AssetType::InputMode;
			m_FileExtension = ".kginput";
			m_RegistryLocation = "InputMode/InputRegistry.kgreg";
			m_Flags.set(AssetManagerOptions::HasAssetCache, false);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
			m_Flags.set(AssetManagerOptions::HasAssetSaving, true);
			m_Flags.set(AssetManagerOptions::HasAssetCreationFromName, true);
		}
		virtual ~InputModeManager() = default;
	public:

		// Class specific functions
		virtual void CreateAssetFileFromName(const std::string& name, Asset& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAsset(Ref<Input::InputMode> assetReference, const std::filesystem::path& assetPath) override;
		virtual Ref<Input::InputMode> DeserializeAsset(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}

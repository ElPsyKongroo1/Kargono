#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class InputModeManager : public AssetManagerTemp<Input::InputMode>
	{
	public:
		InputModeManager() : AssetManagerTemp<Input::InputMode>()
		{
			m_AssetName = "Input Mode";
			m_AssetType = AssetType::InputMode;
			m_FileExtension = ".kginput";
			m_RegistryLocation = "Input/InputRegistry.kgreg";
			m_Flags.set(AssetManagerOptions::HasAssetCache, false);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
			m_Flags.set(AssetManagerOptions::HasAssetModification, true);
		}
		virtual ~InputModeManager() = default;
	public:

		// Class specific functions
		virtual Ref<Input::InputMode> DeserializeAsset(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::Asset& currentAsset) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::Asset& currentAsset) override;
	};
}

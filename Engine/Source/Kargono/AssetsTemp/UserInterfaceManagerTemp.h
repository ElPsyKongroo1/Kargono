#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class UserInterfaceManager : public AssetManagerTemp<RuntimeUI::UserInterface>
	{
	public:
		UserInterfaceManager() : AssetManagerTemp<RuntimeUI::UserInterface>()
		{
			m_AssetName = "User Interface";
			m_AssetType = AssetType::UserInterface;
			m_FileExtension = ".kgui";
			m_RegistryLocation = "UserInterface/UserInterfaceRegistry.kgreg";
			m_Flags.set(AssetManagerOptions::HasAssetCache, false);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
			m_Flags.set(AssetManagerOptions::HasAssetModification, true);
		}
		virtual ~UserInterfaceManager() = default;
	public:
		// Class specific functions
		virtual Ref<RuntimeUI::UserInterface> DeserializeAsset(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::Asset& currentAsset) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::Asset& currentAsset) override;
	};
}

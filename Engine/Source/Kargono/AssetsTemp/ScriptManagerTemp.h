#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class ScriptManager : public AssetManagerTemp<Scripting::Script>
	{
	public:
		ScriptManager() : AssetManagerTemp<Scripting::Script>()
		{
			m_AssetName = "Script";
			m_AssetType = AssetType::Script;
			m_FileExtension = ".kgscript";
			m_RegistryLocation = "Scripting/ScriptRegistry.kgreg";
			m_Flags.set(AssetManagerOptions::HasAssetCache, true);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
			m_Flags.set(AssetManagerOptions::HasAssetModification, true);
		}
		virtual ~ScriptManager() = default;
	public:
		// Class specific functions
		virtual Ref<Scripting::Script> DeserializeAsset(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeRegistrySpecificData(YAML::Emitter& serializer) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::Asset& currentAsset) override;
		virtual void DeserializeRegistrySpecificData(YAML::Node& registryNode) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::Asset& currentAsset) override;

	private:
		std::unordered_set<std::string> m_ScriptSectionLabels{};
	};
}

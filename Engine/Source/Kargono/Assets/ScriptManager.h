#pragma once

#include "Kargono/Assets/AssetManager.h"

namespace Kargono::Scripting { class Script; }

namespace Kargono::Assets
{
	struct ScriptSpec
	{
		std::string Name {};
		Scripting::ScriptType Type {Scripting::ScriptType::None };
		std::string SectionLabel {};
		WrappedFuncType FunctionType{ WrappedFuncType::None };
	};

	class ScriptManager : public AssetManager<Scripting::Script>
	{
	public:
		ScriptManager() : AssetManager<Scripting::Script>()
		{
			m_AssetName = "Script";
			m_AssetType = AssetType::Script;
			m_FileExtension = ".kgscript";
			m_RegistryLocation = "Script/ScriptRegistry.kgreg";
			m_Flags.set(AssetManagerOptions::HasAssetCache, true);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
			m_Flags.set(AssetManagerOptions::HasAssetSaving, false);
			m_Flags.set(AssetManagerOptions::HasAssetCreationFromName, true);
		}
		virtual ~ScriptManager() = default;
	public:
		// Class specific functions
		virtual Ref<Scripting::Script> DeserializeAsset(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeRegistrySpecificData(YAML::Emitter& serializer) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::Asset& currentAsset) override;
		virtual void DeserializeRegistrySpecificData(YAML::Node& registryNode) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::Asset& currentAsset) override;

		std::tuple<AssetHandle, bool> CreateNewScript(ScriptSpec& spec);
		bool SaveScript(AssetHandle scriptHandle, ScriptSpec& spec);
		bool AddScriptSectionLabel(const std::string& newLabel);
		bool EditScriptSectionLabel(const std::string& oldLabel, const std::string& newLabel);
		bool DeleteScriptSectionLabel(const std::string& label);
		void FillScriptMetadata(ScriptSpec& spec, Assets::Asset& newAsset);
	public:
		std::unordered_set<std::string> m_ScriptSectionLabels{};
	};
}
#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class ShaderManager : public AssetManagerTemp<Rendering::Shader>
	{
	public:
		ShaderManager() : AssetManagerTemp<Rendering::Shader>()
		{
			m_AssetName = "Shader";
			m_AssetType = AssetType::Shader;
			m_RegistryLocation = "Shaders/Intermediates/ShaderRegistry.kgreg";
			m_Flags.set(AssetManagerOptions::HasAssetCache, true);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
			m_Flags.set(AssetManagerOptions::HasAssetModification, false);
		}
		virtual ~ShaderManager() = default;
	public:
		// Class specific functions
		virtual Ref<Rendering::Shader> DeserializeAsset(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::Asset& currentAsset) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::Asset& currentAsset) override;
	};
}

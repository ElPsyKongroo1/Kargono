#pragma once
#include "Kargono/Assets/AssetManager.h"


namespace Kargono::Rendering { class Shader; }

namespace Kargono::Assets
{
	class ShaderManager : public AssetManager<Rendering::Shader>
	{
	public:
		ShaderManager() : AssetManager<Rendering::Shader>()
		{
			m_AssetName = "Shader";
			m_AssetType = AssetType::Shader;
			m_RegistryLocation = "Shader/ShaderRegistry.kgreg";
			m_Flags.set(AssetManagerOptions::HasAssetCache, true);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
			m_Flags.set(AssetManagerOptions::HasAssetSaving, false);
			m_Flags.set(AssetManagerOptions::HasAssetCreationFromName, false);
		}
		virtual ~ShaderManager() = default;
	public:
		// Class specific functions
		virtual Ref<Rendering::Shader> DeserializeAsset(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::Asset& currentAsset) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::Asset& currentAsset) override;

		AssetHandle CreateNewShader(const Rendering::ShaderSpecification& shaderSpec);
		std::tuple<AssetHandle, Ref<Kargono::Rendering::Shader>> GetShader(const Rendering::ShaderSpecification& shaderSpec);
		void CreateShaderIntermediate(const Rendering::ShaderSource& shaderSource, Assets::Asset& newAsset, const Rendering::ShaderSpecification& shaderSpec, const Rendering::InputBufferLayout& inputLayout, const Rendering::UniformBufferList& uniformLayout);
	};
}

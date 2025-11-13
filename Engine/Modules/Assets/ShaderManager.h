#pragma once
#include "Modules/Assets/AssetManager.h"


namespace Kargono::Rendering { class Shader; }

namespace Kargono::Assets
{
	class ShaderManager : public AssetManager<Rendering::Shader>
	{
	public:
		ShaderManager() : AssetManager<Rendering::Shader>()
		
		virtual ~ShaderManager() = default;
	public:
		// Class specific functions
		virtual Ref<Rendering::Shader> DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::AssetInfo& currentAsset) override;

		AssetHandle CreateNewShader(const Rendering::ShaderSpecification& shaderSpec);
		std::tuple<AssetHandle, Ref<Kargono::Rendering::Shader>> GetShader(const Rendering::ShaderSpecification& shaderSpec);
		void CreateShaderIntermediate(const Rendering::ShaderSource& shaderSource, Assets::AssetInfo& newAsset, const Rendering::ShaderSpecification& shaderSpec, const Rendering::InputBufferLayout& inputLayout, const Rendering::UniformBufferList& uniformLayout);
	};
}

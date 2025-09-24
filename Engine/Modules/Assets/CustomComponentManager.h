#pragma once
#include "Modules/Assets/AssetManager.h"

#include <vector>
#include <limits>

namespace Kargono::ECSInternal { struct CustomComponent; }
namespace Kargono::Scenes { class Scene; }

namespace Kargono::Assets
{
	class CustomComponentManager : public AssetManager<ECSInternal::CustomComponent>
	{
	public:
		
		virtual ~CustomComponentManager() = default;
	public:
		// Class specific functions
		virtual Ref<void> SaveAssetValidation(Ref<ECSInternal::CustomComponent> newAsset, AssetHandle assetHandle) override;
		virtual void CreateAssetFileFromName(std::string_view name, AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAsset(Ref<ECSInternal::CustomComponent> assetReference, const std::filesystem::path& assetPath) override;
		virtual Ref<ECSInternal::CustomComponent> DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::AssetInfo& currentAsset) override;
		virtual void DeleteAssetValidation(AssetHandle assetHandle) override;
	};
}

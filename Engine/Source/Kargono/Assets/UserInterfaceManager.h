#pragma once
#include "Kargono/Assets/AssetManager.h"

namespace Kargono::RuntimeUI { class UserInterface; }

namespace Kargono::Assets
{
	class UserInterfaceManager : public AssetManager<RuntimeUI::UserInterface>
	{
	public:
		UserInterfaceManager() : AssetManager<RuntimeUI::UserInterface>()
		{
			m_AssetName = "User Interface";
			m_AssetType = AssetType::UserInterface;
			m_FileExtension = ".kgui";
			m_RegistryLocation = "UserInterface/UserInterfaceRegistry.kgreg";
			m_Flags.set(AssetManagerOptions::HasAssetCache, false);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
			m_Flags.set(AssetManagerOptions::HasAssetSaving, true);
			m_Flags.set(AssetManagerOptions::HasAssetCreationFromName, true);
		}
		virtual ~UserInterfaceManager() = default;
	public:
		//============================
		// Parent Class Overrides
		//============================
		virtual void CreateAssetFileFromName(std::string_view name, AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAsset(Ref<RuntimeUI::UserInterface> assetReference, const std::filesystem::path& assetPath) override;
		virtual Ref<RuntimeUI::UserInterface> DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath) override;

		//============================
		// Asset Clean-up Functions
		//============================
		bool RemoveScript(Ref<RuntimeUI::UserInterface> userInterfaceRef, Assets::AssetHandle scriptHandle);
		bool RemoveTexture(Ref<RuntimeUI::UserInterface> userInterfaceRef, Assets::AssetHandle textureHandle);
	};
}

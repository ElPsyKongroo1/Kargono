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
			m_FileExtension = ".kgui";
			m_Flags.set(AssetManagerOptions::HasAssetCache, false);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
		}
		virtual ~UserInterfaceManager() = default;
	public:
		// Class specific functions
		virtual Ref<RuntimeUI::UserInterface> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}

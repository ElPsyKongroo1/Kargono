#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class FontManager : public AssetManagerTemp<RuntimeUI::Font>
	{
	public:
		FontManager() : AssetManagerTemp<RuntimeUI::Font>()
		{
			m_AssetName = "Font";
			m_FileExtension = ".kgfont";
			m_ValidImportFileExtensions = { ".ttf" };
			m_Flags.set(AssetManagerOptions::HasAssetCache, true);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, true);
		}
		virtual ~FontManager() = default;
	public:

		// Class specific functions
		virtual Ref<RuntimeUI::Font> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}

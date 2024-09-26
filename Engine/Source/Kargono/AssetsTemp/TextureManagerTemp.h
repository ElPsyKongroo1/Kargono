#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class Texture2DManager : public AssetManagerTemp<Rendering::Texture2D>
	{
	public:
		Texture2DManager() : AssetManagerTemp<Rendering::Texture2D>()
		{
			m_AssetName = "Texture";
			m_FileExtension = ".kgtexture";
			m_ValidImportFileExtensions = { ".png" };
			m_Flags.set(AssetManagerOptions::HasAssetCache, true);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, true);
		}
		virtual ~Texture2DManager() = default;
	public:
		// Class specific functions
		virtual Ref<Rendering::Texture2D> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}

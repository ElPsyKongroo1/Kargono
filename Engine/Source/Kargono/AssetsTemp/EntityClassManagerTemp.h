#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class EntityClassManager : public AssetManagerTemp<Scenes::EntityClass>
	{
	public:
		EntityClassManager() : AssetManagerTemp<Scenes::EntityClass>()
		{
			m_AssetName = "Entity Class";
			m_FileExtension = ".kgclass";
			m_Flags.set(AssetManagerOptions::HasAssetCache, false);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
		}
		virtual ~EntityClassManager() = default;
	public:

		// Class specific functions
		virtual Ref<Scenes::EntityClass> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}

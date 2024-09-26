#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class GameStateManager : public AssetManagerTemp<Scenes::GameState>
	{
	public:
		GameStateManager() : AssetManagerTemp<Scenes::GameState>()
		{
			m_AssetName = "Game State";
			m_FileExtension = ".kgstate";
			m_Flags.set(AssetManagerOptions::HasAssetCache, false);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
		}
		virtual ~GameStateManager() = default;
	public:

		// Class specific functions
		virtual Ref<Scenes::GameState> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}

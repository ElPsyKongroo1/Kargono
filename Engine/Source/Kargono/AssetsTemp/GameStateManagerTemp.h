#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class GameStateManager : public AssetManagerTemp<Scenes::GameState>
	{
	public:
		GameStateManager() : AssetManagerTemp<Scenes::GameState>()
		{
			m_Flags.set(AssetManagerOptions::UseRuntimeCache, false);
		}
		virtual ~GameStateManager() = default;
	public:
		// Override virtual functions
		virtual Ref<Scenes::GameState> InstantiateAssetIntoMemory(Assets::Asset& asset) override;

		// Class specific functions
		bool DeserializeGameState(Ref<Scenes::GameState> GameState, const std::filesystem::path& filepath);
	};
}

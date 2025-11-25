#include "kgpch.h"

#include "Modules/GlobalState/GameStateContext.h"

namespace Kargono::GlobalState
{
	void GameStateContext::ClearActiveGameState()
	{
		m_ActiveGameState.Reset();
	}
	void GameStateContext::SetActiveGameState(Assets::AssetRef<GameState> newGameState)
	{
		KG_ASSERT(newGameState.IsUsable());

		m_ActiveGameState = newGameState;
	}
	Assets::AssetRef<GameState> GameStateContext::GetActiveGameState()
	{
		return m_ActiveGameState.GetAssetRef();
	}
}

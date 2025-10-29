#include "kgpch.h"

#include "Modules/GlobalState/GameStateContext.h"

namespace Kargono::GlobalState
{
	void GameStateContext::ClearActiveGameState()
	{
		m_ActiveGameState = nullptr;
		m_ActiveGameStateHandle = Assets::k_EmptyHandle;
	}
	void GameStateContext::SetActiveGameState(Ref<GameState> newGameState, Assets::AssetHandle newHandle)
	{
		m_ActiveGameState = newGameState;
		m_ActiveGameStateHandle = newHandle;
	}
	Ref<GameState> GameStateContext::GetActiveGameState()
	{
		return m_ActiveGameState;
	}
	Assets::AssetHandle GameStateContext::GetActiveGameStateHandle()
	{
		return m_ActiveGameStateHandle;
	}
}

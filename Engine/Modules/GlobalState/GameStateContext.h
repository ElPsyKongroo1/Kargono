#pragma once

#include "Modules/GlobalState/Assets/GameState.h"

namespace Kargono::GlobalState
{
	class GameStateContext
	{
	public:
		//=========================
		// Modify Active Game State
		//=========================
		void SetActiveGameState(Ref<GameState> newGameState, Assets::AssetHandle newHandle);
		void ClearActiveGameState();
	public:
		//=========================
		// Get Active Game State
		//=========================
		Assets::AssetHandle GetActiveGameStateHandle();
		Ref<GameState> GetActiveGameState();
	private:
		//=========================
		// Internal Fields
		//=========================
		Ref<GameState> m_ActiveGameState{ nullptr };
		Assets::AssetHandle m_ActiveGameStateHandle{ Assets::k_EmptyHandle };
	};

	class GameStateService // TODO: EWWWWW UGHHHHHHH
	{
	public:
		//==============================
		// Getters/Setters
		//==============================
		static GameStateContext& GetActiveContext() { return s_GameStateContext; }
	private:
		//==============================
		// Internal Fields
		//==============================
		static inline GameStateContext s_GameStateContext{};
	};
}
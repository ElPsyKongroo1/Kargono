#pragma once

#include "Modules/GlobalState/Assets/GameState.h"
#include "Modules/Assets/TrackedAssetReference.h"

namespace Kargono::GlobalState
{
	class GameStateContext
	{
	public:
		//=========================
		// Modify Active Game State
		//=========================
		void SetActiveGameState(Assets::AssetRef<GameState> newGameState);
		void ClearActiveGameState();
	public:
		//=========================
		// Get Active Game State
		//=========================
		Assets::AssetRef<GameState> GetActiveGameState();
	private:
		//=========================
		// Internal Fields
		//=========================
		Assets::TAssetRef<GameState> m_ActiveGameState{};
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
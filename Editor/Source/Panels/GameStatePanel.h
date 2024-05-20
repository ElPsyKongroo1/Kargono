#pragma once
#include "Kargono/Assets/Asset.h"
#include "Kargono/Scene/GameState.h"

namespace Kargono
{
	class GameStatePanel
	{
	public:
		GameStatePanel();

		void OnEditorUIRender();
	public:
		Ref<GameState> m_EditorGameState {nullptr};
		Assets::AssetHandle m_EditorGameStateHandle {0};

	};
}

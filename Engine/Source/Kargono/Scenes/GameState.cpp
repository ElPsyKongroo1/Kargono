#include "kgpch.h"

#include "Kargono/Scenes/GameState.h"

namespace Kargono::Scenes
{
	Ref<GameState> GameStateService::s_ActiveGameState { nullptr };
	Assets::AssetHandle GameStateService::s_ActiveGameStateHandle {0};
}

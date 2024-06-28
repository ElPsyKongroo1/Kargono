#include "kgpch.h"

#include "Kargono/Scenes/GameState.h"

namespace Kargono::Scenes
{
	Ref<GameState> GameState::s_GameState { nullptr };
	Assets::AssetHandle GameState::s_GameStateHandle {0};
}

#include "kgpch.h"
#include "Kargono/Scene/GameState.h"

namespace Kargono
{
	Ref<GameState> GameState::s_GameState { nullptr };
	Assets::AssetHandle GameState::s_GameStateHandle {0};
}

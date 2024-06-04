#pragma once
#include "Kargono/Assets/Asset.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Scene/GameState.h"

#include <string>

namespace Kargono
{
	class GameStatePanel
	{
	public:
		GameStatePanel();

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
	public:
		Ref<GameState> m_EditorGameState {nullptr};
		Assets::AssetHandle m_EditorGameStateHandle {0};
	private:
		std::string m_PanelName{ "Game State Editor" };

	};
}

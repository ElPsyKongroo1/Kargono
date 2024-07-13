#pragma once
#include "Kargono/Assets/Asset.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Scenes/GameState.h"

#include <string>

namespace Kargono::Panels
{
	class GameStatePanel
	{
	public:
		GameStatePanel();

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);

		void ResetPanelResources();
	public:
		Ref<Scenes::GameState> m_EditorGameState {nullptr};
		Assets::AssetHandle m_EditorGameStateHandle {0};
	private:
		std::string m_PanelName{ "Game State Editor" };

	};
}

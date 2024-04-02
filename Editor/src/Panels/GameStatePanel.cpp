#include "GameStatePanel.h"

#include "Kargono.h"

namespace Kargono
{
	static Ref<GameState> s_ActiveGameState {nullptr};

	GameStatePanel::GameStatePanel()
	{

	}
	void GameStatePanel::OnEditorUIRender()
	{
		UI::Editor::StartWindow("Game State Editor");

		/*for (auto& [id, asset] : Assets::AssetManager::GetGameStateRegistry())
		{
			if (ImGui::Button(((Assets::GameStateMetaData*)asset.Data.SpecificFileData.get())->Name.c_str()))
			{
				
			}
		}*/

		UI::Editor::TitleText("Hello this is the game state editor");

		UI::Editor::EndWindow();
	}
}

#include "GameStatePanel.h"

#include "EditorLayer.h"
#include "Kargono.h"

namespace Kargono
{
	static EditorLayer* s_EditorLayer { nullptr };

	static Ref<GameState> s_EditorGameState {nullptr};
	static Assets::AssetHandle s_EditorGameStateHandle {0};

	static std::function<void()> s_OnOpenGameState { nullptr };
	static std::function<void()> s_OnCreateGameState { nullptr };
	static UI::GenericPopupSpec s_CreateGameStatePopupSpec {};
	static UI::TextInputSpec s_SelectGameStateNameSpec {};


	GameStatePanel::GameStatePanel()
	{
		s_EditorLayer = EditorLayer::GetCurrentLayer();

		s_OnOpenGameState = [&]()
		{
			
		};

		s_OnCreateGameState = [&]()
		{
			s_CreateGameStatePopupSpec.PopupActive = true;
		};

		s_SelectGameStateNameSpec.Label = "New Name";
		s_SelectGameStateNameSpec.WidgetID = 0x1938fc315cf34a63;
		s_SelectGameStateNameSpec.CurrentOption = "Empty";
		s_SelectGameStateNameSpec.ConfirmAction = [&](const std::string& option)
		{
			s_SelectGameStateNameSpec.CurrentOption = option;
		};

		s_CreateGameStatePopupSpec.Label = "Create Game State";
		s_CreateGameStatePopupSpec.WidgetID = 0xd24ee715292c;
		s_CreateGameStatePopupSpec.PopupWidth = 420.0f;
		s_CreateGameStatePopupSpec.ConfirmAction = [&]()
		{
			if (s_SelectGameStateNameSpec.CurrentOption == "")
			{
				return;
			}

			for (auto& [id, asset] : Assets::AssetManager::GetGameStateRegistry())
			{
				if (asset.Data.GetSpecificFileData<Assets::GameStateMetaData>()->Name == s_SelectGameStateNameSpec.CurrentOption)
				{
					return;
				}
			}
			s_EditorGameStateHandle = Assets::AssetManager::CreateNewGameState(s_SelectGameStateNameSpec.CurrentOption);
			s_EditorGameState = Assets::AssetManager::GetGameState(s_EditorGameStateHandle);
		};
		s_CreateGameStatePopupSpec.PopupContents = [&]()
		{
			UI::Editor::TextInputModal(s_SelectGameStateNameSpec);
		};

		
	}
	void GameStatePanel::OnEditorUIRender()
	{
		UI::Editor::StartWindow("Game State Editor", &s_EditorLayer->m_ShowGameStateEditor);

		if (!s_EditorGameState)
		{
			UI::Editor::NewItemScreen("Open Existing Game State", s_OnOpenGameState, "Create New Game State", s_OnCreateGameState);
			UI::Editor::GenericPopup(s_CreateGameStatePopupSpec);
		}
		else
		{
			UI::Editor::SelectorHeader(s_EditorGameState->GetName());

			
		}

		/*for (auto& [id, asset] : Assets::AssetManager::GetGameStateRegistry())
		{
			if (ImGui::Button(((Assets::GameStateMetaData*)asset.Data.SpecificFileData.get())->Name.c_str()))
			{
				
			}
		}*/

		UI::Editor::EndWindow();
	}
}

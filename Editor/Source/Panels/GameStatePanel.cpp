#include "GameStatePanel.h"

#include "EditorApp.h"
#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{
	static GameStatePanel* s_GameStatePanel { nullptr };
	static std::string s_CurrentField {};
	// Null State
	static std::function<void()> s_OnOpenGameState { nullptr };
	static std::function<void()> s_OnCreateGameState { nullptr };
	static EditorUI::GenericPopupSpec s_CreateGameStatePopupSpec {};
	static EditorUI::SelectOptionSpec s_OpenGameStatePopupSpec {};
	// Header
	static EditorUI::TextInputSpec s_SelectGameStateNameSpec {};
	static EditorUI::SelectorHeaderSpec s_MainHeader {};
	static EditorUI::GenericPopupSpec s_DeleteGameStateWarning {};
	static EditorUI::GenericPopupSpec s_CloseGameStateWarning {};
	// Main Panel
	static EditorUI::TableSpec s_FieldsTable {};
	static EditorUI::SelectOptionSpec s_AddFieldPopup {};
	static EditorUI::GenericPopupSpec s_EditFieldPopup {};
	static EditorUI::TextInputSpec s_EditFieldName {};
	static EditorUI::SelectOptionSpec s_EditFieldType {};
	static EditorUI::EditVariableSpec s_EditFieldValue {};

	static void InitializeOpeningScreen()
	{
		s_OnOpenGameState = [&]()
		{
			s_OpenGameStatePopupSpec.PopupActive = true;
		};

		s_OnCreateGameState = [&]()
		{
			s_CreateGameStatePopupSpec.PopupActive = true;
		};

		s_OpenGameStatePopupSpec.Label = "Open Game State";
		s_OpenGameStatePopupSpec.CurrentOption = { "None", Assets::EmptyHandle };
		s_OpenGameStatePopupSpec.Flags |= EditorUI::SelectOption_PopupOnly;
		s_OpenGameStatePopupSpec.PopupAction = [&]()
		{
			s_OpenGameStatePopupSpec.GetAllOptions().clear();
			s_OpenGameStatePopupSpec.CurrentOption = { "None", Assets::EmptyHandle };

			s_OpenGameStatePopupSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetManager::GetGameStateRegistry())
			{
				s_OpenGameStatePopupSpec.AddToOptions("All Options", asset.Data.IntermediateLocation.string(), handle);
			}
		};

		s_OpenGameStatePopupSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.Handle == Assets::EmptyHandle)
			{
				KG_WARN("No Game State Selected");
				return;
			}
			if (!Assets::AssetManager::GetGameStateRegistry().contains(selection.Handle))
			{
				KG_WARN("Could not find on game state in game state editor");
				return;
			}

			s_GameStatePanel->m_EditorGameState = Assets::AssetManager::GetGameState(selection.Handle);
			s_GameStatePanel->m_EditorGameStateHandle = selection.Handle;
			s_MainHeader.EditColorActive = false;
			s_MainHeader.Label = Assets::AssetManager::GetGameStateRegistry().at(
				s_GameStatePanel->m_EditorGameStateHandle).Data.IntermediateLocation.string();
			s_FieldsTable.OnRefresh();
		};

		s_SelectGameStateNameSpec.Label = "New Name";
		s_SelectGameStateNameSpec.CurrentOption = "Empty";
		s_SelectGameStateNameSpec.ConfirmAction = [&](const std::string& option)
		{
			s_SelectGameStateNameSpec.CurrentOption = option;
		};

		s_CreateGameStatePopupSpec.Label = "Create Game State";
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
			s_GameStatePanel->m_EditorGameStateHandle = Assets::AssetManager::CreateNewGameState(s_SelectGameStateNameSpec.CurrentOption);
			s_GameStatePanel->m_EditorGameState = Assets::AssetManager::GetGameState(s_GameStatePanel->m_EditorGameStateHandle);
			s_MainHeader.EditColorActive = false;
			s_MainHeader.Label = Assets::AssetManager::GetGameStateRegistry().at(
				s_GameStatePanel->m_EditorGameStateHandle).Data.IntermediateLocation.string();
			s_FieldsTable.OnRefresh();
		};
		s_CreateGameStatePopupSpec.PopupContents = [&]()
		{
			EditorUI::EditorUIService::TextInputPopup(s_SelectGameStateNameSpec);
		};
	}

	static void InitializeDisplayGameStateScreen()
	{
		// Header (Game State Name and Options)
		s_DeleteGameStateWarning.Label = "Delete Game State";
		s_DeleteGameStateWarning.ConfirmAction = [&]()
		{
			Assets::AssetManager::DeleteGameState(s_GameStatePanel->m_EditorGameStateHandle);
			s_GameStatePanel->m_EditorGameStateHandle = 0;
			s_GameStatePanel->m_EditorGameState = nullptr;
		};
		s_DeleteGameStateWarning.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to delete this game state object?");
		};

		s_CloseGameStateWarning.Label = "Close Game State";
		s_CloseGameStateWarning.ConfirmAction = [&]()
		{
			s_GameStatePanel->m_EditorGameStateHandle = 0;
			s_GameStatePanel->m_EditorGameState = nullptr;
		};
		s_CloseGameStateWarning.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to close this game state object without saving?");
		};

		s_MainHeader.AddToSelectionList("Save", [&]()
			{
				Assets::AssetManager::SaveGameState(s_GameStatePanel->m_EditorGameStateHandle, s_GameStatePanel->m_EditorGameState);
				s_MainHeader.EditColorActive = false;
			});
		s_MainHeader.AddToSelectionList("Close", [&]()
			{
				if (s_MainHeader.EditColorActive)
				{
					s_CloseGameStateWarning.PopupActive = true;
				}
				else
				{
					s_GameStatePanel->m_EditorGameStateHandle = 0;
					s_GameStatePanel->m_EditorGameState = nullptr;
				}
			});
		s_MainHeader.AddToSelectionList("Delete", [&]()
			{
				s_DeleteGameStateWarning.PopupActive = true;
			});


		// Fields Table
		s_FieldsTable.Label = "Fields";
		s_FieldsTable.Expanded = true;
		s_FieldsTable.OnRefresh = [&]()
		{
			s_FieldsTable.ClearTable();
			if (s_GameStatePanel->m_EditorGameState)
			{
				for (auto& [name, field] : s_GameStatePanel->m_EditorGameState->GetAllFields())
				{
					switch (field->Type())
					{
					case WrappedVarType::UInteger16:
					{
						s_FieldsTable.InsertTableEntry(name,
							std::to_string(field->GetWrappedValue<uint16_t>()),
							[&](EditorUI::TableEntry& entry)
							{
								s_EditFieldPopup.PopupActive = true;
								s_CurrentField = name;
							});
						break;
					}
					default:
					{
						KG_WARN("Attempt to add invalid/unsupported type to game state fields UI");
						break;
					}
					}
				}
			}
		};
		s_FieldsTable.Column1Title = "Field Name";
		s_FieldsTable.Column2Title = "Field Value";
		s_FieldsTable.AddToSelectionList("Add New Field", [&]()
			{
				s_AddFieldPopup.PopupActive = true;
			});

		s_AddFieldPopup.Label = "Add New Field";
		s_AddFieldPopup.Flags |= EditorUI::SelectOption_PopupOnly;
		s_AddFieldPopup.CurrentOption = { "None", Assets::EmptyHandle };
		s_AddFieldPopup.LineCount = 2;
		s_AddFieldPopup.PopupAction = [&]()
		{
			s_AddFieldPopup.ClearOptions();
			s_AddFieldPopup.AddToOptions("Clear", "None", Assets::EmptyHandle);
			s_AddFieldPopup.AddToOptions("All Options", "UInteger16", Assets::EmptyHandle);
		};
		s_AddFieldPopup.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.Label == "None")
			{
				return;
			}

			if (selection.Label == "UInteger16")
			{
				uint32_t iteration{ 1 };
				while (!s_GameStatePanel->m_EditorGameState->AddField("New Field " + std::to_string(iteration), WrappedVarType::UInteger16))
				{
					iteration++;
				}
				s_MainHeader.EditColorActive = true;
			}
			s_FieldsTable.OnRefresh();
		};

		s_EditFieldName.Label = "Field Name";
		s_EditFieldName.CurrentOption = "Empty";
		s_EditFieldName.ConfirmAction = [&](const std::string& option)
		{
			s_EditFieldName.CurrentOption = option;
		};

		s_EditFieldType.Label = "Field Type";
		s_EditFieldType.Flags |= EditorUI::SelectOption_PopupOnly;
		s_EditFieldType.CurrentOption = { "None", Assets::EmptyHandle };
		s_EditFieldType.LineCount = 2;
		s_EditFieldType.PopupAction = [&]()
		{
			s_EditFieldType.ClearOptions();
			s_EditFieldType.AddToOptions("All Options", "UInteger16", Assets::EmptyHandle);
		};
		s_EditFieldType.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			s_FieldsTable.OnRefresh();
		};
		s_EditFieldValue.Label = "Field Value";
		s_EditFieldValue.AllocateBuffer();

		s_EditFieldPopup.Label = "Edit Field";
		s_EditFieldPopup.DeleteAction = [&]()
		{
			if (!s_GameStatePanel->m_EditorGameState->DeleteField(s_CurrentField))
			{
				KG_ERROR("Unable to delete field inside game state!");
				return;
			}
			s_MainHeader.EditColorActive = true;
			s_FieldsTable.OnRefresh();
		};
		s_EditFieldPopup.PopupWidth = 420.0f;
		s_EditFieldPopup.PopupAction = [&]()
		{
			Ref<WrappedVariable> field = s_GameStatePanel->m_EditorGameState->GetField(s_CurrentField);

			if (!field)
			{
				KG_ERROR("Unable to retreive field from current game state object");
				return;
			}
			s_EditFieldName.CurrentOption = s_CurrentField;
			s_EditFieldType.CurrentOption.Label = Utility::WrappedVarTypeToString(field->Type());
			std::string convertCache { std::to_string(field->GetWrappedValue<uint16_t>()) };
			s_EditFieldValue.FieldBuffer.SetDataToByte(0);
			memcpy(s_EditFieldValue.FieldBuffer.Data, convertCache.data(), convertCache.size());
		};
		s_EditFieldPopup.ConfirmAction = [&]()
		{
			auto& fieldMap = s_GameStatePanel->m_EditorGameState->GetAllFields();
			Ref<WrappedVariable> oldField = fieldMap.at(s_CurrentField);
			Ref<WrappedVariable> newField { nullptr };
			if (s_CurrentField != s_EditFieldName.CurrentOption)
			{
				if (!oldField)
				{
					KG_ERROR("Could not obtain field pointer from game state field map");
					return;
				}
				fieldMap.erase(s_CurrentField);
				s_FieldsTable.OnRefresh();
			}

			switch (Utility::StringToWrappedVarType(s_EditFieldType.CurrentOption.Label))
			{
				case WrappedVarType::UInteger16:
				{
					newField = CreateRef<WrappedUInteger16>();

					bool success = Utility::Conversions::CharBufferToVariable<uint16_t>(s_EditFieldValue.FieldBuffer,
						((WrappedUInteger16*)newField.get())->m_Value);

					if (!success && oldField->Type() == WrappedVarType::UInteger16)
					{
						((WrappedUInteger16*)newField.get())->m_Value = oldField->GetWrappedValue<uint16_t>();
					}

					break;
				}
				default:
				{
					KG_ERROR("Invalid wrapped type provided to editfieldpopup.confirm function");
					return;
				}
			}

			fieldMap.insert_or_assign(s_EditFieldName.CurrentOption, newField);
			s_MainHeader.EditColorActive = true;
			s_FieldsTable.OnRefresh();
		};
		s_EditFieldPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::TextInputPopup(s_EditFieldName);
			EditorUI::EditorUIService::SelectOption(s_EditFieldType);
			EditorUI::EditorUIService::EditVariable(s_EditFieldValue);
		};
		
	}

	GameStatePanel::GameStatePanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName,
			KG_BIND_CLASS_FN(GameStatePanel::OnKeyPressedEditor));
		s_GameStatePanel = this;

		InitializeOpeningScreen();

		InitializeDisplayGameStateScreen();

	}
	void GameStatePanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowGameStateEditor);

		if (!m_EditorGameState)
		{
			EditorUI::EditorUIService::NewItemScreen("Open Existing Game State", s_OnOpenGameState, "Create New Game State", s_OnCreateGameState);
			EditorUI::EditorUIService::GenericPopup(s_CreateGameStatePopupSpec);
			EditorUI::EditorUIService::SelectOption(s_OpenGameStatePopupSpec);
		}
		else
		{
			EditorUI::EditorUIService::SelectorHeader(s_MainHeader);
			EditorUI::EditorUIService::GenericPopup(s_DeleteGameStateWarning);
			EditorUI::EditorUIService::GenericPopup(s_CloseGameStateWarning);
			EditorUI::EditorUIService::Table(s_FieldsTable);
			EditorUI::EditorUIService::SelectOption(s_AddFieldPopup);
			EditorUI::EditorUIService::GenericPopup(s_EditFieldPopup);
		}

		EditorUI::EditorUIService::EndWindow();
	}
	bool GameStatePanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
}

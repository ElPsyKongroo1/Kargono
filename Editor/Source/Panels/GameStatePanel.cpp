#include "GameStatePanel.h"

#include "EditorApp.h"
#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{
	void GameStatePanel::InitializeOpeningScreen()
	{
		
		m_OpenGameStatePopupSpec.Label = "Open Game State";
		m_OpenGameStatePopupSpec.CurrentOption = { "None", Assets::EmptyHandle };
		m_OpenGameStatePopupSpec.Flags |= EditorUI::SelectOption_PopupOnly;
		m_OpenGameStatePopupSpec.PopupAction = [&]()
		{
			m_OpenGameStatePopupSpec.GetAllOptions().clear();
			m_OpenGameStatePopupSpec.CurrentOption = { "None", Assets::EmptyHandle };

			m_OpenGameStatePopupSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetGameStateRegistry())
			{
				m_OpenGameStatePopupSpec.AddToOptions("All Options", asset.Data.FileLocation.string(), handle);
			}
		};

		m_OpenGameStatePopupSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.Handle == Assets::EmptyHandle)
			{
				KG_WARN("No Game State Selected");
				return;
			}
			if (!Assets::AssetService::GetGameStateRegistry().contains(selection.Handle))
			{
				KG_WARN("Could not find on game state in game state editor");
				return;
			}

			m_EditorGameState = Assets::AssetService::GetGameState(selection.Handle);
			m_EditorGameStateHandle = selection.Handle;
			m_TagHeader.EditColorActive = false;
			m_TagHeader.Label = Assets::AssetService::GetGameStateRegistry().at(
				m_EditorGameStateHandle).Data.FileLocation.string();
			m_FieldsTable.OnRefresh();
		};

		m_SelectGameStateNameSpec.Label = "New Name";
		m_SelectGameStateNameSpec.CurrentOption = "Empty";

		m_CreateGameStatePopupSpec.Label = "Create Game State";
		m_CreateGameStatePopupSpec.PopupWidth = 420.0f;
		m_CreateGameStatePopupSpec.ConfirmAction = [&]()
		{
			if (m_SelectGameStateNameSpec.CurrentOption == "")
			{
				return;
			}

			for (auto& [id, asset] : Assets::AssetService::GetGameStateRegistry())
			{
				if (asset.Data.GetSpecificMetaData<Assets::GameStateMetaData>()->Name == m_SelectGameStateNameSpec.CurrentOption)
				{
					return;
				}
			}
			m_EditorGameStateHandle = Assets::AssetService::CreateGameState(m_SelectGameStateNameSpec.CurrentOption.c_str());
			m_EditorGameState = Assets::AssetService::GetGameState(m_EditorGameStateHandle);
			m_TagHeader.EditColorActive = false;
			m_TagHeader.Label = Assets::AssetService::GetGameStateRegistry().at(
				m_EditorGameStateHandle).Data.FileLocation.string();
			m_FieldsTable.OnRefresh();
		};
		m_CreateGameStatePopupSpec.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_SelectGameStateNameSpec);
		};
	}

	void GameStatePanel::InitializeDisplayGameStateScreen()
	{
		// Header (Game State Name and Options)
		m_DeleteGameStateWarning.Label = "Delete Game State";
		m_DeleteGameStateWarning.ConfirmAction = [&]()
		{
			Assets::AssetService::DeleteGameState(m_EditorGameStateHandle);
			m_EditorGameStateHandle = 0;
			m_EditorGameState = nullptr;
		};
		m_DeleteGameStateWarning.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to delete this game state object?");
		};

		m_CloseGameStateWarning.Label = "Close Game State";
		m_CloseGameStateWarning.ConfirmAction = [&]()
		{
			m_EditorGameStateHandle = 0;
			m_EditorGameState = nullptr;
		};
		m_CloseGameStateWarning.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to close this game state object without saving?");
		};

		m_TagHeader.AddToSelectionList("Save", [&]()
			{
				Assets::AssetService::SaveGameState(m_EditorGameStateHandle, m_EditorGameState);
				m_TagHeader.EditColorActive = false;
			});
		m_TagHeader.AddToSelectionList("Close", [&]()
			{
				if (m_TagHeader.EditColorActive)
				{
					m_CloseGameStateWarning.PopupActive = true;
				}
				else
				{
					m_EditorGameStateHandle = 0;
					m_EditorGameState = nullptr;
				}
			});
		m_TagHeader.AddToSelectionList("Delete", [&]()
			{
				m_DeleteGameStateWarning.PopupActive = true;
			});


		// Fields Table
		m_FieldsTable.Label = "Fields";
		m_FieldsTable.Expanded = true;
		m_FieldsTable.OnRefresh = [&]()
		{
			m_FieldsTable.ClearTable();
			if (m_EditorGameState)
			{
				for (auto& [name, field] : m_EditorGameState->GetAllFields())
				{
					switch (field->Type())
					{
					case WrappedVarType::UInteger16:
					{
						m_FieldsTable.InsertTableEntry(name,
							std::to_string(field->GetWrappedValue<uint16_t>()),
							[&](EditorUI::TableEntry& entry)
							{
								m_EditFieldPopup.PopupActive = true;
								m_CurrentField = name;
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
		m_FieldsTable.Column1Title = "Field Name";
		m_FieldsTable.Column2Title = "Field Value";
		m_FieldsTable.AddToSelectionList("Add New Field", [&]()
			{
				m_AddFieldPopup.PopupActive = true;
			});

		m_AddFieldPopup.Label = "Add New Field";
		m_AddFieldPopup.Flags |= EditorUI::SelectOption_PopupOnly;
		m_AddFieldPopup.CurrentOption = { "None", Assets::EmptyHandle };
		m_AddFieldPopup.LineCount = 2;
		m_AddFieldPopup.PopupAction = [&]()
		{
			m_AddFieldPopup.ClearOptions();
			m_AddFieldPopup.AddToOptions("Clear", "None", Assets::EmptyHandle);
			m_AddFieldPopup.AddToOptions("All Options", "UInteger16", Assets::EmptyHandle);
		};
		m_AddFieldPopup.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.Label == "None")
			{
				return;
			}

			if (selection.Label == "UInteger16")
			{
				uint32_t iteration{ 1 };
				while (!m_EditorGameState->AddField("New Field " + std::to_string(iteration), WrappedVarType::UInteger16))
				{
					iteration++;
				}
				m_TagHeader.EditColorActive = true;
			}
			m_FieldsTable.OnRefresh();
		};

		m_EditFieldName.Label = "Field Name";
		m_EditFieldName.CurrentOption = "Empty";

		m_EditFieldType.Label = "Field Type";
		m_EditFieldType.Flags |= EditorUI::SelectOption_PopupOnly;
		m_EditFieldType.CurrentOption = { "None", Assets::EmptyHandle };
		m_EditFieldType.LineCount = 2;
		m_EditFieldType.PopupAction = [&]()
		{
			m_EditFieldType.ClearOptions();
			m_EditFieldType.AddToOptions("All Options", "UInteger16", Assets::EmptyHandle);
		};
		m_EditFieldType.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			m_FieldsTable.OnRefresh();
		};
		m_EditFieldValue.Label = "Field Value";
		m_EditFieldValue.AllocateBuffer();

		m_EditFieldPopup.Label = "Edit Field";
		m_EditFieldPopup.DeleteAction = [&]()
		{
			if (!m_EditorGameState->DeleteField(m_CurrentField))
			{
				KG_ERROR("Unable to delete field inside game state!");
				return;
			}
			m_TagHeader.EditColorActive = true;
			m_FieldsTable.OnRefresh();
		};
		m_EditFieldPopup.PopupWidth = 420.0f;
		m_EditFieldPopup.PopupAction = [&]()
		{
			Ref<WrappedVariable> field = m_EditorGameState->GetField(m_CurrentField);

			if (!field)
			{
				KG_ERROR("Unable to retreive field from current game state object");
				return;
			}
			m_EditFieldName.CurrentOption = m_CurrentField;
			m_EditFieldType.CurrentOption.Label = Utility::WrappedVarTypeToString(field->Type());
			std::string convertCache { std::to_string(field->GetWrappedValue<uint16_t>()) };
			m_EditFieldValue.FieldBuffer.SetDataToByte(0);
			memcpy(m_EditFieldValue.FieldBuffer.Data, convertCache.data(), convertCache.size());
		};
		m_EditFieldPopup.ConfirmAction = [&]()
		{
			auto& fieldMap = m_EditorGameState->GetAllFields();
			Ref<WrappedVariable> oldField = fieldMap.at(m_CurrentField);
			Ref<WrappedVariable> newField { nullptr };
			if (m_CurrentField != m_EditFieldName.CurrentOption)
			{
				if (!oldField)
				{
					KG_ERROR("Could not obtain field pointer from game state field map");
					return;
				}
				fieldMap.erase(m_CurrentField);
				m_FieldsTable.OnRefresh();
			}

			switch (Utility::StringToWrappedVarType(m_EditFieldType.CurrentOption.Label))
			{
				case WrappedVarType::UInteger16:
				{
					newField = CreateRef<WrappedUInteger16>();

					bool success = Utility::Conversions::CharBufferToVariable<uint16_t>(m_EditFieldValue.FieldBuffer,
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

			fieldMap.insert_or_assign(m_EditFieldName.CurrentOption, newField);
			m_TagHeader.EditColorActive = true;
			m_FieldsTable.OnRefresh();
		};
		m_EditFieldPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_EditFieldName);
			EditorUI::EditorUIService::SelectOption(m_EditFieldType);
			EditorUI::EditorUIService::EditVariable(m_EditFieldValue);
		};
		
	}

	GameStatePanel::GameStatePanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(GameStatePanel::OnKeyPressedEditor));

		InitializeOpeningScreen();
		InitializeDisplayGameStateScreen();

	}
	void GameStatePanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowGameStateEditor);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		if (!m_EditorGameState)
		{
			
			EditorUI::EditorUIService::NewItemScreen("Open Existing Game State", KG_BIND_CLASS_FN(OpenGameState), "Create New Game State", KG_BIND_CLASS_FN(CreateGameState));
			EditorUI::EditorUIService::GenericPopup(m_CreateGameStatePopupSpec);
			EditorUI::EditorUIService::SelectOption(m_OpenGameStatePopupSpec);
		}
		else
		{
			EditorUI::EditorUIService::PanelHeader(m_TagHeader);
			EditorUI::EditorUIService::GenericPopup(m_DeleteGameStateWarning);
			EditorUI::EditorUIService::GenericPopup(m_CloseGameStateWarning);
			EditorUI::EditorUIService::Table(m_FieldsTable);
			EditorUI::EditorUIService::SelectOption(m_AddFieldPopup);
			EditorUI::EditorUIService::GenericPopup(m_EditFieldPopup);
		}

		EditorUI::EditorUIService::EndWindow();
	}
	bool GameStatePanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
	void GameStatePanel::ResetPanelResources()
	{
		m_EditorGameState = nullptr;
		m_EditorGameStateHandle = Assets::EmptyHandle;
	}
	void GameStatePanel::OpenGameState()
	{
		m_OpenGameStatePopupSpec.PopupActive = true;
	}
	void GameStatePanel::CreateGameState()
	{
		m_CreateGameStatePopupSpec.PopupActive = true;
	}
}

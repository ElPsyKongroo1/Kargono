#include "Panels/InputModePanel.h"
#include "EditorApp.h"

namespace Kargono
{
	static EditorApp* s_EditorApp { nullptr };

	// Opening Panel w/ Popups
	static EditorUI::SelectOptionSpec s_OpenInputModePopupSpec {};
	static EditorUI::GenericPopupSpec s_CreateInputModePopupSpec {};
	static EditorUI::TextInputSpec s_SelectInputModeNameSpec {};

	// Input Mode Header
	static EditorUI::SelectorHeaderSpec s_MainHeader {};
	static EditorUI::GenericPopupSpec s_DeleteInputModeWarning {};
	static EditorUI::GenericPopupSpec s_CloseInputModeWarning {};

	// Keyboard Panel
	static EditorUI::TableSpec s_KeyboardOnUpdateTable {};

	// Reusable Functions
	static void OnOpenInputMode()
	{
		s_OpenInputModePopupSpec.PopupActive = true;
	}
	static void OnCreateInputMode()
	{
		s_CreateInputModePopupSpec.PopupActive = true;
	}

	static void OnRefreshData()
	{
		s_KeyboardOnUpdateTable.OnRefresh();
	}
}

namespace Kargono::Panels
{

	InputModePanel::InputModePanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName,
			KG_BIND_CLASS_FN(InputModePanel::OnKeyPressedEditor));
		InitializeOpeningScreen();
		InitializeInputModeHeader();
		InitializeKeyboardScreen();
	}
	void InputModePanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION()
		EditorUI::Editor::StartWindow(m_PanelName, &s_EditorApp->m_ShowInputModeEditor);

		if (!m_EditorInputMode)
		{
			EditorUI::Editor::NewItemScreen("Open Existing Input Mode", OnOpenInputMode, "Create New InputMode", OnCreateInputMode);
			EditorUI::Editor::GenericPopup(s_CreateInputModePopupSpec);
			EditorUI::Editor::SelectOption(s_OpenInputModePopupSpec);
		}
		else
		{
			EditorUI::Editor::SelectorHeader(s_MainHeader);
			EditorUI::Editor::GenericPopup(s_DeleteInputModeWarning);
			EditorUI::Editor::GenericPopup(s_CloseInputModeWarning);

			EditorUI::Editor::BeginTabBar("InputModePanelTabBar");
			if (EditorUI::Editor::BeginTabItem("Keyboard"))
			{
				EditorUI::Editor::Table(s_KeyboardOnUpdateTable);
				/*InputEditor_Keyboard_OnUpdate();
				InputEditor_Keyboard_OnKeyPressed();
				InputEditor_Keyboard_Polling();*/

				EditorUI::Editor::EndTabItem();
			}
			if (EditorUI::Editor::BeginTabItem("Mouse"))
			{
				EditorUI::Editor::Text("Unimplemented Yet????");
				EditorUI::Editor::EndTabItem();
			}
			EditorUI::Editor::EndTabBar();
			
		}

		EditorUI::Editor::EndWindow();
	}
	bool InputModePanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}

	void InputModePanel::InitializeOpeningScreen()
	{
		s_OpenInputModePopupSpec.Label = "Open Input Mode";
		s_OpenInputModePopupSpec.CurrentOption = { "None", Assets::EmptyHandle };
		s_OpenInputModePopupSpec.Flags |= EditorUI::SelectOption_PopupOnly;
		s_OpenInputModePopupSpec.PopupAction = [&](EditorUI::SelectOptionSpec& spec)
		{
			spec.GetAllOptions().clear();
			spec.CurrentOption = { "None", Assets::EmptyHandle };

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetManager::GetInputModeRegistry())
			{
				spec.AddToOptions("All Options", asset.Data.IntermediateLocation.string(), handle);
			}
		};

		s_OpenInputModePopupSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.Handle == Assets::EmptyHandle)
			{
				KG_WARN("No Input Mode Selected");
				return;
			}
			if (!Assets::AssetManager::GetInputModeRegistry().contains(selection.Handle))
			{
				KG_WARN("Could not find the input mode specified");
				return;
			}

			m_EditorInputMode = Assets::AssetManager::GetInputMode(selection.Handle);
			m_EditorInputModeHandle = selection.Handle;
			s_MainHeader.EditColorActive = false;
			s_MainHeader.Label = Assets::AssetManager::GetInputModeRegistry().at(
				m_EditorInputModeHandle).Data.IntermediateLocation.string();
			OnRefreshData();
		};

		s_SelectInputModeNameSpec.Label = "New Name";
		s_SelectInputModeNameSpec.CurrentOption = "Empty";
		s_SelectInputModeNameSpec.ConfirmAction = [&](const std::string& option)
		{
			s_SelectInputModeNameSpec.CurrentOption = option;
		};

		s_CreateInputModePopupSpec.Label = "Create Input Mode";
		s_CreateInputModePopupSpec.PopupWidth = 420.0f;
		s_CreateInputModePopupSpec.ConfirmAction = [&]()
		{
			if (s_SelectInputModeNameSpec.CurrentOption == "")
			{
				return;
			}

			m_EditorInputModeHandle = Assets::AssetManager::CreateNewInputMode(s_SelectInputModeNameSpec.CurrentOption);
			if (m_EditorInputModeHandle == Assets::EmptyHandle)
			{
				KG_WARN("Input Mode was not created");
				return;
			}
			m_EditorInputMode = Assets::AssetManager::GetInputMode(m_EditorInputModeHandle);
			s_MainHeader.EditColorActive = false;
			s_MainHeader.Label = Assets::AssetManager::GetInputModeRegistry().at(
				m_EditorInputModeHandle).Data.IntermediateLocation.string();
			OnRefreshData();
		};
		s_CreateInputModePopupSpec.PopupContents = [&]()
		{
			EditorUI::Editor::TextInputPopup(s_SelectInputModeNameSpec);
		};
	}

	void InputModePanel::InitializeInputModeHeader()
	{
		// Header (Game State Name and Options)
		s_DeleteInputModeWarning.Label = "Delete Input Mode";
		s_DeleteInputModeWarning.ConfirmAction = [&]()
		{
			Assets::AssetManager::DeleteInputMode(m_EditorInputModeHandle);
			m_EditorInputModeHandle = 0;
			m_EditorInputMode = nullptr;
		};
		s_DeleteInputModeWarning.PopupContents = [&]()
		{
			EditorUI::Editor::Text("Are you sure you want to delete this input mode object?");
		};

		s_CloseInputModeWarning.Label = "Close Input Mode";
		s_CloseInputModeWarning.ConfirmAction = [&]()
		{
			m_EditorInputModeHandle = 0;
			m_EditorInputMode = nullptr;
		};
		s_CloseInputModeWarning.PopupContents = [&]()
		{
			EditorUI::Editor::Text("Are you sure you want to close this input mode object without saving?");
		};

		s_MainHeader.AddToSelectionList("Save", [&]()
		{
			Assets::AssetManager::SaveInputMode(m_EditorInputModeHandle, m_EditorInputMode);
			s_MainHeader.EditColorActive = false;
		});
		s_MainHeader.AddToSelectionList("Close", [&]()
		{
			if (s_MainHeader.EditColorActive)
			{
				s_CloseInputModeWarning.PopupActive = true;
			}
			else
			{
				m_EditorInputModeHandle = 0;
				m_EditorInputMode = nullptr;
			}
		});
		s_MainHeader.AddToSelectionList("Delete", [&]()
		{
			s_DeleteInputModeWarning.PopupActive = true;
		});
	}

	void InputModePanel::InitializeKeyboardScreen()
	{
		// On Update Table
		s_KeyboardOnUpdateTable.Label = "On Update";
		s_KeyboardOnUpdateTable.Expanded = true;
		s_KeyboardOnUpdateTable.OnRefresh = [&]()
		{
			s_KeyboardOnUpdateTable.ClearTable();
			/*if (m_EditorGameState)
			{
				for (auto& [name, field] : m_EditorInputMode->GetAllFields())
				{
					switch (field->Type())
					{
					case WrappedVarType::UInteger16:
					{
						s_KeyboardOnUpdateTable.InsertTableEntry(name,
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
						KG_ERROR("Attempt to add invalid/unsupported type to game state fields UI");
						break;
					}
					}
				}
			}*/
		};
		s_KeyboardOnUpdateTable.Column1Title = "Keyboard Button";
		s_KeyboardOnUpdateTable.Column2Title = "Function";
		s_KeyboardOnUpdateTable.AddToSelectionList("Add New Slot", [&]()
		{
			//s_AddFieldPopup.PopupActive = true; TODO Add New Field Popup
		});
	}
	
}

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
	static EditorUI::GenericPopupSpec s_AddKeyboardOnUpdatePopup {};
	static EditorUI::SelectOptionSpec s_SelectKeyCode {};
	static EditorUI::SelectOptionSpec s_SelectFunction {};

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
			// Opening/Null State Screen
			EditorUI::Editor::NewItemScreen("Open Existing Input Mode", OnOpenInputMode, "Create New InputMode", OnCreateInputMode);
			EditorUI::Editor::GenericPopup(s_CreateInputModePopupSpec);
			EditorUI::Editor::SelectOption(s_OpenInputModePopupSpec);
		}
		else
		{
			// Header
			EditorUI::Editor::SelectorHeader(s_MainHeader);
			EditorUI::Editor::GenericPopup(s_DeleteInputModeWarning);
			EditorUI::Editor::GenericPopup(s_CloseInputModeWarning);

			// Main Content
			EditorUI::Editor::BeginTabBar("InputModePanelTabBar");
			// Keyboard Panel
			if (EditorUI::Editor::BeginTabItem("Keyboard"))
			{
				// On Update
				EditorUI::Editor::Table(s_KeyboardOnUpdateTable);
				EditorUI::Editor::GenericPopup(s_AddKeyboardOnUpdatePopup);

				EditorUI::Editor::EndTabItem();
			}
			// Mouse Panel
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
			for (auto& inputBinding : m_EditorInputMode->GetOnUpdateBindings())
			{

				if (inputBinding->GetActionType() != Input::KeyboardAction)
				{
					continue;
				}
				Input::KeyboardActionBinding* keyboardBinding = (Input::KeyboardActionBinding*)inputBinding.get();

				EditorUI::TableEntry newEntry;
				Assets::AssetHandle scriptHandle = keyboardBinding->GetScriptHandle();
				if (scriptHandle == Assets::EmptyHandle)
				{
					newEntry = {
						Utility::KeyCodeToString(keyboardBinding->GetKeyBinding()),
						"None",
						Assets::EmptyHandle,
						nullptr,
						nullptr
					};
				}
				else
				{
					Ref<Scripting::Script> script = Assets::AssetManager::GetScript(scriptHandle);
					std::string displayValue {script->m_SectionLabel + ":" + script->m_ScriptName };
					newEntry = {
						Utility::KeyCodeToString(keyboardBinding->GetKeyBinding()),
						displayValue,
						scriptHandle,
						nullptr,
						nullptr
					};
				}

				s_KeyboardOnUpdateTable.InsertTableEntry(newEntry);
				
			}
		};
		s_KeyboardOnUpdateTable.Column1Title = "KeyCode";
		s_KeyboardOnUpdateTable.Column2Title = "Function";
		s_KeyboardOnUpdateTable.AddToSelectionList("Add New Slot", [&]()
		{
			s_AddKeyboardOnUpdatePopup.PopupActive = true;
		});

		s_AddKeyboardOnUpdatePopup.Label = "Add New Slot";
		s_AddKeyboardOnUpdatePopup.PopupWidth = 420.0f;
		s_AddKeyboardOnUpdatePopup.PopupAction = [&](EditorUI::GenericPopupSpec& spec)
		{
			s_SelectKeyCode.CurrentOption = {Utility::KeyCodeToString(Key::A), Key::A};
			s_SelectFunction.CurrentOption = {"None", Assets::EmptyHandle};
		};
		s_AddKeyboardOnUpdatePopup.PopupContents = [&]()
		{
			EditorUI::Editor::SelectOption(s_SelectKeyCode);
			EditorUI::Editor::SelectOption(s_SelectFunction);
		};

		s_AddKeyboardOnUpdatePopup.ConfirmAction = [&]()
		{
			auto& keyboardBindings = m_EditorInputMode->GetOnUpdateBindings();
			Ref<Input::KeyboardActionBinding> newBinding = CreateRef<Input::KeyboardActionBinding>();
			newBinding->SetKeyBinding((KeyCode)s_SelectKeyCode.CurrentOption.Handle);
			Ref<Scripting::Script> script;
			if (s_SelectFunction.CurrentOption.Handle == Assets::EmptyHandle)
			{
				script = nullptr;
			}
			else
			{
				script = Assets::AssetManager::GetScript(s_SelectFunction.CurrentOption.Handle);
			}
			newBinding->SetScript(script, s_SelectFunction.CurrentOption.Handle);

			keyboardBindings.push_back(newBinding);
			OnRefreshData();
			s_MainHeader.EditColorActive = true;
		};

		s_SelectKeyCode.Label = "Select Key";
		s_SelectKeyCode.LineCount = 7;
		s_SelectKeyCode.PopupAction = [&](EditorUI::SelectOptionSpec& spec)
		{
			spec.ClearOptions();
			for (auto key : Key::s_AllKeyCodes)
			{
				spec.AddToOptions("All KeyCodes", Utility::KeyCodeToString(key), key);
			}
		};

		s_SelectFunction.Label = "Select Function";
		s_SelectFunction.PopupAction = [&](EditorUI::SelectOptionSpec& spec)
		{
			spec.ClearOptions();
			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [uuid, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_ScriptType == Scripting::ScriptType::Class)
				{
					if (script->m_FuncType != WrappedFuncType::Void_UInt64)
					{
						continue;
					}
					spec.AddToOptions("Class::" + script->m_SectionLabel, script->m_ScriptName, uuid);
				}
				if (script->m_ScriptType == Scripting::ScriptType::Global)
				{
					if (script->m_FuncType != WrappedFuncType::Void_None)
					{
						continue;
					}
					spec.AddToOptions("Global::" + script->m_SectionLabel, script->m_ScriptName, uuid);
				}
			}
		};
	}
	
}

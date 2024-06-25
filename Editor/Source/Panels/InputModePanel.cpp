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
	//	OnUpdate
	static EditorUI::TableSpec s_KeyboardOnUpdateTable {};
	static EditorUI::GenericPopupSpec s_KeyboardOnUpdateAddSlot {};
	static EditorUI::SelectOptionSpec s_KeyboardOnUpdateAddKeyCode {};
	static EditorUI::SelectOptionSpec s_KeyboardOnUpdateAddFunction {};
	static EditorUI::GenericPopupSpec s_KeyboardOnUpdateEditSlot {};
	static EditorUI::SelectOptionSpec s_KeyboardOnUpdateEditKeyCode {};
	static EditorUI::SelectOptionSpec s_KeyboardOnUpdateEditFunction {};
	static uint32_t s_KeyboardOnUpdateActiveSlot {};

	//	OnKeyPressed
	static EditorUI::TableSpec s_KeyboardOnKeyPressedTable {};
	static EditorUI::GenericPopupSpec s_KeyboardOnKeyPressedAddSlot {};
	static EditorUI::SelectOptionSpec s_KeyboardOnKeyPressedAddKeyCode {};
	static EditorUI::SelectOptionSpec s_KeyboardOnKeyPressedAddFunction {};
	static EditorUI::GenericPopupSpec s_KeyboardOnKeyPressedEditSlot {};
	static EditorUI::SelectOptionSpec s_KeyboardOnKeyPressedEditKeyCode {};
	static EditorUI::SelectOptionSpec s_KeyboardOnKeyPressedEditFunction {};
	static uint32_t s_KeyboardOnKeyPressedActiveSlot {};

	//  Polling
	static EditorUI::TableSpec s_KeyboardPollingTable {};
	static EditorUI::GenericPopupSpec s_KeyboardPollingAddSlot {};
	static EditorUI::SelectOptionSpec s_KeyboardPollingAddKeyCode {};
	static EditorUI::GenericPopupSpec s_KeyboardPollingEditSlot {};
	static EditorUI::SelectOptionSpec s_KeyboardPollingEditKeyCode {};
	static uint32_t s_KeyboardPollingActiveSlot {};


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
		s_KeyboardOnKeyPressedTable.OnRefresh();
		s_KeyboardPollingTable.OnRefresh();
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
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowInputModeEditor);

		if (!m_EditorInputMode)
		{
			// Opening/Null State Screen
			EditorUI::EditorUIService::NewItemScreen("Open Existing Input Mode", OnOpenInputMode, "Create New InputMode", OnCreateInputMode);
			EditorUI::EditorUIService::GenericPopup(s_CreateInputModePopupSpec);
			EditorUI::EditorUIService::SelectOption(s_OpenInputModePopupSpec);
		}
		else
		{
			// Header
			EditorUI::EditorUIService::SelectorHeader(s_MainHeader);
			EditorUI::EditorUIService::GenericPopup(s_DeleteInputModeWarning);
			EditorUI::EditorUIService::GenericPopup(s_CloseInputModeWarning);

			// Main Content
			EditorUI::EditorUIService::BeginTabBar("InputModePanelTabBar");
			// Keyboard Panel
			if (EditorUI::EditorUIService::BeginTabItem("Keyboard"))
			{
				// On Update
				EditorUI::EditorUIService::Table(s_KeyboardOnUpdateTable);
				EditorUI::EditorUIService::GenericPopup(s_KeyboardOnUpdateAddSlot);
				EditorUI::EditorUIService::GenericPopup(s_KeyboardOnUpdateEditSlot);

				// On Key Pressed
				EditorUI::EditorUIService::Table(s_KeyboardOnKeyPressedTable);
				EditorUI::EditorUIService::GenericPopup(s_KeyboardOnKeyPressedAddSlot);
				EditorUI::EditorUIService::GenericPopup(s_KeyboardOnKeyPressedEditSlot);

				// Keyboard Polling
				EditorUI::EditorUIService::Table(s_KeyboardPollingTable);
				EditorUI::EditorUIService::GenericPopup(s_KeyboardPollingAddSlot);
				EditorUI::EditorUIService::GenericPopup(s_KeyboardPollingEditSlot);

				EditorUI::EditorUIService::EndTabItem();
			}
			// Mouse Panel
			if (EditorUI::EditorUIService::BeginTabItem("Mouse"))
			{
				EditorUI::EditorUIService::Text("Unimplemented Yet????");
				EditorUI::EditorUIService::EndTabItem();
			}
			EditorUI::EditorUIService::EndTabBar();
			
		}

		EditorUI::EditorUIService::EndWindow();
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
		s_OpenInputModePopupSpec.PopupAction = [&]()
		{
			s_OpenInputModePopupSpec.GetAllOptions().clear();
			s_OpenInputModePopupSpec.CurrentOption = { "None", Assets::EmptyHandle };

			s_OpenInputModePopupSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetManager::GetInputModeRegistry())
			{
				s_OpenInputModePopupSpec.AddToOptions("All Options", asset.Data.IntermediateLocation.string(), handle);
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
			EditorUI::EditorUIService::TextInputPopup(s_SelectInputModeNameSpec);
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
			EditorUI::EditorUIService::Text("Are you sure you want to delete this input mode object?");
		};

		s_CloseInputModeWarning.Label = "Close Input Mode";
		s_CloseInputModeWarning.ConfirmAction = [&]()
		{
			m_EditorInputModeHandle = 0;
			m_EditorInputMode = nullptr;
		};
		s_CloseInputModeWarning.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to close this input mode object without saving?");
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
			uint32_t iteration{ 0 };
			for (auto& inputBinding : m_EditorInputMode->GetOnUpdateBindings())
			{

				if (inputBinding->GetActionType() != Input::KeyboardAction)
				{
					continue;
				}
				Input::KeyboardActionBinding* keyboardBinding = (Input::KeyboardActionBinding*)inputBinding.get();

				static std::function<void(EditorUI::TableEntry&)> s_EditKeyboardSlot = [&](EditorUI::TableEntry& entry)
				{
					s_KeyboardOnUpdateActiveSlot = (uint32_t)entry.Handle;
					s_KeyboardOnUpdateEditSlot.PopupActive = true;
				};

				EditorUI::TableEntry newEntry;
				Assets::AssetHandle scriptHandle = keyboardBinding->GetScriptHandle();
				if (scriptHandle == Assets::EmptyHandle)
				{
					newEntry = {
						"Key::" + Utility::KeyCodeToString(keyboardBinding->GetKeyBinding()),
						"None",
						iteration,
						s_EditKeyboardSlot,
						nullptr
					};
				}
				else
				{
					Ref<Scripting::Script> script = Assets::AssetManager::GetScript(scriptHandle);
					std::string displayValue {Utility::ScriptTypeToString(script->m_ScriptType) + "::" + script->m_SectionLabel + "::" + script->m_ScriptName };
					newEntry = {
						"Key::" + Utility::KeyCodeToString(keyboardBinding->GetKeyBinding()),
						displayValue,
						iteration,
						s_EditKeyboardSlot,
						nullptr
					};
				}

				s_KeyboardOnUpdateTable.InsertTableEntry(newEntry);
				iteration++;
			}
		};
		s_KeyboardOnUpdateTable.Column1Title = "KeyCode";
		s_KeyboardOnUpdateTable.Column2Title = "Function";
		s_KeyboardOnUpdateTable.AddToSelectionList("Add New Slot", [&]()
		{
			s_KeyboardOnUpdateAddSlot.PopupActive = true;
		});

		s_KeyboardOnUpdateAddSlot.Label = "Add New Slot";
		s_KeyboardOnUpdateAddSlot.PopupWidth = 420.0f;
		s_KeyboardOnUpdateAddSlot.PopupAction = [&]()
		{
			s_KeyboardOnUpdateAddKeyCode.CurrentOption = {Utility::KeyCodeToString(Key::A), Key::A};
			s_KeyboardOnUpdateAddFunction.CurrentOption = {"None", Assets::EmptyHandle};
		};
		s_KeyboardOnUpdateAddSlot.PopupContents = [&]()
		{
			EditorUI::EditorUIService::SelectOption(s_KeyboardOnUpdateAddKeyCode);
			EditorUI::EditorUIService::SelectOption(s_KeyboardOnUpdateAddFunction);
		};

		s_KeyboardOnUpdateAddSlot.ConfirmAction = [&]()
		{
			auto& keyboardBindings = m_EditorInputMode->GetOnUpdateBindings();
			Ref<Input::KeyboardActionBinding> newBinding = CreateRef<Input::KeyboardActionBinding>();
			newBinding->SetKeyBinding((KeyCode)s_KeyboardOnUpdateAddKeyCode.CurrentOption.Handle);
			Ref<Scripting::Script> script;
			if (s_KeyboardOnUpdateAddFunction.CurrentOption.Handle == Assets::EmptyHandle)
			{
				script = nullptr;
			}
			else
			{
				script = Assets::AssetManager::GetScript(s_KeyboardOnUpdateAddFunction.CurrentOption.Handle);
			}
			newBinding->SetScript(script, s_KeyboardOnUpdateAddFunction.CurrentOption.Handle);

			keyboardBindings.push_back(newBinding);
			OnRefreshData();
			s_MainHeader.EditColorActive = true;
		};

		s_KeyboardOnUpdateAddKeyCode.Label = "Select Key";
		s_KeyboardOnUpdateAddKeyCode.LineCount = 7;
		s_KeyboardOnUpdateAddKeyCode.PopupAction = [&]()
		{
			s_KeyboardOnUpdateAddKeyCode.ClearOptions();
			for (auto key : Key::s_AllKeyCodes)
			{
				s_KeyboardOnUpdateAddKeyCode.AddToOptions("All KeyCodes", Utility::KeyCodeToString(key), key);
			}
		};

		s_KeyboardOnUpdateAddFunction.Label = "Select Function";
		s_KeyboardOnUpdateAddFunction.PopupAction = [&]()
		{
			s_KeyboardOnUpdateAddFunction.ClearOptions();
			s_KeyboardOnUpdateAddFunction.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [uuid, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_ScriptType == Scripting::ScriptType::Class)
				{
					if (script->m_FuncType != WrappedFuncType::Void_UInt64 && script->m_FuncType != WrappedFuncType::Void_UInt64Float)
					{
						continue;
					}
					s_KeyboardOnUpdateAddFunction.AddToOptions( Utility::ScriptTypeToString(script->m_ScriptType) + 
						"::" + script->m_SectionLabel, script->m_ScriptName, uuid);
				}
				if (script->m_ScriptType == Scripting::ScriptType::Global || script->m_ScriptType == Scripting::ScriptType::Engine)
				{
					if (script->m_FuncType != WrappedFuncType::Void_None && script->m_FuncType != WrappedFuncType::Void_Float)
					{
						continue;
					}
					s_KeyboardOnUpdateAddFunction.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType) + 
						"::" + script->m_SectionLabel, script->m_ScriptName, uuid);
				}
			}
		};

		s_KeyboardOnUpdateEditSlot.Label = "Edit Slot";
		s_KeyboardOnUpdateEditSlot.PopupWidth = 420.0f;
		s_KeyboardOnUpdateEditSlot.PopupAction = [&]()
		{
			Input::KeyboardActionBinding* activeBinding = (Input::KeyboardActionBinding*)m_EditorInputMode->GetOnUpdateBindings().at(s_KeyboardOnUpdateActiveSlot).get();
			KG_ASSERT(activeBinding);

			s_KeyboardOnUpdateEditKeyCode.CurrentOption = { Utility::KeyCodeToString(activeBinding->GetKeyBinding()), activeBinding->GetKeyBinding() };
			if (activeBinding->GetScriptHandle() == Assets::EmptyHandle)
			{
				s_KeyboardOnUpdateEditFunction.CurrentOption = { "None", Assets::EmptyHandle };
			}
			else
			{
				KG_ASSERT(activeBinding->GetScript());
				s_KeyboardOnUpdateEditFunction.CurrentOption = { activeBinding->GetScript()->m_ScriptName, activeBinding->GetScriptHandle()};
			}
		};
		s_KeyboardOnUpdateEditSlot.PopupContents = [&]()
		{
			EditorUI::EditorUIService::SelectOption(s_KeyboardOnUpdateEditKeyCode);
			EditorUI::EditorUIService::SelectOption(s_KeyboardOnUpdateEditFunction);
		};
		s_KeyboardOnUpdateEditSlot.DeleteAction = [&]()
		{
			std::vector<Ref<Input::InputActionBinding>>& bindings = m_EditorInputMode->GetOnUpdateBindings();
			bindings.erase(bindings.begin() + s_KeyboardOnUpdateActiveSlot);
			OnRefreshData();
			s_MainHeader.EditColorActive = true;
		};

		s_KeyboardOnUpdateEditSlot.ConfirmAction = [&]()
		{
			Input::KeyboardActionBinding* newBinding = (Input::KeyboardActionBinding*)m_EditorInputMode->GetOnUpdateBindings().at(s_KeyboardOnUpdateActiveSlot).get();
			KG_ASSERT(newBinding);
			newBinding->SetKeyBinding((KeyCode)s_KeyboardOnUpdateEditKeyCode.CurrentOption.Handle);
			Ref<Scripting::Script> script;
			if (s_KeyboardOnUpdateEditFunction.CurrentOption.Handle == Assets::EmptyHandle)
			{
				script = nullptr;
			}
			else
			{
				script = Assets::AssetManager::GetScript(s_KeyboardOnUpdateEditFunction.CurrentOption.Handle);
			}
			newBinding->SetScript(script, s_KeyboardOnUpdateEditFunction.CurrentOption.Handle);

			OnRefreshData();
			s_MainHeader.EditColorActive = true;
		};

		s_KeyboardOnUpdateEditKeyCode.Label = "Select Key";
		s_KeyboardOnUpdateEditKeyCode.LineCount = 7;
		s_KeyboardOnUpdateEditKeyCode.PopupAction = [&]()
		{
			s_KeyboardOnUpdateEditKeyCode.ClearOptions();
			for (auto key : Key::s_AllKeyCodes)
			{
				s_KeyboardOnUpdateEditKeyCode.AddToOptions("All KeyCodes", Utility::KeyCodeToString(key), key);
			}
		};

		s_KeyboardOnUpdateEditFunction.Label = "Select Function";
		s_KeyboardOnUpdateEditFunction.PopupAction = [&]()
		{
			s_KeyboardOnUpdateEditFunction.ClearOptions();
			s_KeyboardOnUpdateEditFunction.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [uuid, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_ScriptType == Scripting::ScriptType::Class)
				{
					if (script->m_FuncType != WrappedFuncType::Void_UInt64 && script->m_FuncType != WrappedFuncType::Void_UInt64Float)
					{
						continue;
					}
					s_KeyboardOnUpdateEditFunction.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType) +
						"::" + script->m_SectionLabel, script->m_ScriptName, uuid);
				}
				if (script->m_ScriptType == Scripting::ScriptType::Global || script->m_ScriptType == Scripting::ScriptType::Engine)
				{
					if (script->m_FuncType != WrappedFuncType::Void_None && script->m_FuncType != WrappedFuncType::Void_Float)
					{
						continue;
					}
					s_KeyboardOnUpdateEditFunction.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType) +
						"::" + script->m_SectionLabel, script->m_ScriptName, uuid);
				}
			}
		};


		// On Key Pressed Table
		s_KeyboardOnKeyPressedTable.Label = "On Key Pressed";
		s_KeyboardOnKeyPressedTable.Expanded = true;
		s_KeyboardOnKeyPressedTable.OnRefresh = [&]()
		{
			s_KeyboardOnKeyPressedTable.ClearTable();
			uint32_t iteration{ 0 };
			for (auto& inputBinding : m_EditorInputMode->GetOnKeyPressedBindings())
			{

				if (inputBinding->GetActionType() != Input::KeyboardAction)
				{
					continue;
				}
				Input::KeyboardActionBinding* keyboardBinding = (Input::KeyboardActionBinding*)inputBinding.get();

				static std::function<void(EditorUI::TableEntry&)> s_EditKeyboardSlot = [&](EditorUI::TableEntry& entry)
				{
					s_KeyboardOnKeyPressedActiveSlot = (uint32_t)entry.Handle;
					s_KeyboardOnKeyPressedEditSlot.PopupActive = true;
				};

				EditorUI::TableEntry newEntry;
				Assets::AssetHandle scriptHandle = keyboardBinding->GetScriptHandle();
				if (scriptHandle == Assets::EmptyHandle)
				{
					newEntry = {
						"Key::" + Utility::KeyCodeToString(keyboardBinding->GetKeyBinding()),
						"None",
						iteration,
						s_EditKeyboardSlot,
						nullptr
					};
				}
				else
				{
					Ref<Scripting::Script> script = Assets::AssetManager::GetScript(scriptHandle);
					std::string displayValue {Utility::ScriptTypeToString(script->m_ScriptType) + "::" + script->m_SectionLabel + ":" + script->m_ScriptName };
					newEntry = {
						"Key::" + Utility::KeyCodeToString(keyboardBinding->GetKeyBinding()),
						displayValue,
						iteration,
						s_EditKeyboardSlot,
						nullptr
					};
				}

				s_KeyboardOnKeyPressedTable.InsertTableEntry(newEntry);
				iteration++;
			}
		};
		s_KeyboardOnKeyPressedTable.Column1Title = "KeyCode";
		s_KeyboardOnKeyPressedTable.Column2Title = "Function";
		s_KeyboardOnKeyPressedTable.AddToSelectionList("Add New Slot", [&]()
			{
				s_KeyboardOnKeyPressedAddSlot.PopupActive = true;
			});

		s_KeyboardOnKeyPressedAddSlot.Label = "Add New Slot";
		s_KeyboardOnKeyPressedAddSlot.PopupWidth = 420.0f;
		s_KeyboardOnKeyPressedAddSlot.PopupAction = [&]()
		{
			s_KeyboardOnKeyPressedAddKeyCode.CurrentOption = { Utility::KeyCodeToString(Key::A), Key::A };
			s_KeyboardOnKeyPressedAddFunction.CurrentOption = { "None", Assets::EmptyHandle };
		};
		s_KeyboardOnKeyPressedAddSlot.PopupContents = [&]()
		{
			EditorUI::EditorUIService::SelectOption(s_KeyboardOnKeyPressedAddKeyCode);
			EditorUI::EditorUIService::SelectOption(s_KeyboardOnKeyPressedAddFunction);
		};

		s_KeyboardOnKeyPressedAddSlot.ConfirmAction = [&]()
		{
			auto& keyboardBindings = m_EditorInputMode->GetOnKeyPressedBindings();
			Ref<Input::KeyboardActionBinding> newBinding = CreateRef<Input::KeyboardActionBinding>();
			newBinding->SetKeyBinding((KeyCode)s_KeyboardOnKeyPressedAddKeyCode.CurrentOption.Handle);
			Ref<Scripting::Script> script;
			if (s_KeyboardOnKeyPressedAddFunction.CurrentOption.Handle == Assets::EmptyHandle)
			{
				script = nullptr;
			}
			else
			{
				script = Assets::AssetManager::GetScript(s_KeyboardOnKeyPressedAddFunction.CurrentOption.Handle);
			}
			newBinding->SetScript(script, s_KeyboardOnKeyPressedAddFunction.CurrentOption.Handle);

			keyboardBindings.push_back(newBinding);
			OnRefreshData();
			s_MainHeader.EditColorActive = true;
		};

		s_KeyboardOnKeyPressedAddKeyCode.Label = "Select Key";
		s_KeyboardOnKeyPressedAddKeyCode.LineCount = 7;
		s_KeyboardOnKeyPressedAddKeyCode.PopupAction = [&]()
		{
			s_KeyboardOnKeyPressedAddKeyCode.ClearOptions();
			for (auto key : Key::s_AllKeyCodes)
			{
				s_KeyboardOnKeyPressedAddKeyCode.AddToOptions("All KeyCodes", Utility::KeyCodeToString(key), key);
			}
		};

		s_KeyboardOnKeyPressedAddFunction.Label = "Select Function";
		s_KeyboardOnKeyPressedAddFunction.PopupAction = [&]()
		{
			s_KeyboardOnKeyPressedAddFunction.ClearOptions();
			s_KeyboardOnKeyPressedAddFunction.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [uuid, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_ScriptType == Scripting::ScriptType::Class)
				{
					if (script->m_FuncType != WrappedFuncType::Void_UInt64)
					{
						continue;
					}
					s_KeyboardOnKeyPressedAddFunction.AddToOptions( Utility::ScriptTypeToString(script->m_ScriptType) 
						+ "::" + script->m_SectionLabel, script->m_ScriptName, uuid);
				}
				if (script->m_ScriptType == Scripting::ScriptType::Global || script->m_ScriptType == Scripting::ScriptType::Engine)
				{
					if (script->m_FuncType != WrappedFuncType::Void_None)
					{
						continue;
					}
					s_KeyboardOnKeyPressedAddFunction.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
						+ "::" + script->m_SectionLabel, script->m_ScriptName, uuid);
				}
			}
		};

		s_KeyboardOnKeyPressedEditSlot.Label = "Edit Slot";
		s_KeyboardOnKeyPressedEditSlot.PopupWidth = 420.0f;
		s_KeyboardOnKeyPressedEditSlot.PopupAction = [&]()
		{
			Input::KeyboardActionBinding* activeBinding = (Input::KeyboardActionBinding*)m_EditorInputMode->GetOnKeyPressedBindings().at(s_KeyboardOnKeyPressedActiveSlot).get();
			KG_ASSERT(activeBinding);

			s_KeyboardOnKeyPressedEditKeyCode.CurrentOption = { Utility::KeyCodeToString(activeBinding->GetKeyBinding()), activeBinding->GetKeyBinding() };
			if (activeBinding->GetScriptHandle() == Assets::EmptyHandle)
			{
				s_KeyboardOnKeyPressedEditFunction.CurrentOption = { "None", Assets::EmptyHandle };
			}
			else
			{
				KG_ASSERT(activeBinding->GetScript());
				s_KeyboardOnKeyPressedEditFunction.CurrentOption = { activeBinding->GetScript()->m_ScriptName, activeBinding->GetScriptHandle() };
			}
		};
		s_KeyboardOnKeyPressedEditSlot.PopupContents = [&]()
		{
			EditorUI::EditorUIService::SelectOption(s_KeyboardOnKeyPressedEditKeyCode);
			EditorUI::EditorUIService::SelectOption(s_KeyboardOnKeyPressedEditFunction);
		};
		s_KeyboardOnKeyPressedEditSlot.DeleteAction = [&]()
		{
			std::vector<Ref<Input::InputActionBinding>>& bindings = m_EditorInputMode->GetOnKeyPressedBindings();
			bindings.erase(bindings.begin() + s_KeyboardOnKeyPressedActiveSlot);
			OnRefreshData();
			s_MainHeader.EditColorActive = true;
		};

		s_KeyboardOnKeyPressedEditSlot.ConfirmAction = [&]()
		{
			Input::KeyboardActionBinding* newBinding = (Input::KeyboardActionBinding*)m_EditorInputMode->GetOnKeyPressedBindings().at(s_KeyboardOnKeyPressedActiveSlot).get();
			KG_ASSERT(newBinding);
			newBinding->SetKeyBinding((KeyCode)s_KeyboardOnKeyPressedEditKeyCode.CurrentOption.Handle);
			Ref<Scripting::Script> script;
			if (s_KeyboardOnKeyPressedEditFunction.CurrentOption.Handle == Assets::EmptyHandle)
			{
				script = nullptr;
			}
			else
			{
				script = Assets::AssetManager::GetScript(s_KeyboardOnKeyPressedEditFunction.CurrentOption.Handle);
			}
			newBinding->SetScript(script, s_KeyboardOnKeyPressedEditFunction.CurrentOption.Handle);

			OnRefreshData();
			s_MainHeader.EditColorActive = true;
		};

		s_KeyboardOnKeyPressedEditKeyCode.Label = "Select Key";
		s_KeyboardOnKeyPressedEditKeyCode.LineCount = 7;
		s_KeyboardOnKeyPressedEditKeyCode.PopupAction = [&]()
		{
			s_KeyboardOnKeyPressedEditKeyCode.ClearOptions();
			for (auto key : Key::s_AllKeyCodes)
			{
				s_KeyboardOnKeyPressedEditKeyCode.AddToOptions("All KeyCodes", Utility::KeyCodeToString(key), key);
			}
		};

		s_KeyboardOnKeyPressedEditFunction.Label = "Select Function";
		s_KeyboardOnKeyPressedEditFunction.PopupAction = [&]()
		{
			s_KeyboardOnKeyPressedEditFunction.ClearOptions();
			s_KeyboardOnKeyPressedEditFunction.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [uuid, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_ScriptType == Scripting::ScriptType::Class)
				{
					if (script->m_FuncType != WrappedFuncType::Void_UInt64)
					{
						continue;
					}
					s_KeyboardOnKeyPressedEditFunction.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
						+ "::" + script->m_SectionLabel, script->m_ScriptName, uuid);
				}
				if (script->m_ScriptType == Scripting::ScriptType::Global || script->m_ScriptType == Scripting::ScriptType::Engine)
				{
					if (script->m_FuncType != WrappedFuncType::Void_None)
					{
						continue;
					}
					s_KeyboardOnKeyPressedEditFunction.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
						+ "::" + script->m_SectionLabel, script->m_ScriptName, uuid);
				}
			}
		};

		// Polling Table
		s_KeyboardPollingTable.Label = "Polling";
		s_KeyboardPollingTable.Expanded = true;
		s_KeyboardPollingTable.OnRefresh = [&]()
		{
			s_KeyboardPollingTable.ClearTable();
			uint32_t iteration{ 0 };
			for (auto code: m_EditorInputMode->GetKeyboardPolling())
			{

				static std::function<void(EditorUI::TableEntry&)> s_EditKeyboardSlot = [&](EditorUI::TableEntry& entry)
				{
					s_KeyboardPollingActiveSlot = (uint32_t)entry.Handle;
					s_KeyboardPollingEditSlot.PopupActive = true;
				};

				EditorUI::TableEntry newEntry = {
					"Slot::" + std::to_string(iteration),
					"Key::" + Utility::KeyCodeToString(code),
					iteration,
					s_EditKeyboardSlot,
					nullptr
				};

				s_KeyboardPollingTable.InsertTableEntry(newEntry);
				iteration++;
			}
		};
		s_KeyboardPollingTable.Column1Title = "Slot";
		s_KeyboardPollingTable.Column2Title = "KeyCode";
		s_KeyboardPollingTable.AddToSelectionList("Add New Slot", [&]()
		{
			s_KeyboardPollingAddSlot.PopupActive = true;
		});

		s_KeyboardPollingAddSlot.Label = "Add New Slot";
		s_KeyboardPollingAddSlot.PopupWidth = 420.0f;
		s_KeyboardPollingAddSlot.PopupAction = [&]()
		{
			s_KeyboardPollingAddKeyCode.CurrentOption = { Utility::KeyCodeToString(Key::A), Key::A };
		};
		s_KeyboardPollingAddSlot.PopupContents = [&]()
		{
			EditorUI::EditorUIService::SelectOption(s_KeyboardPollingAddKeyCode);
		};

		s_KeyboardPollingAddSlot.ConfirmAction = [&]()
		{
			auto& keyboardPolling = m_EditorInputMode->GetKeyboardPolling();
			KeyCode code = (KeyCode)s_KeyboardPollingAddKeyCode.CurrentOption.Handle;
			keyboardPolling.push_back(code);
			OnRefreshData();
			s_MainHeader.EditColorActive = true;
		};

		s_KeyboardPollingAddKeyCode.Label = "Select Key";
		s_KeyboardPollingAddKeyCode.LineCount = 7;
		s_KeyboardPollingAddKeyCode.PopupAction = [&]()
		{
			s_KeyboardPollingAddKeyCode.ClearOptions();
			for (auto key : Key::s_AllKeyCodes)
			{
				s_KeyboardPollingAddKeyCode.AddToOptions("All KeyCodes", Utility::KeyCodeToString(key), key);
			}
		};

		s_KeyboardPollingEditSlot.Label = "Edit Slot";
		s_KeyboardPollingEditSlot.PopupWidth = 420.0f;
		s_KeyboardPollingEditSlot.PopupAction = [&]()
		{
			KeyCode activeCode = m_EditorInputMode->GetKeyboardPolling().at(s_KeyboardPollingActiveSlot);

			s_KeyboardPollingEditKeyCode.CurrentOption = { Utility::KeyCodeToString(activeCode), activeCode };
		};
		s_KeyboardPollingEditSlot.PopupContents = [&]()
		{
			EditorUI::EditorUIService::SelectOption(s_KeyboardPollingEditKeyCode);
		};

		s_KeyboardPollingEditSlot.DeleteAction = [&]()
		{
			std::vector<KeyCode>& codes = m_EditorInputMode->GetKeyboardPolling();
			codes.erase(codes.begin() + s_KeyboardPollingActiveSlot);
			OnRefreshData();
			s_MainHeader.EditColorActive = true;
		};

		s_KeyboardPollingEditSlot.ConfirmAction = [&]()
		{
			auto& keyboardPolling = m_EditorInputMode->GetKeyboardPolling();
			KeyCode code = (KeyCode)s_KeyboardPollingEditKeyCode.CurrentOption.Handle;
			keyboardPolling[s_KeyboardPollingActiveSlot] = code;
			OnRefreshData();
			s_MainHeader.EditColorActive = true;
		};

		s_KeyboardPollingEditKeyCode.Label = "Select Key";
		s_KeyboardPollingEditKeyCode.LineCount = 7;
		s_KeyboardPollingEditKeyCode.PopupAction = [&]()
		{
			s_KeyboardPollingEditKeyCode.ClearOptions();
			for (auto key : Key::s_AllKeyCodes)
			{
				s_KeyboardPollingEditKeyCode.AddToOptions("All KeyCodes", Utility::KeyCodeToString(key), key);
			}
		};
	}
	
}

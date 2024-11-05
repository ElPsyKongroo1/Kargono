#include "Panels/InputMapPanel.h"
#include "EditorApp.h"

namespace Kargono
{
	static EditorApp* s_EditorApp { nullptr };
}

namespace Kargono::Panels
{
	void InputMapPanel::OnOpenInputMap()
	{
		m_OpenInputMapPopupSpec.PopupActive = true;
	}
	void InputMapPanel::OnCreateInputMap()
	{
		m_CreateInputMapPopupSpec.PopupActive = true;
	}

	void InputMapPanel::OnRefreshData()
	{
		m_KeyboardOnUpdateTable.OnRefresh();
		m_KeyboardOnKeyPressedTable.OnRefresh();
		m_KeyboardPollingTable.OnRefresh();
	}

	InputMapPanel::InputMapPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(InputMapPanel::OnKeyPressedEditor));
		InitializeOpeningScreen();
		InitializeInputMapHeader();
		InitializeKeyboardScreen();
	}
	void InputMapPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION()
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowInputMapEditor);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		if (!m_EditorInputMap)
		{
			// Opening/Null State Screen
			EditorUI::EditorUIService::NewItemScreen("Open Existing Input Map", KG_BIND_CLASS_FN(OnOpenInputMap), "Create New Input Map", KG_BIND_CLASS_FN(OnCreateInputMap));
			EditorUI::EditorUIService::GenericPopup(m_CreateInputMapPopupSpec);
			EditorUI::EditorUIService::SelectOption(m_OpenInputMapPopupSpec);
		}
		else
		{
			// Header
			EditorUI::EditorUIService::PanelHeader(m_MainHeader);
			EditorUI::EditorUIService::GenericPopup(m_DeleteInputMapWarning);
			EditorUI::EditorUIService::GenericPopup(m_CloseInputMapWarning);

			// Main Content
			EditorUI::EditorUIService::BeginTabBar("InputMapPanelTabBar");
			// Keyboard Panel
			if (EditorUI::EditorUIService::BeginTabItem("Keyboard"))
			{
				// On Update
				EditorUI::EditorUIService::Table(m_KeyboardOnUpdateTable);
				EditorUI::EditorUIService::GenericPopup(m_KeyboardOnUpdateAddSlot);
				EditorUI::EditorUIService::GenericPopup(m_KeyboardOnUpdateEditSlot);

				// On Key Pressed
				EditorUI::EditorUIService::Table(m_KeyboardOnKeyPressedTable);
				EditorUI::EditorUIService::GenericPopup(m_KeyboardOnKeyPressedAddSlot);
				EditorUI::EditorUIService::GenericPopup(m_KeyboardOnKeyPressedEditSlot);

				// Keyboard Polling
				EditorUI::EditorUIService::Table(m_KeyboardPollingTable);
				EditorUI::EditorUIService::GenericPopup(m_KeyboardPollingAddSlot);
				EditorUI::EditorUIService::GenericPopup(m_KeyboardPollingEditSlot);

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
	bool InputMapPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}

	void InputMapPanel::ResetPanelResources()
	{
		m_EditorInputMap = nullptr;
		m_EditorInputMapHandle = Assets::EmptyHandle;
	}

	void InputMapPanel::InitializeOpeningScreen()
	{
		m_OpenInputMapPopupSpec.Label = "Open Input Map";
		m_OpenInputMapPopupSpec.LineCount = 2;
		m_OpenInputMapPopupSpec.CurrentOption = { "None", Assets::EmptyHandle };
		m_OpenInputMapPopupSpec.Flags |= EditorUI::SelectOption_PopupOnly;
		m_OpenInputMapPopupSpec.PopupAction = [&]()
		{
			m_OpenInputMapPopupSpec.GetAllOptions().clear();
			m_OpenInputMapPopupSpec.CurrentOption = { "None", Assets::EmptyHandle };

			m_OpenInputMapPopupSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetInputMapRegistry())
			{
				m_OpenInputMapPopupSpec.AddToOptions("All Options", asset.Data.FileLocation.string(), handle);
			}
		};

		m_OpenInputMapPopupSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.Handle == Assets::EmptyHandle)
			{
				KG_WARN("No Input Map Selected");
				return;
			}
			if (!Assets::AssetService::GetInputMapRegistry().contains(selection.Handle))
			{
				KG_WARN("Could not find the input map specified");
				return;
			}

			m_EditorInputMap = Assets::AssetService::GetInputMap(selection.Handle);
			m_EditorInputMapHandle = selection.Handle;
			m_MainHeader.EditColorActive = false;
			m_MainHeader.Label = Assets::AssetService::GetInputMapRegistry().at(
				m_EditorInputMapHandle).Data.FileLocation.string();
			OnRefreshData();
		};

		m_SelectInputMapNameSpec.Label = "New Name";
		m_SelectInputMapNameSpec.CurrentOption = "Empty";

		m_CreateInputMapPopupSpec.Label = "Create Input Map";
		m_CreateInputMapPopupSpec.PopupWidth = 420.0f;
		m_CreateInputMapPopupSpec.ConfirmAction = [&]()
		{
			if (m_SelectInputMapNameSpec.CurrentOption == "")
			{
				return;
			}

			m_EditorInputMapHandle = Assets::AssetService::CreateInputMap(m_SelectInputMapNameSpec.CurrentOption.c_str());
			if (m_EditorInputMapHandle == Assets::EmptyHandle)
			{
				KG_WARN("Input Map was not created");
				return;
			}
			m_EditorInputMap = Assets::AssetService::GetInputMap(m_EditorInputMapHandle);
			m_MainHeader.EditColorActive = false;
			m_MainHeader.Label = Assets::AssetService::GetInputMapRegistry().at(
				m_EditorInputMapHandle).Data.FileLocation.string();
			OnRefreshData();
		};
		m_CreateInputMapPopupSpec.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_SelectInputMapNameSpec);
		};
	}

	void InputMapPanel::InitializeInputMapHeader()
	{
		// Header (Game State Name and Options)
		m_DeleteInputMapWarning.Label = "Delete Input Map";
		m_DeleteInputMapWarning.ConfirmAction = [&]()
		{
			Assets::AssetService::DeleteInputMap(m_EditorInputMapHandle);
			m_EditorInputMapHandle = 0;
			m_EditorInputMap = nullptr;
		};
		m_DeleteInputMapWarning.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to delete this input map object?");
		};

		m_CloseInputMapWarning.Label = "Close Input Map";
		m_CloseInputMapWarning.ConfirmAction = [&]()
		{
			m_EditorInputMapHandle = 0;
			m_EditorInputMap = nullptr;
		};
		m_CloseInputMapWarning.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to close this input map object without saving?");
		};

		m_MainHeader.AddToSelectionList("Save", [&]()
		{
			Assets::AssetService::SaveInputMap(m_EditorInputMapHandle, m_EditorInputMap);
			m_MainHeader.EditColorActive = false;
		});
		m_MainHeader.AddToSelectionList("Close", [&]()
		{
			if (m_MainHeader.EditColorActive)
			{
				m_CloseInputMapWarning.PopupActive = true;
			}
			else
			{
				m_EditorInputMapHandle = 0;
				m_EditorInputMap = nullptr;
			}
		});
		m_MainHeader.AddToSelectionList("Delete", [&]()
		{
			m_DeleteInputMapWarning.PopupActive = true;
		});
	}

	void InputMapPanel::InitializeKeyboardScreen()
	{
		// On Update Table
		m_KeyboardOnUpdateTable.Label = "On Update";
		m_KeyboardOnUpdateTable.Expanded = true;
		m_KeyboardOnUpdateTable.OnRefresh = [&]()
		{
			m_KeyboardOnUpdateTable.ClearTable();
			uint32_t iteration{ 0 };
			for (auto& inputBinding : m_EditorInputMap->GetOnUpdateBindings())
			{

				if (inputBinding->GetActionType() != Input::KeyboardAction)
				{
					continue;
				}
				Input::KeyboardActionBinding* keyboardBinding = (Input::KeyboardActionBinding*)inputBinding.get();

				static std::function<void(EditorUI::TableEntry&)> m_EditKeyboardSlot = [&](EditorUI::TableEntry& entry)
				{
					m_KeyboardOnUpdateActiveSlot = (uint32_t)entry.Handle;
					m_KeyboardOnUpdateEditSlot.PopupActive = true;
				};

				EditorUI::TableEntry newEntry;
				Assets::AssetHandle scriptHandle = keyboardBinding->GetScriptHandle();
				if (scriptHandle == Assets::EmptyHandle)
				{
					newEntry = {
						"Key::" + Utility::KeyCodeToString(keyboardBinding->GetKeyBinding()),
						"None",
						iteration,
						m_EditKeyboardSlot,
						nullptr
					};
				}
				else
				{
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
					newEntry = {
						"Key::" + Utility::KeyCodeToString(keyboardBinding->GetKeyBinding()),
						Utility::ScriptToString(script),
						iteration,
						m_EditKeyboardSlot,
						nullptr
					};
				}

				m_KeyboardOnUpdateTable.InsertTableEntry(newEntry);
				iteration++;
			}
		};
		m_KeyboardOnUpdateTable.Column1Title = "KeyCode";
		m_KeyboardOnUpdateTable.Column2Title = "Function";
		m_KeyboardOnUpdateTable.AddToSelectionList("Add New Slot", [&]()
		{
			m_KeyboardOnUpdateAddSlot.PopupActive = true;
		});

		m_KeyboardOnUpdateAddSlot.Label = "Add New Slot";
		m_KeyboardOnUpdateAddSlot.PopupWidth = 420.0f;
		m_KeyboardOnUpdateAddSlot.PopupAction = [&]()
		{
			m_KeyboardOnUpdateAddKeyCode.CurrentOption = {Utility::KeyCodeToString(Key::A), Key::A};
			m_KeyboardOnUpdateAddFunction.CurrentOption = {"None", Assets::EmptyHandle};
		};
		m_KeyboardOnUpdateAddSlot.PopupContents = [&]()
		{
			EditorUI::EditorUIService::SelectOption(m_KeyboardOnUpdateAddKeyCode);
			EditorUI::EditorUIService::SelectOption(m_KeyboardOnUpdateAddFunction);
		};

		m_KeyboardOnUpdateAddSlot.ConfirmAction = [&]()
		{
			auto& keyboardBindings = m_EditorInputMap->GetOnUpdateBindings();
			Ref<Input::KeyboardActionBinding> newBinding = CreateRef<Input::KeyboardActionBinding>();
			newBinding->SetKeyBinding((KeyCode)m_KeyboardOnUpdateAddKeyCode.CurrentOption.Handle);
			Ref<Scripting::Script> script;
			if (m_KeyboardOnUpdateAddFunction.CurrentOption.Handle == Assets::EmptyHandle)
			{
				script = nullptr;
			}
			else
			{
				script = Assets::AssetService::GetScript(m_KeyboardOnUpdateAddFunction.CurrentOption.Handle);
			}
			newBinding->SetScript(script, m_KeyboardOnUpdateAddFunction.CurrentOption.Handle);

			keyboardBindings.push_back(newBinding);
			OnRefreshData();
			m_MainHeader.EditColorActive = true;
		};

		m_KeyboardOnUpdateAddKeyCode.Label = "Select Key";
		m_KeyboardOnUpdateAddKeyCode.LineCount = 7;
		m_KeyboardOnUpdateAddKeyCode.PopupAction = [&]()
		{
			m_KeyboardOnUpdateAddKeyCode.ClearOptions();
			for (auto key : Key::s_AllKeyCodes)
			{
				m_KeyboardOnUpdateAddKeyCode.AddToOptions("All KeyCodes", Utility::KeyCodeToString(key), key);
			}
		};

		m_KeyboardOnUpdateAddFunction.Label = "Select Function";
		m_KeyboardOnUpdateAddFunction.PopupAction = [&]()
		{
			m_KeyboardOnUpdateAddFunction.ClearOptions();
			m_KeyboardOnUpdateAddFunction.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [uuid, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_FuncType != WrappedFuncType::Void_None && script->m_FuncType != WrappedFuncType::Void_Float)
				{
					continue;
				}
				m_KeyboardOnUpdateAddFunction.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType) + 
					"::" + script->m_SectionLabel, script->m_ScriptName, uuid);
			}
		};

		m_KeyboardOnUpdateEditSlot.Label = "Edit Slot";
		m_KeyboardOnUpdateEditSlot.PopupWidth = 420.0f;
		m_KeyboardOnUpdateEditSlot.PopupAction = [&]()
		{
			Input::KeyboardActionBinding* activeBinding = (Input::KeyboardActionBinding*)m_EditorInputMap->GetOnUpdateBindings().at(m_KeyboardOnUpdateActiveSlot).get();
			KG_ASSERT(activeBinding);

			m_KeyboardOnUpdateEditKeyCode.CurrentOption = { Utility::KeyCodeToString(activeBinding->GetKeyBinding()), activeBinding->GetKeyBinding() };
			if (activeBinding->GetScriptHandle() == Assets::EmptyHandle)
			{
				m_KeyboardOnUpdateEditFunction.CurrentOption = { "None", Assets::EmptyHandle };
			}
			else
			{
				KG_ASSERT(activeBinding->GetScript());
				m_KeyboardOnUpdateEditFunction.CurrentOption = { activeBinding->GetScript()->m_ScriptName, activeBinding->GetScriptHandle()};
			}
		};
		m_KeyboardOnUpdateEditSlot.PopupContents = [&]()
		{
			EditorUI::EditorUIService::SelectOption(m_KeyboardOnUpdateEditKeyCode);
			EditorUI::EditorUIService::SelectOption(m_KeyboardOnUpdateEditFunction);
		};
		m_KeyboardOnUpdateEditSlot.DeleteAction = [&]()
		{
			std::vector<Ref<Input::InputActionBinding>>& bindings = m_EditorInputMap->GetOnUpdateBindings();
			bindings.erase(bindings.begin() + m_KeyboardOnUpdateActiveSlot);
			OnRefreshData();
			m_MainHeader.EditColorActive = true;
		};

		m_KeyboardOnUpdateEditSlot.ConfirmAction = [&]()
		{
			Input::KeyboardActionBinding* newBinding = (Input::KeyboardActionBinding*)m_EditorInputMap->GetOnUpdateBindings().at(m_KeyboardOnUpdateActiveSlot).get();
			KG_ASSERT(newBinding);
			newBinding->SetKeyBinding((KeyCode)m_KeyboardOnUpdateEditKeyCode.CurrentOption.Handle);
			Ref<Scripting::Script> script;
			if (m_KeyboardOnUpdateEditFunction.CurrentOption.Handle == Assets::EmptyHandle)
			{
				script = nullptr;
			}
			else
			{
				script = Assets::AssetService::GetScript(m_KeyboardOnUpdateEditFunction.CurrentOption.Handle);
			}
			newBinding->SetScript(script, m_KeyboardOnUpdateEditFunction.CurrentOption.Handle);

			OnRefreshData();
			m_MainHeader.EditColorActive = true;
		};

		m_KeyboardOnUpdateEditKeyCode.Label = "Select Key";
		m_KeyboardOnUpdateEditKeyCode.LineCount = 7;
		m_KeyboardOnUpdateEditKeyCode.PopupAction = [&]()
		{
			m_KeyboardOnUpdateEditKeyCode.ClearOptions();
			for (auto key : Key::s_AllKeyCodes)
			{
				m_KeyboardOnUpdateEditKeyCode.AddToOptions("All KeyCodes", Utility::KeyCodeToString(key), key);
			}
		};

		m_KeyboardOnUpdateEditFunction.Label = "Select Function";
		m_KeyboardOnUpdateEditFunction.PopupAction = [&]()
		{
			m_KeyboardOnUpdateEditFunction.ClearOptions();
			m_KeyboardOnUpdateEditFunction.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [uuid, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_FuncType != WrappedFuncType::Void_None && script->m_FuncType != WrappedFuncType::Void_Float)
				{
					continue;
				}
				m_KeyboardOnUpdateEditFunction.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType) +
					"::" + script->m_SectionLabel, script->m_ScriptName, uuid);
			}
		};


		// On Key Pressed Table
		m_KeyboardOnKeyPressedTable.Label = "On Key Pressed";
		m_KeyboardOnKeyPressedTable.Expanded = true;
		m_KeyboardOnKeyPressedTable.OnRefresh = [&]()
		{
			m_KeyboardOnKeyPressedTable.ClearTable();
			uint32_t iteration{ 0 };
			for (auto& inputBinding : m_EditorInputMap->GetOnKeyPressedBindings())
			{

				if (inputBinding->GetActionType() != Input::KeyboardAction)
				{
					continue;
				}
				Input::KeyboardActionBinding* keyboardBinding = (Input::KeyboardActionBinding*)inputBinding.get();

				static std::function<void(EditorUI::TableEntry&)> m_EditKeyboardSlot = [&](EditorUI::TableEntry& entry)
				{
					m_KeyboardOnKeyPressedActiveSlot = (uint32_t)entry.Handle;
					m_KeyboardOnKeyPressedEditSlot.PopupActive = true;
				};

				EditorUI::TableEntry newEntry;
				Assets::AssetHandle scriptHandle = keyboardBinding->GetScriptHandle();
				if (scriptHandle == Assets::EmptyHandle)
				{
					newEntry = {
						"Key::" + Utility::KeyCodeToString(keyboardBinding->GetKeyBinding()),
						"None",
						iteration,
						m_EditKeyboardSlot,
						nullptr
					};
				}
				else
				{
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
					newEntry = {
						"Key::" + Utility::KeyCodeToString(keyboardBinding->GetKeyBinding()),
						Utility::ScriptToString(script),
						iteration,
						m_EditKeyboardSlot,
						nullptr
					};
				}

				m_KeyboardOnKeyPressedTable.InsertTableEntry(newEntry);
				iteration++;
			}
		};
		m_KeyboardOnKeyPressedTable.Column1Title = "KeyCode";
		m_KeyboardOnKeyPressedTable.Column2Title = "Function";
		m_KeyboardOnKeyPressedTable.AddToSelectionList("Add New Slot", [&]()
			{
				m_KeyboardOnKeyPressedAddSlot.PopupActive = true;
			});

		m_KeyboardOnKeyPressedAddSlot.Label = "Add New Slot";
		m_KeyboardOnKeyPressedAddSlot.PopupWidth = 420.0f;
		m_KeyboardOnKeyPressedAddSlot.PopupAction = [&]()
		{
			m_KeyboardOnKeyPressedAddKeyCode.CurrentOption = { Utility::KeyCodeToString(Key::A), Key::A };
			m_KeyboardOnKeyPressedAddFunction.CurrentOption = { "None", Assets::EmptyHandle };
		};
		m_KeyboardOnKeyPressedAddSlot.PopupContents = [&]()
		{
			EditorUI::EditorUIService::SelectOption(m_KeyboardOnKeyPressedAddKeyCode);
			EditorUI::EditorUIService::SelectOption(m_KeyboardOnKeyPressedAddFunction);
		};

		m_KeyboardOnKeyPressedAddSlot.ConfirmAction = [&]()
		{
			auto& keyboardBindings = m_EditorInputMap->GetOnKeyPressedBindings();
			Ref<Input::KeyboardActionBinding> newBinding = CreateRef<Input::KeyboardActionBinding>();
			newBinding->SetKeyBinding((KeyCode)m_KeyboardOnKeyPressedAddKeyCode.CurrentOption.Handle);
			Ref<Scripting::Script> script;
			if (m_KeyboardOnKeyPressedAddFunction.CurrentOption.Handle == Assets::EmptyHandle)
			{
				script = nullptr;
			}
			else
			{
				script = Assets::AssetService::GetScript(m_KeyboardOnKeyPressedAddFunction.CurrentOption.Handle);
			}
			newBinding->SetScript(script, m_KeyboardOnKeyPressedAddFunction.CurrentOption.Handle);

			keyboardBindings.push_back(newBinding);
			OnRefreshData();
			m_MainHeader.EditColorActive = true;
		};

		m_KeyboardOnKeyPressedAddKeyCode.Label = "Select Key";
		m_KeyboardOnKeyPressedAddKeyCode.LineCount = 7;
		m_KeyboardOnKeyPressedAddKeyCode.PopupAction = [&]()
		{
			m_KeyboardOnKeyPressedAddKeyCode.ClearOptions();
			for (auto key : Key::s_AllKeyCodes)
			{
				m_KeyboardOnKeyPressedAddKeyCode.AddToOptions("All KeyCodes", Utility::KeyCodeToString(key), key);
			}
		};

		m_KeyboardOnKeyPressedAddFunction.Label = "Select Function";
		m_KeyboardOnKeyPressedAddFunction.PopupAction = [&]()
		{
			m_KeyboardOnKeyPressedAddFunction.ClearOptions();
			m_KeyboardOnKeyPressedAddFunction.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [uuid, script] : Assets::AssetService::GetScriptCache())
			{
				
				if (script->m_FuncType != WrappedFuncType::Void_None)
				{
					continue;
				}
				m_KeyboardOnKeyPressedAddFunction.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, uuid);
			}
		};

		m_KeyboardOnKeyPressedEditSlot.Label = "Edit Slot";
		m_KeyboardOnKeyPressedEditSlot.PopupWidth = 420.0f;
		m_KeyboardOnKeyPressedEditSlot.PopupAction = [&]()
		{
			Input::KeyboardActionBinding* activeBinding = (Input::KeyboardActionBinding*)m_EditorInputMap->GetOnKeyPressedBindings().at(m_KeyboardOnKeyPressedActiveSlot).get();
			KG_ASSERT(activeBinding);

			m_KeyboardOnKeyPressedEditKeyCode.CurrentOption = { Utility::KeyCodeToString(activeBinding->GetKeyBinding()), activeBinding->GetKeyBinding() };
			if (activeBinding->GetScriptHandle() == Assets::EmptyHandle)
			{
				m_KeyboardOnKeyPressedEditFunction.CurrentOption = { "None", Assets::EmptyHandle };
			}
			else
			{
				KG_ASSERT(activeBinding->GetScript());
				m_KeyboardOnKeyPressedEditFunction.CurrentOption = { activeBinding->GetScript()->m_ScriptName, activeBinding->GetScriptHandle() };
			}
		};
		m_KeyboardOnKeyPressedEditSlot.PopupContents = [&]()
		{
			EditorUI::EditorUIService::SelectOption(m_KeyboardOnKeyPressedEditKeyCode);
			EditorUI::EditorUIService::SelectOption(m_KeyboardOnKeyPressedEditFunction);
		};
		m_KeyboardOnKeyPressedEditSlot.DeleteAction = [&]()
		{
			std::vector<Ref<Input::InputActionBinding>>& bindings = m_EditorInputMap->GetOnKeyPressedBindings();
			bindings.erase(bindings.begin() + m_KeyboardOnKeyPressedActiveSlot);
			OnRefreshData();
			m_MainHeader.EditColorActive = true;
		};

		m_KeyboardOnKeyPressedEditSlot.ConfirmAction = [&]()
		{
			Input::KeyboardActionBinding* newBinding = (Input::KeyboardActionBinding*)m_EditorInputMap->GetOnKeyPressedBindings().at(m_KeyboardOnKeyPressedActiveSlot).get();
			KG_ASSERT(newBinding);
			newBinding->SetKeyBinding((KeyCode)m_KeyboardOnKeyPressedEditKeyCode.CurrentOption.Handle);
			Ref<Scripting::Script> script;
			if (m_KeyboardOnKeyPressedEditFunction.CurrentOption.Handle == Assets::EmptyHandle)
			{
				script = nullptr;
			}
			else
			{
				script = Assets::AssetService::GetScript(m_KeyboardOnKeyPressedEditFunction.CurrentOption.Handle);
			}
			newBinding->SetScript(script, m_KeyboardOnKeyPressedEditFunction.CurrentOption.Handle);

			OnRefreshData();
			m_MainHeader.EditColorActive = true;
		};

		m_KeyboardOnKeyPressedEditKeyCode.Label = "Select Key";
		m_KeyboardOnKeyPressedEditKeyCode.LineCount = 7;
		m_KeyboardOnKeyPressedEditKeyCode.PopupAction = [&]()
		{
			m_KeyboardOnKeyPressedEditKeyCode.ClearOptions();
			for (auto key : Key::s_AllKeyCodes)
			{
				m_KeyboardOnKeyPressedEditKeyCode.AddToOptions("All KeyCodes", Utility::KeyCodeToString(key), key);
			}
		};

		m_KeyboardOnKeyPressedEditFunction.Label = "Select Function";
		m_KeyboardOnKeyPressedEditFunction.PopupAction = [&]()
		{
			m_KeyboardOnKeyPressedEditFunction.ClearOptions();
			m_KeyboardOnKeyPressedEditFunction.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [uuid, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_FuncType != WrappedFuncType::Void_None)
				{
					continue;
				}
				m_KeyboardOnKeyPressedEditFunction.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, uuid);
			}
		};

		// Polling Table
		m_KeyboardPollingTable.Label = "Polling";
		m_KeyboardPollingTable.Expanded = true;
		m_KeyboardPollingTable.OnRefresh = [&]()
		{
			m_KeyboardPollingTable.ClearTable();
			uint32_t iteration{ 0 };
			for (auto code: m_EditorInputMap->GetKeyboardPolling())
			{

				static std::function<void(EditorUI::TableEntry&)> m_EditKeyboardSlot = [&](EditorUI::TableEntry& entry)
				{
					m_KeyboardPollingActiveSlot = (uint32_t)entry.Handle;
					m_KeyboardPollingEditSlot.PopupActive = true;
				};

				EditorUI::TableEntry newEntry = {
					"Slot::" + std::to_string(iteration),
					"Key::" + Utility::KeyCodeToString(code),
					iteration,
					m_EditKeyboardSlot,
					nullptr
				};

				m_KeyboardPollingTable.InsertTableEntry(newEntry);
				iteration++;
			}
		};
		m_KeyboardPollingTable.Column1Title = "Slot";
		m_KeyboardPollingTable.Column2Title = "KeyCode";
		m_KeyboardPollingTable.AddToSelectionList("Add New Slot", [&]()
		{
			m_KeyboardPollingAddSlot.PopupActive = true;
		});

		m_KeyboardPollingAddSlot.Label = "Add New Slot";
		m_KeyboardPollingAddSlot.PopupWidth = 420.0f;
		m_KeyboardPollingAddSlot.PopupAction = [&]()
		{
			m_KeyboardPollingAddKeyCode.CurrentOption = { Utility::KeyCodeToString(Key::A), Key::A };
		};
		m_KeyboardPollingAddSlot.PopupContents = [&]()
		{
			EditorUI::EditorUIService::SelectOption(m_KeyboardPollingAddKeyCode);
		};

		m_KeyboardPollingAddSlot.ConfirmAction = [&]()
		{
			std::vector<KeyCode>& keyboardPolling = m_EditorInputMap->GetKeyboardPolling();
			KeyCode code = (KeyCode)m_KeyboardPollingAddKeyCode.CurrentOption.Handle;
			keyboardPolling.push_back(code);
			OnRefreshData();
			m_MainHeader.EditColorActive = true;
		};

		m_KeyboardPollingAddKeyCode.Label = "Select Key";
		m_KeyboardPollingAddKeyCode.LineCount = 7;
		m_KeyboardPollingAddKeyCode.PopupAction = [&]()
		{
			m_KeyboardPollingAddKeyCode.ClearOptions();
			for (uint16_t key : Key::s_AllKeyCodes)
			{
				if (key == 0)
				{
					m_KeyboardPollingAddKeyCode.AddToOptions("Clear", Utility::KeyCodeToString(key), key);
				}
				else
				{
					m_KeyboardPollingAddKeyCode.AddToOptions("All KeyCodes", Utility::KeyCodeToString(key), key);
				}
				
			}
		};

		m_KeyboardPollingEditSlot.Label = "Edit Slot";
		m_KeyboardPollingEditSlot.PopupWidth = 420.0f;
		m_KeyboardPollingEditSlot.PopupAction = [&]()
		{
			KeyCode activeCode = m_EditorInputMap->GetKeyboardPolling().at(m_KeyboardPollingActiveSlot);

			m_KeyboardPollingEditKeyCode.CurrentOption = { Utility::KeyCodeToString(activeCode), activeCode };
		};
		m_KeyboardPollingEditSlot.PopupContents = [&]()
		{
			EditorUI::EditorUIService::SelectOption(m_KeyboardPollingEditKeyCode);
		};

		m_KeyboardPollingEditSlot.DeleteAction = [&]()
		{
			std::vector<KeyCode>& codes = m_EditorInputMap->GetKeyboardPolling();
			codes.erase(codes.begin() + m_KeyboardPollingActiveSlot);
			OnRefreshData();
			m_MainHeader.EditColorActive = true;
		};

		m_KeyboardPollingEditSlot.ConfirmAction = [&]()
		{
			auto& keyboardPolling = m_EditorInputMap->GetKeyboardPolling();
			KeyCode code = (KeyCode)m_KeyboardPollingEditKeyCode.CurrentOption.Handle;
			keyboardPolling[m_KeyboardPollingActiveSlot] = code;
			OnRefreshData();
			m_MainHeader.EditColorActive = true;
		};

		m_KeyboardPollingEditKeyCode.Label = "Select Key";
		m_KeyboardPollingEditKeyCode.LineCount = 7;
		m_KeyboardPollingEditKeyCode.PopupAction = [&]()
		{
			m_KeyboardPollingEditKeyCode.ClearOptions();
			for (uint16_t key : Key::s_AllKeyCodes)
			{
				if (key == 0)
				{
					m_KeyboardPollingEditKeyCode.AddToOptions("Clear", Utility::KeyCodeToString(key), key);
				}
				else
				{
					m_KeyboardPollingEditKeyCode.AddToOptions("All KeyCodes", Utility::KeyCodeToString(key), key);
				}

			}
		};
	}
	
}

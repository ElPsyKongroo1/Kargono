#include "Panels/InputMapPanel.h"
#include "EditorApp.h"

namespace Kargono
{
	static EditorApp* s_EditorApp { nullptr };
}

namespace Kargono::Panels
{
	void InputMapPanel::OnOpenInputMapDialog()
	{
		m_OpenInputMapPopupSpec.OpenPopup = true;
	}
	void InputMapPanel::OnCreateInputMapDialog()
	{
		KG_ASSERT(Projects::ProjectService::GetActive());
		m_SelectInputMapLocationSpec.CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
		m_CreateInputMapPopupSpec.OpenPopup = true;
	}

	void InputMapPanel::OnOpenInputMap(Assets::AssetHandle newHandle)
	{
		// Open dialog to create editor input map
		m_EditorInputMap = Assets::AssetService::GetInputMap(newHandle);
		m_EditorInputMapHandle = newHandle;
		m_MainHeader.EditColorActive = false;
		m_MainHeader.Label = Assets::AssetService::GetInputMapRegistry().at(
			m_EditorInputMapHandle).Data.FileLocation.filename().string();
		OnRefreshData();
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
			EditorUI::EditorUIService::NewItemScreen("Open Existing Input Map", KG_BIND_CLASS_FN(OnOpenInputMapDialog), "Create New Input Map", KG_BIND_CLASS_FN(OnCreateInputMapDialog));
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
				EditorUI::EditorUIService::List(m_KeyboardOnUpdateTable);
				EditorUI::EditorUIService::GenericPopup(m_KeyboardOnUpdateAddPopup);
				EditorUI::EditorUIService::GenericPopup(m_KeyboardOnUpdateEditPopup);

				// On Key Pressed
				EditorUI::EditorUIService::List(m_KeyboardOnKeyPressedTable);
				EditorUI::EditorUIService::GenericPopup(m_KeyboardOnKeyPressedAddPopup);
				EditorUI::EditorUIService::GenericPopup(m_KeyboardOnKeyPressedEditPopup);

				// Keyboard Polling
				EditorUI::EditorUIService::List(m_KeyboardPollingTable);
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

	bool InputMapPanel::OnAssetEvent(Events::Event* event)
	{
		Events::ManageAsset* manageAsset = (Events::ManageAsset*)event;

		// Manage script deletion event
		if (manageAsset->GetAssetType() == Assets::AssetType::Script &&
			manageAsset->GetAction() == Events::ManageAssetAction::Delete)
		{
			if (m_KeyboardOnUpdateAddFunction.CurrentOption.Handle == manageAsset->GetAssetID())
			{
				m_KeyboardOnUpdateAddFunction.CurrentOption = { "None", Assets::EmptyHandle };
			}

			if (m_KeyboardOnUpdateEditFunction.CurrentOption.Handle == manageAsset->GetAssetID())
			{
				m_KeyboardOnUpdateEditFunction.CurrentOption = { "None", Assets::EmptyHandle };
			}

			if (m_KeyboardOnKeyPressedAddFunction.CurrentOption.Handle == manageAsset->GetAssetID())
			{
				m_KeyboardOnKeyPressedAddFunction.CurrentOption = { "None", Assets::EmptyHandle };
			}

			if (m_KeyboardOnKeyPressedEditFunction.CurrentOption.Handle == manageAsset->GetAssetID())
			{
				m_KeyboardOnKeyPressedEditFunction.CurrentOption = { "None", Assets::EmptyHandle };
			}

			// Check input maps assets
			if (m_EditorInputMap)
			{
				Assets::AssetService::RemoveScriptFromInputMap(m_EditorInputMap, manageAsset->GetAssetID());
				OnRefreshData();
			}
			
		}

		if (manageAsset->GetAssetType() == Assets::AssetType::InputMap &&
			manageAsset->GetAction() == Events::ManageAssetAction::Delete)
		{
			// Handle deletion of asset
			if (manageAsset->GetAssetID() != m_EditorInputMapHandle)
			{
				return false;
			}

			ResetPanelResources();
			return true;
		}

		if (manageAsset->GetAssetType() == Assets::AssetType::InputMap &&
			manageAsset->GetAction() == Events::ManageAssetAction::UpdateAssetInfo)
		{
			// Handle deletion of asset
			if (manageAsset->GetAssetID() != m_EditorInputMapHandle)
			{
				return false;
			}

			// Update header
			m_MainHeader.Label = Assets::AssetService::GetInputMapFileLocation(manageAsset->GetAssetID()).filename().string();
			return true;
		}
		return false;
	}

	void InputMapPanel::ResetPanelResources()
	{
		m_EditorInputMap = nullptr;
		m_EditorInputMapHandle = Assets::EmptyHandle;
	}

	void InputMapPanel::OpenCreateDialog(std::filesystem::path& createLocation)
	{
		// Open input map Window
		s_EditorApp->m_ShowInputMapEditor = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);

		if (!m_EditorInputMap)
		{
			// Open dialog to create editor input map
			OnCreateInputMapDialog();
			m_SelectInputMapLocationSpec.CurrentOption = createLocation;
		}
		else
		{
			// Add warning to close active input map before creating a new input map
			s_EditorApp->OpenWarningMessage("A input map is already active inside the editor. Please close the current input map before creating a new one.");
		}
	}

	void InputMapPanel::OpenAssetInEditor(std::filesystem::path& assetLocation)
	{
		// Ensure provided path is within the active asset directory
		std::filesystem::path activeAssetDirectory = Projects::ProjectService::GetActiveAssetDirectory();
		if (!Utility::FileSystem::DoesPathContainSubPath(activeAssetDirectory, assetLocation))
		{
			KG_WARN("Could not open asset in editor. Provided path does not exist within active asset directory");
			return;
		}

		// Look for asset in registry using the file location
		std::filesystem::path relativePath{ Utility::FileSystem::GetRelativePath(activeAssetDirectory, assetLocation) };
		Assets::AssetHandle assetHandle = Assets::AssetService::GetInputMapHandleFromFileLocation(relativePath);

		// Validate resulting handle
		if (!assetHandle)
		{
			KG_WARN("Could not open asset in editor. Provided path does not result in an asset inside the registry.");
			return;
		}

		// Open the editor panel to be visible
		s_EditorApp->m_ShowInputMapEditor = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);

		// Early out if asset is already open
		if (m_EditorInputMapHandle == assetHandle)
		{
			return;
		}

		// Check if panel is already occupied by an asset
		if (!m_EditorInputMap)
		{
			OnOpenInputMap(assetHandle);
		}
		else
		{
			// Add warning to close active AI state before opening a new AIState
			s_EditorApp->OpenWarningMessage("An input map is already active inside the editor. Please close the current input map before opening a new one.");
		}
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
				m_OpenInputMapPopupSpec.AddToOptions("All Options", asset.Data.FileLocation.filename().string(), handle);
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

			OnOpenInputMap(selection.Handle);
		};

		m_SelectInputMapNameSpec.Label = "New Name";
		m_SelectInputMapNameSpec.CurrentOption = "Empty";

		m_SelectInputMapLocationSpec.Label = "Location";
		m_SelectInputMapLocationSpec.CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
		m_SelectInputMapLocationSpec.ConfirmAction = [&](const std::string& path)
		{
			if (!Utility::FileSystem::DoesPathContainSubPath(Projects::ProjectService::GetActiveAssetDirectory(), path))
			{
				KG_WARN("Cannot create an asset outside of the project's asset directory.");
				m_SelectInputMapLocationSpec.CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
			}
		};

		m_CreateInputMapPopupSpec.Label = "Create Input Map";
		m_CreateInputMapPopupSpec.ConfirmAction = [&]()
		{
			if (m_SelectInputMapNameSpec.CurrentOption == "")
			{
				return;
			}

			m_EditorInputMapHandle = Assets::AssetService::CreateInputMap(m_SelectInputMapNameSpec.CurrentOption.c_str(), m_SelectInputMapLocationSpec.CurrentOption);
			if (m_EditorInputMapHandle == Assets::EmptyHandle)
			{
				KG_WARN("Input Map was not created");
				return;
			}
			m_EditorInputMap = Assets::AssetService::GetInputMap(m_EditorInputMapHandle);
			m_MainHeader.EditColorActive = false;
			m_MainHeader.Label = Assets::AssetService::GetInputMapRegistry().at(
				m_EditorInputMapHandle).Data.FileLocation.filename().string();
			OnRefreshData();
		};
		m_CreateInputMapPopupSpec.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_SelectInputMapNameSpec);
			EditorUI::EditorUIService::ChooseDirectory(m_SelectInputMapLocationSpec);
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
				m_CloseInputMapWarning.OpenPopup = true;
			}
			else
			{
				m_EditorInputMapHandle = 0;
				m_EditorInputMap = nullptr;
			}
		});
		m_MainHeader.AddToSelectionList("Delete", [&]()
		{
			m_DeleteInputMapWarning.OpenPopup = true;
		});
	}

	void InputMapPanel::InitializeKeyboardScreen()
	{
		// On Update List
		m_KeyboardOnUpdateTable.Label = "On Update";
		m_KeyboardOnUpdateTable.Expanded = true;
		m_KeyboardOnUpdateTable.OnRefresh = [&]()
		{
			m_KeyboardOnUpdateTable.ClearList();
			uint32_t iteration{ 0 };
			for (auto& inputBinding : m_EditorInputMap->GetOnUpdateBindings())
			{

				if (inputBinding->GetActionType() != Input::KeyboardAction)
				{
					continue;
				}
				Input::KeyboardActionBinding* keyboardBinding = (Input::KeyboardActionBinding*)inputBinding.get();

				static std::function<void(EditorUI::ListEntry&, std::size_t iteration)> m_EditKeyboardSlot = [&](EditorUI::ListEntry& entry, std::size_t iteration)
				{
					m_KeyboardOnUpdateActiveSlot = (uint32_t)entry.Handle;
					m_KeyboardOnUpdateEditPopup.OpenPopup = true;
				};

				EditorUI::ListEntry newEntry;
				Assets::AssetHandle scriptHandle = keyboardBinding->GetScriptHandle();
				if (scriptHandle == Assets::EmptyHandle)
				{
					newEntry = {
						"Key::" + Utility::KeyCodeToString(keyboardBinding->GetKeyBinding()),
						"None",
						iteration,
						m_EditKeyboardSlot
					};
				}
				else
				{
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
					newEntry = {
						"Key::" + Utility::KeyCodeToString(keyboardBinding->GetKeyBinding()),
						script->m_ScriptName,
						iteration,
						m_EditKeyboardSlot
					};
				}

				m_KeyboardOnUpdateTable.InsertListEntry(newEntry);
				iteration++;
			}
		};
		m_KeyboardOnUpdateTable.Column1Title = "KeyCode";
		m_KeyboardOnUpdateTable.Column2Title = "Function";
		m_KeyboardOnUpdateTable.AddToSelectionList("Add New Slot", [&]()
		{
			m_KeyboardOnUpdateAddPopup.OpenPopup = true;
		});

		m_KeyboardOnUpdateAddPopup.Label = "Add New Slot";
		m_KeyboardOnUpdateAddPopup.PopupAction = [&]()
		{
			m_KeyboardOnUpdateAddKeyCode.CurrentOption = {Utility::KeyCodeToString(Key::A), Key::A};
			m_KeyboardOnUpdateAddFunction.CurrentOption = {"None", Assets::EmptyHandle};
		};
		m_KeyboardOnUpdateAddPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::SelectOption(m_KeyboardOnUpdateAddKeyCode);
			EditorUI::EditorUIService::SelectOption(m_KeyboardOnUpdateAddFunction);
			EditorUI::EditorUIService::Tooltip(m_SelectScriptTooltip);
			s_EditorApp->m_ScriptEditorPanel->DrawOnCreatePopup();
		};

		m_KeyboardOnUpdateAddPopup.ConfirmAction = [&]()
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
		m_KeyboardOnUpdateAddFunction.Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_KeyboardOnUpdateAddFunction.PopupAction = [&]()
		{
			m_KeyboardOnUpdateAddFunction.ClearOptions();
			m_KeyboardOnUpdateAddFunction.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, assetInfo] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				if (script->m_FuncType != WrappedFuncType::Void_None && script->m_FuncType != WrappedFuncType::Void_Float)
				{
					continue;
				}
				m_KeyboardOnUpdateAddFunction.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}
		};
		m_KeyboardOnUpdateAddFunction.OnEdit = [&]()
			{
				// Initialize tooltip with options
				m_SelectScriptTooltip.ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					m_KeyboardOnUpdateAddFunction.OpenPopup = true;
				} };
				m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						// Open create script dialog in script editor
						s_EditorApp->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_None, [&](Assets::AssetHandle scriptHandle)
						{
								// Ensure handle provides a script in the registry
								if (!Assets::AssetService::HasScript(scriptHandle))
								{
									KG_WARN("Could not find script");
									return;
								}

								// Ensure function type matches definition
								Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
								if (script->m_FuncType != WrappedFuncType::Void_None)
								{
									KG_WARN("Incorrect function type returned when linking script to usage point");
									return;
								}

								// Fill the new script handle
								m_KeyboardOnUpdateAddFunction.CurrentOption = { script->m_ScriptName, scriptHandle };
						},{}, false);
					}};
				m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_SelectScriptTooltip.TooltipActive = true;
			};

		m_KeyboardOnUpdateEditPopup.Label = "Edit Slot";
		m_KeyboardOnUpdateEditPopup.PopupAction = [&]()
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
		m_KeyboardOnUpdateEditPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::SelectOption(m_KeyboardOnUpdateEditKeyCode);
			EditorUI::EditorUIService::SelectOption(m_KeyboardOnUpdateEditFunction);
			EditorUI::EditorUIService::Tooltip(m_SelectScriptTooltip);
			s_EditorApp->m_ScriptEditorPanel->DrawOnCreatePopup();
		};
		m_KeyboardOnUpdateEditPopup.DeleteAction = [&]()
		{
			std::vector<Ref<Input::InputActionBinding>>& bindings = m_EditorInputMap->GetOnUpdateBindings();
			bindings.erase(bindings.begin() + m_KeyboardOnUpdateActiveSlot);
			OnRefreshData();
			m_MainHeader.EditColorActive = true;
		};

		m_KeyboardOnUpdateEditPopup.ConfirmAction = [&]()
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
		m_KeyboardOnUpdateEditFunction.Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_KeyboardOnUpdateEditFunction.PopupAction = [&]()
		{
			m_KeyboardOnUpdateEditFunction.ClearOptions();
			m_KeyboardOnUpdateEditFunction.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, assetInfo] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				if (script->m_FuncType != WrappedFuncType::Void_None && script->m_FuncType != WrappedFuncType::Void_Float)
				{
					continue;
				}
				m_KeyboardOnUpdateEditFunction.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}
		};

		m_KeyboardOnUpdateEditFunction.OnEdit = [&]()
		{
			// Initialize tooltip with options
			m_SelectScriptTooltip.ClearEntries();
			EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
			{
				m_KeyboardOnUpdateEditFunction.OpenPopup = true;
			} };
			m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

			EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
			{
					// Open create script dialog in script editor
					s_EditorApp->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_None, [&](Assets::AssetHandle scriptHandle)
					{
							// Ensure handle provides a script in the registry
							if (!Assets::AssetService::HasScript(scriptHandle))
							{
								KG_WARN("Could not find script");
								return;
							}

							// Ensure function type matches definition
							Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
							if (script->m_FuncType != WrappedFuncType::Void_None)
							{
								KG_WARN("Incorrect function type returned when linking script to usage point");
								return;
							}

							// Fill the new script handle
							m_KeyboardOnUpdateEditFunction.CurrentOption = { script->m_ScriptName, scriptHandle };
					},{}, false);
				} };
			m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

			// Open tooltip
			m_SelectScriptTooltip.TooltipActive = true;
		};


		// On Key Pressed List
		m_KeyboardOnKeyPressedTable.Label = "On Key Pressed";
		m_KeyboardOnKeyPressedTable.Expanded = true;
		m_KeyboardOnKeyPressedTable.OnRefresh = [&]()
		{
			m_KeyboardOnKeyPressedTable.ClearList();
			uint32_t iteration{ 0 };
			for (auto& inputBinding : m_EditorInputMap->GetOnKeyPressedBindings())
			{

				if (inputBinding->GetActionType() != Input::KeyboardAction)
				{
					continue;
				}
				Input::KeyboardActionBinding* keyboardBinding = (Input::KeyboardActionBinding*)inputBinding.get();

				static std::function<void(EditorUI::ListEntry&, std::size_t)> m_EditKeyboardSlot = [&](EditorUI::ListEntry& entry, std::size_t iteration)
				{
					m_KeyboardOnKeyPressedActiveSlot = (uint32_t)entry.Handle;
					m_KeyboardOnKeyPressedEditPopup.OpenPopup = true;
				};

				EditorUI::ListEntry newEntry;
				Assets::AssetHandle scriptHandle = keyboardBinding->GetScriptHandle();
				if (scriptHandle == Assets::EmptyHandle)
				{
					newEntry = {
						"Key::" + Utility::KeyCodeToString(keyboardBinding->GetKeyBinding()),
						"None",
						iteration,
						m_EditKeyboardSlot
					};
				}
				else
				{
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
					newEntry = {
						"Key::" + Utility::KeyCodeToString(keyboardBinding->GetKeyBinding()),
						script->m_ScriptName,
						iteration,
						m_EditKeyboardSlot
					};
				}

				m_KeyboardOnKeyPressedTable.InsertListEntry(newEntry);
				iteration++;
			}
		};
		m_KeyboardOnKeyPressedTable.Column1Title = "KeyCode";
		m_KeyboardOnKeyPressedTable.Column2Title = "Function";
		m_KeyboardOnKeyPressedTable.AddToSelectionList("Add New Slot", [&]()
			{
				m_KeyboardOnKeyPressedAddPopup.OpenPopup = true;
			});

		m_KeyboardOnKeyPressedAddPopup.Label = "Add New Slot";
		m_KeyboardOnKeyPressedAddPopup.PopupAction = [&]()
		{
			m_KeyboardOnKeyPressedAddKeyCode.CurrentOption = { Utility::KeyCodeToString(Key::A), Key::A };
			m_KeyboardOnKeyPressedAddFunction.CurrentOption = { "None", Assets::EmptyHandle };
		};
		m_KeyboardOnKeyPressedAddPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::SelectOption(m_KeyboardOnKeyPressedAddKeyCode);
			EditorUI::EditorUIService::SelectOption(m_KeyboardOnKeyPressedAddFunction);
			EditorUI::EditorUIService::Tooltip(m_SelectScriptTooltip);
			s_EditorApp->m_ScriptEditorPanel->DrawOnCreatePopup();
		};

		m_KeyboardOnKeyPressedAddPopup.ConfirmAction = [&]()
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
		m_KeyboardOnKeyPressedAddFunction.Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_KeyboardOnKeyPressedAddFunction.PopupAction = [&]()
		{
			m_KeyboardOnKeyPressedAddFunction.ClearOptions();
			m_KeyboardOnKeyPressedAddFunction.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, assetInfo] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				if (script->m_FuncType != WrappedFuncType::Void_None)
				{
					continue;
				}
				m_KeyboardOnKeyPressedAddFunction.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}
		};
		m_KeyboardOnKeyPressedAddFunction.OnEdit = [&]()
			{
				// Initialize tooltip with options
				m_SelectScriptTooltip.ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					m_KeyboardOnKeyPressedAddFunction.OpenPopup = true;
				} };
				m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						// Open create script dialog in script editor
						s_EditorApp->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_None, [&](Assets::AssetHandle scriptHandle)
						{
								// Ensure handle provides a script in the registry
								if (!Assets::AssetService::HasScript(scriptHandle))
								{
									KG_WARN("Could not find script");
									return;
								}

								// Ensure function type matches definition
								Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
								if (script->m_FuncType != WrappedFuncType::Void_None)
								{
									KG_WARN("Incorrect function type returned when linking script to usage point");
									return;
								}

								// Fill the new script handle
								m_KeyboardOnKeyPressedAddFunction.CurrentOption = { script->m_ScriptName, scriptHandle };
							},{},  false);
						} };
				m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_SelectScriptTooltip.TooltipActive = true;
			};

		m_KeyboardOnKeyPressedEditPopup.Label = "Edit Slot";
		m_KeyboardOnKeyPressedEditPopup.PopupAction = [&]()
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
		m_KeyboardOnKeyPressedEditPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::SelectOption(m_KeyboardOnKeyPressedEditKeyCode);
			EditorUI::EditorUIService::SelectOption(m_KeyboardOnKeyPressedEditFunction);
			EditorUI::EditorUIService::Tooltip(m_SelectScriptTooltip);
			s_EditorApp->m_ScriptEditorPanel->DrawOnCreatePopup();
		};
		m_KeyboardOnKeyPressedEditPopup.DeleteAction = [&]()
		{
			std::vector<Ref<Input::InputActionBinding>>& bindings = m_EditorInputMap->GetOnKeyPressedBindings();
			bindings.erase(bindings.begin() + m_KeyboardOnKeyPressedActiveSlot);
			OnRefreshData();
			m_MainHeader.EditColorActive = true;
		};

		m_KeyboardOnKeyPressedEditPopup.ConfirmAction = [&]()
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
		m_KeyboardOnKeyPressedEditFunction.Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_KeyboardOnKeyPressedEditFunction.PopupAction = [&]()
		{
			m_KeyboardOnKeyPressedEditFunction.ClearOptions();
			m_KeyboardOnKeyPressedEditFunction.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, assetInfo] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				if (script->m_FuncType != WrappedFuncType::Void_None)
				{
					continue;
				}
				m_KeyboardOnKeyPressedEditFunction.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}
		};
		m_KeyboardOnKeyPressedEditFunction.OnEdit = [&]()
		{
			// Initialize tooltip with options
			m_SelectScriptTooltip.ClearEntries();
			EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
			{
				m_KeyboardOnKeyPressedEditFunction.OpenPopup = true;
			} };
			m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

			EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
			{
					// Open create script dialog in script editor
					s_EditorApp->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_None, [&](Assets::AssetHandle scriptHandle)
					{
							// Ensure handle provides a script in the registry
							if (!Assets::AssetService::HasScript(scriptHandle))
							{
								KG_WARN("Could not find script");
								return;
							}

							// Ensure function type matches definition
							Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
							if (script->m_FuncType != WrappedFuncType::Void_None)
							{
								KG_WARN("Incorrect function type returned when linking script to usage point");
								return;
							}

							// Fill the new script handle
							m_KeyboardOnKeyPressedEditFunction.CurrentOption = { script->m_ScriptName, scriptHandle };
						},{}, false);
					} };
			m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

			// Open tooltip
			m_SelectScriptTooltip.TooltipActive = true;
		};

		// Polling List
		m_KeyboardPollingTable.Label = "Polling";
		m_KeyboardPollingTable.Expanded = true;
		m_KeyboardPollingTable.OnRefresh = [&]()
		{
			m_KeyboardPollingTable.ClearList();
			uint32_t iteration{ 0 };
			for (auto code: m_EditorInputMap->GetKeyboardPolling())
			{

				static std::function<void(EditorUI::ListEntry&, std::size_t)> m_EditKeyboardSlot = [&](EditorUI::ListEntry& entry, std::size_t iteration)
				{
					m_KeyboardPollingActiveSlot = (uint32_t)entry.Handle;
					m_KeyboardPollingEditSlot.OpenPopup = true;
				};

				EditorUI::ListEntry newEntry = {
					"Slot::" + std::to_string(iteration),
					"Key::" + Utility::KeyCodeToString(code),
					iteration,
					m_EditKeyboardSlot
				};

				m_KeyboardPollingTable.InsertListEntry(newEntry);
				iteration++;
			}
		};
		m_KeyboardPollingTable.Column1Title = "Slot";
		m_KeyboardPollingTable.Column2Title = "KeyCode";
		m_KeyboardPollingTable.AddToSelectionList("Add New Slot", [&]()
		{
			m_KeyboardPollingAddSlot.OpenPopup = true;
		});

		m_KeyboardPollingAddSlot.Label = "Add New Slot";
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

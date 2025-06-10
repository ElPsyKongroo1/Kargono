#include "Windows/MainWindow/InputMapPanel.h"

#include "EditorApp.h"

namespace Kargono
{
	static EditorApp* s_EditorApp { nullptr };
	static Windows::MainWindow* s_MainWindow{ nullptr };
}

namespace Kargono::Panels
{
	void InputMapPanel::OnOpenInputMapDialog()
	{
		m_OpenInputMapPopupSpec.m_OpenPopup = true;
	}
	void InputMapPanel::OnCreateInputMapDialog()
	{
		Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

		m_SelectInputMapLocationSpec.m_CurrentOption = projectPaths.GetAssetDirectory();
		m_CreateInputMapPopupSpec.m_OpenPopup = true;
	}

	void InputMapPanel::OnOpenInputMap(Assets::AssetHandle newHandle)
	{
		// Open dialog to create editor input map
		m_EditorInputMap = Assets::AssetService::GetInputMap(newHandle);
		m_EditorInputMapHandle = newHandle;
		m_MainHeader.m_EditColorActive = false;
		m_MainHeader.m_Label = Assets::AssetService::GetInputMapRegistry().at(
			m_EditorInputMapHandle).Data.FileLocation.filename().string();
		OnRefreshData();
	}

	void InputMapPanel::OnRefreshData()
	{
		m_KeyboardOnUpdateTable.m_OnRefresh();
		m_KeyboardOnKeyPressedTable.m_OnRefresh();
		m_KeyboardPollingTable.m_OnRefresh();
	}

	InputMapPanel::InputMapPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
		s_MainWindow->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(InputMapPanel::OnKeyPressedEditor));
		InitializeOpeningScreen();
		InitializeInputMapHeader();
		InitializeKeyboardScreen();
	}
	void InputMapPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION()
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_MainWindow->m_ShowInputMapEditor);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		if (!m_EditorInputMap)
		{
			// Opening/Null State Screen
			EditorUI::EditorUIService::NewItemScreen("Open Existing Input Map", KG_BIND_CLASS_FN(OnOpenInputMapDialog), "Create New Input Map", KG_BIND_CLASS_FN(OnCreateInputMapDialog));
			m_CreateInputMapPopupSpec.RenderPopup();
			m_OpenInputMapPopupSpec.RenderOptions();
		}
		else
		{
			// Header
			m_MainHeader.RenderHeader();
			m_DeleteInputMapWarning.RenderPopup();
			m_CloseInputMapWarning.RenderPopup();

			// Main Content
			EditorUI::EditorUIService::BeginTabBar("InputMapPanelTabBar");
			// Keyboard Panel
			if (EditorUI::EditorUIService::BeginTabItem("Keyboard"))
			{
				// On Update
				m_KeyboardOnUpdateTable.RenderList();
				m_KeyboardOnUpdateAddPopup.RenderPopup();
				m_KeyboardOnUpdateEditPopup.RenderPopup();

				// On Key Pressed
				m_KeyboardOnKeyPressedTable.RenderList();
				m_KeyboardOnKeyPressedAddPopup.RenderPopup();
				m_KeyboardOnKeyPressedEditPopup.RenderPopup();

				// Keyboard Polling
				m_KeyboardPollingTable.RenderList();
				m_KeyboardPollingAddSlot.RenderPopup();
				m_KeyboardPollingEditSlot.RenderPopup();

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
			manageAsset->GetAction() == Events::ManageAssetAction::PreDelete)
		{
			if (m_KeyboardOnUpdateAddFunction.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_KeyboardOnUpdateAddFunction.m_CurrentOption = { "None", Assets::EmptyHandle };
			}

			if (m_KeyboardOnUpdateEditFunction.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_KeyboardOnUpdateEditFunction.m_CurrentOption = { "None", Assets::EmptyHandle };
			}

			if (m_KeyboardOnKeyPressedAddFunction.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_KeyboardOnKeyPressedAddFunction.m_CurrentOption = { "None", Assets::EmptyHandle };
			}

			if (m_KeyboardOnKeyPressedEditFunction.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_KeyboardOnKeyPressedEditFunction.m_CurrentOption = { "None", Assets::EmptyHandle };
			}

			// Check input maps assets
			if (m_EditorInputMap)
			{
				Assets::AssetService::RemoveScriptFromInputMap(m_EditorInputMap, manageAsset->GetAssetID());
				OnRefreshData();
			}
			
		}

		if (manageAsset->GetAssetType() == Assets::AssetType::InputMap &&
			manageAsset->GetAction() == Events::ManageAssetAction::PreDelete)
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
			m_MainHeader.m_Label = Assets::AssetService::GetInputMapFileLocation(manageAsset->GetAssetID()).filename().string();
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
		s_MainWindow->m_ShowInputMapEditor = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);

		if (!m_EditorInputMap)
		{
			// Open dialog to create editor input map
			OnCreateInputMapDialog();
			m_SelectInputMapLocationSpec.m_CurrentOption = createLocation;
		}
		else
		{
			// Add warning to close active input map before creating a new input map
			s_MainWindow->OpenWarningMessage("A input map is already active inside the editor. Please close the current input map before creating a new one.");
		}
	}

	void InputMapPanel::OpenAssetInEditor(std::filesystem::path& assetLocation)
	{
		// Ensure provided path is within the active asset directory
		std::filesystem::path activeAssetDirectory = Projects::ProjectService::GetActiveContext().GetProjectPaths().GetAssetDirectory();
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
		s_MainWindow->m_ShowInputMapEditor = true;
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
			s_MainWindow->OpenWarningMessage("An input map is already active inside the editor. Please close the current input map before opening a new one.");
		}
	}

	void InputMapPanel::InitializeOpeningScreen()
	{
		m_OpenInputMapPopupSpec.m_Label = "Open Input Map";
		m_OpenInputMapPopupSpec.m_LineCount = 2;
		m_OpenInputMapPopupSpec.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_OpenInputMapPopupSpec.m_Flags |= EditorUI::SelectOption_PopupOnly;
		m_OpenInputMapPopupSpec.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.GetAllOptions().clear();
			spec.m_CurrentOption = { "None", Assets::EmptyHandle };

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetInputMapRegistry())
			{
				spec.AddToOptions("All Options", asset.Data.FileLocation.filename().string(), handle);
			}
		};

		m_OpenInputMapPopupSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.m_Handle == Assets::EmptyHandle)
			{
				KG_WARN("No Input Map Selected");
				return;
			}
			if (!Assets::AssetService::GetInputMapRegistry().contains(selection.m_Handle))
			{
				KG_WARN("Could not find the input map specified");
				return;
			}

			OnOpenInputMap(selection.m_Handle);
		};

		m_SelectInputMapNameSpec.m_Label = "New Name";
		m_SelectInputMapNameSpec.m_CurrentOption = "Empty";

		m_SelectInputMapLocationSpec.m_Label = "Location";
		m_SelectInputMapLocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveContext().GetProjectPaths().GetAssetDirectory();
		m_SelectInputMapLocationSpec.m_ConfirmAction = [&](std::string_view path)
		{
			if (!Utility::FileSystem::DoesPathContainSubPath(Projects::ProjectService::GetActiveContext().GetProjectPaths().GetAssetDirectory(), path))
			{
				KG_WARN("Cannot create an asset outside of the project's asset directory.");
				m_SelectInputMapLocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveContext().GetProjectPaths().GetAssetDirectory();
			}
		};

		m_CreateInputMapPopupSpec.m_Label = "Create Input Map";
		m_CreateInputMapPopupSpec.m_ConfirmAction = [&]()
		{
			if (m_SelectInputMapNameSpec.m_CurrentOption == "")
			{
				return;
			}

			m_EditorInputMapHandle = Assets::AssetService::CreateInputMap(m_SelectInputMapNameSpec.m_CurrentOption.c_str(), m_SelectInputMapLocationSpec.m_CurrentOption);
			if (m_EditorInputMapHandle == Assets::EmptyHandle)
			{
				KG_WARN("Input Map was not created");
				return;
			}
			m_EditorInputMap = Assets::AssetService::GetInputMap(m_EditorInputMapHandle);
			m_MainHeader.m_EditColorActive = false;
			m_MainHeader.m_Label = Assets::AssetService::GetInputMapRegistry().at(
				m_EditorInputMapHandle).Data.FileLocation.filename().string();
			OnRefreshData();
		};
		m_CreateInputMapPopupSpec.m_PopupContents = [&]()
		{
			m_SelectInputMapNameSpec.RenderText();
			m_SelectInputMapLocationSpec.RenderChooseDir();
		};
	}

	void InputMapPanel::InitializeInputMapHeader()
	{
		// Header (Game State Name and Options)
		m_DeleteInputMapWarning.m_Label = "Delete Input Map";
		m_DeleteInputMapWarning.m_ConfirmAction = [&]()
		{
			Assets::AssetService::DeleteInputMap(m_EditorInputMapHandle);
			m_EditorInputMapHandle = 0;
			m_EditorInputMap = nullptr;
		};
		m_DeleteInputMapWarning.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to delete this input map object?");
		};

		m_CloseInputMapWarning.m_Label = "Close Input Map";
		m_CloseInputMapWarning.m_ConfirmAction = [&]()
		{
			m_EditorInputMapHandle = 0;
			m_EditorInputMap = nullptr;
		};
		m_CloseInputMapWarning.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to close this input map object without saving?");
		};

		m_MainHeader.AddToSelectionList("Save", [&]()
		{
			Assets::AssetService::SaveInputMap(m_EditorInputMapHandle, m_EditorInputMap);
			m_MainHeader.m_EditColorActive = false;
		});
		m_MainHeader.AddToSelectionList("Close", [&]()
		{
			if (m_MainHeader.m_EditColorActive)
			{
				m_CloseInputMapWarning.m_OpenPopup = true;
			}
			else
			{
				m_EditorInputMapHandle = 0;
				m_EditorInputMap = nullptr;
			}
		});
		m_MainHeader.AddToSelectionList("Delete", [&]()
		{
			m_DeleteInputMapWarning.m_OpenPopup = true;
		});
	}

	void InputMapPanel::InitializeKeyboardScreen()
	{
		// On Update List
		m_KeyboardOnUpdateTable.m_Label = "On Update";
		m_KeyboardOnUpdateTable.m_Expanded = true;
		m_KeyboardOnUpdateTable.m_OnRefresh = [&]()
		{
			m_KeyboardOnUpdateTable.ClearList();
			uint32_t iteration{ 0 };
			for (Ref<Input::InputActionBinding> inputBinding : m_EditorInputMap->GetOnUpdateBindings())
			{

				if (inputBinding->GetActionType() != Input::KeyboardAction)
				{
					continue;
				}
				Input::KeyboardActionBinding* keyboardBinding = (Input::KeyboardActionBinding*)inputBinding.get();

				static std::function<void(EditorUI::ListEntry&, std::size_t iteration)> m_EditKeyboardSlot = [&](EditorUI::ListEntry& entry, std::size_t iteration)
				{
					UNREFERENCED_PARAMETER(iteration);
					m_KeyboardOnUpdateActiveSlot = (uint32_t)entry.m_Handle;
					m_KeyboardOnUpdateEditPopup.m_OpenPopup = true;
				};

				EditorUI::ListEntry newEntry;
				Assets::AssetHandle scriptHandle = keyboardBinding->GetScriptHandle();
				if (scriptHandle == Assets::EmptyHandle)
				{
					newEntry = {
						std::string("Key::") + Utility::KeyCodeToString(keyboardBinding->GetKeyBinding()),
						"None",
						iteration,
						m_EditKeyboardSlot
					};
				}
				else
				{
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
					newEntry = {
						std::string("Key::") + Utility::KeyCodeToString(keyboardBinding->GetKeyBinding()),
						script->m_ScriptName,
						iteration,
						m_EditKeyboardSlot
					};
				}

				m_KeyboardOnUpdateTable.InsertListEntry(newEntry);
				iteration++;
			}
		};
		m_KeyboardOnUpdateTable.m_Column1Title = "KeyCode";
		m_KeyboardOnUpdateTable.m_Column2Title = "Function";
		m_KeyboardOnUpdateTable.AddToSelectionList("Add New Slot", [&]()
		{
			m_KeyboardOnUpdateAddPopup.m_OpenPopup = true;
		});

		m_KeyboardOnUpdateAddPopup.m_Label = "Add New Slot";
		m_KeyboardOnUpdateAddPopup.m_PopupAction = [&]()
		{
			m_KeyboardOnUpdateAddKeyCode.m_CurrentOption = {Utility::KeyCodeToString(Key::A), Key::A};
			m_KeyboardOnUpdateAddFunction.m_CurrentOption = {"None", Assets::EmptyHandle};
		};
		m_KeyboardOnUpdateAddPopup.m_PopupContents = [&]()
		{
			m_KeyboardOnUpdateAddKeyCode.RenderOptions();
			m_KeyboardOnUpdateAddFunction.RenderOptions();
			m_SelectScriptTooltip.RenderTooltip();
			s_MainWindow->m_ScriptEditorPanel->DrawOnCreatePopup();
		};

		m_KeyboardOnUpdateAddPopup.m_ConfirmAction = [&]()
		{
			auto& keyboardBindings = m_EditorInputMap->GetOnUpdateBindings();
			Ref<Input::KeyboardActionBinding> newBinding = CreateRef<Input::KeyboardActionBinding>();
			newBinding->SetKeyBinding((KeyCode)m_KeyboardOnUpdateAddKeyCode.m_CurrentOption.m_Handle);
			Ref<Scripting::Script> script;
			if (m_KeyboardOnUpdateAddFunction.m_CurrentOption.m_Handle == Assets::EmptyHandle)
			{
				script = nullptr;
			}
			else
			{
				script = Assets::AssetService::GetScript(m_KeyboardOnUpdateAddFunction.m_CurrentOption.m_Handle);
			}
			newBinding->SetScript(script, m_KeyboardOnUpdateAddFunction.m_CurrentOption.m_Handle);

			keyboardBindings.push_back(newBinding);
			OnRefreshData();
			m_MainHeader.m_EditColorActive = true;
		};

		m_KeyboardOnUpdateAddKeyCode.m_Label = "Select Key";
		m_KeyboardOnUpdateAddKeyCode.m_LineCount = 7;
		m_KeyboardOnUpdateAddKeyCode.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			for (auto key : Key::s_AllKeyCodes)
			{
				spec.AddToOptions("All KeyCodes", Utility::KeyCodeToString(key), key);
			}
		};

		m_KeyboardOnUpdateAddFunction.m_Label = "Select Function";
		m_KeyboardOnUpdateAddFunction.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_KeyboardOnUpdateAddFunction.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, assetInfo] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				if (script->m_FuncType != WrappedFuncType::Void_None && script->m_FuncType != WrappedFuncType::Void_Float)
				{
					continue;
				}
				spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}
		};
		m_KeyboardOnUpdateAddFunction.m_OnEdit = [&](EditorUI::SelectOptionWidget& spec)
			{
				UNREFERENCED_PARAMETER(spec);
				// Initialize tooltip with options
				m_SelectScriptTooltip.ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					UNREFERENCED_PARAMETER(entry);
					m_KeyboardOnUpdateAddFunction.m_OpenPopup = true;
				} };
				m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						UNREFERENCED_PARAMETER(entry);
						// Open create script dialog in script editor
						s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_None, [&](Assets::AssetHandle scriptHandle)
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
								m_KeyboardOnUpdateAddFunction.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
						},{}, false);
					}};
				m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_SelectScriptTooltip.m_TooltipActive = true;
			};

		m_KeyboardOnUpdateEditPopup.m_Label = "Edit Slot";
		m_KeyboardOnUpdateEditPopup.m_PopupAction = [&]()
		{
			Input::KeyboardActionBinding* activeBinding = (Input::KeyboardActionBinding*)m_EditorInputMap->GetOnUpdateBindings().at(m_KeyboardOnUpdateActiveSlot).get();
			KG_ASSERT(activeBinding);

			m_KeyboardOnUpdateEditKeyCode.m_CurrentOption = { Utility::KeyCodeToString(activeBinding->GetKeyBinding()), activeBinding->GetKeyBinding() };
			if (activeBinding->GetScriptHandle() == Assets::EmptyHandle)
			{
				m_KeyboardOnUpdateEditFunction.m_CurrentOption = { "None", Assets::EmptyHandle };
			}
			else
			{
				KG_ASSERT(activeBinding->GetScript());
				m_KeyboardOnUpdateEditFunction.m_CurrentOption = { activeBinding->GetScript()->m_ScriptName.c_str(), activeBinding->GetScriptHandle()};
			}
		};
		m_KeyboardOnUpdateEditPopup.m_PopupContents = [&]()
		{
			m_KeyboardOnUpdateEditKeyCode.RenderOptions();
			m_KeyboardOnUpdateEditFunction.RenderOptions();
			m_SelectScriptTooltip.RenderTooltip();
			s_MainWindow->m_ScriptEditorPanel->DrawOnCreatePopup();
		};
		m_KeyboardOnUpdateEditPopup.m_DeleteAction = [&]()
		{
			std::vector<Ref<Input::InputActionBinding>>& bindings = m_EditorInputMap->GetOnUpdateBindings();
			bindings.erase(bindings.begin() + m_KeyboardOnUpdateActiveSlot);
			OnRefreshData();
			m_MainHeader.m_EditColorActive = true;
		};

		m_KeyboardOnUpdateEditPopup.m_ConfirmAction = [&]()
		{
			Input::KeyboardActionBinding* newBinding = (Input::KeyboardActionBinding*)m_EditorInputMap->GetOnUpdateBindings().at(m_KeyboardOnUpdateActiveSlot).get();
			KG_ASSERT(newBinding);
			newBinding->SetKeyBinding((KeyCode)m_KeyboardOnUpdateEditKeyCode.m_CurrentOption.m_Handle);
			Ref<Scripting::Script> script;
			if (m_KeyboardOnUpdateEditFunction.m_CurrentOption.m_Handle == Assets::EmptyHandle)
			{
				script = nullptr;
			}
			else
			{
				script = Assets::AssetService::GetScript(m_KeyboardOnUpdateEditFunction.m_CurrentOption.m_Handle);
			}
			newBinding->SetScript(script, m_KeyboardOnUpdateEditFunction.m_CurrentOption.m_Handle);

			OnRefreshData();
			m_MainHeader.m_EditColorActive = true;
		};

		m_KeyboardOnUpdateEditKeyCode.m_Label = "Select Key";
		m_KeyboardOnUpdateEditKeyCode.m_LineCount = 7;
		m_KeyboardOnUpdateEditKeyCode.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			for (auto key : Key::s_AllKeyCodes)
			{
				spec.AddToOptions("All KeyCodes", Utility::KeyCodeToString(key), key);
			}
		};

		m_KeyboardOnUpdateEditFunction.m_Label = "Select Function";
		m_KeyboardOnUpdateEditFunction.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_KeyboardOnUpdateEditFunction.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, assetInfo] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				if (script->m_FuncType != WrappedFuncType::Void_None && script->m_FuncType != WrappedFuncType::Void_Float)
				{
					continue;
				}
				spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}
		};

		m_KeyboardOnUpdateEditFunction.m_OnEdit = [&](EditorUI::SelectOptionWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			// Initialize tooltip with options
			m_SelectScriptTooltip.ClearEntries();
			EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
			{
				UNREFERENCED_PARAMETER(entry);
				m_KeyboardOnUpdateEditFunction.m_OpenPopup = true;
			} };
			m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

			EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
			{
					UNREFERENCED_PARAMETER(entry);
					// Open create script dialog in script editor
					s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_None, [&](Assets::AssetHandle scriptHandle)
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
							m_KeyboardOnUpdateEditFunction.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
					},{}, false);
				} };
			m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

			// Open tooltip
			m_SelectScriptTooltip.m_TooltipActive = true;
		};


		// On Key Pressed List
		m_KeyboardOnKeyPressedTable.m_Label = "On Key Pressed";
		m_KeyboardOnKeyPressedTable.m_Expanded = true;
		m_KeyboardOnKeyPressedTable.m_OnRefresh = [&]()
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
					UNREFERENCED_PARAMETER(iteration);
					m_KeyboardOnKeyPressedActiveSlot = (uint32_t)entry.m_Handle;
					m_KeyboardOnKeyPressedEditPopup.m_OpenPopup = true;
				};

				EditorUI::ListEntry newEntry;
				Assets::AssetHandle scriptHandle = keyboardBinding->GetScriptHandle();
				if (scriptHandle == Assets::EmptyHandle)
				{
					newEntry = {
						std::string("Key::") + Utility::KeyCodeToString(keyboardBinding->GetKeyBinding()),
						"None",
						iteration,
						m_EditKeyboardSlot
					};
				}
				else
				{
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
					newEntry = {
						std::string("Key::") + Utility::KeyCodeToString(keyboardBinding->GetKeyBinding()),
						script->m_ScriptName,
						iteration,
						m_EditKeyboardSlot
					};
				}

				m_KeyboardOnKeyPressedTable.InsertListEntry(newEntry);
				iteration++;
			}
		};
		m_KeyboardOnKeyPressedTable.m_Column1Title = "KeyCode";
		m_KeyboardOnKeyPressedTable.m_Column2Title = "Function";
		m_KeyboardOnKeyPressedTable.AddToSelectionList("Add New Slot", [&]()
			{
				m_KeyboardOnKeyPressedAddPopup.m_OpenPopup = true;
			});

		m_KeyboardOnKeyPressedAddPopup.m_Label = "Add New Slot";
		m_KeyboardOnKeyPressedAddPopup.m_PopupAction = [&]()
		{
			m_KeyboardOnKeyPressedAddKeyCode.m_CurrentOption = { Utility::KeyCodeToString(Key::A), Key::A };
			m_KeyboardOnKeyPressedAddFunction.m_CurrentOption = { "None", Assets::EmptyHandle };
		};
		m_KeyboardOnKeyPressedAddPopup.m_PopupContents = [&]()
		{
			m_KeyboardOnKeyPressedAddKeyCode.RenderOptions();
			m_KeyboardOnKeyPressedAddFunction.RenderOptions();
			m_SelectScriptTooltip.RenderTooltip();
			s_MainWindow->m_ScriptEditorPanel->DrawOnCreatePopup();
		};

		m_KeyboardOnKeyPressedAddPopup.m_ConfirmAction = [&]()
		{
			auto& keyboardBindings = m_EditorInputMap->GetOnKeyPressedBindings();
			Ref<Input::KeyboardActionBinding> newBinding = CreateRef<Input::KeyboardActionBinding>();
			newBinding->SetKeyBinding((KeyCode)m_KeyboardOnKeyPressedAddKeyCode.m_CurrentOption.m_Handle);
			Ref<Scripting::Script> script;
			if (m_KeyboardOnKeyPressedAddFunction.m_CurrentOption.m_Handle == Assets::EmptyHandle)
			{
				script = nullptr;
			}
			else
			{
				script = Assets::AssetService::GetScript(m_KeyboardOnKeyPressedAddFunction.m_CurrentOption.m_Handle);
			}
			newBinding->SetScript(script, m_KeyboardOnKeyPressedAddFunction.m_CurrentOption.m_Handle);

			keyboardBindings.push_back(newBinding);
			OnRefreshData();
			m_MainHeader.m_EditColorActive = true;
		};

		m_KeyboardOnKeyPressedAddKeyCode.m_Label = "Select Key";
		m_KeyboardOnKeyPressedAddKeyCode.m_LineCount = 7;
		m_KeyboardOnKeyPressedAddKeyCode.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			for (auto key : Key::s_AllKeyCodes)
			{
				spec.AddToOptions("All KeyCodes", Utility::KeyCodeToString(key), key);
			}
		};

		m_KeyboardOnKeyPressedAddFunction.m_Label = "Select Function";
		m_KeyboardOnKeyPressedAddFunction.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_KeyboardOnKeyPressedAddFunction.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, assetInfo] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				if (script->m_FuncType != WrappedFuncType::Void_None)
				{
					continue;
				}
				spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}
		};
		m_KeyboardOnKeyPressedAddFunction.m_OnEdit = [&](EditorUI::SelectOptionWidget& spec)
			{
				UNREFERENCED_PARAMETER(spec);
				// Initialize tooltip with options
				m_SelectScriptTooltip.ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					UNREFERENCED_PARAMETER(entry);
					m_KeyboardOnKeyPressedAddFunction.m_OpenPopup = true;
				} };
				m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						UNREFERENCED_PARAMETER(entry);
						// Open create script dialog in script editor
						s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_None, [&](Assets::AssetHandle scriptHandle)
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
								m_KeyboardOnKeyPressedAddFunction.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
							},{},  false);
						} };
				m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_SelectScriptTooltip.m_TooltipActive = true;
			};

		m_KeyboardOnKeyPressedEditPopup.m_Label = "Edit Slot";
		m_KeyboardOnKeyPressedEditPopup.m_PopupAction = [&]()
		{
			Input::KeyboardActionBinding* activeBinding = (Input::KeyboardActionBinding*)m_EditorInputMap->GetOnKeyPressedBindings().at(m_KeyboardOnKeyPressedActiveSlot).get();
			KG_ASSERT(activeBinding);

			m_KeyboardOnKeyPressedEditKeyCode.m_CurrentOption = { Utility::KeyCodeToString(activeBinding->GetKeyBinding()), activeBinding->GetKeyBinding() };
			if (activeBinding->GetScriptHandle() == Assets::EmptyHandle)
			{
				m_KeyboardOnKeyPressedEditFunction.m_CurrentOption = { "None", Assets::EmptyHandle };
			}
			else
			{
				KG_ASSERT(activeBinding->GetScript());
				m_KeyboardOnKeyPressedEditFunction.m_CurrentOption = { activeBinding->GetScript()->m_ScriptName.c_str(), activeBinding->GetScriptHandle()};
			}
		};
		m_KeyboardOnKeyPressedEditPopup.m_PopupContents = [&]()
		{
			m_KeyboardOnKeyPressedEditKeyCode.RenderOptions();
			m_KeyboardOnKeyPressedEditFunction.RenderOptions();
			m_SelectScriptTooltip.RenderTooltip();
			s_MainWindow->m_ScriptEditorPanel->DrawOnCreatePopup();
		};
		m_KeyboardOnKeyPressedEditPopup.m_DeleteAction = [&]()
		{
			std::vector<Ref<Input::InputActionBinding>>& bindings = m_EditorInputMap->GetOnKeyPressedBindings();
			bindings.erase(bindings.begin() + m_KeyboardOnKeyPressedActiveSlot);
			OnRefreshData();
			m_MainHeader.m_EditColorActive = true;
		};

		m_KeyboardOnKeyPressedEditPopup.m_ConfirmAction = [&]()
		{
			Input::KeyboardActionBinding* newBinding = (Input::KeyboardActionBinding*)m_EditorInputMap->GetOnKeyPressedBindings().at(m_KeyboardOnKeyPressedActiveSlot).get();
			KG_ASSERT(newBinding);
			newBinding->SetKeyBinding((KeyCode)m_KeyboardOnKeyPressedEditKeyCode.m_CurrentOption.m_Handle);
			Ref<Scripting::Script> script;
			if (m_KeyboardOnKeyPressedEditFunction.m_CurrentOption.m_Handle == Assets::EmptyHandle)
			{
				script = nullptr;
			}
			else
			{
				script = Assets::AssetService::GetScript(m_KeyboardOnKeyPressedEditFunction.m_CurrentOption.m_Handle);
			}
			newBinding->SetScript(script, m_KeyboardOnKeyPressedEditFunction.m_CurrentOption.m_Handle);

			OnRefreshData();
			m_MainHeader.m_EditColorActive = true;
		};

		m_KeyboardOnKeyPressedEditKeyCode.m_Label = "Select Key";
		m_KeyboardOnKeyPressedEditKeyCode.m_LineCount = 7;
		m_KeyboardOnKeyPressedEditKeyCode.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			for (KeyCode key : Key::s_AllKeyCodes)
			{
				spec.AddToOptions("All KeyCodes", Utility::KeyCodeToString(key), key);
			}
		};

		m_KeyboardOnKeyPressedEditFunction.m_Label = "Select Function";
		m_KeyboardOnKeyPressedEditFunction.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_KeyboardOnKeyPressedEditFunction.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, assetInfo] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				if (script->m_FuncType != WrappedFuncType::Void_None)
				{
					continue;
				}
				spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}
		};
		m_KeyboardOnKeyPressedEditFunction.m_OnEdit = [&](EditorUI::SelectOptionWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			// Initialize tooltip with options
			m_SelectScriptTooltip.ClearEntries();
			EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
			{
				UNREFERENCED_PARAMETER(entry);
				m_KeyboardOnKeyPressedEditFunction.m_OpenPopup = true;
			} };
			m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

			EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
			{
					UNREFERENCED_PARAMETER(entry);
					// Open create script dialog in script editor
					s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_None, [&](Assets::AssetHandle scriptHandle)
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
							m_KeyboardOnKeyPressedEditFunction.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
						},{}, false);
					} };
			m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

			// Open tooltip
			m_SelectScriptTooltip.m_TooltipActive = true;
		};

		// Polling List
		m_KeyboardPollingTable.m_Label = "Polling";
		m_KeyboardPollingTable.m_Expanded = true;
		m_KeyboardPollingTable.m_OnRefresh = [&]()
		{
			m_KeyboardPollingTable.ClearList();
			uint32_t iteration{ 0 };
			for (auto code: m_EditorInputMap->GetKeyboardPolling())
			{

				static std::function<void(EditorUI::ListEntry&, std::size_t)> m_EditKeyboardSlot = [&](EditorUI::ListEntry& entry, std::size_t iteration)
				{
					UNREFERENCED_PARAMETER(iteration);
					m_KeyboardPollingActiveSlot = (uint32_t)entry.m_Handle;
					m_KeyboardPollingEditSlot.m_OpenPopup = true;
				};

				EditorUI::ListEntry newEntry = {
					"Slot::" + std::to_string(iteration),
					std::string("Key::") + Utility::KeyCodeToString(code),
					iteration,
					m_EditKeyboardSlot
				};

				m_KeyboardPollingTable.InsertListEntry(newEntry);
				iteration++;
			}
		};
		m_KeyboardPollingTable.m_Column1Title = "Slot";
		m_KeyboardPollingTable.m_Column2Title = "KeyCode";
		m_KeyboardPollingTable.AddToSelectionList("Add New Slot", [&]()
		{
			m_KeyboardPollingAddSlot.m_OpenPopup = true;
		});

		m_KeyboardPollingAddSlot.m_Label = "Add New Slot";
		m_KeyboardPollingAddSlot.m_PopupAction = [&]()
		{
			m_KeyboardPollingAddKeyCode.m_CurrentOption = { Utility::KeyCodeToString(Key::A), Key::A };
		};
		m_KeyboardPollingAddSlot.m_PopupContents = [&]()
		{
			m_KeyboardPollingAddKeyCode.RenderOptions();
		};

		m_KeyboardPollingAddSlot.m_ConfirmAction = [&]()
		{
			std::vector<KeyCode>& keyboardPolling = m_EditorInputMap->GetKeyboardPolling();
			KeyCode code = (KeyCode)m_KeyboardPollingAddKeyCode.m_CurrentOption.m_Handle;
			keyboardPolling.push_back(code);
			OnRefreshData();
			m_MainHeader.m_EditColorActive = true;
		};

		m_KeyboardPollingAddKeyCode.m_Label = "Select Key";
		m_KeyboardPollingAddKeyCode.m_LineCount = 7;
		m_KeyboardPollingAddKeyCode.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			for (uint16_t key : Key::s_AllKeyCodes)
			{
				if (key == 0)
				{
					spec.AddToOptions("Clear", Utility::KeyCodeToString(key), key);
				}
				else
				{
					spec.AddToOptions("All KeyCodes", Utility::KeyCodeToString(key), key);
				}
				
			}
		};

		m_KeyboardPollingEditSlot.m_Label = "Edit Slot";
		m_KeyboardPollingEditSlot.m_PopupAction = [&]()
		{
			KeyCode activeCode = m_EditorInputMap->GetKeyboardPolling().at(m_KeyboardPollingActiveSlot);

			m_KeyboardPollingEditKeyCode.m_CurrentOption = { Utility::KeyCodeToString(activeCode), activeCode };
		};
		m_KeyboardPollingEditSlot.m_PopupContents = [&]()
		{
			m_KeyboardPollingEditKeyCode.RenderOptions();
		};

		m_KeyboardPollingEditSlot.m_DeleteAction = [&]()
		{
			std::vector<KeyCode>& codes = m_EditorInputMap->GetKeyboardPolling();
			codes.erase(codes.begin() + m_KeyboardPollingActiveSlot);
			OnRefreshData();
			m_MainHeader.m_EditColorActive = true;
		};

		m_KeyboardPollingEditSlot.m_ConfirmAction = [&]()
		{
			auto& keyboardPolling = m_EditorInputMap->GetKeyboardPolling();
			KeyCode code = (KeyCode)m_KeyboardPollingEditKeyCode.m_CurrentOption.m_Handle;
			keyboardPolling[m_KeyboardPollingActiveSlot] = code;
			OnRefreshData();
			m_MainHeader.m_EditColorActive = true;
		};

		m_KeyboardPollingEditKeyCode.m_Label = "Select Key";
		m_KeyboardPollingEditKeyCode.m_LineCount = 7;
		m_KeyboardPollingEditKeyCode.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			for (uint16_t key : Key::s_AllKeyCodes)
			{
				if (key == 0)
				{
					spec.AddToOptions("Clear", Utility::KeyCodeToString(key), key);
				}
				else
				{
					spec.AddToOptions("All KeyCodes", Utility::KeyCodeToString(key), key);
				}
			}
		};
	}
	
}

#include "Windows/MainWindow/StateEditorPanel.h"
#include "EditorApp.h"


namespace Kargono
{
	static EditorApp* s_EditorApp { nullptr };
	static Windows::MainWindow* s_MainWindow{ nullptr };
}

namespace Kargono::Panels
{
	void StateEditorPanel::OnOpenStateDialog()
	{
		m_OpenStatePopupSpec.m_OpenPopup = true;
	}
	void StateEditorPanel::OnCreateStateDialog()
	{
		m_SelectStateLocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveContext().GetProjectPaths().GetAssetDirectory();
		m_CreateStatePopupSpec.m_OpenPopup = true;
	}

	void StateEditorPanel::OnRefreshData()
	{
		if (m_EditorState)
		{
			// Refresh scripts
		    Assets::AssetRef<Scripting::Script> onUpdateScript = Assets::AssetService::GetScript(m_EditorState->OnUpdateHandle);
			m_SelectOnUpdateScript.m_CurrentOption = onUpdateScript ? EditorUI::OptionEntry(onUpdateScript->m_ScriptName.c_str(), m_EditorState->OnUpdateHandle) : EditorUI::OptionEntry("None", Assets::k_EmptyHandle);

		    Assets::AssetRef<Scripting::Script> onEnterStateScript = Assets::AssetService::GetScript(m_EditorState->OnEnterStateHandle);
			m_SelectOnEnterStateScript.m_CurrentOption = onEnterStateScript ? EditorUI::OptionEntry(onEnterStateScript->m_ScriptName.c_str(), m_EditorState->OnEnterStateHandle) : EditorUI::OptionEntry("None", Assets::k_EmptyHandle);

		    Assets::AssetRef<Scripting::Script> onExitStateScript = Assets::AssetService::GetScript(m_EditorState->OnExitStateHandle);
			m_SelectOnExitStateScript.m_CurrentOption = onExitStateScript ? EditorUI::OptionEntry(onExitStateScript->m_ScriptName.c_str(), m_EditorState->OnExitStateHandle) : EditorUI::OptionEntry("None", Assets::k_EmptyHandle);

		    Assets::AssetRef<Scripting::Script> onAIMessageScript = Assets::AssetService::GetScript(m_EditorState->OnMessageHandle);
			m_SelectOnAIMessageScript.m_CurrentOption = onAIMessageScript ? EditorUI::OptionEntry(onAIMessageScript->m_ScriptName.c_str(), m_EditorState->OnMessageHandle) : EditorUI::OptionEntry("None", Assets::k_EmptyHandle);
		}
	}

	void StateEditorPanel::OnOpenState(Assets::AssetHandle newHandle)
	{
		m_EditorState = Assets::s_StateManager.GetAssetByHandle(newHandle);
		m_EditorStateHandle = newHandle;
		m_MainHeader.m_EditColorActive = false;
		m_MainHeader.m_Label = Assets::s_StateManager.GetAssetRegistry().at(
			m_EditorStateHandle).Data.FileLocation.filename().string();
		OnRefreshData();
	}

	StateEditorPanel::StateEditorPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
		s_MainWindow->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(StateEditorPanel::OnKeyPressedEditor));
		InitializeOpeningScreen();
		InitializeStateHeader();
		InitializeMainPanel();
	}
	void StateEditorPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION()
			EditorUI::EditorUIContext::StartRenderWindow(m_PanelName, &s_MainWindow->m_ShowStateEditor);

		if (!EditorUI::EditorUIContext::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIContext::EndRenderWindow();
			return;
		}

		if (!m_EditorState)
		{
			// Opening/Null State Screen
			EditorUI::EditorUIContext::NewItemScreen("Open Existing AI State", KG_BIND_CLASS_FN(OnOpenStateDialog), "Create New AI State", KG_BIND_CLASS_FN(OnCreateStateDialog));
			m_CreateStatePopupSpec.RenderPopup();
			m_OpenStatePopupSpec.RenderOptions();
		}
		else
		{
			// Header
			m_MainHeader.RenderHeader();
			m_DeleteStateWarning.RenderPopup();
			m_CloseStateWarning.RenderPopup();

			m_SelectOnUpdateScript.RenderOptions();
			m_SelectOnEnterStateScript.RenderOptions();
			m_SelectOnExitStateScript.RenderOptions();
			m_SelectOnAIMessageScript.RenderOptions();
			m_SelectScriptTooltip.RenderTooltip();

		}

		EditorUI::EditorUIContext::EndRenderWindow();
	}
	bool StateEditorPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}

	bool StateEditorPanel::OnAssetEvent(Events::Event* event)
	{

		Events::ManageAsset* manageAsset = (Events::ManageAsset*)event;
		// Manage script deletion event
		if (manageAsset->GetAssetType() == Assets::AssetType::Script &&
			manageAsset->GetAction() == Events::ManageAssetAction::PreDelete)
		{
			if (m_SelectOnUpdateScript.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectOnUpdateScript.m_CurrentOption = { "None", Assets::k_EmptyHandle };
			}

			if (m_SelectOnEnterStateScript.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectOnEnterStateScript.m_CurrentOption = { "None", Assets::k_EmptyHandle };
			}

			if (m_SelectOnExitStateScript.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectOnExitStateScript.m_CurrentOption = { "None", Assets::k_EmptyHandle };
			}

			if (m_SelectOnAIMessageScript.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectOnAIMessageScript.m_CurrentOption = { "None", Assets::k_EmptyHandle };
			}

			if (m_EditorState)
			{
				Assets::AssetService::RemoveScriptFromState(m_EditorState, manageAsset->GetAssetID());
			}
		}

		if (manageAsset->GetAssetType() == Assets::AssetType::State &&
			manageAsset->GetAction() == Events::ManageAssetAction::PreDelete)
		{
			if (manageAsset->GetAssetID() != m_EditorStateHandle)
			{
				return false;
			}

			// Handle deletion of asset
			ResetPanelResources();
			return true;
		}

		if (manageAsset->GetAssetType() == Assets::AssetType::State &&
			manageAsset->GetAction() == Events::ManageAssetAction::UpdateAssetInfo)
		{
			if (manageAsset->GetAssetID() != m_EditorStateHandle)
			{
				return false;
			}

			// Update header
			m_MainHeader.m_Label = Assets::AssetService::GetStateFileLocation(manageAsset->GetAssetID()).filename().string();
			return true;
		}

		return false;
	}

	void StateEditorPanel::ResetPanelResources()
	{
		m_EditorState = nullptr;
		m_EditorStateHandle = Assets::k_EmptyHandle;
	}

	void StateEditorPanel::OpenCreateDialog(std::filesystem::path& createLocation)
	{
		// Open StateMachines State Window
		s_MainWindow->m_ShowStateEditor = true;
		EditorUI::EditorUIContext::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIContext::SetFocusedWindow(m_PanelName);

		if (!m_EditorState)
		{
			// Open dialog to create editor StateMachines State
			OnCreateStateDialog();
			m_SelectStateLocationSpec.m_CurrentOption = createLocation;
		}
		else
		{
			// Add warning to close active StateMachines state before creating a new State
			s_MainWindow->OpenWarningMessage("An AI State is already active inside the editor. Please close the current AI State before creating a new one.");
		}

	}

	void StateEditorPanel::OpenAssetInEditor(std::filesystem::path& assetLocation)
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
		Assets::AssetHandle assetHandle = Assets::s_StateHandleFromFileLocationManager.GetAssetByHandle(relativePath);

		// Validate resulting handle
		if (!assetHandle)
		{
			KG_WARN("Could not open asset in editor. Provided path does not result in an asset inside the registry.");
			return;
		}

		// Open the editor panel to be visible
		s_MainWindow->m_ShowStateEditor = true;
		EditorUI::EditorUIContext::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIContext::SetFocusedWindow(m_PanelName);

		// Early out if asset is already open
		if (m_EditorStateHandle == assetHandle)
		{
			return;
		}

		// Check if panel is already occupied by an asset
		if (!m_EditorState)
		{
			// Open dialog to create editor StateMachines State
			OnOpenState(assetHandle);
		}
		else
		{
			// Add warning to close active StateMachines state before opening a new State
			s_MainWindow->OpenWarningMessage("An AI State is already active inside the editor. Please close the current AI State before opening a new one.");
		}
	}

	void StateEditorPanel::InitializeOpeningScreen()
	{
		m_OpenStatePopupSpec.m_Label = "Open AI State";
		m_OpenStatePopupSpec.m_LineCount = 2;
		m_OpenStatePopupSpec.m_CurrentOption = { "None", Assets::k_EmptyHandle };
		m_OpenStatePopupSpec.m_Flags |= EditorUI::SelectOption_PopupOnly;
		m_OpenStatePopupSpec.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.GetAllOptions().clear();
			spec.m_CurrentOption = { "None", Assets::k_EmptyHandle };

			spec.AddToOptions("Clear", "None", Assets::k_EmptyHandle);
			for (auto& [handle, asset] : Assets::s_StateManager.GetAssetRegistry())
			{
				spec.AddToOptions("All Options", asset.Data.FileLocation.filename().string(), handle);
			}
		};

		m_OpenStatePopupSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.m_Handle == Assets::k_EmptyHandle)
			{
				KG_WARN("No AI State Selected");
				return;
			}
			if (!Assets::s_StateManager.GetAssetRegistry().contains(selection.m_Handle))
			{
				KG_WARN("Could not find the AI State specified");
				return;
			}

			OnOpenState(selection.m_Handle);
		};

		m_SelectStateNameSpec.m_Label = "New Name";
		m_SelectStateNameSpec.m_CurrentOption = "Empty";

		m_SelectStateLocationSpec.m_Label = "Location";
		m_SelectStateLocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveContext().GetProjectPaths().GetAssetDirectory();
		m_SelectStateLocationSpec.m_ConfirmAction = [&](std::string_view path) 
		{
			if (!Utility::FileSystem::DoesPathContainSubPath(Projects::ProjectService::GetActiveContext().GetProjectPaths().GetAssetDirectory(), path))
			{
				KG_WARN("Cannot create an asset outside of the project's asset directory.");
				m_SelectStateLocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveContext().GetProjectPaths().GetAssetDirectory();
			}
		};

		m_CreateStatePopupSpec.m_Label = "Create AI State";
		m_CreateStatePopupSpec.m_ConfirmAction = [&]()
		{
			if (m_SelectStateNameSpec.m_CurrentOption == "")
			{
				return;
			}

			m_EditorStateHandle = Assets::AssetService::CreateState(m_SelectStateNameSpec.m_CurrentOption.c_str(), m_SelectStateLocationSpec.m_CurrentOption);
			if (m_EditorStateHandle == Assets::k_EmptyHandle)
			{
				KG_WARN("AI state was not created");
				return;
			}
			m_EditorState = Assets::s_StateManager.GetAssetByHandle(m_EditorStateHandle);
			m_MainHeader.m_EditColorActive = false;
			m_MainHeader.m_Label = Assets::s_StateManager.GetAssetRegistry().at(
				m_EditorStateHandle).Data.FileLocation.filename().string();
			OnRefreshData();
		};
		m_CreateStatePopupSpec.m_PopupContents = [&]()
		{
			m_SelectStateNameSpec.RenderText();
			m_SelectStateLocationSpec.RenderChooseDir();
		};
	}

	void StateEditorPanel::InitializeStateHeader()
	{
		// Header (Game State Name and Options)
		m_DeleteStateWarning.m_Label = "Delete AI State";
		m_DeleteStateWarning.m_ConfirmAction = [&]()
		{
			Assets::AssetService::DeleteState(m_EditorStateHandle);
			m_EditorStateHandle = 0;
			m_EditorState = nullptr;
		};
		m_DeleteStateWarning.m_PopupContents = [&]()
		{
			EditorUI::EditorUIContext::Text("Are you sure you want to delete this AI state object?");
		};

		m_CloseStateWarning.m_Label = "Close AI State";
		m_CloseStateWarning.m_ConfirmAction = [&]()
		{
			m_EditorStateHandle = 0;
			m_EditorState = nullptr;
		};
		m_CloseStateWarning.m_PopupContents = [&]()
		{
			EditorUI::EditorUIContext::Text("Are you sure you want to close this ai state object without saving?");
		};

		m_MainHeader.AddToSelectionList("Save", [&]()
			{
				Assets::s_StateManager.UpdateAsset(m_EditorState)
				m_MainHeader.m_EditColorActive = false;
			});
		m_MainHeader.AddToSelectionList("Close", [&]()
			{
				if (m_MainHeader.m_EditColorActive)
				{
					m_CloseStateWarning.m_OpenPopup = true;
				}
				else
				{
					m_EditorStateHandle = 0;
					m_EditorState = nullptr;
				}
			});
		m_MainHeader.AddToSelectionList("Delete", [&]()
			{
				m_DeleteStateWarning.m_OpenPopup = true;
			});
	}

	void StateEditorPanel::InitializeMainPanel()
	{
		// On Update Script
		m_SelectOnUpdateScript.m_Label = "On Update Script";
		m_SelectOnUpdateScript.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectOnUpdateScript.m_LineCount = 3;
		m_SelectOnUpdateScript.m_CurrentOption = { "None", Assets::k_EmptyHandle };
		m_SelectOnUpdateScript.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.GetAllOptions().clear();

			spec.AddToOptions("Clear", "None", Assets::k_EmptyHandle);
			for (auto& [handle, asset] : Assets::s_ScriptManager.GetAssetRegistry())
			{
			    Assets::AssetRef<Scripting::Script> script = Assets::s_ScriptManager.GetAssetByHandle(handle);
				if (script->m_FuncType != WrappedFuncType::Void_EntityFloat)
				{
					continue;
				}
				spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}
		};
		m_SelectOnUpdateScript.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			// If empty option is selected, clear script
			if (selection.m_Handle == Assets::k_EmptyHandle)
			{
				m_EditorState->OnUpdateHandle = Assets::k_EmptyHandle;
				m_EditorState->OnUpdate = nullptr;
				m_MainHeader.m_EditColorActive = true;
				return;
			}

			// Get Script
		    Assets::AssetRef<Scripting::Script> selectedScript = Assets::s_ScriptManager.GetAssetByHandle(selection.m_Handle);
			KG_ASSERT(selectedScript);

			// Update ai state's script
			m_EditorState->OnUpdateHandle = selection.m_Handle;
			m_EditorState->OnUpdate = selectedScript;
			m_MainHeader.m_EditColorActive = true;
		};
		m_SelectOnUpdateScript.m_OnEdit = [&](EditorUI::SelectOptionWidget& /*spec*/)
		{
			// Initialize tooltip with options
			m_SelectScriptTooltip.ClearEntries();
			EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& /*entry*/)
			{
				m_SelectOnUpdateScript.m_OpenPopup = true;
			} };
			m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

			EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
			{
					UNREFERENCED_PARAMETER(entry);
					// Open create script dialog in script editor
					s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_EntityFloat, [&](Assets::AssetHandle scriptHandle)
					{
							// Ensure handle provides a script in the registry
							if (!Assets::AssetService::HasScript(scriptHandle))
							{
								KG_WARN("Could not find on update function in ai state panel");
								return;
							}

							// Ensure function type matches definition
						    Assets::AssetRef<Scripting::Script> script = Assets::s_ScriptManager.GetAssetByHandle(scriptHandle);
							if (script->m_FuncType != WrappedFuncType::Void_EntityFloat)
							{
								KG_WARN("Incorrect function type returned when linking script to usage point");
								return;
							}

							// Fill the new script handle
							m_EditorState->OnUpdateHandle = scriptHandle;
							m_EditorState->OnUpdate = script;
							m_MainHeader.m_EditColorActive = true;
							m_SelectOnUpdateScript.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
						}, {"activeEntity", "deltaTime"});

					} };
			m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

			// Open tooltip
			m_SelectScriptTooltip.m_TooltipActive = true;
		};

		// On Enter State Script
		m_SelectOnEnterStateScript.m_Label = "On Enter State Script";
		m_SelectOnEnterStateScript.m_LineCount = 3;
		m_SelectOnEnterStateScript.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectOnEnterStateScript.m_CurrentOption = { "None", Assets::k_EmptyHandle };
		m_SelectOnEnterStateScript.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.GetAllOptions().clear();

			spec.AddToOptions("Clear", "None", Assets::k_EmptyHandle);
			for (auto& [handle, asset] : Assets::s_ScriptManager.GetAssetRegistry())
			{
			    Assets::AssetRef<Scripting::Script> script = Assets::s_ScriptManager.GetAssetByHandle(handle);
				if (script->m_FuncType != WrappedFuncType::Void_Entity)
				{
					continue;
				}
				spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}
		};
		m_SelectOnEnterStateScript.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			// If empty option is selected, clear script
			if (selection.m_Handle == Assets::k_EmptyHandle)
			{
				m_EditorState->OnEnterStateHandle = Assets::k_EmptyHandle;
				m_EditorState->OnEnterState = nullptr;
				m_MainHeader.m_EditColorActive = true;
				return;
			}

			// Get Script
		    Assets::AssetRef<Scripting::Script> selectedScript = Assets::s_ScriptManager.GetAssetByHandle(selection.m_Handle);
			KG_ASSERT(selectedScript);

			// Update ai state's script
			m_EditorState->OnEnterStateHandle = selection.m_Handle;
			m_EditorState->OnEnterState = selectedScript;
			m_MainHeader.m_EditColorActive = true;
		};
		m_SelectOnEnterStateScript.m_OnEdit = [&](EditorUI::SelectOptionWidget& spec)
			{
				UNREFERENCED_PARAMETER(spec);
				// Initialize tooltip with options
				m_SelectScriptTooltip.ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					UNREFERENCED_PARAMETER(entry);
					m_SelectOnEnterStateScript.m_OpenPopup = true;
				} };
				m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						UNREFERENCED_PARAMETER(entry);
						// Open create script dialog in script editor
						s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_Entity, [&](Assets::AssetHandle scriptHandle)
						{
								// Ensure handle provides a script in the registry
								if (!Assets::AssetService::HasScript(scriptHandle))
								{
									KG_WARN("Could not find on enter state function in ai state panel");
									return;
								}

								// Ensure function type matches definition
							    Assets::AssetRef<Scripting::Script> script = Assets::s_ScriptManager.GetAssetByHandle(scriptHandle);
								if (script->m_FuncType != WrappedFuncType::Void_Entity)
								{
									KG_WARN("Incorrect function type returned when linking script to usage point");
									return;
								}

								// Fill the new script handle
								m_EditorState->OnEnterStateHandle = scriptHandle;
								m_EditorState->OnEnterState = script;
								m_MainHeader.m_EditColorActive = true;
								m_SelectOnEnterStateScript.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
						}, {"activeEntity"});

						} };
				m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_SelectScriptTooltip.m_TooltipActive = true;
			};

		// On Exit State Script
		m_SelectOnExitStateScript.m_Label = "On Exit State Script";
		m_SelectOnExitStateScript.m_LineCount = 3;
		m_SelectOnExitStateScript.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectOnExitStateScript.m_CurrentOption = { "None", Assets::k_EmptyHandle };
		m_SelectOnExitStateScript.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.GetAllOptions().clear();

			spec.AddToOptions("Clear", "None", Assets::k_EmptyHandle);
			for (auto& [handle, asset] : Assets::s_ScriptManager.GetAssetRegistry())
			{
			    Assets::AssetRef<Scripting::Script> script = Assets::s_ScriptManager.GetAssetByHandle(handle);
				if (script->m_FuncType != WrappedFuncType::Void_Entity)
				{
					continue;
				}
				spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}
		};
		m_SelectOnExitStateScript.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			// If empty option is selected, clear script
			if (selection.m_Handle == Assets::k_EmptyHandle)
			{
				m_EditorState->OnExitStateHandle = Assets::k_EmptyHandle;
				m_EditorState->OnExitState = nullptr;
				m_MainHeader.m_EditColorActive = true;
				return;
			}

			// Get Script
		    Assets::AssetRef<Scripting::Script> selectedScript = Assets::s_ScriptManager.GetAssetByHandle(selection.m_Handle);
			KG_ASSERT(selectedScript);

			// Update ai state's script
			m_EditorState->OnExitStateHandle = selection.m_Handle;
			m_EditorState->OnExitState = selectedScript;
			m_MainHeader.m_EditColorActive = true;
		};
		m_SelectOnExitStateScript.m_OnEdit = [&](EditorUI::SelectOptionWidget& spec)
			{
				UNREFERENCED_PARAMETER(spec);
				// Initialize tooltip with options
				m_SelectScriptTooltip.ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					UNREFERENCED_PARAMETER(entry);
					m_SelectOnExitStateScript.m_OpenPopup = true;
				} };
				m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						UNREFERENCED_PARAMETER(entry);
						// Open create script dialog in script editor
						s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_Entity, [&](Assets::AssetHandle scriptHandle)
						{
								// Ensure handle provides a script in the registry
								if (!Assets::AssetService::HasScript(scriptHandle))
								{
									KG_WARN("Could not find on exit state function in ai state panel");
									return;
								}

								// Ensure function type matches definition
							    Assets::AssetRef<Scripting::Script> script = Assets::s_ScriptManager.GetAssetByHandle(scriptHandle);
								if (script->m_FuncType != WrappedFuncType::Void_Entity)
								{
									KG_WARN("Incorrect function type returned when linking script to usage point");
									return;
								}

								// Fill the new script handle
								m_EditorState->OnExitStateHandle = scriptHandle;
								m_EditorState->OnExitState = script;
								m_MainHeader.m_EditColorActive = true;
								m_SelectOnExitStateScript.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
						}, {"activeEntity"});

						} };
				m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_SelectScriptTooltip.m_TooltipActive = true;
			};

		// On Message Script
		m_SelectOnAIMessageScript.m_Label = "On AI Message Script";
		m_SelectOnAIMessageScript.m_LineCount = 3;
		m_SelectOnAIMessageScript.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectOnAIMessageScript.m_CurrentOption = { "None", Assets::k_EmptyHandle };
		m_SelectOnAIMessageScript.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.GetAllOptions().clear();

			spec.AddToOptions("Clear", "None", Assets::k_EmptyHandle);
			for (auto& [handle, asset] : Assets::s_ScriptManager.GetAssetRegistry())
			{
			    Assets::AssetRef<Scripting::Script> script = Assets::s_ScriptManager.GetAssetByHandle(handle);
				if (script->m_FuncType != WrappedFuncType::Void_UInt32EntityEntityFloat)
				{
					continue;
				}
				spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}
		};
		m_SelectOnAIMessageScript.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			// If empty option is selected, clear script
			if (selection.m_Handle == Assets::k_EmptyHandle)
			{
				m_EditorState->OnMessageHandle = Assets::k_EmptyHandle;
				m_EditorState->OnMessage = nullptr;
				m_MainHeader.m_EditColorActive = true;
				return;
			}

			// Get Script
		    Assets::AssetRef<Scripting::Script> selectedScript = Assets::s_ScriptManager.GetAssetByHandle(selection.m_Handle);
			KG_ASSERT(selectedScript);

			// Update ai state's script
			m_EditorState->OnMessageHandle = selection.m_Handle;
			m_EditorState->OnMessage = selectedScript;
			m_MainHeader.m_EditColorActive = true;
		};
		m_SelectOnAIMessageScript.m_OnEdit = [&](EditorUI::SelectOptionWidget& spec)
			{
				UNREFERENCED_PARAMETER(spec);
				// Initialize tooltip with options
				m_SelectScriptTooltip.ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					UNREFERENCED_PARAMETER(entry);
					m_SelectOnAIMessageScript.m_OpenPopup = true;
				} };
				m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						UNREFERENCED_PARAMETER(entry);
						// Open create script dialog in script editor
						s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_UInt32EntityEntityFloat, [&](Assets::AssetHandle scriptHandle)
						{
								// Ensure handle provides a script in the registry
								if (!Assets::AssetService::HasScript(scriptHandle))
								{
									KG_WARN("Could not find on message function in ai state panel");
									return;
								}

								// Ensure function type matches definition
							    Assets::AssetRef<Scripting::Script> script = Assets::s_ScriptManager.GetAssetByHandle(scriptHandle);
								if (script->m_FuncType != WrappedFuncType::Void_UInt32EntityEntityFloat)
								{
									KG_WARN("Incorrect function type returned when linking script to usage point");
									return;
								}

								// Fill the new script handle
								m_EditorState->OnMessageHandle = scriptHandle;
								m_EditorState->OnMessage = script;
								m_MainHeader.m_EditColorActive = true;
								m_SelectOnAIMessageScript.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
							}, {"messageType", "senderEntity", "receiverEntity", "delayTime"});

				} };
				m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_SelectScriptTooltip.m_TooltipActive = true;
			};


		m_SelectScriptTooltip.m_Label = "AI Script Tooltip";

	}

}

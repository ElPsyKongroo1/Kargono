#include "Panels/AIStateEditorPanel.h"
#include "EditorApp.h"

namespace Kargono
{
	static EditorApp* s_EditorApp { nullptr };
}

namespace Kargono::Panels
{
	void AIStateEditorPanel::OnOpenAIStateDialog()
	{
		m_OpenAIStatePopupSpec.PopupActive = true;
	}
	void AIStateEditorPanel::OnCreateAIStateDialog()
	{
		KG_ASSERT(Projects::ProjectService::GetActive());
		m_SelectAIStateLocationSpec.CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
		m_CreateAIStatePopupSpec.PopupActive = true;
	}

	void AIStateEditorPanel::OnRefreshData()
	{
		if (m_EditorAIState)
		{
			// Refresh scripts
			Ref<Scripting::Script> onUpdateScript = Assets::AssetService::GetScript(m_EditorAIState->OnUpdateHandle);
			m_SelectOnUpdateScript.CurrentOption = onUpdateScript ? EditorUI::OptionEntry(onUpdateScript->m_ScriptName, m_EditorAIState->OnUpdateHandle) : EditorUI::OptionEntry("None", Assets::EmptyHandle);

			Ref<Scripting::Script> onEnterStateScript = Assets::AssetService::GetScript(m_EditorAIState->OnEnterStateHandle);
			m_SelectOnEnterStateScript.CurrentOption = onEnterStateScript ? EditorUI::OptionEntry(onEnterStateScript->m_ScriptName, m_EditorAIState->OnEnterStateHandle) : EditorUI::OptionEntry("None", Assets::EmptyHandle);

			Ref<Scripting::Script> onExitStateScript = Assets::AssetService::GetScript(m_EditorAIState->OnExitStateHandle);
			m_SelectOnExitStateScript.CurrentOption = onExitStateScript ? EditorUI::OptionEntry(onExitStateScript->m_ScriptName, m_EditorAIState->OnExitStateHandle) : EditorUI::OptionEntry("None", Assets::EmptyHandle);

			Ref<Scripting::Script> onAIMessageScript = Assets::AssetService::GetScript(m_EditorAIState->OnMessageHandle);
			m_SelectOnAIMessageScript.CurrentOption = onAIMessageScript ? EditorUI::OptionEntry(onAIMessageScript->m_ScriptName, m_EditorAIState->OnMessageHandle) : EditorUI::OptionEntry("None", Assets::EmptyHandle);
		}
	}

	void AIStateEditorPanel::OnOpenAIState(Assets::AssetHandle newHandle)
	{
		m_EditorAIState = Assets::AssetService::GetAIState(newHandle);
		m_EditorAIStateHandle = newHandle;
		m_MainHeader.EditColorActive = false;
		m_MainHeader.Label = Assets::AssetService::GetAIStateRegistry().at(
			m_EditorAIStateHandle).Data.FileLocation.filename().string();
		OnRefreshData();
	}

	AIStateEditorPanel::AIStateEditorPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(AIStateEditorPanel::OnKeyPressedEditor));
		InitializeOpeningScreen();
		InitializeAIStateHeader();
		InitializeMainPanel();
	}
	void AIStateEditorPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION()
			EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowAIStateEditor);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		if (!m_EditorAIState)
		{
			// Opening/Null State Screen
			EditorUI::EditorUIService::NewItemScreen("Open Existing AI State", KG_BIND_CLASS_FN(OnOpenAIStateDialog), "Create New AI State", KG_BIND_CLASS_FN(OnCreateAIStateDialog));
			EditorUI::EditorUIService::GenericPopup(m_CreateAIStatePopupSpec);
			EditorUI::EditorUIService::SelectOption(m_OpenAIStatePopupSpec);
		}
		else
		{
			// Header
			EditorUI::EditorUIService::PanelHeader(m_MainHeader);
			EditorUI::EditorUIService::GenericPopup(m_DeleteAIStateWarning);
			EditorUI::EditorUIService::GenericPopup(m_CloseAIStateWarning);

			EditorUI::EditorUIService::SelectOption(m_SelectOnUpdateScript);
			EditorUI::EditorUIService::SelectOption(m_SelectOnEnterStateScript);
			EditorUI::EditorUIService::SelectOption(m_SelectOnExitStateScript);
			EditorUI::EditorUIService::SelectOption(m_SelectOnAIMessageScript);

		}

		EditorUI::EditorUIService::EndWindow();
	}
	bool AIStateEditorPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}

	bool AIStateEditorPanel::OnAssetEvent(Events::Event* event)
	{
		// Validate event type and asset type
		if (event->GetEventType() != Events::EventType::ManageAsset)
		{
			return false;
		}
		Events::ManageAsset* manageAsset = (Events::ManageAsset*)event;
		if (manageAsset->GetAssetType() != Assets::AssetType::AIState)
		{
			return false;
		}

		if (manageAsset->GetAssetID() != m_EditorAIStateHandle)
		{
			return false;
		}

		// Handle deletion of asset
		if (manageAsset->GetAction() == Events::ManageAssetAction::Delete)
		{
			ResetPanelResources();
			return true;
		}

		// Handle updating of asset
		if (manageAsset->GetAction() == Events::ManageAssetAction::UpdateAssetInfo)
		{
			// Update header
			m_MainHeader.Label = Assets::AssetService::GetAIStateFileLocation(manageAsset->GetAssetID()).filename().string();
			return true;
		}
		return false;
	}

	void AIStateEditorPanel::ResetPanelResources()
	{
		m_EditorAIState = nullptr;
		m_EditorAIStateHandle = Assets::EmptyHandle;
	}

	void AIStateEditorPanel::OpenCreateDialog(std::filesystem::path& createLocation)
	{
		// Open AI State Window
		s_EditorApp->m_ShowAIStateEditor = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);

		if (!m_EditorAIState)
		{
			// Open dialog to create editor AI State
			OnCreateAIStateDialog();
			m_SelectAIStateLocationSpec.CurrentOption = createLocation;
		}
		else
		{
			// Add warning to close active AI state before creating a new AIState
			s_EditorApp->OpenWarningMessage("An AI State is already active inside the editor. Please close the current AI State before creating a new one.");
		}

	}

	void AIStateEditorPanel::OpenAssetInEditor(std::filesystem::path& assetLocation)
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
		Assets::AssetHandle assetHandle = Assets::AssetService::GetAIStateHandleFromFileLocation(relativePath);

		// Validate resulting handle
		if (!assetHandle)
		{
			KG_WARN("Could not open asset in editor. Provided path does not result in an asset inside the registry.");
			return;
		}
		
		// Open the editor panel to be visible
		s_EditorApp->m_ShowAIStateEditor = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);

		// Early out if asset is already open
		if (m_EditorAIStateHandle == assetHandle)
		{
			return;
		}

		// Check if panel is already occupied by an asset
		if (!m_EditorAIState)
		{
			// Open dialog to create editor AI State
			OnOpenAIState(assetHandle);
		}
		else
		{
			// Add warning to close active AI state before opening a new AIState
			s_EditorApp->OpenWarningMessage("An AI State is already active inside the editor. Please close the current AI State before opening a new one.");
		}
	}

	void AIStateEditorPanel::InitializeOpeningScreen()
	{
		m_OpenAIStatePopupSpec.Label = "Open AI State";
		m_OpenAIStatePopupSpec.LineCount = 2;
		m_OpenAIStatePopupSpec.CurrentOption = { "None", Assets::EmptyHandle };
		m_OpenAIStatePopupSpec.Flags |= EditorUI::SelectOption_PopupOnly;
		m_OpenAIStatePopupSpec.PopupAction = [&]()
		{
			m_OpenAIStatePopupSpec.GetAllOptions().clear();
			m_OpenAIStatePopupSpec.CurrentOption = { "None", Assets::EmptyHandle };

			m_OpenAIStatePopupSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetAIStateRegistry())
			{
				m_OpenAIStatePopupSpec.AddToOptions("All Options", asset.Data.FileLocation.string(), handle);
			}
		};

		m_OpenAIStatePopupSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.Handle == Assets::EmptyHandle)
			{
				KG_WARN("No AI State Selected");
				return;
			}
			if (!Assets::AssetService::GetAIStateRegistry().contains(selection.Handle))
			{
				KG_WARN("Could not find the AI State specified");
				return;
			}

			OnOpenAIState(selection.Handle);
		};

		m_SelectAIStateNameSpec.Label = "New Name";
		m_SelectAIStateNameSpec.CurrentOption = "Empty";

		m_SelectAIStateLocationSpec.Label = "Location";
		m_SelectAIStateLocationSpec.CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
		m_SelectAIStateLocationSpec.ConfirmAction = [&](const std::string& path) 
		{
			if (!Utility::FileSystem::DoesPathContainSubPath(Projects::ProjectService::GetActiveAssetDirectory(), path))
			{
				KG_WARN("Cannot create an asset outside of the project's asset directory.");
				m_SelectAIStateLocationSpec.CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
			}
		};

		m_CreateAIStatePopupSpec.Label = "Create AI State";
		m_CreateAIStatePopupSpec.ConfirmAction = [&]()
		{
			if (m_SelectAIStateNameSpec.CurrentOption == "")
			{
				return;
			}

			m_EditorAIStateHandle = Assets::AssetService::CreateAIState(m_SelectAIStateNameSpec.CurrentOption.c_str(), m_SelectAIStateLocationSpec.CurrentOption);
			if (m_EditorAIStateHandle == Assets::EmptyHandle)
			{
				KG_WARN("AI state was not created");
				return;
			}
			m_EditorAIState = Assets::AssetService::GetAIState(m_EditorAIStateHandle);
			m_MainHeader.EditColorActive = false;
			m_MainHeader.Label = Assets::AssetService::GetAIStateRegistry().at(
				m_EditorAIStateHandle).Data.FileLocation.filename().string();
			OnRefreshData();
		};
		m_CreateAIStatePopupSpec.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_SelectAIStateNameSpec);
			EditorUI::EditorUIService::ChooseDirectory(m_SelectAIStateLocationSpec);
		};
	}

	void AIStateEditorPanel::InitializeAIStateHeader()
	{
		// Header (Game State Name and Options)
		m_DeleteAIStateWarning.Label = "Delete AI State";
		m_DeleteAIStateWarning.ConfirmAction = [&]()
		{
			Assets::AssetService::DeleteAIState(m_EditorAIStateHandle);
			m_EditorAIStateHandle = 0;
			m_EditorAIState = nullptr;
		};
		m_DeleteAIStateWarning.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to delete this AI state object?");
		};

		m_CloseAIStateWarning.Label = "Close AI State";
		m_CloseAIStateWarning.ConfirmAction = [&]()
		{
			m_EditorAIStateHandle = 0;
			m_EditorAIState = nullptr;
		};
		m_CloseAIStateWarning.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to close this ai state object without saving?");
		};

		m_MainHeader.AddToSelectionList("Save", [&]()
			{
				Assets::AssetService::SaveAIState(m_EditorAIStateHandle, m_EditorAIState);
				m_MainHeader.EditColorActive = false;
			});
		m_MainHeader.AddToSelectionList("Close", [&]()
			{
				if (m_MainHeader.EditColorActive)
				{
					m_CloseAIStateWarning.PopupActive = true;
				}
				else
				{
					m_EditorAIStateHandle = 0;
					m_EditorAIState = nullptr;
				}
			});
		m_MainHeader.AddToSelectionList("Delete", [&]()
			{
				m_DeleteAIStateWarning.PopupActive = true;
			});
	}

	void AIStateEditorPanel::InitializeMainPanel()
	{
		// On Update Script
		m_SelectOnUpdateScript.Label = "On Update Script";
		m_SelectOnUpdateScript.LineCount = 3;
		m_SelectOnUpdateScript.CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectOnUpdateScript.PopupAction = [&]()
		{
			m_SelectOnUpdateScript.GetAllOptions().clear();

			m_SelectOnUpdateScript.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				if (script->m_FuncType != WrappedFuncType::Void_UInt64Float)
				{
					continue;
				}
				m_SelectOnUpdateScript.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}
		};
		m_SelectOnUpdateScript.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			// If empty option is selected, clear script
			if (selection.Handle == Assets::EmptyHandle)
			{
				m_EditorAIState->OnUpdateHandle = Assets::EmptyHandle;
				m_EditorAIState->OnUpdate = nullptr;
				m_MainHeader.EditColorActive = true;
				return;
			}

			// Get Script
			Ref<Scripting::Script> selectedScript = Assets::AssetService::GetScript(selection.Handle);
			KG_ASSERT(selectedScript);

			// Update ai state's script
			m_EditorAIState->OnUpdateHandle = selection.Handle;
			m_EditorAIState->OnUpdate = selectedScript;
			m_MainHeader.EditColorActive = true;
		};

		// On Enter State Script
		m_SelectOnEnterStateScript.Label = "On Enter State Script";
		m_SelectOnEnterStateScript.LineCount = 3;
		m_SelectOnEnterStateScript.CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectOnEnterStateScript.PopupAction = [&]()
		{
			m_SelectOnEnterStateScript.GetAllOptions().clear();

			m_SelectOnEnterStateScript.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				if (script->m_FuncType != WrappedFuncType::Void_UInt64Float)
				{
					continue;
				}
				m_SelectOnEnterStateScript.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}
		};
		m_SelectOnEnterStateScript.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			// If empty option is selected, clear script
			if (selection.Handle == Assets::EmptyHandle)
			{
				m_EditorAIState->OnEnterStateHandle = Assets::EmptyHandle;
				m_EditorAIState->OnEnterState = nullptr;
				m_MainHeader.EditColorActive = true;
				return;
			}

			// Get Script
			Ref<Scripting::Script> selectedScript = Assets::AssetService::GetScript(selection.Handle);
			KG_ASSERT(selectedScript);

			// Update ai state's script
			m_EditorAIState->OnEnterStateHandle = selection.Handle;
			m_EditorAIState->OnEnterState = selectedScript;
			m_MainHeader.EditColorActive = true;
		};

		// On Exit State Script
		m_SelectOnExitStateScript.Label = "On Exit State Script";
		m_SelectOnExitStateScript.LineCount = 3;
		m_SelectOnExitStateScript.CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectOnExitStateScript.PopupAction = [&]()
		{
			m_SelectOnExitStateScript.GetAllOptions().clear();

			m_SelectOnExitStateScript.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				if (script->m_FuncType != WrappedFuncType::Void_UInt64Float)
				{
					continue;
				}
				m_SelectOnExitStateScript.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}
		};
		m_SelectOnExitStateScript.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			// If empty option is selected, clear script
			if (selection.Handle == Assets::EmptyHandle)
			{
				m_EditorAIState->OnExitStateHandle = Assets::EmptyHandle;
				m_EditorAIState->OnExitState = nullptr;
				m_MainHeader.EditColorActive = true;
				return;
			}

			// Get Script
			Ref<Scripting::Script> selectedScript = Assets::AssetService::GetScript(selection.Handle);
			KG_ASSERT(selectedScript);

			// Update ai state's script
			m_EditorAIState->OnExitStateHandle = selection.Handle;
			m_EditorAIState->OnExitState = selectedScript;
			m_MainHeader.EditColorActive = true;
		};

		// On Message Script
		m_SelectOnAIMessageScript.Label = "On AI Message Script";
		m_SelectOnAIMessageScript.LineCount = 3;
		m_SelectOnAIMessageScript.CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectOnAIMessageScript.PopupAction = [&]()
		{
			m_SelectOnAIMessageScript.GetAllOptions().clear();

			m_SelectOnAIMessageScript.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				if (script->m_FuncType != WrappedFuncType::Void_UInt32UInt64UInt64Float)
				{
					continue;
				}
				m_SelectOnAIMessageScript.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}
		};
		m_SelectOnAIMessageScript.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			// If empty option is selected, clear script
			if (selection.Handle == Assets::EmptyHandle)
			{
				m_EditorAIState->OnMessageHandle = Assets::EmptyHandle;
				m_EditorAIState->OnMessage = nullptr;
				m_MainHeader.EditColorActive = true;
				return;
			}

			// Get Script
			Ref<Scripting::Script> selectedScript = Assets::AssetService::GetScript(selection.Handle);
			KG_ASSERT(selectedScript);

			// Update ai state's script
			m_EditorAIState->OnMessageHandle = selection.Handle;
			m_EditorAIState->OnMessage = selectedScript;
			m_MainHeader.EditColorActive = true;
		};

	}

}

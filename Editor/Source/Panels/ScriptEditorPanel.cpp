#include "Panels/ScriptEditorPanel.h"

#include "EditorApp.h"

#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{
	void ScriptEditorPanel::UpdateScript ()
	{
		Ref<Scripting::Script> script = Assets::AssetService::GetScript(m_ActiveScriptHandle);
		if (!script)
		{
			KG_WARN("No script pointer available from asset manager when editing script in script editor");
			return;
		}
		Scripting::ScriptType originalType = script->m_ScriptType;
		std::string originalLabel = script->m_SectionLabel;
		Assets::ScriptSpec spec {};
		spec.Name = m_EditScriptName.CurrentOption;
		spec.Type = Scripting::ScriptType::Project;
		spec.m_SectionLabel = m_EditScriptSectionLabel.CurrentOption.Label;
		spec.m_FunctionType = (WrappedFuncType)(uint64_t)m_EditScriptFuncType.CurrentOption.Handle;
		auto successful = Assets::AssetService::SaveScript(m_ActiveScriptHandle, spec);
		if (!successful)
		{
			KG_ERROR("Unsuccessful at creating new script");
		}

		m_AllScriptsList.OnRefresh();
	}
	void ScriptEditorPanel::OnOpenScriptDialog(EditorUI::ListEntry& entry, std::size_t iteration)
	{
		m_ActiveScriptHandle = entry.Handle;
		m_EditScriptPopup.OpenPopup = true;
	}
	void ScriptEditorPanel::OnCreateScriptDialog()
	{
		m_OnCreateScriptConfirm = nullptr;
		m_CreateWidgets.m_MainPopup.OpenPopup = true;
	}

	void ScriptEditorPanel::InitializeScriptPanel()
	{
		m_AllScriptsList.Label = "All Scripts";
		m_AllScriptsList.Column1Title = "Group";
		m_AllScriptsList.Column2Title = "Name";
		m_AllScriptsList.Expanded = true;
		m_AllScriptsList.AddToSelectionList("Create New Script", KG_BIND_CLASS_FN(OnCreateScriptDialog));
		m_AllScriptsList.OnRefresh = [&]()
		{
			m_AllScriptsList.ClearList();
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_ScriptType == Scripting::ScriptType::Engine)
				{
					continue;
				}
				std::string scriptType = "Project";
				std::string label = scriptType + std::string("::") + script->m_SectionLabel;
				auto onEdit = KG_BIND_CLASS_FN(OnOpenScriptDialog);
#if 0
				auto onLink = [&](EditorUI::ListEntry& entry)
				{
					if (!Assets::AssetService::GetScriptRegistry().contains(entry.Handle))
					{
						KG_WARN("Unable to open script in text editor. Script does not exist in registry");
						return;
					}
					Assets::AssetInfo& asset = Assets::AssetService::GetScriptRegistry().at(entry.Handle);
					s_EditorApp->m_TextEditorPanel->OpenFile(Projects::ProjectService::GetActiveAssetDirectory() / asset.Data.FileLocation);
				};
#endif
				EditorUI::ListEntry newEntry
				{
					label,
						script->m_ScriptName,
						handle,
						onEdit
				};
				m_AllScriptsList.InsertListEntry(newEntry);
			}

			s_EditorApp->m_TextEditorPanel->RefreshKGScriptEditor();
		};
		m_AllScriptsList.OnRefresh();

		m_ScriptTooltip.m_Label = "Script Tooltip";

		m_CreateWidgets.m_MainPopup.Label = "Create New Script";
		m_CreateWidgets.m_MainPopup.PopupAction = [&]()
		{
			m_CreateWidgets.m_EditName.CurrentOption = "Empty";
			
			if (m_OnCreateScriptConfirm)
			{
				m_CreateWidgets.m_SelectFuncType.CurrentOption = { Utility::WrappedFuncTypeToString(m_OnCreateFunctionType), (uint64_t)m_OnCreateFunctionType };
			}
			else
			{
				m_CreateWidgets.m_SelectFuncType.CurrentOption = { Utility::WrappedFuncTypeToString(WrappedFuncType::None), (uint64_t)WrappedFuncType::None };
			}
			m_CreateWidgets.m_SelectSectionLabel.CurrentOption = { "None", Assets::EmptyHandle };
			m_CreateWidgets.m_SelectReturnType.CurrentOption = { Utility::WrappedVarTypeToString(WrappedVarType::Void), (uint64_t)WrappedVarType::Void };
		};
		m_CreateWidgets.m_MainPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_CreateWidgets.m_EditName);
			EditorUI::EditorUIService::SelectOption(m_CreateWidgets.m_SelectFuncType);
			EditorUI::EditorUIService::SelectOption(m_CreateWidgets.m_SelectReturnType);
			EditorUI::EditorUIService::List(m_CreateWidgets.m_ParameterList);
			EditorUI::EditorUIService::SelectOption(m_CreateWidgets.m_SelectSectionLabel);
			EditorUI::EditorUIService::GenericPopup(m_CreateWidgets.m_CreateParameterPopup);
			EditorUI::EditorUIService::GenericPopup(m_CreateWidgets.m_EditParameterPopup);
			EditorUI::EditorUIService::Tooltip(m_ScriptTooltip);
		};

		m_CreateWidgets.m_MainPopup.ConfirmAction = [&]()
		{
			Assets::ScriptSpec spec {};
			spec.Name = m_CreateWidgets.m_EditName.CurrentOption;
			spec.Type = Scripting::ScriptType::Project;
			spec.m_SectionLabel = m_CreateWidgets.m_SelectSectionLabel.CurrentOption.Label;
			spec.m_FunctionType = (WrappedFuncType)(uint64_t)m_CreateWidgets.m_SelectFuncType.CurrentOption.Handle;
			auto [handle, successful] = Assets::AssetService::CreateNewScript(spec);
			if (!successful)
			{
				KG_ERROR("Unsuccessful at creating new script");
				m_OnCreateScriptConfirm = nullptr;
			}

			if (m_OnCreateScriptConfirm)
			{
				m_OnCreateScriptConfirm(handle);
			}
			m_OnCreateScriptConfirm = nullptr;

			m_AllScriptsList.OnRefresh();
		};

		m_CreateWidgets.m_EditName.Label = "Name";
		m_CreateWidgets.m_EditName.CurrentOption = "Empty";

		m_CreateWidgets.m_SelectFuncType.Label = "Function Type";
		m_CreateWidgets.m_SelectFuncType.CurrentOption.Label = Utility::WrappedFuncTypeToString(WrappedFuncType::None);
		m_CreateWidgets.m_SelectFuncType.PopupAction = [&]()
		{
			
			m_CreateWidgets.m_SelectFuncType.ClearOptions();
			for (WrappedFuncType func : s_AllWrappedFuncs)
			{
				m_CreateWidgets.m_SelectFuncType.AddToOptions("All Options", Utility::WrappedFuncTypeToString(func), (uint64_t)func);
			}
		};

		m_CreateWidgets.m_SelectReturnType.Label = "Return Type";
		m_CreateWidgets.m_SelectReturnType.CurrentOption = { Utility::WrappedVarTypeToString(WrappedVarType::None), Assets::EmptyHandle};
		m_CreateWidgets.m_SelectReturnType.PopupAction = [&]() 
		{
			m_CreateWidgets.m_SelectReturnType.ClearOptions();

			// Add all possible return types
			for (WrappedVarType type: s_AllWrappedVarTypes)
			{
				m_CreateWidgets.m_SelectReturnType.AddToOptions("All Options", Utility::WrappedVarTypeToString(type), (uint64_t)type);
			}

		};

		m_CreateWidgets.m_ParameterList.Label = "Parameters";
		m_CreateWidgets.m_ParameterList.Flags |= EditorUI::List_RegularSizeTitle;
		m_CreateWidgets.m_ParameterList.Column1Title = "Name";
		m_CreateWidgets.m_ParameterList.Column2Title = "Type";
		m_CreateWidgets.m_ParameterList.AddToSelectionList("Add Parameter", [&]() 
		{
			m_CreateWidgets.m_CreateParameterPopup.OpenPopup = true;
		});

		m_CreateWidgets.m_CreateParameterPopup.Label = "Create Parameter";
		m_CreateWidgets.m_CreateParameterPopup.PopupAction = [&]() 
		{
			m_CreateWidgets.m_CreateParameterName.CurrentOption = "NewParameter";
			m_CreateWidgets.m_CreateParameterType.CurrentOption = { Utility::WrappedVarTypeToString(WrappedVarType::Float), (uint64_t)WrappedVarType::Float };
		};

		m_CreateWidgets.m_CreateParameterPopup.PopupContents = [&]() 
		{
			EditorUI::EditorUIService::EditText(m_CreateWidgets.m_CreateParameterName);
			EditorUI::EditorUIService::SelectOption(m_CreateWidgets.m_CreateParameterType);
		};
		m_CreateWidgets.m_CreateParameterPopup.ConfirmAction = [&]() 
		{
			// Fill new parameter entry with appropriate data from popup dialog
			EditorUI::ListEntry newEntry{};
			newEntry.Label = m_CreateWidgets.m_CreateParameterName.CurrentOption;
			newEntry.Value = Utility::WrappedVarTypeToString((WrappedVarType)(uint64_t)m_CreateWidgets.m_CreateParameterType.CurrentOption.Handle);
			newEntry.Handle = m_CreateWidgets.m_CreateParameterType.CurrentOption.Handle;
			newEntry.OnEdit = [&](EditorUI::ListEntry& entry, std::size_t iteration) 
			{
				// Open tooltip
				m_ScriptTooltip.TooltipActive = true;

				// Initialize tooltip entries
				m_ScriptTooltip.ClearEntries();
				EditorUI::TooltipEntry editTooltipEntry{"Edit", [&](EditorUI::TooltipEntry& entry)
				{
					m_CreateWidgets.m_EditParameterPopup.OpenPopup = true;
				}};
				m_ScriptTooltip.AddTooltipEntry(editTooltipEntry);


				EditorUI::TooltipEntry deleteTooltipEntry{ "Delete", [&](EditorUI::TooltipEntry& entry)
				{
					// Delete current selected entry
					EngineService::SubmitToMainThread([&]() 
					{
						m_CreateWidgets.m_ParameterList.RemoveEntry(m_ActiveParameterLocation);
					});

				} };
				m_ScriptTooltip.AddTooltipEntry(deleteTooltipEntry);

				// Store parameter location inside list
				m_ActiveParameterLocation = iteration;
			};
			// Add parameter to parameter list
			m_CreateWidgets.m_ParameterList.InsertListEntry(newEntry);
		};

		m_CreateWidgets.m_CreateParameterName.Label = "Name";

		m_CreateWidgets.m_CreateParameterType.Label = "Type";
		m_CreateWidgets.m_CreateParameterType.PopupAction = [&]()
		{
			m_CreateWidgets.m_CreateParameterType.ClearOptions();

			// Add all possible return types
			for (WrappedVarType type : s_AllWrappedVarTypes)
			{
				// Void makes no sense as a parameter
				if (type == WrappedVarType::Void)
				{
					continue;
				}

				// Fill options list
				m_CreateWidgets.m_CreateParameterType.AddToOptions("All Options", Utility::WrappedVarTypeToString(type), (uint64_t)type);
			}

		};

		m_CreateWidgets.m_EditParameterPopup.Label = "Edit Parameter";
		m_CreateWidgets.m_EditParameterPopup.PopupAction = [&]()
		{
			KG_ASSERT(m_ActiveParameterLocation < m_CreateWidgets.m_ParameterList.GetEntriesListSize());
			EditorUI::ListEntry& entry = m_CreateWidgets.m_ParameterList.GetEntry(m_ActiveParameterLocation);

			m_CreateWidgets.m_EditParameterName.CurrentOption = entry.Label;
			m_CreateWidgets.m_EditParameterType.CurrentOption = { Utility::WrappedVarTypeToString((WrappedVarType)(uint64_t)entry.Handle), (uint64_t)entry.Handle };
		};

		m_CreateWidgets.m_EditParameterPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_CreateWidgets.m_EditParameterName);
			EditorUI::EditorUIService::SelectOption(m_CreateWidgets.m_EditParameterType);
		};
		m_CreateWidgets.m_EditParameterPopup.ConfirmAction = [&]()
		{
			// Fill new parameter entry with appropriate data from popup dialog
			KG_ASSERT(m_ActiveParameterLocation < m_CreateWidgets.m_ParameterList.GetEntriesListSize());
			EditorUI::ListEntry& currentEntry = m_CreateWidgets.m_ParameterList.GetEntry(m_ActiveParameterLocation);
			currentEntry.Label = m_CreateWidgets.m_EditParameterName.CurrentOption;
			currentEntry.Value = Utility::WrappedVarTypeToString((WrappedVarType)(uint64_t)m_CreateWidgets.m_EditParameterType.CurrentOption.Handle);
			currentEntry.Handle = m_CreateWidgets.m_EditParameterType.CurrentOption.Handle;
			currentEntry.OnEdit = [&](EditorUI::ListEntry& entry, std::size_t iteration)
			{
				// Open tooltip
				m_ScriptTooltip.TooltipActive = true;

				// Initialize tooltip entries
				m_ScriptTooltip.ClearEntries();
				EditorUI::TooltipEntry editTooltipEntry{ "Edit", [&](EditorUI::TooltipEntry& entry)
				{
					m_CreateWidgets.m_EditParameterPopup.OpenPopup = true;
				} };
				m_ScriptTooltip.AddTooltipEntry(editTooltipEntry);
				EditorUI::TooltipEntry deleteTooltipEntry{ "Delete", [&](EditorUI::TooltipEntry& entry)
				{
						// Delete current selected entry
						EngineService::SubmitToMainThread([&]()
						{
							m_CreateWidgets.m_ParameterList.RemoveEntry(m_ActiveParameterLocation);
						});

					} };
				m_ScriptTooltip.AddTooltipEntry(deleteTooltipEntry);

				// Store parameter location inside list
				m_ActiveParameterLocation = iteration;
			};
		};

		m_CreateWidgets.m_EditParameterName.Label = "Name";
		m_CreateWidgets.m_EditParameterType.Label = "Type";
		m_CreateWidgets.m_EditParameterType.PopupAction = [&]()
		{
			m_CreateWidgets.m_EditParameterType.ClearOptions();

			// Add all possible return types
			for (WrappedVarType type : s_AllWrappedVarTypes)
			{
				// Void makes no sense as a parameter
				if (type == WrappedVarType::Void)
				{
					continue;
				}

				// Fill options list
				m_CreateWidgets.m_EditParameterType.AddToOptions("All Options", Utility::WrappedVarTypeToString(type), (uint64_t)type);
			}

		};

		m_CreateWidgets.m_SelectSectionLabel.Label = "Group";
		m_CreateWidgets.m_SelectSectionLabel.CurrentOption = { "None", Assets::EmptyHandle };
		m_CreateWidgets.m_SelectSectionLabel.PopupAction = [&]()
		{
			m_CreateWidgets.m_SelectSectionLabel.ClearOptions();
			m_CreateWidgets.m_SelectSectionLabel.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& label : Assets::AssetService::GetScriptSectionLabels())
			{
				m_CreateWidgets.m_SelectSectionLabel.AddToOptions("All Project Groups", label, Assets::EmptyHandle);
			}
		};

		m_EditScriptPopup.Label = "Edit New Script";
		m_EditScriptPopup.PopupAction = [&]()
		{
			m_EditScriptName.CurrentOption = Assets::AssetService::GetScript(m_ActiveScriptHandle)->m_ScriptName;
			m_EditScriptSectionLabel.CurrentOption.Label = Assets::AssetService::GetScript(m_ActiveScriptHandle)->m_SectionLabel;
			WrappedFuncType currentFuncType = Assets::AssetService::GetScript(m_ActiveScriptHandle)->m_FuncType;
			m_EditScriptFuncType.CurrentOption = { Utility::WrappedFuncTypeToString(currentFuncType),
				(uint64_t)currentFuncType };
			
		};
		m_EditScriptPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::LabeledText("Script Name", Assets::AssetService::GetScript(m_ActiveScriptHandle)->m_ScriptName);
			//EditorUI::Editor::EditText(m_EditScriptName);
			EditorUI::EditorUIService::SelectOption(m_EditScriptFuncType);
			EditorUI::EditorUIService::SelectOption(m_EditScriptSectionLabel);
		};
		m_EditScriptPopup.DeleteAction = [&]()
		{
			m_DeleteScriptWarning.OpenPopup = true;
		};
		m_EditScriptPopup.ConfirmAction = [&]()
		{
			if (Utility::StringToWrappedFuncType(m_EditScriptFuncType.CurrentOption.Label) !=
				Assets::AssetService::GetScript(m_ActiveScriptHandle)->m_FuncType)
			{
				m_EditScriptFuncTypeWarning.OpenPopup = true;
			}
			else
			{
				UpdateScript();
			}
		};

		m_DeleteScriptWarning.Label = "Delete Script";
		m_DeleteScriptWarning.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to delete this script?");
		};
		m_DeleteScriptWarning.ConfirmAction = [&]()
		{
			Ref<Scripting::Script> script = Assets::AssetService::GetScript(m_ActiveScriptHandle);
			Scripting::ScriptType type = script->m_ScriptType;
			std::string sectionLabel = script->m_SectionLabel;

			bool success = Assets::AssetService::DeleteScript(m_ActiveScriptHandle);
			if (!success)
			{
				KG_WARN("Unable to delete script!");
				return;
			}

			m_AllScriptsList.OnRefresh();
		};

		m_EditScriptFuncTypeWarning.Label = "Edit Script";
		m_EditScriptFuncTypeWarning.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Changing the function type can cause the existing function code to not compile.");
			EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);
			EditorUI::EditorUIService::Text("Are you sure you want to modify the function type?");
		};
		m_EditScriptFuncTypeWarning.ConfirmAction = [&]()
		{
			UpdateScript();
		};

		m_EditScriptName.Label = "Name";
		m_EditScriptName.CurrentOption = "Empty";

		m_EditScriptFuncType.Label = "Function Type";
		m_EditScriptFuncType.CurrentOption.Label = Utility::WrappedFuncTypeToString(WrappedFuncType::None);
		m_EditScriptFuncType.PopupAction = [&]()
		{
			m_EditScriptFuncType.ClearOptions();
			for (auto func : s_AllWrappedFuncs)
			{
				m_EditScriptFuncType.AddToOptions("All Options", Utility::WrappedFuncTypeToString(func), (uint64_t)func);
			}
			
		};

		m_EditScriptSectionLabel.Label = "Group";
		m_EditScriptSectionLabel.CurrentOption = { "None", Assets::EmptyHandle };
		m_EditScriptSectionLabel.PopupAction = [&]()
		{
			m_EditScriptSectionLabel.ClearOptions();
			m_EditScriptSectionLabel.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& label : Assets::AssetService::GetScriptSectionLabels())
			{
				m_EditScriptSectionLabel.AddToOptions("All Project Groups", label, Assets::EmptyHandle);
			}
			
		};

		// Group Labels
		m_GroupLabelsTable.Label = "All Group Labels";
		m_GroupLabelsTable.Column1Title = "Label";
		m_GroupLabelsTable.Column2Title = "";
		m_GroupLabelsTable.Expanded = false;
		m_GroupLabelsTable.AddToSelectionList("Create New Group Label", [&]()
			{
				m_CreateGroupLabelPopup.StartPopup = true;
			});
		m_GroupLabelsTable.OnRefresh = [&]()
		{
			m_GroupLabelsTable.ClearList();
			for (auto& label : Assets::AssetService::GetScriptSectionLabels())
			{
				m_GroupLabelsTable.InsertListEntry(label, "", [&](EditorUI::ListEntry& entry, std::size_t iteration)
				{
					m_ActiveLabel = entry.Label;
					m_EditGroupLabelPopup.OpenPopup = true;
				});
			}
		};
		m_GroupLabelsTable.OnRefresh();

		m_CreateGroupLabelPopup.Label = "Create New Group Label";
		m_CreateGroupLabelPopup.Flags |= EditorUI::EditText_PopupOnly;
		m_CreateGroupLabelPopup.ConfirmAction = [&](EditorUI::EditTextSpec& spec)
		{
			// Create new group label
			bool success = Assets::AssetService::AddScriptSectionLabel(m_CreateGroupLabelPopup.CurrentOption);
			if (!success)
			{
				KG_WARN("Failed to create group label");
				return;
			}
			m_GroupLabelsTable.OnRefresh();
		};

		m_EditGroupLabelPopup.Label = "Edit Group Label";
		m_EditGroupLabelPopup.PopupAction = [&]()
		{
			m_EditGroupLabelText.CurrentOption = m_ActiveLabel;
		};
		m_EditGroupLabelPopup.ConfirmAction = [&]()
		{
			// Create new group label
			bool success = Assets::AssetService::EditScriptSectionLabel(
				m_ActiveLabel, m_EditGroupLabelText.CurrentOption);
			if (!success)
			{
				KG_WARN("Failed to edit group label");
				return;
			}
			m_GroupLabelsTable.OnRefresh();
			m_AllScriptsList.OnRefresh();
		};
		m_EditGroupLabelPopup.DeleteAction = [&]()
		{
			bool success = Assets::AssetService::DeleteScriptSectionLabel(m_ActiveLabel);
			if (!success)
			{
				KG_WARN("Failed to delete section label");
				return;
			}

			m_AllScriptsList.OnRefresh();
			m_GroupLabelsTable.OnRefresh();
		};
		m_EditGroupLabelPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_EditGroupLabelText);
		};

		m_EditGroupLabelText.Label = "Group Label";
		m_EditGroupLabelText.CurrentOption = "Empty";
	}

	ScriptEditorPanel::ScriptEditorPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(ScriptEditorPanel::OnKeyPressedEditor));

		InitializeScriptPanel();

	}
	void ScriptEditorPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowScriptEditor);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		EditorUI::EditorUIService::List(m_AllScriptsList);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		EditorUI::EditorUIService::List(m_GroupLabelsTable);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Popups
		EditorUI::EditorUIService::GenericPopup(m_CreateWidgets.m_MainPopup);
		EditorUI::EditorUIService::GenericPopup(m_EditScriptPopup);
		EditorUI::EditorUIService::GenericPopup(m_DeleteScriptWarning);
		EditorUI::EditorUIService::GenericPopup(m_EditScriptFuncTypeWarning);
		EditorUI::EditorUIService::EditText(m_CreateGroupLabelPopup);
		EditorUI::EditorUIService::GenericPopup(m_EditGroupLabelPopup);

		EditorUI::EditorUIService::EndWindow();
	}
	bool ScriptEditorPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}

	bool ScriptEditorPanel::OnAssetEvent(Events::Event* event)
	{
		Events::ManageAsset& manageEvent = *(Events::ManageAsset*)event;

		// Validate asset is a script
		if (manageEvent.GetAssetType() != Assets::AssetType::Script)
		{
			return false;
		}

		// Handle deletion event
		if (manageEvent.GetAction() == Events::ManageAssetAction::Delete)
		{
			// Search table for deleted asset
			std::size_t assetLocation = m_AllScriptsList.SearchEntries([&](const EditorUI::ListEntry& currentEntry)
			{
				// Check if handle matches
				if (currentEntry.Handle != manageEvent.GetAssetID())
				{
					return false;
				}

				// Index has been found
				return true;

			});

			// Validate that table search was successful
			KG_ASSERT(assetLocation != EditorUI::k_ListSearchIndex, "Asset being deleted was not found in asset table");

			// Delete entry and validate deletion
			bool deletionSuccess = m_AllScriptsList.RemoveEntry(assetLocation);
			KG_ASSERT(deletionSuccess, "Unable to delete asset inside script editor panel");
		}

		// Handle all othe event types
		else
		{
			ResetPanelResources();
		}

		return true;
	}
	void ScriptEditorPanel::ResetPanelResources()
	{
		m_AllScriptsList.OnRefresh();
		m_GroupLabelsTable.OnRefresh();
	}
	void ScriptEditorPanel::OpenCreateScriptDialogFromUsagePoint(WrappedFuncType scriptType, std::function<void(Assets::AssetHandle)> onConfirm, bool openScriptEditor)
	{
		if (openScriptEditor)
		{
			// Open the editor panel to be visible
			s_EditorApp->m_ShowScriptEditor = true;
			EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
			EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);
		}
		
		// Open the create dialog
		OnCreateScriptDialog();

		// Store onConfirm and indication that onConfirm should be used
		m_OnCreateScriptConfirm = onConfirm;
		m_OnCreateFunctionType = scriptType;
	}
	void ScriptEditorPanel::DrawOnCreatePopup()
	{
		EditorUI::EditorUIService::GenericPopup(m_CreateWidgets.m_MainPopup);
	}
}

#include "Panels/ScriptEditorPanel.h"

#include "EditorApp.h"

#include "Kargono.h"

#include <unordered_set>

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
		// TODO: Ensure works with new system
		Assets::ScriptSpec spec {};
		spec.Name = m_EditWidgets.m_EditName.CurrentOption;
		spec.Type = Scripting::ScriptType::Project;
		spec.m_SectionLabel = m_EditWidgets.m_SelectSectionLabel.CurrentOption.m_Label;

		auto successful = Assets::AssetService::SaveScript(m_ActiveScriptHandle, spec);
		if (!successful)
		{
			KG_ERROR("Unsuccessful at updating script");
		}

		m_AllScriptsList.OnRefresh();
	}
	void ScriptEditorPanel::OnOpenScriptDialog(EditorUI::ListEntry& entry, std::size_t iteration)
	{
		m_ActiveScriptHandle = entry.m_Handle;
		m_EditWidgets.m_MainPopup.m_OpenPopup = true;
	}
	void ScriptEditorPanel::OnCreateScriptDialog()
	{
		m_OnCreateScriptConfirm = nullptr;
		m_CreateWidgets.m_MainPopup.m_OpenPopup = true;
	}

	void ScriptEditorPanel::OnCreateScriptEditParameter(EditorUI::ListEntry& entry, std::size_t iteration)
	{
		// Open tooltip
		m_ScriptTooltip.TooltipActive = true;

		// Initialize tooltip entries
		m_ScriptTooltip.ClearEntries();
		EditorUI::TooltipEntry editTooltipEntry{ "Edit", [&](EditorUI::TooltipEntry& entry)
		{
			m_CreateWidgets.m_EditParameterPopup.m_OpenPopup = true;
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
	}

	void ScriptEditorPanel::InitializeAllScriptsList()
	{
		m_AllScriptsList.m_Label = "All Scripts";
		m_AllScriptsList.Column1Title = "Group";
		m_AllScriptsList.Column2Title = "Name";
		m_AllScriptsList.m_Expanded = true;
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
	}

	void ScriptEditorPanel::InitializeCreateScriptPopup()
	{
		m_CreateWidgets.m_MainPopup.m_Label = "Create New Script";
		m_CreateWidgets.m_MainPopup.m_PopupAction = [&]()
		{
			m_CreateWidgets.m_EditName.CurrentOption = "Empty";

			if (m_OnCreateScriptConfirm)
			{
				WrappedVarType currentReturnType = Utility::WrappedFuncTypeToReturnType(m_OnCreateFunctionType);
				std::vector<WrappedVarType> currentParameters = Utility::WrappedFuncTypeToParameterTypes(m_OnCreateFunctionType);

				m_CreateWidgets.m_SelectReturnType.CurrentOption = { Utility::WrappedVarTypeToString(currentReturnType) , (uint64_t)currentReturnType};

				m_CreateWidgets.m_ParameterList.ClearList();
				std::size_t iteration{ 0 };
				for (WrappedVarType type : currentParameters)
				{
					// Fill new parameter entry with appropriate data from popup dialog
					EditorUI::ListEntry newEntry{};
					newEntry.m_Label = m_OnCreateParameterNames.at(iteration).CString();
					newEntry.Value = Utility::WrappedVarTypeToString(type);
					newEntry.m_Handle = (uint64_t)type;
					newEntry.OnEdit = KG_BIND_CLASS_FN(OnCreateScriptEditParameter);

					// Add parameter to parameter list
					m_CreateWidgets.m_ParameterList.InsertListEntry(newEntry);
					iteration++;
				}

			}
			else
			{
				// Clear current return type and parameter list in widgets
				m_CreateWidgets.m_SelectReturnType.CurrentOption = { Utility::WrappedVarTypeToString(WrappedVarType::Void), (uint64_t)WrappedVarType::Void };
				m_CreateWidgets.m_ParameterList.ClearList();
			}
			m_CreateWidgets.m_SelectSectionLabel.CurrentOption = { "None", Assets::EmptyHandle };
			m_CreateWidgets.m_ParameterList.m_Expanded = true;
		};
		m_CreateWidgets.m_MainPopup.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_CreateWidgets.m_EditName);
			EditorUI::EditorUIService::SelectOption(m_CreateWidgets.m_SelectSectionLabel);
			EditorUI::EditorUIService::SelectOption(m_CreateWidgets.m_SelectReturnType);
			EditorUI::EditorUIService::List(m_CreateWidgets.m_ParameterList);
			EditorUI::EditorUIService::GenericPopup(m_CreateWidgets.m_CreateParameterPopup);
			EditorUI::EditorUIService::GenericPopup(m_CreateWidgets.m_EditParameterPopup);
			EditorUI::EditorUIService::Tooltip(m_ScriptTooltip);
		};

		m_CreateWidgets.m_MainPopup.m_ConfirmAction = [&]()
		{
			Assets::ScriptSpec spec{};
			spec.Name = m_CreateWidgets.m_EditName.CurrentOption;
			spec.Type = Scripting::ScriptType::Project;
			spec.m_SectionLabel = m_CreateWidgets.m_SelectSectionLabel.CurrentOption.m_Label;
			
			// Get return type from select widget
			WrappedVarType returnType = (WrappedVarType)(uint64_t)m_CreateWidgets.m_SelectReturnType.CurrentOption.m_Handle;

			// Get vector of parameters from list widget
			std::vector<WrappedVarType> parameterTypeList;
			std::vector<FixedString32> parameterNameList;
			m_CreateWidgets.m_ParameterList.EditEntries([&](const EditorUI::ListEntry& currentEntry) 
			{
				parameterTypeList.emplace_back((WrappedVarType)(uint64_t)currentEntry.m_Handle);
				parameterNameList.emplace_back(currentEntry.m_Label.c_str());
			});

			// Validate that there are no duplicate names
			std::unordered_set<uint32_t> nameValidationSet;
			for (FixedString32& name : parameterNameList)
			{
				auto [iter, success] = nameValidationSet.insert(Utility::FileSystem::CRCFromString(name));
				if (!success)
				{
					KG_WARN("Could not create new script. Duplicate parameter names found.");
					return;
				}
			}

			// Check if provided signature matches a predefined type
			WrappedFuncType matchingFuncType = Utility::ExplicitSignatureToWrappedFuncType(returnType, parameterTypeList);
			if (matchingFuncType == WrappedFuncType::None)
			{
				// Push an arbitrary type with the custom signature
				spec.m_FunctionType = WrappedFuncType::ArbitraryFunction;
				spec.m_ExplicitFuncType.m_ReturnType = returnType;
				spec.m_ExplicitFuncType.m_ParameterNames = parameterNameList;
				spec.m_ExplicitFuncType.m_ParameterTypes = std::move(parameterTypeList);
			}
			else
			{
				// Push indicated function type
				spec.m_FunctionType = matchingFuncType;
				spec.m_ExplicitFuncType.m_ParameterNames = parameterNameList;
			}



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

		m_CreateWidgets.m_EditName.m_Label = "Name";
		m_CreateWidgets.m_EditName.CurrentOption = "Empty";

		m_CreateWidgets.m_SelectReturnType.m_Label = "Return Type";
		m_CreateWidgets.m_SelectReturnType.CurrentOption = { Utility::WrappedVarTypeToString(WrappedVarType::None), Assets::EmptyHandle };
		m_CreateWidgets.m_SelectReturnType.m_PopupAction = [&]()
			{
				m_CreateWidgets.m_SelectReturnType.ClearOptions();

				// Add all possible return types
				for (WrappedVarType type : s_AllWrappedVarTypes)
				{
					m_CreateWidgets.m_SelectReturnType.AddToOptions("All Options", Utility::WrappedVarTypeToString(type), (uint64_t)type);
				}

			};

		m_CreateWidgets.m_ParameterList.m_Label = "Parameters";
		m_CreateWidgets.m_ParameterList.m_Flags |= EditorUI::List_RegularSizeTitle;
		m_CreateWidgets.m_ParameterList.Column1Title = "Name";
		m_CreateWidgets.m_ParameterList.Column2Title = "Type";
		m_CreateWidgets.m_ParameterList.AddToSelectionList("Add Parameter", [&]()
			{
				m_CreateWidgets.m_CreateParameterPopup.m_OpenPopup = true;
			});

		m_CreateWidgets.m_CreateParameterPopup.m_Label = "Create Parameter";
		m_CreateWidgets.m_CreateParameterPopup.m_PopupAction = [&]()
			{
				m_CreateWidgets.m_CreateParameterName.CurrentOption = "NewParameter";
				m_CreateWidgets.m_CreateParameterType.CurrentOption = { Utility::WrappedVarTypeToString(WrappedVarType::Float), (uint64_t)WrappedVarType::Float };
			};

		m_CreateWidgets.m_CreateParameterPopup.m_PopupContents = [&]()
			{
				EditorUI::EditorUIService::EditText(m_CreateWidgets.m_CreateParameterName);
				EditorUI::EditorUIService::SelectOption(m_CreateWidgets.m_CreateParameterType);
			};
		m_CreateWidgets.m_CreateParameterPopup.m_ConfirmAction = [&]()
		{
			// Fill new parameter entry with appropriate data from popup dialog
			EditorUI::ListEntry newEntry{};
			newEntry.m_Label = m_CreateWidgets.m_CreateParameterName.CurrentOption;
			newEntry.Value = Utility::WrappedVarTypeToString((WrappedVarType)(uint64_t)m_CreateWidgets.m_CreateParameterType.CurrentOption.m_Handle);
			newEntry.m_Handle = m_CreateWidgets.m_CreateParameterType.CurrentOption.m_Handle;
			newEntry.OnEdit = KG_BIND_CLASS_FN(OnCreateScriptEditParameter);
				
			// Add parameter to parameter list
			m_CreateWidgets.m_ParameterList.InsertListEntry(newEntry);
		};

		m_CreateWidgets.m_CreateParameterName.m_Label = "Name";

		m_CreateWidgets.m_CreateParameterType.m_Label = "Type";
		m_CreateWidgets.m_CreateParameterType.m_PopupAction = [&]()
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

		m_CreateWidgets.m_EditParameterPopup.m_Label = "Edit Parameter";
		m_CreateWidgets.m_EditParameterPopup.m_PopupAction = [&]()
			{
				KG_ASSERT(m_ActiveParameterLocation < m_CreateWidgets.m_ParameterList.GetEntriesListSize());
				EditorUI::ListEntry& entry = m_CreateWidgets.m_ParameterList.GetEntry(m_ActiveParameterLocation);

				m_CreateWidgets.m_EditParameterName.CurrentOption = entry.m_Label;
				m_CreateWidgets.m_EditParameterType.CurrentOption = { Utility::WrappedVarTypeToString((WrappedVarType)(uint64_t)entry.m_Handle), (uint64_t)entry.m_Handle };
			};

		m_CreateWidgets.m_EditParameterPopup.m_PopupContents = [&]()
			{
				EditorUI::EditorUIService::EditText(m_CreateWidgets.m_EditParameterName);
				EditorUI::EditorUIService::SelectOption(m_CreateWidgets.m_EditParameterType);
			};
		m_CreateWidgets.m_EditParameterPopup.m_ConfirmAction = [&]()
			{
				// Fill new parameter entry with appropriate data from popup dialog
				KG_ASSERT(m_ActiveParameterLocation < m_CreateWidgets.m_ParameterList.GetEntriesListSize());
				EditorUI::ListEntry& currentEntry = m_CreateWidgets.m_ParameterList.GetEntry(m_ActiveParameterLocation);
				currentEntry.m_Label = m_CreateWidgets.m_EditParameterName.CurrentOption;
				currentEntry.Value = Utility::WrappedVarTypeToString((WrappedVarType)(uint64_t)m_CreateWidgets.m_EditParameterType.CurrentOption.m_Handle);
				currentEntry.m_Handle = m_CreateWidgets.m_EditParameterType.CurrentOption.m_Handle;
				currentEntry.OnEdit = [&](EditorUI::ListEntry& entry, std::size_t iteration)
					{
						// Open tooltip
						m_ScriptTooltip.TooltipActive = true;

						// Initialize tooltip entries
						m_ScriptTooltip.ClearEntries();
						EditorUI::TooltipEntry editTooltipEntry{ "Edit", [&](EditorUI::TooltipEntry& entry)
						{
							m_CreateWidgets.m_EditParameterPopup.m_OpenPopup = true;
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

		m_CreateWidgets.m_EditParameterName.m_Label = "Name";
		m_CreateWidgets.m_EditParameterType.m_Label = "Type";
		m_CreateWidgets.m_EditParameterType.m_PopupAction = [&]()
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

		m_CreateWidgets.m_SelectSectionLabel.m_Label = "Group";
		m_CreateWidgets.m_SelectSectionLabel.CurrentOption = { "None", Assets::EmptyHandle };
		m_CreateWidgets.m_SelectSectionLabel.m_PopupAction = [&]()
			{
				m_CreateWidgets.m_SelectSectionLabel.ClearOptions();
				m_CreateWidgets.m_SelectSectionLabel.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& label : Assets::AssetService::GetScriptSectionLabels())
				{
					m_CreateWidgets.m_SelectSectionLabel.AddToOptions("All Project Groups", label, Assets::EmptyHandle);
				}
			};
	}

	void ScriptEditorPanel::InitializeEditScriptPopup()
	{
		m_EditWidgets.m_MainPopup.m_Label = "Edit Script";
		m_EditWidgets.m_MainPopup.m_PopupAction = [&]()
		{
			m_EditWidgets.m_EditName.CurrentOption = Assets::AssetService::GetScript(m_ActiveScriptHandle)->m_ScriptName;
			m_EditWidgets.m_SelectSectionLabel.CurrentOption.m_Label = Assets::AssetService::GetScript(m_ActiveScriptHandle)->m_SectionLabel;
		};
		m_EditWidgets.m_MainPopup.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_EditWidgets.m_EditName);
			EditorUI::EditorUIService::SelectOption(m_EditWidgets.m_SelectSectionLabel);
		};
		m_EditWidgets.m_MainPopup.m_DeleteAction = [&]()
		{
			m_EditWidgets.m_DeleteWarning.m_OpenPopup = true;
		};
		m_EditWidgets.m_MainPopup.m_ConfirmAction = [&]()
		{
			m_EditWidgets.m_EditWarning.m_OpenPopup = true;
		};

		m_EditWidgets.m_DeleteWarning.m_Label = "Delete Script";
		m_EditWidgets.m_DeleteWarning.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to delete this script?");
		};
		m_EditWidgets.m_DeleteWarning.m_ConfirmAction = [&]()
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

		m_EditWidgets.m_EditWarning.m_Label = "Edit Script";
		m_EditWidgets.m_EditWarning.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Modifying the function's information can cause compilation issues in your project. Are you sure you want to modify the function?");
		};
		m_EditWidgets.m_EditWarning.m_ConfirmAction = [&]()
		{
			UpdateScript();
		};

		m_EditWidgets.m_EditName.m_Label = "Name";
		m_EditWidgets.m_EditName.CurrentOption = "Empty";

		m_EditWidgets.m_SelectSectionLabel.m_Label = "Group";
		m_EditWidgets.m_SelectSectionLabel.CurrentOption = { "None", Assets::EmptyHandle };
		m_EditWidgets.m_SelectSectionLabel.m_PopupAction = [&]()
		{
			m_EditWidgets.m_SelectSectionLabel.ClearOptions();
			m_EditWidgets.m_SelectSectionLabel.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& label : Assets::AssetService::GetScriptSectionLabels())
			{
				m_EditWidgets.m_SelectSectionLabel.AddToOptions("All Project Groups", label, Assets::EmptyHandle);
			}

		};

	}

	void ScriptEditorPanel::InitializeGroupList()
	{
		// Group Labels
		m_GroupLabelsTable.m_Label = "All Group Labels";
		m_GroupLabelsTable.Column1Title = "Label";
		m_GroupLabelsTable.Column2Title = "";
		m_GroupLabelsTable.m_Expanded = false;
		m_GroupLabelsTable.AddToSelectionList("Create New Group Label", [&]()
			{
				m_CreateGroupLabelPopup.m_StartPopup = true;
			});
		m_GroupLabelsTable.OnRefresh = [&]()
			{
				m_GroupLabelsTable.ClearList();
				for (auto& label : Assets::AssetService::GetScriptSectionLabels())
				{
					m_GroupLabelsTable.InsertListEntry(label, "", [&](EditorUI::ListEntry& entry, std::size_t iteration)
						{
							m_ActiveLabel = entry.m_Label;
							m_EditGroupLabelPopup.m_OpenPopup = true;
						});
				}
			};
		m_GroupLabelsTable.OnRefresh();

		m_CreateGroupLabelPopup.m_Label = "Create New Group Label";
		m_CreateGroupLabelPopup.m_Flags |= EditorUI::EditText_PopupOnly;
		m_CreateGroupLabelPopup.m_ConfirmAction = [&](EditorUI::EditTextSpec& spec)
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

		m_EditGroupLabelPopup.m_Label = "Edit Group Label";
		m_EditGroupLabelPopup.m_PopupAction = [&]()
			{
				m_EditGroupLabelText.CurrentOption = m_ActiveLabel;
			};
		m_EditGroupLabelPopup.m_ConfirmAction = [&]()
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
		m_EditGroupLabelPopup.m_DeleteAction = [&]()
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
		m_EditGroupLabelPopup.m_PopupContents = [&]()
			{
				EditorUI::EditorUIService::EditText(m_EditGroupLabelText);
			};

		m_EditGroupLabelText.m_Label = "Group Label";
		m_EditGroupLabelText.CurrentOption = "Empty";
	}

	ScriptEditorPanel::ScriptEditorPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(ScriptEditorPanel::OnKeyPressedEditor));

		InitializeAllScriptsList();
		InitializeCreateScriptPopup();
		InitializeEditScriptPopup();
		InitializeGroupList();

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
		EditorUI::EditorUIService::GenericPopup(m_EditWidgets.m_MainPopup);
		EditorUI::EditorUIService::EditText(m_CreateGroupLabelPopup);
		EditorUI::EditorUIService::GenericPopup(m_EditGroupLabelPopup);
		EditorUI::EditorUIService::GenericPopup(m_EditWidgets.m_DeleteWarning);
		EditorUI::EditorUIService::GenericPopup(m_EditWidgets.m_EditWarning);

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
				if (currentEntry.m_Handle != manageEvent.GetAssetID())
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
	void ScriptEditorPanel::OpenCreateScriptDialogFromUsagePoint(WrappedFuncType scriptType, std::function<void(Assets::AssetHandle)> onConfirm, const std::vector<FixedString32>& parameterNames, bool openScriptEditor)
	{
		KG_ASSERT(parameterNames.size() == Utility::WrappedFuncTypeToParameterTypes(scriptType).size(), "Invalid parameter name list provided for function usage point");

		// Store names list
		m_OnCreateParameterNames = parameterNames;

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

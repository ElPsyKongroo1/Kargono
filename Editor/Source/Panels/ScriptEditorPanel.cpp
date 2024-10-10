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
		spec.Type = m_EditScriptType.SelectedOption == 0 ? Scripting::ScriptType::Class : Scripting::ScriptType::Global;
		spec.SectionLabel = m_EditScriptSectionLabel.CurrentOption.Label;
		spec.FunctionType = (WrappedFuncType)(uint64_t)m_EditScriptFuncType.CurrentOption.Handle;
		auto successful = Assets::AssetService::SaveScript(m_ActiveScriptHandle, spec);
		if (!successful)
		{
			KG_ERROR("Unsuccessful at creating new script");
		}
		if (spec.Type == Scripting::ScriptType::Class || originalType == Scripting::ScriptType::Class)
		{
			for (auto& [handle, asset] : Assets::AssetService::GetEntityClassRegistry())
			{
				if (asset.Data.GetSpecificMetaData<Assets::EntityClassMetaData>()->Name == spec.SectionLabel
					|| asset.Data.GetSpecificMetaData<Assets::EntityClassMetaData>()->Name == originalLabel)
				{
					s_EditorApp->m_EntityClassEditor->RefreshEntityScripts(handle);
					break;
				}
			}
		}

		m_AllScriptsTable.OnRefresh();
	};

	void ScriptEditorPanel::InitializeScriptPanel()
	{
		m_AllScriptsTable.Label = "All Scripts";
		m_AllScriptsTable.Column1Title = "Group";
		m_AllScriptsTable.Column2Title = "Name";
		m_AllScriptsTable.Expanded = true;
		m_AllScriptsTable.AddToSelectionList("Create New Script", [&]()
			{
				m_CreateScriptPopup.PopupActive = true;
			});
		m_AllScriptsTable.OnRefresh = [&]()
		{
			m_AllScriptsTable.ClearTable();
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_ScriptType == Scripting::ScriptType::Engine)
				{
					continue;
				}
				std::string scriptType = script->m_ScriptType == Scripting::ScriptType::Class ? "Class" : "Global";
				std::string label = scriptType + std::string("::") + script->m_SectionLabel;
				auto onEdit = [&](EditorUI::TableEntry& entry)
				{
					m_ActiveScriptHandle = entry.Handle;
					m_EditScriptPopup.PopupActive = true;
				};
				auto onLink = [&](EditorUI::TableEntry& entry)
				{
					if (!Assets::AssetService::GetScriptRegistry().contains(entry.Handle))
					{
						KG_WARN("Unable to open script in text editor. Script does not exist in registry");
						return;
					}
					Assets::Asset& asset = Assets::AssetService::GetScriptRegistry().at(entry.Handle);
					s_EditorApp->m_TextEditorPanel->OpenFile(Projects::ProjectService::GetActiveAssetDirectory() / asset.Data.FileLocation);
				};
				EditorUI::TableEntry newEntry
				{
					label,
						script->m_ScriptName,
						handle,
						onEdit,
						onLink
				};
				m_AllScriptsTable.InsertTableEntry(newEntry);
			}

			s_EditorApp->m_TextEditorPanel->RefreshKGScriptEditor();
		};
		m_AllScriptsTable.OnRefresh();

		m_CreateScriptPopup.Label = "Create New Script";
		m_CreateScriptPopup.PopupWidth = 420.0f;
		m_CreateScriptPopup.PopupAction = [&]()
		{
			m_CreateScriptName.CurrentOption = "Empty";
			m_CreateScriptFuncType.CurrentOption.Label = Utility::WrappedFuncTypeToString(WrappedFuncType::None);
			m_CreateScriptType.SelectedOption = 1;
			m_CreateScriptSectionLabel.CurrentOption = { "None", Assets::EmptyHandle };
		};
		m_CreateScriptPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_CreateScriptName);
			EditorUI::EditorUIService::SelectOption(m_CreateScriptFuncType);
			EditorUI::EditorUIService::RadioSelector(m_CreateScriptType);
			EditorUI::EditorUIService::SelectOption(m_CreateScriptSectionLabel);
		};
		m_CreateScriptPopup.ConfirmAction = [&]()
		{
			Assets::ScriptSpec spec {};
			spec.Name = m_CreateScriptName.CurrentOption;
			spec.Type = m_CreateScriptType.SelectedOption == 0 ? Scripting::ScriptType::Class : Scripting::ScriptType::Global;
			spec.SectionLabel = m_CreateScriptSectionLabel.CurrentOption.Label;
			spec.FunctionType = (WrappedFuncType)(uint64_t)m_CreateScriptFuncType.CurrentOption.Handle;
			auto [handle, successful] = Assets::AssetService::CreateNewScript(spec);
			if (!successful)
			{
				KG_ERROR("Unsuccessful at creating new script");
			}

			if (spec.Type == Scripting::ScriptType::Class)
			{
				for (auto& [handle, asset] : Assets::AssetService::GetEntityClassRegistry())
				{
					if (asset.Data.GetSpecificMetaData<Assets::EntityClassMetaData>()->Name == spec.SectionLabel)
					{
						s_EditorApp->m_EntityClassEditor->RefreshEntityScripts(handle);
						break;
					}
				}
			}

			m_AllScriptsTable.OnRefresh();
		};

		m_CreateScriptName.Label = "Name";
		m_CreateScriptName.CurrentOption = "Empty";

		m_CreateScriptFuncType.Label = "Function Type";
		m_CreateScriptFuncType.CurrentOption.Label = Utility::WrappedFuncTypeToString(WrappedFuncType::None);
		m_CreateScriptFuncType.PopupAction = [&]()
		{
			if (m_CreateScriptType.SelectedOption == 0)
			{
				m_CreateScriptFuncType.ClearOptions();
				for (auto func : s_AllWrappedFuncs)
				{
					// Check if function's first parameter is a UInt64
					bool match = Utility::Regex::GetMatchSuccess(Utility::WrappedFuncTypeToString(func), "_UInt64");

					if (match)
					{
						// Display the function without the first parameter, since it is used as the entity number in the func
						// Replace with _None, if only one parameter is present
						std::string funcDisplayName;
						if (Utility::WrappedFuncTypeToParameterTypes(func).size() == 1)
						{
							funcDisplayName = Utility::Regex::ReplaceMatches(Utility::WrappedFuncTypeToString(func),
								"_UInt64", "_None");
						}
						else
						{
							funcDisplayName = Utility::Regex::ReplaceMatches(Utility::WrappedFuncTypeToString(func),
								"_UInt64", "_");
						}

						m_CreateScriptFuncType.AddToOptions("All Options", funcDisplayName, (uint64_t)func);
					}

				}
			}
			else
			{
				m_CreateScriptFuncType.ClearOptions();
				for (auto func : s_AllWrappedFuncs)
				{
					m_CreateScriptFuncType.AddToOptions("All Options", Utility::WrappedFuncTypeToString(func), (uint64_t)func);
				}
			}
		};

		m_CreateScriptType.Label = "Script Type";
		m_CreateScriptType.Editing = true;
		m_CreateScriptType.FirstOptionLabel = "Class";
		m_CreateScriptType.SecondOptionLabel = "Global";
		m_CreateScriptType.SelectAction = [&]()
		{
			// If we set the option to 'Class', try to get the first entity class option
			if (m_CreateScriptType.SelectedOption == 0)
			{
				if (!Assets::AssetService::GetEntityClassRegistry().empty())
				{
					m_CreateScriptSectionLabel.CurrentOption.Label = Assets::AssetService::GetEntityClassRegistry().begin()->second.Data.GetSpecificMetaData<Assets::EntityClassMetaData>()->Name;
					m_CreateScriptSectionLabel.CurrentOption.Handle = Assets::AssetService::GetEntityClassRegistry().begin()->first;
				}
			}
			else
			{
				m_CreateScriptSectionLabel.CurrentOption = { "None", Assets::EmptyHandle };
			}
			m_CreateScriptSectionLabel.PopupAction();

			// Reset Func Type to prevent incorrect types
			m_CreateScriptFuncType.CurrentOption = { "None", Assets::EmptyHandle };
		};

		m_CreateScriptSectionLabel.Label = "Group";
		m_CreateScriptSectionLabel.CurrentOption = { "None", Assets::EmptyHandle };
		m_CreateScriptSectionLabel.PopupAction = [&]()
		{
			if (m_CreateScriptType.SelectedOption == 0)
			{
				m_CreateScriptSectionLabel.ClearOptions();
				//spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& [handle, entityClass] : Assets::AssetService::GetEntityClassRegistry())
				{
					m_CreateScriptSectionLabel.AddToOptions("All Classes", reinterpret_cast<Assets::EntityClassMetaData*>(entityClass.Data.SpecificFileData.get())->Name,
						handle);
				}
			}
			else
			{
				m_CreateScriptSectionLabel.ClearOptions();
				m_CreateScriptSectionLabel.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& label : Assets::AssetService::GetScriptSectionLabels())
				{
					m_CreateScriptSectionLabel.AddToOptions("All Global Groups", label, Assets::EmptyHandle);
				}
			}
		};

		m_EditScriptPopup.Label = "Edit New Script";
		m_EditScriptPopup.PopupWidth = 420.0f;
		m_EditScriptPopup.PopupAction = [&]()
		{
			m_EditScriptName.CurrentOption = Assets::AssetService::GetScript(m_ActiveScriptHandle)->m_ScriptName;
			m_EditScriptType.SelectedOption = Assets::AssetService::GetScript(
				m_ActiveScriptHandle)->m_ScriptType == Scripting::ScriptType::Class ? 0 : 1;
			m_EditScriptSectionLabel.CurrentOption.Label = Assets::AssetService::GetScript(m_ActiveScriptHandle)->m_SectionLabel;

			if (m_EditScriptType.SelectedOption == 0)
			{
				WrappedFuncType currentFuncType = Assets::AssetService::GetScript(m_ActiveScriptHandle)->m_FuncType;
				std::string funcDisplayName;

				if (Utility::WrappedFuncTypeToParameterTypes(currentFuncType).size() == 1)
				{
					funcDisplayName = Utility::Regex::ReplaceMatches(Utility::WrappedFuncTypeToString(currentFuncType),
						"_UInt64", "_None");
				}
				else
				{
					funcDisplayName = Utility::Regex::ReplaceMatches(Utility::WrappedFuncTypeToString(currentFuncType),
						"_UInt64", "_");
				}
				m_EditScriptFuncType.CurrentOption = { funcDisplayName,
					(uint64_t)currentFuncType };
			}
			else
			{
				WrappedFuncType currentFuncType = Assets::AssetService::GetScript(m_ActiveScriptHandle)->m_FuncType;
				m_EditScriptFuncType.CurrentOption = { Utility::WrappedFuncTypeToString(currentFuncType),
					(uint64_t)currentFuncType };
			}
		};
		m_EditScriptPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::LabeledText("Script Name", Assets::AssetService::GetScript(m_ActiveScriptHandle)->m_ScriptName);
			//EditorUI::Editor::EditText(m_EditScriptName);
			EditorUI::EditorUIService::SelectOption(m_EditScriptFuncType);
			EditorUI::EditorUIService::RadioSelector(m_EditScriptType);
			EditorUI::EditorUIService::SelectOption(m_EditScriptSectionLabel);
		};
		m_EditScriptPopup.DeleteAction = [&]()
		{
			m_DeleteScriptWarning.PopupActive = true;
		};
		m_EditScriptPopup.ConfirmAction = [&]()
		{
			if (Utility::StringToWrappedFuncType(m_EditScriptFuncType.CurrentOption.Label) !=
				Assets::AssetService::GetScript(m_ActiveScriptHandle)->m_FuncType)
			{
				m_EditScriptFuncTypeWarning.PopupActive = true;
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

			if (type == Scripting::ScriptType::Class)
			{
				for (auto& [handle, asset] : Assets::AssetService::GetEntityClassRegistry())
				{
					if (asset.Data.GetSpecificMetaData<Assets::EntityClassMetaData>()->Name == sectionLabel)
					{
						s_EditorApp->m_EntityClassEditor->RefreshEntityScripts(handle);
						break;
					}
				}
			}

			m_AllScriptsTable.OnRefresh();
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
			if (m_EditScriptType.SelectedOption == 0)
			{
				m_EditScriptFuncType.ClearOptions();
				for (auto func : s_AllWrappedFuncs)
				{
					// Check if function's first parameter is a UInt64
					bool match = Utility::Regex::GetMatchSuccess(Utility::WrappedFuncTypeToString(func), "_UInt64");

					if (match)
					{
						// Display the function without the first parameter, since it is used as the entity number in the func
						// Replace with _None, if only one parameter is present
						std::string funcDisplayName;
						if (Utility::WrappedFuncTypeToParameterTypes(func).size() == 1)
						{
							funcDisplayName = Utility::Regex::ReplaceMatches(Utility::WrappedFuncTypeToString(func),
								"_UInt64", "_None");
						}
						else
						{
							funcDisplayName = Utility::Regex::ReplaceMatches(Utility::WrappedFuncTypeToString(func),
								"_UInt64", "_");
						}

						m_EditScriptFuncType.AddToOptions("All Options", funcDisplayName, (uint64_t)func);
					}

				}
			}
			else
			{
				m_EditScriptFuncType.ClearOptions();
				for (auto func : s_AllWrappedFuncs)
				{
					m_EditScriptFuncType.AddToOptions("All Options", Utility::WrappedFuncTypeToString(func), (uint64_t)func);
				}
			}
		};

		m_EditScriptType.Label = "Script Type";
		m_EditScriptType.Editing = true;
		m_EditScriptType.FirstOptionLabel = "Class";
		m_EditScriptType.SecondOptionLabel = "Global";
		m_EditScriptType.SelectAction = [&]()
		{
			if (m_EditScriptType.SelectedOption == 0)
			{
				if (Assets::AssetService::GetEntityClassRegistry().size() > 0)
				{
					m_EditScriptSectionLabel.CurrentOption.Label = Assets::AssetService::GetEntityClassRegistry().begin()->second.Data.GetSpecificMetaData<Assets::EntityClassMetaData>()->Name;
					m_EditScriptSectionLabel.CurrentOption.Handle = Assets::AssetService::GetEntityClassRegistry().begin()->first;
				}
			}
			else
			{
				m_EditScriptSectionLabel.CurrentOption = { "None", Assets::EmptyHandle };
			}
			m_EditScriptSectionLabel.PopupAction();
			// Reset Func Type to prevent issues with cache
			m_EditScriptFuncType.CurrentOption = { "None", Assets::EmptyHandle };
		};

		m_EditScriptSectionLabel.Label = "Group";
		m_EditScriptSectionLabel.CurrentOption = { "None", Assets::EmptyHandle };
		m_EditScriptSectionLabel.PopupAction = [&]()
		{
			if (m_EditScriptType.SelectedOption == 0)
			{
				m_EditScriptSectionLabel.ClearOptions();
				//spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& [handle, entityClass] : Assets::AssetService::GetEntityClassRegistry())
				{
					m_EditScriptSectionLabel.AddToOptions("All Classes", reinterpret_cast<Assets::EntityClassMetaData*>(entityClass.Data.SpecificFileData.get())->Name,
						handle);
				}
			}
			else
			{
				m_EditScriptSectionLabel.ClearOptions();
				m_EditScriptSectionLabel.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& label : Assets::AssetService::GetScriptSectionLabels())
				{
					m_EditScriptSectionLabel.AddToOptions("All Global Groups", label, Assets::EmptyHandle);
				}

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
			m_GroupLabelsTable.ClearTable();
			for (auto& label : Assets::AssetService::GetScriptSectionLabels())
			{
				m_GroupLabelsTable.InsertTableEntry(label, "", [&](EditorUI::TableEntry& entry)
					{
						m_ActiveLabel = entry.Label;
						m_EditGroupLabelPopup.PopupActive = true;
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
		m_EditGroupLabelPopup.PopupWidth = 420.0f;
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
			m_AllScriptsTable.OnRefresh();
		};
		m_EditGroupLabelPopup.DeleteAction = [&]()
		{
			bool success = Assets::AssetService::DeleteScriptSectionLabel(m_ActiveLabel);
			if (!success)
			{
				KG_WARN("Failed to delete section label");
				return;
			}

			m_AllScriptsTable.OnRefresh();
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
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName,
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

		EditorUI::EditorUIService::Table(m_AllScriptsTable);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		EditorUI::EditorUIService::Table(m_GroupLabelsTable);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Popups
		EditorUI::EditorUIService::GenericPopup(m_CreateScriptPopup);
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
	void ScriptEditorPanel::ResetPanelResources()
	{
		m_AllScriptsTable.OnRefresh();
		m_GroupLabelsTable.OnRefresh();
	}
}

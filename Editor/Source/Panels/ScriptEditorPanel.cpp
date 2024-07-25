#include "Panels/ScriptEditorPanel.h"

#include "EditorApp.h"

#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{

	// Script Table (Create)
	static EditorUI::TableSpec s_AllScriptsTable {};
	static EditorUI::GenericPopupSpec s_CreateScriptPopup {};
	static EditorUI::TextInputSpec s_CreateScriptName{};
	static EditorUI::SelectOptionSpec s_CreateScriptFuncType{};
	static EditorUI::RadioSelectorSpec s_CreateScriptType{};
	static EditorUI::SelectOptionSpec s_CreateScriptSectionLabel{};
	// Script Table (Edit)
	static std::function<void()> s_UpdateScript {};
	static EditorUI::GenericPopupSpec s_EditScriptPopup {};
	static EditorUI::GenericPopupSpec s_DeleteScriptWarning {};
	static EditorUI::GenericPopupSpec s_EditScriptFuncTypeWarning {};
	static EditorUI::TextInputSpec s_EditScriptName{};
	static EditorUI::SelectOptionSpec s_EditScriptFuncType{};
	static EditorUI::RadioSelectorSpec s_EditScriptType{};
	static EditorUI::SelectOptionSpec s_EditScriptSectionLabel{};
	static Assets::AssetHandle s_ActiveScriptHandle {Assets::EmptyHandle};

	// Group Label Table
	static EditorUI::TableSpec s_GroupLabelsTable {};
	static EditorUI::TextInputSpec s_CreateGroupLabelPopup {};
	static EditorUI::GenericPopupSpec s_EditGroupLabelPopup {};
	static EditorUI::TextInputSpec s_EditGroupLabelText {};
	static std::string s_ActiveLabel {};

	ScriptEditorPanel::ScriptEditorPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName,
			KG_BIND_CLASS_FN(ScriptEditorPanel::OnKeyPressedEditor));

		s_AllScriptsTable.Label = "All Scripts";
		s_AllScriptsTable.Column1Title = "Group";
		s_AllScriptsTable.Column2Title = "Name";
		s_AllScriptsTable.Expanded = true;
		s_AllScriptsTable.AddToSelectionList("Create New Script", [&]()
		{
			s_CreateScriptPopup.PopupActive = true;
		});
		s_AllScriptsTable.OnRefresh = [&]()
		{
			s_AllScriptsTable.ClearTable();
			for (auto& [handle, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_ScriptType == Scripting::ScriptType::Engine)
				{
					continue;
				}
				std::string scriptType = script->m_ScriptType == Scripting::ScriptType::Class ? "Class" : "Global";
				std::string label = scriptType + std::string("::") + script->m_SectionLabel;
				auto onEdit = [&](EditorUI::TableEntry& entry)
				{
					s_ActiveScriptHandle = entry.Handle;
					s_EditScriptPopup.PopupActive = true;
				};
				auto onLink = [&](EditorUI::TableEntry& entry)
				{
					if (!Assets::AssetManager::GetScriptRegistryMap().contains(entry.Handle))
					{
						KG_WARN("Unable to open script in text editor. Script does not exist in registry");
						return;
					}
					Assets::Asset& asset = Assets::AssetManager::GetScriptRegistryMap().at(entry.Handle);
					s_EditorApp->m_TextEditorPanel->OpenFile(Projects::ProjectService::GetActiveAssetDirectory() / asset.Data.IntermediateLocation);
				};
				EditorUI::TableEntry newEntry
				{
					label,
					script->m_ScriptName,
					handle,
					onEdit,
					onLink
				};
				s_AllScriptsTable.InsertTableEntry(newEntry);
			}
		};
		s_AllScriptsTable.OnRefresh();

		s_CreateScriptPopup.Label = "Create New Script";
		s_CreateScriptPopup.PopupWidth = 420.0f;
		s_CreateScriptPopup.PopupAction = [&]()
		{
			s_CreateScriptName.CurrentOption = "Empty";
			s_CreateScriptFuncType.CurrentOption.Label = Utility::WrappedFuncTypeToString(WrappedFuncType::None);
			s_CreateScriptType.SelectedOption = 1;
			s_CreateScriptSectionLabel.CurrentOption = { "None", Assets::EmptyHandle };
		};
		s_CreateScriptPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::TextInputPopup(s_CreateScriptName);
			EditorUI::EditorUIService::SelectOption(s_CreateScriptFuncType);
			EditorUI::EditorUIService::RadioSelector(s_CreateScriptType);
			EditorUI::EditorUIService::SelectOption(s_CreateScriptSectionLabel);
		};
		s_CreateScriptPopup.ConfirmAction = [&]()
		{
			Assets::AssetManager::ScriptSpec spec {};
			spec.Name = s_CreateScriptName.CurrentOption;
			spec.Type = s_CreateScriptType.SelectedOption == 0 ? Scripting::ScriptType::Class : Scripting::ScriptType::Global;
			spec.SectionLabel = s_CreateScriptSectionLabel.CurrentOption.Label;
			spec.FunctionType = (WrappedFuncType)(uint64_t)s_CreateScriptFuncType.CurrentOption.Handle;
			auto [handle, successful] = Assets::AssetManager::CreateNewScript(spec);
			if (!successful)
			{
				KG_ERROR("Unsuccessful at creating new script");
			}

			if (spec.Type == Scripting::ScriptType::Class)
			{
				for (auto& [handle, asset] : Assets::AssetManager::GetEntityClassRegistry())
				{
					if (asset.Data.GetSpecificFileData<Assets::EntityClassMetaData>()->Name == spec.SectionLabel)
					{
						s_EditorApp->m_EntityClassEditor->RefreshEntityScripts(handle);
						break;
					}
				}
			}

			s_AllScriptsTable.OnRefresh();
		};

		s_CreateScriptName.Label = "Name";
		s_CreateScriptName.CurrentOption = "Empty";

		s_CreateScriptFuncType.Label = "Function Type";
		s_CreateScriptFuncType.CurrentOption.Label = Utility::WrappedFuncTypeToString(WrappedFuncType::None);
		s_CreateScriptFuncType.PopupAction = [&]()
		{
			if (s_CreateScriptType.SelectedOption == 0)
			{
				s_CreateScriptFuncType.ClearOptions();
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

						s_CreateScriptFuncType.AddToOptions("All Options", funcDisplayName, (uint64_t)func);
					}

				}
			}
			else
			{
				s_CreateScriptFuncType.ClearOptions();
				for (auto func : s_AllWrappedFuncs)
				{
					s_CreateScriptFuncType.AddToOptions("All Options", Utility::WrappedFuncTypeToString(func), (uint64_t)func);
				}
			}
		};

		s_CreateScriptType.Label = "Script Type";
		s_CreateScriptType.Editing = true;
		s_CreateScriptType.FirstOptionLabel = "Class";
		s_CreateScriptType.SecondOptionLabel = "Global";
		s_CreateScriptType.SelectAction = [&]()
		{
			// If we set the option to 'Class', try to get the first entity class option
			if (s_CreateScriptType.SelectedOption == 0)
			{
				if (!Assets::AssetManager::GetEntityClassRegistry().empty())
				{
					s_CreateScriptSectionLabel.CurrentOption.Label = Assets::AssetManager::GetEntityClassRegistry().begin()->second.Data.GetSpecificFileData<Assets::EntityClassMetaData>()->Name;
					s_CreateScriptSectionLabel.CurrentOption.Handle = Assets::AssetManager::GetEntityClassRegistry().begin()->first;
				}
			}
			else
			{
				s_CreateScriptSectionLabel.CurrentOption = { "None", Assets::EmptyHandle };
			}
			s_CreateScriptSectionLabel.PopupAction();

			// Reset Func Type to prevent incorrect types
			s_CreateScriptFuncType.CurrentOption = { "None", Assets::EmptyHandle };
		};

		s_CreateScriptSectionLabel.Label = "Group";
		s_CreateScriptSectionLabel.CurrentOption = { "None", Assets::EmptyHandle };
		s_CreateScriptSectionLabel.PopupAction = [&]()
		{
			if (s_CreateScriptType.SelectedOption == 0)
			{
				s_CreateScriptSectionLabel.ClearOptions();
				//spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& [handle, entityClass] : Assets::AssetManager::GetEntityClassRegistry())
				{
					s_CreateScriptSectionLabel.AddToOptions("All Classes", reinterpret_cast<Assets::EntityClassMetaData*>(entityClass.Data.SpecificFileData.get())->Name,
						handle);
				}
			}
			else
			{
				s_CreateScriptSectionLabel.ClearOptions();
				s_CreateScriptSectionLabel.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& label : Assets::AssetManager::GetScriptSectionLabels())
				{
					s_CreateScriptSectionLabel.AddToOptions("All Global Groups", label, Assets::EmptyHandle);
				}
			}
		};

		s_UpdateScript = [&]()
		{
			Ref<Scripting::Script> script = Assets::AssetManager::GetScript(s_ActiveScriptHandle);
			if (!script)
			{
				KG_WARN("No script pointer available from asset manager when editing script in script editor");
				return;
			}
			Scripting::ScriptType originalType = script->m_ScriptType;
			std::string originalLabel = script->m_SectionLabel;
			Assets::AssetManager::ScriptSpec spec {};
			spec.Name = s_EditScriptName.CurrentOption;
			spec.Type = s_EditScriptType.SelectedOption == 0 ? Scripting::ScriptType::Class : Scripting::ScriptType::Global;
			spec.SectionLabel = s_EditScriptSectionLabel.CurrentOption.Label;
			spec.FunctionType = (WrappedFuncType)(uint64_t)s_EditScriptFuncType.CurrentOption.Handle;
			auto successful = Assets::AssetManager::UpdateScript(s_ActiveScriptHandle, spec);
			if (!successful)
			{
				KG_ERROR("Unsuccessful at creating new script");
			}
			if (spec.Type == Scripting::ScriptType::Class || originalType == Scripting::ScriptType::Class)
			{
				for (auto& [handle, asset] : Assets::AssetManager::GetEntityClassRegistry())
				{
					if (asset.Data.GetSpecificFileData<Assets::EntityClassMetaData>()->Name == spec.SectionLabel
						|| asset.Data.GetSpecificFileData<Assets::EntityClassMetaData>()->Name == originalLabel)
					{
						s_EditorApp->m_EntityClassEditor->RefreshEntityScripts(handle);
						break;
					}
				}
			}

			s_AllScriptsTable.OnRefresh();
		};

		s_EditScriptPopup.Label = "Edit New Script";
		s_EditScriptPopup.PopupWidth = 420.0f;
		s_EditScriptPopup.PopupAction = [&]()
		{
			s_EditScriptName.CurrentOption = Assets::AssetManager::GetScript(s_ActiveScriptHandle)->m_ScriptName;
			s_EditScriptType.SelectedOption = Assets::AssetManager::GetScript(
				s_ActiveScriptHandle)->m_ScriptType == Scripting::ScriptType::Class ? 0 : 1;
			s_EditScriptSectionLabel.CurrentOption.Label = Assets::AssetManager::GetScript(s_ActiveScriptHandle)->m_SectionLabel;

			if (s_EditScriptType.SelectedOption == 0)
			{
				WrappedFuncType currentFuncType = Assets::AssetManager::GetScript(s_ActiveScriptHandle)->m_FuncType;
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
				s_EditScriptFuncType.CurrentOption = { funcDisplayName,
					(uint64_t)currentFuncType };
			}
			else
			{
				WrappedFuncType currentFuncType = Assets::AssetManager::GetScript(s_ActiveScriptHandle)->m_FuncType;
				s_EditScriptFuncType.CurrentOption = { Utility::WrappedFuncTypeToString(currentFuncType),
					(uint64_t)currentFuncType };
			}
		};
		s_EditScriptPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::LabeledText("Script Name", Assets::AssetManager::GetScript(s_ActiveScriptHandle)->m_ScriptName);
			//EditorUI::Editor::TextInputPopup(s_EditScriptName);
			EditorUI::EditorUIService::SelectOption(s_EditScriptFuncType);
			EditorUI::EditorUIService::RadioSelector(s_EditScriptType);
			EditorUI::EditorUIService::SelectOption(s_EditScriptSectionLabel);
		};
		s_EditScriptPopup.DeleteAction = [&]()
		{
			s_DeleteScriptWarning.PopupActive = true;
		};
		s_EditScriptPopup.ConfirmAction = [&]()
		{
			if (Utility::StringToWrappedFuncType(s_EditScriptFuncType.CurrentOption.Label) != 
				Assets::AssetManager::GetScript(s_ActiveScriptHandle)->m_FuncType)
			{
				s_EditScriptFuncTypeWarning.PopupActive = true;
			}
			else
			{
				s_UpdateScript();
			}
		};

		s_DeleteScriptWarning.Label = "Delete Script";
		s_DeleteScriptWarning.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to delete this script?");
		};
		s_DeleteScriptWarning.ConfirmAction = [&]()
		{
			Ref<Scripting::Script> script = Assets::AssetManager::GetScript(s_ActiveScriptHandle);
			Scripting::ScriptType type = script->m_ScriptType;
			std::string sectionLabel = script->m_SectionLabel;

			bool success = Assets::AssetManager::DeleteScript(s_ActiveScriptHandle);
			if (!success)
			{
				KG_WARN("Unable to delete script!");
				return;
			}

			if (type == Scripting::ScriptType::Class)
			{
				for (auto& [handle, asset] : Assets::AssetManager::GetEntityClassRegistry())
				{
					if (asset.Data.GetSpecificFileData<Assets::EntityClassMetaData>()->Name == sectionLabel)
					{
						s_EditorApp->m_EntityClassEditor->RefreshEntityScripts(handle);
						break;
					}
				}
			}

			s_AllScriptsTable.OnRefresh();
		};

		s_EditScriptFuncTypeWarning.Label = "Edit Script";
		s_EditScriptFuncTypeWarning.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Changing the function type can cause the existing function code to not compile.");
			EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);
			EditorUI::EditorUIService::Text("Are you sure you want to modify the function type?");
		};
		s_EditScriptFuncTypeWarning.ConfirmAction = [&]()
		{
			s_UpdateScript();
		};

		s_EditScriptName.Label = "Name";
		s_EditScriptName.CurrentOption = "Empty";

		s_EditScriptFuncType.Label = "Function Type";
		s_EditScriptFuncType.CurrentOption.Label = Utility::WrappedFuncTypeToString(WrappedFuncType::None);
		s_EditScriptFuncType.PopupAction = [&]()
		{
			if (s_EditScriptType.SelectedOption == 0)
			{
				s_EditScriptFuncType.ClearOptions();
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

						s_EditScriptFuncType.AddToOptions("All Options", funcDisplayName, (uint64_t)func);
					}

				}
			}
			else
			{
				s_EditScriptFuncType.ClearOptions();
				for (auto func : s_AllWrappedFuncs)
				{
					s_EditScriptFuncType.AddToOptions("All Options", Utility::WrappedFuncTypeToString(func), (uint64_t)func);
				}
			}
		};

		s_EditScriptType.Label = "Script Type";
		s_EditScriptType.Editing = true;
		s_EditScriptType.FirstOptionLabel = "Class";
		s_EditScriptType.SecondOptionLabel = "Global";
		s_EditScriptType.SelectAction = [&]()
		{
			if (s_EditScriptType.SelectedOption == 0)
			{
				if (Assets::AssetManager::GetEntityClassRegistry().size() > 0)
				{
					s_EditScriptSectionLabel.CurrentOption.Label = Assets::AssetManager::GetEntityClassRegistry().begin()->second.Data.GetSpecificFileData<Assets::EntityClassMetaData>()->Name;
					s_EditScriptSectionLabel.CurrentOption.Handle = Assets::AssetManager::GetEntityClassRegistry().begin()->first;
				}
			}
			else
			{
				s_EditScriptSectionLabel.CurrentOption = { "None", Assets::EmptyHandle };
			}
			s_EditScriptSectionLabel.PopupAction();
			// Reset Func Type to prevent issues with cache
			s_EditScriptFuncType.CurrentOption = { "None", Assets::EmptyHandle };
		};

		s_EditScriptSectionLabel.Label = "Group";
		s_EditScriptSectionLabel.CurrentOption = {"None", Assets::EmptyHandle};
		s_EditScriptSectionLabel.PopupAction = [&]()
		{
			if (s_EditScriptType.SelectedOption == 0)
			{
				s_EditScriptSectionLabel.ClearOptions();
				//spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& [handle, entityClass] : Assets::AssetManager::GetEntityClassRegistry())
				{
					s_EditScriptSectionLabel.AddToOptions("All Classes", reinterpret_cast<Assets::EntityClassMetaData*>(entityClass.Data.SpecificFileData.get())->Name,
						handle);
				}
			}
			else
			{
				s_EditScriptSectionLabel.ClearOptions();
				s_EditScriptSectionLabel.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& label : Assets::AssetManager::GetScriptSectionLabels())
				{
					s_EditScriptSectionLabel.AddToOptions("All Global Groups", label, Assets::EmptyHandle);
				}
				
			}
		};

		// Group Labels
		s_GroupLabelsTable.Label = "All Group Labels";
		s_GroupLabelsTable.Column1Title = "Label";
		s_GroupLabelsTable.Column2Title = "";
		s_GroupLabelsTable.Expanded = false;
		s_GroupLabelsTable.AddToSelectionList("Create New Group Label", [&]()
		{
			s_CreateGroupLabelPopup.StartPopup = true;
		});
		s_GroupLabelsTable.OnRefresh = [&]()
		{
			s_GroupLabelsTable.ClearTable();
			for (auto& label : Assets::AssetManager::GetScriptSectionLabels())
			{
				s_GroupLabelsTable.InsertTableEntry(label,"", [&](EditorUI::TableEntry& entry)
				{
					s_ActiveLabel = entry.Label;
					s_EditGroupLabelPopup.PopupActive = true;
				});
			}
		};
		s_GroupLabelsTable.OnRefresh();

		s_CreateGroupLabelPopup.Label = "Create New Group Label";
		s_CreateGroupLabelPopup.Flags |= EditorUI::TextInput_PopupOnly;
		s_CreateGroupLabelPopup.ConfirmAction = [&]()
		{
			// Create new group label
			bool success = Assets::AssetManager::AddScriptSectionLabel(s_CreateGroupLabelPopup.CurrentOption);
			if (!success)
			{
				KG_WARN("Failed to create group label");
				return;
			}
			s_GroupLabelsTable.OnRefresh();
		};

		s_EditGroupLabelPopup.Label = "Edit Group Label";
		s_EditGroupLabelPopup.PopupWidth = 420.0f;
		s_EditGroupLabelPopup.PopupAction = []()
		{
			s_EditGroupLabelText.CurrentOption = s_ActiveLabel;
		};
		s_EditGroupLabelPopup.ConfirmAction = [&]()
		{
			// Create new group label
			bool success = Assets::AssetManager::EditScriptSectionLabel(
				s_ActiveLabel, s_EditGroupLabelText.CurrentOption);
			if (!success)
			{
				KG_WARN("Failed to edit group label");
				return;
			}
			s_GroupLabelsTable.OnRefresh();
			s_AllScriptsTable.OnRefresh();
		};
		s_EditGroupLabelPopup.DeleteAction = [&]()
		{
			bool success = Assets::AssetManager::DeleteScriptSectionLabel(s_ActiveLabel);
			if (!success)
			{
				KG_WARN("Failed to delete section label");
				return;
			}

			s_AllScriptsTable.OnRefresh();
			s_GroupLabelsTable.OnRefresh();
		};
		s_EditGroupLabelPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::TextInputPopup(s_EditGroupLabelText);
		};

		s_EditGroupLabelText.Label = "Group Label";
		s_EditGroupLabelText.CurrentOption = "Empty";

	}
	void ScriptEditorPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowScriptEditor);

		EditorUI::EditorUIService::Table(s_AllScriptsTable);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		EditorUI::EditorUIService::Table(s_GroupLabelsTable);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Popups
		EditorUI::EditorUIService::GenericPopup(s_CreateScriptPopup);
		EditorUI::EditorUIService::GenericPopup(s_EditScriptPopup);
		EditorUI::EditorUIService::GenericPopup(s_DeleteScriptWarning);
		EditorUI::EditorUIService::GenericPopup(s_EditScriptFuncTypeWarning);
		EditorUI::EditorUIService::TextInputPopup(s_CreateGroupLabelPopup);
		EditorUI::EditorUIService::GenericPopup(s_EditGroupLabelPopup);

		EditorUI::EditorUIService::EndWindow();
	}
	bool ScriptEditorPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
	void ScriptEditorPanel::ResetPanelResources()
	{
		s_AllScriptsTable.OnRefresh();
		s_GroupLabelsTable.OnRefresh();
	}
}

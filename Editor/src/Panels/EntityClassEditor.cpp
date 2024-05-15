#include "Panels/EntityClassEditor.h"

#include "Kargono.h"
#include "EditorLayer.h"

namespace Kargono
{
	static EditorLayer* s_EditorLayer { nullptr };
	static Ref<EntityClass> s_EditorEntityClass { nullptr };
	static Assets::AssetHandle s_EditorEntityClassHandle { Assets::EmptyHandle };

	static std::string s_CurrentField {};

	// Initial Panel
	static std::function<void()> s_OnOpenClass { nullptr };
	static std::function<void()> s_OnCreateClass { nullptr };
	static std::function<void()> s_OnRefreshData { nullptr };
	static EditorUI::GenericPopupSpec s_CreateClassPopupSpec {};
	static EditorUI::SelectOptionSpec s_OpenClassPopupSpec {};
	static EditorUI::TextInputSpec s_SelectClassNameSpec {};
	// Header
	static EditorUI::SelectorHeaderSpec s_MainHeader {};
	static EditorUI::GenericPopupSpec s_DeleteEntityClassWarning {};
	static EditorUI::GenericPopupSpec s_CloseEntityClassWarning {};
	// Fields Table
	static EditorUI::TableSpec s_FieldsTable {};
	static EditorUI::SelectOptionSpec s_AddFieldPopup {};
	static EditorUI::GenericPopupSpec s_EditFieldPopup {};
	static EditorUI::TextInputSpec s_EditFieldName {};
	static EditorUI::SelectOptionSpec s_EditFieldType {};
	// Static Function Widgets
	static EditorUI::CollapsingHeaderSpec s_StaticFunctionHeaderSpec {};
	static EditorUI::SelectOptionSpec s_SelectOnPhysicsCollisionStartSpec {};
	static EditorUI::SelectOptionSpec s_SelectOnPhysicsCollisionEndSpec {};
	static EditorUI::SelectOptionSpec s_SelectOnCreateSpec {};
	static EditorUI::SelectOptionSpec s_SelectOnUpdateSpec {};
	// All Scripts Table
	static EditorUI::TableSpec s_AllScriptsTableSpec {};

	static void InitializeOpeningScreen()
	{
		s_OnOpenClass = [&]()
		{
			s_OpenClassPopupSpec.StartPopup = true;
		};

		s_OnCreateClass = [&]()
		{
			s_CreateClassPopupSpec.PopupActive = true;
		};
		s_OnRefreshData = [&]()
		{
			s_FieldsTable.OnRefresh();
			s_AllScriptsTableSpec.OnRefresh();
			Assets::AssetHandle handle = s_EditorEntityClass->GetScripts().OnPhysicsCollisionStartHandle;
			s_SelectOnPhysicsCollisionStartSpec.CurrentOption = { handle ?
				Assets::AssetManager::GetScript(handle)->m_ScriptName : "None",
				handle};
			handle = s_EditorEntityClass->GetScripts().OnPhysicsCollisionEndHandle;
			s_SelectOnPhysicsCollisionEndSpec.CurrentOption = { handle ?
				Assets::AssetManager::GetScript(handle)->m_ScriptName : "None",
				handle};
			handle = s_EditorEntityClass->GetScripts().OnCreateHandle;
			s_SelectOnCreateSpec.CurrentOption = { handle ?
				Assets::AssetManager::GetScript(handle)->m_ScriptName : "None",
				handle};
			handle = s_EditorEntityClass->GetScripts().OnUpdateHandle;
			s_SelectOnUpdateSpec.CurrentOption = { handle ?
				Assets::AssetManager::GetScript(handle)->m_ScriptName : "None",
				handle};
		};

		s_OpenClassPopupSpec.Label = "Open Class";
		s_OpenClassPopupSpec.CurrentOption = { "None", Assets::EmptyHandle };
		s_OpenClassPopupSpec.PopupOnly = true;
		s_OpenClassPopupSpec.PopupAction = [&](EditorUI::SelectOptionSpec& spec)
		{
			spec.GetAllOptions().clear();
			spec.CurrentOption = { "None", Assets::EmptyHandle };

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetManager::GetEntityClassRegistry())
			{
				spec.AddToOptions("All Options", asset.Data.IntermediateLocation.string(), handle);
			}
		};

		s_OpenClassPopupSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.Handle == Assets::EmptyHandle)
			{
				KG_WARN("No Entity Selected");
				return;
			}
			if (!Assets::AssetManager::GetEntityClassRegistry().contains(selection.Handle))
			{
				KG_WARN("Could not find on entity class in entity class editor");
				return;
			}

			Assets::Asset asset = Assets::AssetManager::GetEntityClassRegistry().at(selection.Handle);
			s_EditorEntityClass = Assets::AssetManager::GetEntityClass(asset.Handle);
			s_EditorEntityClassHandle = asset.Handle;
			s_MainHeader.EditColorActive = false;
			s_OnRefreshData();
		};

		s_SelectClassNameSpec.Label = "New Name";
		s_SelectClassNameSpec.CurrentOption = "Empty";
		s_SelectClassNameSpec.ConfirmAction = [&](const std::string& option)
		{
			s_SelectClassNameSpec.CurrentOption = option;
		};

		s_CreateClassPopupSpec.Label = "Create Entity Class";
		s_CreateClassPopupSpec.PopupWidth = 420.0f;
		s_CreateClassPopupSpec.ConfirmAction = [&]()
		{
			if (s_SelectClassNameSpec.CurrentOption == "")
			{
				return;
			}

			for (auto& [id, asset] : Assets::AssetManager::GetEntityClassRegistry())
			{
				if (asset.Data.GetSpecificFileData<Assets::EntityClassMetaData>()->Name == s_SelectClassNameSpec.CurrentOption)
				{
					return;
				}
			}
			s_EditorEntityClassHandle = Assets::AssetManager::CreateNewEntityClass(s_SelectClassNameSpec.CurrentOption);
			s_EditorEntityClass = Assets::AssetManager::GetEntityClass(s_EditorEntityClassHandle);
			s_MainHeader.EditColorActive = false;
			s_OnRefreshData();
		};
		s_CreateClassPopupSpec.PopupContents = [&]()
		{
			EditorUI::Editor::TextInputPopup(s_SelectClassNameSpec);
		};
	}

	static void InitializeDisplayEntityClassScreen()
	{
		// Header (Entity Class Name and Options)
		s_DeleteEntityClassWarning.Label = "Delete Entity Class";
		s_DeleteEntityClassWarning.ConfirmAction = [&]()
		{
			Assets::AssetManager::DeleteEntityClass(s_EditorEntityClassHandle);
			s_EditorEntityClassHandle = 0;
			s_EditorEntityClass = nullptr;
		};
		s_DeleteEntityClassWarning.PopupContents = [&]()
		{
			EditorUI::Editor::Text("Are you sure you want to delete this entity class object?");
		};

		s_CloseEntityClassWarning.Label = "Close Entity Class";
		s_CloseEntityClassWarning.ConfirmAction = [&]()
		{
			s_EditorEntityClassHandle = 0;
			s_EditorEntityClass = nullptr;
		};
		s_CloseEntityClassWarning.PopupContents = [&]()
		{
			EditorUI::Editor::Text("Are you sure you want to close this entity class object without saving?");
		};

		s_MainHeader.AddToSelectionList("Save", [&]()
			{
				Assets::AssetManager::SaveEntityClass(s_EditorEntityClassHandle, s_EditorEntityClass);
				s_MainHeader.EditColorActive = false;
			});
		s_MainHeader.AddToSelectionList("Close", [&]()
			{
				if (s_MainHeader.EditColorActive)
				{
					s_CloseEntityClassWarning.PopupActive = true;
				}
				else
				{
					s_EditorEntityClassHandle = 0;
					s_EditorEntityClass = nullptr;
				}
			});
		s_MainHeader.AddToSelectionList("Delete", [&]()
			{
				s_DeleteEntityClassWarning.PopupActive = true;
			});

		// Fields Table
		s_FieldsTable.Label = "Fields";
		s_FieldsTable.Expanded = true;
		s_FieldsTable.OnRefresh = [&]()
		{
			s_FieldsTable.ClearTable();
			if (s_EditorEntityClass)
			{
				for (auto& [name, field] : s_EditorEntityClass->GetFields())
				{
					s_FieldsTable.InsertTableEntry(name,
						Utility::WrappedVarTypeToString(field),
						[&](EditorUI::TableEntry& entry)
						{
							s_EditFieldPopup.PopupActive = true;
							s_CurrentField = name;
						});
				}
			}
		};
		s_FieldsTable.Column1Title = "Field Name";
		s_FieldsTable.Column2Title = "Field Type";
		s_FieldsTable.AddToSelectionList("Add New Field", [&]()
			{
				s_AddFieldPopup.StartPopup = true;
			});

		s_AddFieldPopup.Label = "Add New Field";
		s_AddFieldPopup.PopupOnly = true;
		s_AddFieldPopup.CurrentOption = { "None", Assets::EmptyHandle };
		s_AddFieldPopup.LineCount = 2;
		s_AddFieldPopup.PopupAction = [&](EditorUI::SelectOptionSpec& spec)
		{
			spec.ClearOptions();
			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& type : Kargono::s_AllWrappedVarTypes)
			{
				spec.AddToOptions("All Options", Utility::WrappedVarTypeToString(type), Assets::EmptyHandle);
			}
		};
		s_AddFieldPopup.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.Label == "None")
			{
				return;
			}

			uint32_t iteration{ 1 };
			while (!s_EditorEntityClass->AddField("New Field " + std::to_string(iteration),
				Utility::StringToWrappedVarType(selection.Label)))
			{
				iteration++;
			}
			s_MainHeader.EditColorActive = true;
			s_FieldsTable.OnRefresh();
		};

		s_EditFieldName.Label = "Field Name";
		s_EditFieldName.CurrentOption = "Empty";
		s_EditFieldName.ConfirmAction = [&](const std::string& option)
		{
			s_EditFieldName.CurrentOption = option;
		};

		s_EditFieldType.Label = "Field Type";
		s_EditFieldType.PopupOnly = false;
		s_EditFieldType.CurrentOption = { "None", Assets::EmptyHandle };
		s_EditFieldType.LineCount = 2;
		s_EditFieldType.PopupAction = [&](EditorUI::SelectOptionSpec& spec)
		{
			spec.ClearOptions();
			for (auto& type : Kargono::s_AllWrappedVarTypes)
			{
				spec.AddToOptions("All Options", Utility::WrappedVarTypeToString(type), Assets::EmptyHandle);
			}
		};
		s_EditFieldType.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			s_FieldsTable.OnRefresh();
		};

		s_EditFieldPopup.Label = "Edit Field";
		s_EditFieldPopup.DeleteAction = [&]()
		{
			if (!s_EditorEntityClass->DeleteField(s_CurrentField))
			{
				KG_ERROR("Unable to delete field inside entity class!");
				return;
			}
			s_MainHeader.EditColorActive = true;
			s_FieldsTable.OnRefresh();
		};
		s_EditFieldPopup.PopupWidth = 420.0f;
		s_EditFieldPopup.PopupAction = [&](EditorUI::GenericPopupSpec& spec)
		{
			WrappedVarType fieldType = s_EditorEntityClass->GetField(s_CurrentField);

			if (fieldType == WrappedVarType::None)
			{
				KG_ERROR("Unable to retreive field from current entity class object");
				return;
			}
			s_EditFieldName.CurrentOption = s_CurrentField;
			s_EditFieldType.CurrentOption.Label = Utility::WrappedVarTypeToString(fieldType);
		};
		s_EditFieldPopup.ConfirmAction = [&]()
		{
			auto& fieldMap = s_EditorEntityClass->GetFields();

			if (!fieldMap.contains(s_CurrentField))
			{
				KG_ERROR("Could not find reference to original entity class in field map");
				return;
			}

			fieldMap.erase(s_CurrentField);
			fieldMap.insert_or_assign(s_EditFieldName.CurrentOption,
				Utility::StringToWrappedVarType(s_EditFieldType.CurrentOption.Label));
			s_MainHeader.EditColorActive = true;
			s_FieldsTable.OnRefresh();
		};
		s_EditFieldPopup.PopupContents = [&]()
		{
			EditorUI::Editor::TextInputPopup(s_EditFieldName);
			EditorUI::Editor::SelectOption(s_EditFieldType);
		};

		// Static Function Section
		s_StaticFunctionHeaderSpec.Label = "Static Functions";
		s_StaticFunctionHeaderSpec.Expanded = true;

		// Update OnPhysicsCollisionStart
		s_SelectOnPhysicsCollisionStartSpec.Label = "On Collision Start";
		s_SelectOnPhysicsCollisionStartSpec.LineCount = 3;
		s_SelectOnPhysicsCollisionStartSpec.CurrentOption = { "None", Assets::EmptyHandle };
		s_SelectOnPhysicsCollisionStartSpec.Indented = true;
		s_SelectOnPhysicsCollisionStartSpec.PopupAction = [](EditorUI::SelectOptionSpec& spec)
		{
			spec.GetAllOptions().clear();

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_Function->Type() == WrappedFuncType::Bool_UInt64)
				{
					spec.AddToOptions("All Options", script->m_ScriptName, handle);
				}
			}
			const Assets::AssetHandle handle = s_EditorEntityClass->GetScripts().OnPhysicsCollisionStartHandle;
			spec.CurrentOption = { handle ?
				Assets::AssetManager::GetScript(handle)->m_ScriptName : "None",
				handle};
		};
		s_SelectOnPhysicsCollisionStartSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.Handle == Assets::EmptyHandle)
			{
				s_EditorEntityClass->GetScripts().OnPhysicsCollisionStartHandle = 0;
				s_EditorEntityClass->GetScripts().OnPhysicsCollisionStart = nullptr;
				s_MainHeader.EditColorActive = true;
				return;
			}

			if (!Assets::AssetManager::GetScriptRegistryMap().contains(selection.Handle))
			{
				KG_WARN("Could not locate OnPhysicsCollisionStart function in Entity Editor Panel");
				return;
			}

			s_EditorEntityClass->GetScripts().OnPhysicsCollisionStartHandle = selection.Handle;
			s_EditorEntityClass->GetScripts().OnPhysicsCollisionStart =
				Assets::AssetManager::GetScript(selection.Handle).get();
			s_MainHeader.EditColorActive = true;
		};

		// Update OnPhysicsCollisionEnd
		s_SelectOnPhysicsCollisionEndSpec.Label = "On Collision End";
		s_SelectOnPhysicsCollisionEndSpec.LineCount = 3;
		s_SelectOnPhysicsCollisionEndSpec.Indented = true;
		s_SelectOnPhysicsCollisionEndSpec.CurrentOption = { "None", Assets::EmptyHandle };
		s_SelectOnPhysicsCollisionEndSpec.PopupAction = [](EditorUI::SelectOptionSpec& spec)
		{
			spec.GetAllOptions().clear();

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_Function->Type() == WrappedFuncType::Bool_UInt64)
				{
					spec.AddToOptions("All Options", script->m_ScriptName, handle);
				}
			}
			Assets::AssetHandle handle = s_EditorEntityClass->GetScripts().OnPhysicsCollisionEndHandle;
			spec.CurrentOption = { handle ?
				Assets::AssetManager::GetScript(handle)->m_ScriptName : "None",
				handle};
		};
		s_SelectOnPhysicsCollisionEndSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.Handle == Assets::EmptyHandle)
			{
				s_EditorEntityClass->GetScripts().OnPhysicsCollisionEndHandle = 0;
				s_EditorEntityClass->GetScripts().OnPhysicsCollisionEnd = nullptr;
				s_MainHeader.EditColorActive = true;
				return;
			}

			if (!Assets::AssetManager::GetScriptRegistryMap().contains(selection.Handle))
			{
				KG_WARN("Could not locate OnPhysicsCollisionEnd function in Entity Editor Panel");
				return;
			}

			s_EditorEntityClass->GetScripts().OnPhysicsCollisionEndHandle = selection.Handle;
			s_EditorEntityClass->GetScripts().OnPhysicsCollisionEnd =
				Assets::AssetManager::GetScript(selection.Handle).get();
			s_MainHeader.EditColorActive = true;
		};

		// Update OnCreate
		s_SelectOnCreateSpec.Label = "On Create";
		s_SelectOnCreateSpec.LineCount = 3;
		s_SelectOnCreateSpec.Indented = true;
		s_SelectOnCreateSpec.CurrentOption = { "None", Assets::EmptyHandle};
		s_SelectOnCreateSpec.PopupAction = [](EditorUI::SelectOptionSpec& spec)
		{
			spec.GetAllOptions().clear();

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_Function->Type() == WrappedFuncType::Void_None)
				{
					spec.AddToOptions("All Options", script->m_ScriptName, handle);
				}
			}
			Assets::AssetHandle handle = s_EditorEntityClass->GetScripts().OnCreateHandle;
			spec.CurrentOption = { handle ?
				Assets::AssetManager::GetScript(handle)->m_ScriptName : "None",
				handle};
		};
		s_SelectOnCreateSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.Handle == Assets::EmptyHandle)
			{
				s_EditorEntityClass->GetScripts().OnCreateHandle = 0;
				s_EditorEntityClass->GetScripts().OnCreate = nullptr;
				s_MainHeader.EditColorActive = true;
				return;
			}

			if (!Assets::AssetManager::GetScriptRegistryMap().contains(selection.Handle))
			{
				KG_WARN("Could not locate OnCreate function in Entity Editor Panel");
				return;
			}

			s_EditorEntityClass->GetScripts().OnCreateHandle = selection.Handle;
			s_EditorEntityClass->GetScripts().OnCreate =
				Assets::AssetManager::GetScript(selection.Handle).get();
			s_MainHeader.EditColorActive = true;
		};

		// Update OnUpdate
		s_SelectOnUpdateSpec.Label = "On Update";
		s_SelectOnUpdateSpec.LineCount = 3;
		s_SelectOnUpdateSpec.Indented = true;
		s_SelectOnUpdateSpec.CurrentOption = { "None", Assets::EmptyHandle };
		s_SelectOnUpdateSpec.PopupAction = [](EditorUI::SelectOptionSpec& spec)
		{
			spec.GetAllOptions().clear();

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_Function->Type() == WrappedFuncType::Void_Float)
				{
					spec.AddToOptions("All Options", script->m_ScriptName, handle);
				}
			}
			Assets::AssetHandle handle = s_EditorEntityClass->GetScripts().OnUpdateHandle;
			spec.CurrentOption = { handle ?
				Assets::AssetManager::GetScript(handle)->m_ScriptName : "None",
				handle};
		};
		s_SelectOnUpdateSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.Handle == Assets::EmptyHandle)
			{
				s_EditorEntityClass->GetScripts().OnUpdateHandle = 0;
				s_EditorEntityClass->GetScripts().OnUpdate = nullptr;
				s_MainHeader.EditColorActive = true;
				return;
			}

			if (!Assets::AssetManager::GetScriptRegistryMap().contains(selection.Handle))
			{
				KG_WARN("Could not locate OnUpdate function in Entity Editor Panel");
				return;
			}

			s_EditorEntityClass->GetScripts().OnUpdateHandle = selection.Handle;
			s_EditorEntityClass->GetScripts().OnUpdate =
				Assets::AssetManager::GetScript(selection.Handle).get();
			s_MainHeader.EditColorActive = true;
		};

		// AllScripts Table
		s_AllScriptsTableSpec.Label = "All Class Scripts";
		s_AllScriptsTableSpec.Expanded = true;
		s_AllScriptsTableSpec.OnRefresh = [&]()
		{
			s_AllScriptsTableSpec.ClearTable();
			if (s_EditorEntityClass)
			{
				for (auto& scriptHandle: s_EditorEntityClass->GetScripts().AllClassScripts)
				{
					Ref<Scripting::Script> currentScript = Assets::AssetManager::GetScript(scriptHandle);
					if (!currentScript)
					{
						KG_WARN("Could not locate script in asset manager for all scripts table");
						continue;
					}

					s_AllScriptsTableSpec.InsertTableEntry(currentScript->m_ScriptName,
						scriptHandle,
						nullptr);
				}
			}
		};
		s_AllScriptsTableSpec.Column1Title = "Script Name";
		s_AllScriptsTableSpec.Column2Title = "Script ID";
	}

	EntityClassEditor::EntityClassEditor()
	{
		s_EditorLayer = EditorLayer::GetCurrentLayer();

		InitializeOpeningScreen();

		InitializeDisplayEntityClassScreen();

	}
	void EntityClassEditor::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::Editor::StartWindow("Class Editor", &s_EditorLayer->m_ShowClassEditor);

		if (!s_EditorEntityClass)
		{
			EditorUI::Editor::NewItemScreen("Open Existing Class", s_OnOpenClass, "Create New Class", s_OnCreateClass);
			EditorUI::Editor::GenericPopup(s_CreateClassPopupSpec);
			EditorUI::Editor::SelectOption(s_OpenClassPopupSpec);
		}
		else
		{
			s_MainHeader.Label = s_EditorEntityClass->GetName();
			EditorUI::Editor::SelectorHeader(s_MainHeader);
			EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);
			EditorUI::Editor::GenericPopup(s_DeleteEntityClassWarning);
			EditorUI::Editor::GenericPopup(s_CloseEntityClassWarning);
			EditorUI::Editor::Table(s_FieldsTable);
			EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

			EditorUI::Editor::CollapsingHeader(s_StaticFunctionHeaderSpec);
			EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);
			if (s_StaticFunctionHeaderSpec.Expanded)
			{
				EditorUI::Editor::SelectOption(s_SelectOnPhysicsCollisionStartSpec);
				EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

				EditorUI::Editor::SelectOption(s_SelectOnPhysicsCollisionEndSpec);
				EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

				EditorUI::Editor::SelectOption(s_SelectOnCreateSpec);
				EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

				EditorUI::Editor::SelectOption(s_SelectOnUpdateSpec);
				EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);
			}

			EditorUI::Editor::Table(s_AllScriptsTableSpec);
			EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

			EditorUI::Editor::SelectOption(s_AddFieldPopup);
			EditorUI::Editor::GenericPopup(s_EditFieldPopup);
		}
		EditorUI::Editor::EndWindow();
	}
}

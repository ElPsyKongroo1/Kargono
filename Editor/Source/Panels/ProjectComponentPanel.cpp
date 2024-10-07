#include "Panels/ProjectComponentPanel.h"

#include "EditorApp.h"
#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{
	ProjectComponentPanel::ProjectComponentPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName,
			KG_BIND_CLASS_FN(ProjectComponentPanel::OnKeyPressedEditor));
		InitializeOpeningPanel();
		InitializeComponentFieldsSection();
	}
	void ProjectComponentPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowProjectComponent);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		if (!m_EditorProjectComponent)
		{
			EditorUI::EditorUIService::NewItemScreen("Open Existing Component", KG_BIND_CLASS_FN(OpenComponent), "Create New Component", KG_BIND_CLASS_FN(CreateComponent));
			EditorUI::EditorUIService::GenericPopup(m_CreateComponentPopup);
			EditorUI::EditorUIService::SelectOption(m_OpenComponentPopup);
		}
		else
		{
			// Header
			EditorUI::EditorUIService::PanelHeader(m_TagHeader);
			EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

			// Header Popups
			EditorUI::EditorUIService::GenericPopup(m_DeleteComponentWarning);
			EditorUI::EditorUIService::GenericPopup(m_CloseComponentWarning);

			// Table
			EditorUI::EditorUIService::Table(m_FieldsTable);

			// Table Popups
			EditorUI::EditorUIService::GenericPopup(m_AddFieldPopup);
			EditorUI::EditorUIService::GenericPopup(m_EditFieldPopup);

		}


		EditorUI::EditorUIService::EndWindow();
	}
	void ProjectComponentPanel::InitializeOpeningPanel()
	{
		m_OpenComponentPopup.Label = "Open Component";
		m_OpenComponentPopup.Flags |= EditorUI::SelectOption_PopupOnly;
		m_OpenComponentPopup.CurrentOption = { "None", Assets::EmptyHandle };
		m_OpenComponentPopup.PopupAction = [&]()
		{
			m_OpenComponentPopup.GetAllOptions().clear();
			m_OpenComponentPopup.CurrentOption = { "None", Assets::EmptyHandle };

			m_OpenComponentPopup.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetProjectComponentRegistry())
			{
				m_OpenComponentPopup.AddToOptions("All Options", asset.Data.FileLocation.string(), handle);
			}
		};

		m_OpenComponentPopup.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.Handle == Assets::EmptyHandle)
			{
				KG_WARN("No  Selected");
				return;
			}
			if (!Assets::AssetService::GetProjectComponentRegistry().contains(selection.Handle))
			{
				KG_WARN("Could not find component in component editor");
				return;
			}

			m_EditorProjectComponent = Assets::AssetService::GetProjectComponent(selection.Handle);
			m_EditorProjectComponentHandle = selection.Handle;
			m_TagHeader.Label = Assets::AssetService::GetProjectComponentRegistry().at(
				selection.Handle).Data.FileLocation.string();
			m_TagHeader.EditColorActive = false;
			RefreshData();
		};

		m_SelectComponentName.Label = "New Name";
		m_SelectComponentName.CurrentOption = "Empty";

		m_CreateComponentPopup.Label = "Create  Component";
		m_CreateComponentPopup.PopupWidth = 420.0f;
		m_CreateComponentPopup.ConfirmAction = [&]()
		{
			if (m_SelectComponentName.CurrentOption == "")
			{
				return;
			}

			for (auto& [id, asset] : Assets::AssetService::GetProjectComponentRegistry())
			{
				if (asset.Data.GetSpecificMetaData<Assets::ProjectComponentMetaData>()->Name == m_SelectComponentName.CurrentOption)
				{
					return;
				}
			}
			m_EditorProjectComponentHandle = Assets::AssetService::CreateProjectComponent(m_SelectComponentName.CurrentOption);
			m_EditorProjectComponent = Assets::AssetService::GetProjectComponent(m_EditorProjectComponentHandle);
			m_TagHeader.EditColorActive = false;
			m_TagHeader.Label = Assets::AssetService::GetProjectComponentRegistry().at(
				m_EditorProjectComponentHandle).Data.FileLocation.string();
			RefreshData();
		};
		m_CreateComponentPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_SelectComponentName);
		};
	}
	void ProjectComponentPanel::InitializeComponentFieldsSection()
	{
		// Header (Component Name and Options)
		m_DeleteComponentWarning.Label = "Delete Component";
		m_DeleteComponentWarning.ConfirmAction = [&]()
		{
			Assets::AssetService::DeleteProjectComponent(m_EditorProjectComponentHandle);
			// TODO: Refresh scene data and whatnot
			m_EditorProjectComponentHandle = 0;
			m_EditorProjectComponent = nullptr;
		};
		m_DeleteComponentWarning.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to delete this component object?");
		};

		m_CloseComponentWarning.Label = "Close Component";
		m_CloseComponentWarning.ConfirmAction = [&]()
		{
			m_EditorProjectComponentHandle = 0;
			m_EditorProjectComponent = nullptr;
		};
		m_CloseComponentWarning.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to close this component object without saving?");
		};

		m_TagHeader.AddToSelectionList("Save", [&]()
		{
			Assets::AssetService::SaveProjectComponent(m_EditorProjectComponentHandle, m_EditorProjectComponent);
			// TODO: Refresh scene data and whatnot
			m_TagHeader.EditColorActive = false;
		});
		m_TagHeader.AddToSelectionList("Close", [&]()
		{
			if (m_TagHeader.EditColorActive)
			{
				m_CloseComponentWarning.PopupActive = true;
			}
			else
			{
				m_EditorProjectComponentHandle = 0;
				m_EditorProjectComponent = nullptr;
			}
		});
		m_TagHeader.AddToSelectionList("Delete", [&]()
		{
			m_DeleteComponentWarning.PopupActive = true;
		});

		// Fields Table
		m_FieldsTable.Label = "Fields";
		m_FieldsTable.Expanded = true;
		m_FieldsTable.OnRefresh = [&]()
		{
			m_FieldsTable.ClearTable();
			if (m_EditorProjectComponent)
			{
				for (size_t iteration{0}; iteration < m_EditorProjectComponent->m_DataNames.size(); iteration++)
				{
					m_FieldsTable.InsertTableEntry(m_EditorProjectComponent->m_DataNames.at(iteration),
						Utility::WrappedVarTypeToString(m_EditorProjectComponent->m_DataTypes.at(iteration)),
						[&](EditorUI::TableEntry& entry)
						{
							m_EditFieldPopup.PopupActive = true;
							m_ActiveField = entry.Handle;
						}, iteration);
				}
			}
		};
		m_FieldsTable.Column1Title = "Field Name";
		m_FieldsTable.Column2Title = "Field Type";
		m_FieldsTable.AddToSelectionList("Add New Field", [&]()
		{
			m_AddFieldPopup.PopupActive = true;
		});

		m_AddFieldName.Label = "Field Name";
		m_AddFieldName.CurrentOption = "Empty";

		m_AddFieldType.Label = "Field Type";
		m_AddFieldType.CurrentOption = { "None", Assets::EmptyHandle };
		m_AddFieldType.LineCount = 2;
		m_AddFieldType.PopupAction = [&]()
		{
			m_AddFieldType.ClearOptions();
			for (auto& type : Kargono::s_AllWrappedVarTypes)
			{
				m_AddFieldType.AddToOptions("All Options", Utility::WrappedVarTypeToString(type), Assets::EmptyHandle);
			}
		};
		m_AddFieldType.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			RefreshData();
		};

		m_AddFieldPopup.Label = "Add Field";
		m_AddFieldPopup.PopupWidth = 420.0f;
		m_AddFieldPopup.PopupAction = [&]()
		{
			m_AddFieldName.CurrentOption = "New Field";
			m_AddFieldType.CurrentOption.Label = Utility::WrappedVarTypeToString(WrappedVarType::None);
		};
		m_AddFieldPopup.ConfirmAction = [&]()
		{
			bool success = ECS::ProjectComponentService::AddFieldToProjectComponent(m_EditorProjectComponent, 
				Utility::StringToWrappedVarType(m_AddFieldType.CurrentOption.Label),
				m_AddFieldName.CurrentOption);
			if (!success)
			{
				KG_WARN("Add field failed. Returning to previous window.");
				return;
			}
			m_TagHeader.EditColorActive = true;
			m_FieldsTable.OnRefresh();
		};
		m_AddFieldPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_AddFieldName);
			EditorUI::EditorUIService::SelectOption(m_AddFieldType);
		};

		m_EditFieldName.Label = "Field Name";
		m_EditFieldName.CurrentOption = "Empty";

		m_EditFieldType.Label = "Field Type";
		m_EditFieldType.CurrentOption = { "None", Assets::EmptyHandle };
		m_EditFieldType.LineCount = 2;
		m_EditFieldType.PopupAction = [&]()
		{
			m_EditFieldType.ClearOptions();
			for (auto& type : Kargono::s_AllWrappedVarTypes)
			{
				m_EditFieldType.AddToOptions("All Options", Utility::WrappedVarTypeToString(type), Assets::EmptyHandle);
			}
		};
		m_EditFieldType.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			RefreshData();
		};

		m_EditFieldPopup.Label = "Edit Field";
		m_EditFieldPopup.DeleteAction = [&]()
		{
			ECS::ProjectComponentService::DeleteFieldFromProjectComponent(m_EditorProjectComponent, m_ActiveField);
			m_TagHeader.EditColorActive = true;
			RefreshData();
		};
		m_EditFieldPopup.PopupWidth = 420.0f;
		m_EditFieldPopup.PopupAction = [&]()
		{
			KG_ASSERT(m_ActiveField < m_EditorProjectComponent->m_DataNames.size(),
				"Unable to retreive field from current component object. Active field index is out of bounds.");
			m_EditFieldName.CurrentOption = m_EditorProjectComponent->m_DataNames.at(m_ActiveField);
			m_EditFieldType.CurrentOption.Label = Utility::WrappedVarTypeToString(m_EditorProjectComponent->m_DataTypes.at(m_ActiveField));
		};
		m_EditFieldPopup.ConfirmAction = [&]()
		{

			bool success = ECS::ProjectComponentService::EditFieldInProjectComponent(m_EditorProjectComponent, m_ActiveField,
				m_EditFieldName.CurrentOption, Utility::StringToWrappedVarType(m_EditFieldType.CurrentOption.Label));
			if (!success)
			{
				KG_WARN("Edit field failed. Returning to previous window.");
				return;
			}
			m_TagHeader.EditColorActive = true;
			m_FieldsTable.OnRefresh();
		};
		m_EditFieldPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_EditFieldName);
			EditorUI::EditorUIService::SelectOption(m_EditFieldType);
		};
	}
	bool ProjectComponentPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
	void ProjectComponentPanel::OpenComponent()
	{
		m_OpenComponentPopup.PopupActive = true;
	}
	void ProjectComponentPanel::CreateComponent()
	{
		m_CreateComponentPopup.PopupActive = true;
	}
	void ProjectComponentPanel::RefreshData()
	{
		m_FieldsTable.OnRefresh();
	}
}
			

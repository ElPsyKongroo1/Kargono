#include "Panels/ProjectComponentPanel.h"
#include "Kargono/Scripting/ScriptCompilerService.h"

#include "EditorApp.h"
#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{
	ProjectComponentPanel::ProjectComponentPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
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
			EditorUI::EditorUIService::NewItemScreen("Open Existing Component", KG_BIND_CLASS_FN(OpenComponentDialog), "Create New Component", KG_BIND_CLASS_FN(CreateComponentDialog));
			EditorUI::EditorUIService::GenericPopup(m_CreateComponentPopup);
			EditorUI::EditorUIService::SelectOption(m_OpenComponentPopup);
		}
		else
		{
			// Header
			EditorUI::EditorUIService::PanelHeader(m_MainHeader);
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

			OnOpenComponent(selection.Handle);
		};

		m_SelectComponentName.Label = "New Name";
		m_SelectComponentName.CurrentOption = "Empty";

		m_SelectProjectComponentLocationSpec.Label = "Location";
		m_SelectProjectComponentLocationSpec.CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
		m_SelectProjectComponentLocationSpec.ConfirmAction = [&](const std::string& path)
		{
			if (!Utility::FileSystem::DoesPathContainSubPath(Projects::ProjectService::GetActiveAssetDirectory(), path))
			{
				KG_WARN("Cannot create an asset outside of the project's asset directory.");
				m_SelectProjectComponentLocationSpec.CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
			}
		};

		m_CreateComponentPopup.Label = "Create  Component";
		m_CreateComponentPopup.ConfirmAction = [&]()
		{
			// Ensure input string does not use whitespace
			Utility::Operations::RemoveWhitespaceFromString(m_SelectComponentName.CurrentOption);

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
			m_EditorProjectComponentHandle = Assets::AssetService::CreateProjectComponent(m_SelectComponentName.CurrentOption.c_str(), m_SelectProjectComponentLocationSpec.CurrentOption);
			m_EditorProjectComponent = Assets::AssetService::GetProjectComponent(m_EditorProjectComponentHandle);
			m_MainHeader.EditColorActive = false;
			m_MainHeader.Label = Assets::AssetService::GetProjectComponentRegistry().at(
				m_EditorProjectComponentHandle).Data.FileLocation.string();
			RefreshData();
			Scripting::ScriptCompilerService::CreateKGScriptLanguageDefinition();
		};
		m_CreateComponentPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_SelectComponentName);
			EditorUI::EditorUIService::ChooseDirectory(m_SelectProjectComponentLocationSpec);
		};
	}
	void ProjectComponentPanel::InitializeComponentFieldsSection()
	{
		// Header (Component Name and Options)
		m_DeleteComponentWarning.Label = "Delete Component";
		m_DeleteComponentWarning.ConfirmAction = [&]()
		{
			Assets::AssetService::DeleteProjectComponent(m_EditorProjectComponentHandle);
			Scripting::ScriptCompilerService::CreateKGScriptLanguageDefinition();
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

		m_MainHeader.AddToSelectionList("Save", [&]()
		{
			Assets::AssetService::SaveProjectComponent(m_EditorProjectComponentHandle, m_EditorProjectComponent);
			Scripting::ScriptCompilerService::CreateKGScriptLanguageDefinition();
			m_MainHeader.EditColorActive = false;
		});
		m_MainHeader.AddToSelectionList("Close", [&]()
		{
			if (m_MainHeader.EditColorActive)
			{
				m_CloseComponentWarning.PopupActive = true;
			}
			else
			{
				m_EditorProjectComponentHandle = 0;
				m_EditorProjectComponent = nullptr;
			}
		});
		m_MainHeader.AddToSelectionList("Delete", [&]()
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
		m_AddFieldPopup.PopupAction = [&]()
		{
			m_AddFieldName.CurrentOption = "New Field";
			m_AddFieldType.CurrentOption.Label = Utility::WrappedVarTypeToString(WrappedVarType::None);
		};
		m_AddFieldPopup.ConfirmAction = [&]()
		{
			// Ensure input string does not use whitespace
			Utility::Operations::RemoveWhitespaceFromString(m_AddFieldName.CurrentOption);
			bool success = ECS::ProjectComponentService::AddFieldToProjectComponent(m_EditorProjectComponent, 
				Utility::StringToWrappedVarType(m_AddFieldType.CurrentOption.Label),
				m_AddFieldName.CurrentOption);
			if (!success)
			{
				KG_WARN("Add field failed. Returning to previous window.");
				return;
			}
			m_MainHeader.EditColorActive = true;
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
			m_MainHeader.EditColorActive = true;
			RefreshData();
		};
		m_EditFieldPopup.PopupAction = [&]()
		{
			KG_ASSERT(m_ActiveField < m_EditorProjectComponent->m_DataNames.size(),
				"Unable to retreive field from current component object. Active field index is out of bounds.");
			m_EditFieldName.CurrentOption = m_EditorProjectComponent->m_DataNames.at(m_ActiveField);
			m_EditFieldType.CurrentOption.Label = Utility::WrappedVarTypeToString(m_EditorProjectComponent->m_DataTypes.at(m_ActiveField));
		};
		m_EditFieldPopup.ConfirmAction = [&]()
		{
			// Ensure input string does not use whitespace
			Utility::Operations::RemoveWhitespaceFromString(m_EditFieldName.CurrentOption);
			bool success = ECS::ProjectComponentService::EditFieldInProjectComponent(m_EditorProjectComponent, m_ActiveField,
				m_EditFieldName.CurrentOption, Utility::StringToWrappedVarType(m_EditFieldType.CurrentOption.Label));
			if (!success)
			{
				KG_WARN("Edit field failed. Returning to previous window.");
				return;
			}
			m_MainHeader.EditColorActive = true;
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
	bool ProjectComponentPanel::OnAssetEvent(Events::Event* event)
	{
		// Validate event type and asset type
		if (event->GetEventType() != Events::EventType::ManageAsset)
		{
			return false;
		}
		Events::ManageAsset* manageAsset = (Events::ManageAsset*)event;
		if (manageAsset->GetAssetType() != Assets::AssetType::ProjectComponent)
		{
			return false;
		}

		// Handle deletion of asset
		if (manageAsset->GetAssetID() != m_EditorProjectComponentHandle)
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
			m_MainHeader.Label = Assets::AssetService::GetProjectComponentFileLocation(manageAsset->GetAssetID()).string();
			return true;
		}
		return false;
	}
	void ProjectComponentPanel::ResetPanelResources()
	{
		m_EditorProjectComponent = nullptr;
		m_EditorProjectComponentHandle = Assets::EmptyHandle;
	}
	void ProjectComponentPanel::OpenCreateDialog(std::filesystem::path& createLocation)
	{
		// Open project component Window
		s_EditorApp->m_ShowProjectComponent = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);

		if (!m_EditorProjectComponent)
		{
			// Open dialog to create editor project component
			CreateComponentDialog();
			m_SelectProjectComponentLocationSpec.CurrentOption = createLocation;
		}
		else
		{
			// Add warning to close active project component before creating a new project component
			s_EditorApp->OpenWarningMessage("A project component is already active inside the editor. Please close the current project component before creating a new one.");
		}
	}
	void ProjectComponentPanel::OpenAssetInEditor(std::filesystem::path& assetLocation)
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
		Assets::AssetHandle assetHandle = Assets::AssetService::GetProjectComponentHandleFromFileLocation(relativePath);

		// Validate resulting handle
		if (!assetHandle)
		{
			KG_WARN("Could not open asset in editor. Provided path does not result in an asset inside the registry.");
			return;
		}

		// Open the editor panel to be visible
		s_EditorApp->m_ShowProjectComponent = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);

		// Early out if asset is already open
		if (m_EditorProjectComponentHandle == assetHandle)
		{
			return;
		}

		// Check if panel is already occupied by an asset
		if (!m_EditorProjectComponent)
		{
			OnOpenComponent(assetHandle);
		}
		else
		{
			// Add warning to close active AI state before opening a new AIState
			s_EditorApp->OpenWarningMessage("An project component is already active inside the editor. Please close the current project component before opening a new one.");
		}
	}
	void ProjectComponentPanel::OpenComponentDialog()
	{
		m_OpenComponentPopup.PopupActive = true;
	}
	void ProjectComponentPanel::CreateComponentDialog()
	{
		KG_ASSERT(Projects::ProjectService::GetActive());
		m_SelectProjectComponentLocationSpec.CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
		m_CreateComponentPopup.PopupActive = true;
	}
	void ProjectComponentPanel::RefreshData()
	{
		m_FieldsTable.OnRefresh();
	}
	void ProjectComponentPanel::OnOpenComponent(Assets::AssetHandle newHandle)
	{
		m_EditorProjectComponent = Assets::AssetService::GetProjectComponent(newHandle);
		m_EditorProjectComponentHandle = newHandle;
		m_MainHeader.Label = Assets::AssetService::GetProjectComponentRegistry().at(
			newHandle).Data.FileLocation.string();
		m_MainHeader.EditColorActive = false;
		RefreshData();
	}
}
			

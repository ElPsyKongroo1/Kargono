#include "Windows/MainWindow/CustomComponentPanel.h"

#include "EditorApp.h"

#include "Modules/Scripting/ScriptCompilerService.h"
#include "Kargono/Utility/Operations.h"

static Kargono::EditorApp* s_EditorApp { nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };

namespace Kargono::Panels
{
	CustomComponentPanel::CustomComponentPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
		s_MainWindow->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(CustomComponentPanel::OnKeyPressedEditor));
		InitializeOpeningPanel();
		InitializeComponentFieldsSection();
	}
	void CustomComponentPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_MainWindow->m_ShowCustomComponent);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		if (!m_EditorCustomComponent)
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

			// List
			EditorUI::EditorUIService::List(m_FieldsTable);

			// List Popups
			EditorUI::EditorUIService::GenericPopup(m_AddFieldPopup);
			EditorUI::EditorUIService::GenericPopup(m_EditFieldPopup);

		}


		EditorUI::EditorUIService::EndWindow();
	}
	void CustomComponentPanel::InitializeOpeningPanel()
	{
		m_OpenComponentPopup.m_Label = "Open Component";
		m_OpenComponentPopup.m_Flags |= EditorUI::SelectOption_PopupOnly;
		m_OpenComponentPopup.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_OpenComponentPopup.m_PopupAction = [&](EditorUI::SelectOptionSpec& spec)
		{
			spec.GetAllOptions().clear();
			spec.m_CurrentOption = { "None", Assets::EmptyHandle };

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetCustomComponentRegistry())
			{
				spec.AddToOptions("All Options", asset.Data.FileLocation.filename().string(), handle);
			}
		};

		m_OpenComponentPopup.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.m_Handle == Assets::EmptyHandle)
			{
				KG_WARN("No  Selected");
				return;
			}
			if (!Assets::AssetService::GetCustomComponentRegistry().contains(selection.m_Handle))
			{
				KG_WARN("Could not find component in component editor");
				return;
			}

			OnOpenComponent(selection.m_Handle);
		};

		m_SelectComponentName.m_Label = "New Name";
		m_SelectComponentName.m_CurrentOption = "Empty";

		m_SelectCustomComponentLocationSpec.m_Label = "Location";
		m_SelectCustomComponentLocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
		m_SelectCustomComponentLocationSpec.m_ConfirmAction = [&](std::string_view path)
		{
			if (!Utility::FileSystem::DoesPathContainSubPath(Projects::ProjectService::GetActiveAssetDirectory(), path))
			{
				KG_WARN("Cannot create an asset outside of the project's asset directory.");
				m_SelectCustomComponentLocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
			}
		};

		m_CreateComponentPopup.m_Label = "Create  Component";
		m_CreateComponentPopup.m_ConfirmAction = [&]()
		{
			// Ensure input string does not use whitespace
			Utility::Operations::RemoveWhitespaceFromString(m_SelectComponentName.m_CurrentOption);

			if (m_SelectComponentName.m_CurrentOption == "")
			{
				return;
			}

			for (auto& [id, asset] : Assets::AssetService::GetCustomComponentRegistry())
			{
				if (asset.Data.GetSpecificMetaData<Assets::CustomComponentMetaData>()->Name == m_SelectComponentName.m_CurrentOption)
				{
					return;
				}
			}
			m_EditorCustomComponentHandle = Assets::AssetService::CreateCustomComponent(m_SelectComponentName.m_CurrentOption.c_str(), m_SelectCustomComponentLocationSpec.m_CurrentOption);
			m_EditorCustomComponent = CreateRef<ECSInternal::CustomComponent>(*Assets::AssetService::GetCustomComponent(m_EditorCustomComponentHandle));
			m_MainHeader.m_EditColorActive = false;
			m_MainHeader.m_Label = Assets::AssetService::GetCustomComponentRegistry().at(
				m_EditorCustomComponentHandle).Data.FileLocation.filename().string();
			RefreshData();
			Scripting::ScriptCompilerService::CreateKGScriptLanguageDefinition();
		};
		m_CreateComponentPopup.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_SelectComponentName);
			EditorUI::EditorUIService::ChooseDirectory(m_SelectCustomComponentLocationSpec);
		};
	}
	void CustomComponentPanel::InitializeComponentFieldsSection()
	{
		// Header (Component Name and Options)
		m_DeleteComponentWarning.m_Label = "Delete Component";
		m_DeleteComponentWarning.m_ConfirmAction = [&]()
		{
			Assets::AssetService::DeleteCustomComponent(m_EditorCustomComponentHandle);
			Scripting::ScriptCompilerService::CreateKGScriptLanguageDefinition();
			m_EditorCustomComponentHandle = 0;
			m_EditorCustomComponent = nullptr;
		};
		m_DeleteComponentWarning.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to delete this component object?");
		};

		m_CloseComponentWarning.m_Label = "Close Component";
		m_CloseComponentWarning.m_ConfirmAction = [&]()
		{
			m_EditorCustomComponentHandle = 0;
			m_EditorCustomComponent = nullptr;
		};
		m_CloseComponentWarning.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to close this component object without saving?");
		};

		m_MainHeader.AddToSelectionList("Save", [&]()
		{
			Assets::AssetService::SaveCustomComponent(m_EditorCustomComponentHandle, m_EditorCustomComponent);
			Scripting::ScriptCompilerService::CreateKGScriptLanguageDefinition();
			m_MainHeader.m_EditColorActive = false;
		});
		m_MainHeader.AddToSelectionList("Close", [&]()
		{
			if (m_MainHeader.m_EditColorActive)
			{
				m_CloseComponentWarning.m_OpenPopup = true;
			}
			else
			{
				m_EditorCustomComponentHandle = 0;
				m_EditorCustomComponent = nullptr;
			}
		});
		m_MainHeader.AddToSelectionList("Delete", [&]()
		{
			m_DeleteComponentWarning.m_OpenPopup = true;
		});

		// Fields List
		m_FieldsTable.m_Label = "Fields";
		m_FieldsTable.m_Expanded = true;
		m_FieldsTable.m_OnRefresh = [&]()
		{
			m_FieldsTable.ClearList();
			if (m_EditorCustomComponent)
			{
				for (size_t iteration{0}; iteration < m_EditorCustomComponent->m_DataNames.size(); iteration++)
				{
					m_FieldsTable.InsertListEntry(m_EditorCustomComponent->m_DataNames.at(iteration).CString(),
						Utility::WrappedVarTypeToString(m_EditorCustomComponent->m_DataTypes.at(iteration)),
						[&](EditorUI::ListEntry& entry, std::size_t iteration)
						{
							UNREFERENCED_PARAMETER(iteration);
							m_EditFieldPopup.m_OpenPopup = true;
							m_ActiveField = entry.m_Handle;
						}, iteration);
				}
			}
		};
		m_FieldsTable.m_Column1Title = "Field Name";
		m_FieldsTable.m_Column2Title = "Field Type";
		m_FieldsTable.AddToSelectionList("Add New Field", [&]()
		{
			m_AddFieldPopup.m_OpenPopup = true;
		});

		m_AddFieldName.m_Label = "Field Name";
		m_AddFieldName.m_CurrentOption = "Empty";

		m_AddFieldType.m_Label = "Field Type";
		m_AddFieldType.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_AddFieldType.m_LineCount = 2;
		m_AddFieldType.m_PopupAction = [&](EditorUI::SelectOptionSpec& spec)
		{
			spec.ClearOptions();
			for (WrappedVarType type : Kargono::s_AllWrappedVarTypes)
			{
				spec.AddToOptions("All Options", Utility::WrappedVarTypeToString(type), Assets::EmptyHandle);
			}
		};
		m_AddFieldType.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			UNREFERENCED_PARAMETER(selection);
			RefreshData();
		};

		m_AddFieldPopup.m_Label = "Add Field";
		m_AddFieldPopup.m_PopupAction = [&]()
		{
			m_AddFieldName.m_CurrentOption = "New Field";
			m_AddFieldType.m_CurrentOption.m_Label = Utility::WrappedVarTypeToString(WrappedVarType::None);
		};
		m_AddFieldPopup.m_ConfirmAction = [&]()
		{
			// Ensure input string does not use whitespace
			Utility::Operations::RemoveWhitespaceFromString(m_AddFieldName.m_CurrentOption);
			bool success = m_EditorCustomComponent->AddField(
				Utility::StringToWrappedVarType(m_AddFieldType.m_CurrentOption.m_Label.CString()),
				m_AddFieldName.m_CurrentOption.c_str());
			if (!success)
			{
				KG_WARN("Add field failed. Returning to previous window.");
				return;
			}
			m_MainHeader.m_EditColorActive = true;
			m_FieldsTable.m_OnRefresh();
		};
		m_AddFieldPopup.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_AddFieldName);
			EditorUI::EditorUIService::SelectOption(m_AddFieldType);
		};

		m_EditFieldName.m_Label = "Field Name";
		m_EditFieldName.m_CurrentOption = "Empty";

		m_EditFieldType.m_Label = "Field Type";
		m_EditFieldType.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_EditFieldType.m_LineCount = 2;
		m_EditFieldType.m_PopupAction = [&](EditorUI::SelectOptionSpec& spec)
		{
			spec.ClearOptions();
			for (WrappedVarType type : Kargono::s_AllWrappedVarTypes)
			{
				spec.AddToOptions("All Options", Utility::WrappedVarTypeToString(type), Assets::EmptyHandle);
			}
		};
		m_EditFieldType.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			UNREFERENCED_PARAMETER(selection);
			RefreshData();
		};

		m_EditFieldPopup.m_Label = "Edit Field";
		m_EditFieldPopup.m_DeleteAction = [&]()
		{
			m_EditorCustomComponent->DeleteField(m_ActiveField);
			m_MainHeader.m_EditColorActive = true;
			RefreshData();
		};
		m_EditFieldPopup.m_PopupAction = [&]()
		{
			KG_ASSERT(m_ActiveField < m_EditorCustomComponent->m_DataNames.size(),
				"Unable to retreive field from current component object. Active field index is out of bounds.");
			m_EditFieldName.m_CurrentOption = m_EditorCustomComponent->m_DataNames.at(m_ActiveField);
			m_EditFieldType.m_CurrentOption.m_Label = Utility::WrappedVarTypeToString(m_EditorCustomComponent->m_DataTypes.at(m_ActiveField));
		};
		m_EditFieldPopup.m_ConfirmAction = [&]()
		{
			// Ensure input string does not use whitespace
			Utility::Operations::RemoveWhitespaceFromString(m_EditFieldName.m_CurrentOption);
			bool success = m_EditorCustomComponent->EditField(m_ActiveField,
				m_EditFieldName.m_CurrentOption.c_str(), 
				Utility::StringToWrappedVarType(m_EditFieldType.m_CurrentOption.m_Label.CString()));
			if (!success)
			{
				KG_WARN("Edit field failed. Returning to previous window.");
				return;
			}
			m_MainHeader.m_EditColorActive = true;
			m_FieldsTable.m_OnRefresh();
		};
		m_EditFieldPopup.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_EditFieldName);
			EditorUI::EditorUIService::SelectOption(m_EditFieldType);
		};
	}
	bool CustomComponentPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
	bool CustomComponentPanel::OnAssetEvent(Events::Event* event)
	{
		// Validate event type and asset type
		if (event->GetEventType() != Events::EventType::ManageAsset)
		{
			return false;
		}
		Events::ManageAsset* manageAsset = (Events::ManageAsset*)event;
		if (manageAsset->GetAssetType() != Assets::AssetType::CustomComponent)
		{
			return false;
		}

		// Handle deletion of asset
		if (manageAsset->GetAssetID() != m_EditorCustomComponentHandle)
		{
			return false;
		}

		// Handle deletion of asset
		if (manageAsset->GetAction() == Events::ManageAssetAction::PreDelete)
		{
			ResetPanelResources();
			return true;
		}

		// Handle updating of asset
		if (manageAsset->GetAction() == Events::ManageAssetAction::UpdateAssetInfo)
		{
			// Update header
			m_MainHeader.m_Label = Assets::AssetService::GetCustomComponentFileLocation(manageAsset->GetAssetID()).filename().string();
			return true;
		}
		return false;
	}
	void CustomComponentPanel::ResetPanelResources()
	{
		m_EditorCustomComponent = nullptr;
		m_EditorCustomComponentHandle = Assets::EmptyHandle;
	}
	void CustomComponentPanel::OpenCreateDialog(std::filesystem::path& createLocation)
	{
		// Open custom component Window
		s_MainWindow->m_ShowCustomComponent = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);

		if (!m_EditorCustomComponent)
		{
			// Open dialog to create editor custom component
			CreateComponentDialog();
			m_SelectCustomComponentLocationSpec.m_CurrentOption = createLocation;
		}
		else
		{
			// Add warning to close active custom component before creating a new custom component
			s_MainWindow->OpenWarningMessage("A custom component is already active inside the editor. Please close the current custom component before creating a new one.");
		}
	}
	void CustomComponentPanel::OpenAssetInEditor(std::filesystem::path& assetLocation)
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
		Assets::AssetHandle assetHandle = Assets::AssetService::GetCustomComponentHandleFromFileLocation(relativePath);

		// Validate resulting handle
		if (!assetHandle)
		{
			KG_WARN("Could not open asset in editor. Provided path does not result in an asset inside the registry.");
			return;
		}

		// Open the editor panel to be visible
		s_MainWindow->m_ShowCustomComponent = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);

		// Early out if asset is already open
		if (m_EditorCustomComponentHandle == assetHandle)
		{
			return;
		}

		// Check if panel is already occupied by an asset
		if (!m_EditorCustomComponent)
		{
			OnOpenComponent(assetHandle);
		}
		else
		{
			// Add warning to close active AI state before opening a new AIState
			s_MainWindow->OpenWarningMessage("An custom component is already active inside the editor. Please close the current custom component before opening a new one.");
		}
	}
	void CustomComponentPanel::OpenComponentDialog()
	{
		m_OpenComponentPopup.m_OpenPopup = true;
	}
	void CustomComponentPanel::CreateComponentDialog()
	{
		KG_ASSERT(Projects::ProjectService::GetActive());
		m_SelectCustomComponentLocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
		m_CreateComponentPopup.m_OpenPopup = true;
	}
	void CustomComponentPanel::RefreshData()
	{
		m_FieldsTable.m_OnRefresh();
	}
	void CustomComponentPanel::OnOpenComponent(Assets::AssetHandle newHandle)
	{
		m_EditorCustomComponent = CreateRef<ECSInternal::CustomComponent>(*Assets::AssetService::GetCustomComponent(newHandle));
		m_EditorCustomComponentHandle = newHandle;
		m_MainHeader.m_Label = Assets::AssetService::GetCustomComponentRegistry().at(
			newHandle).Data.FileLocation.filename().string();
		m_MainHeader.m_EditColorActive = false;
		RefreshData();
	}
}
			

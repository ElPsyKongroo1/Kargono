#include "Windows/MainWindow/ProjectEnumPanel.h"

#include "EditorApp.h"

static Kargono::EditorApp* s_EditorApp{ nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };

namespace Kargono::Panels
{
	void ProjectEnumPanel::InitializeOpeningScreen()
	{
		m_OpenProjectEnumPopupSpec.m_Label = "Open Enum";
		m_OpenProjectEnumPopupSpec.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_OpenProjectEnumPopupSpec.m_Flags |= EditorUI::SelectOption_PopupOnly;
		m_OpenProjectEnumPopupSpec.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.GetAllOptions().clear();
			spec.m_CurrentOption = { "None", Assets::EmptyHandle };

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetProjectEnumRegistry())
			{
				spec.AddToOptions("All Options", asset.Data.FileLocation.filename().string(), handle);
			}
		};

		m_OpenProjectEnumPopupSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.m_Handle == Assets::EmptyHandle)
			{
				KG_WARN("No Project Enum Selected");
				return;
			}
			if (!Assets::AssetService::GetProjectEnumRegistry().contains(selection.m_Handle))
			{
				KG_WARN("Could not find on Project Enum in Project Enum editor");
				return;
			}

			OnOpenProjectEnum(selection.m_Handle);
		};

		m_SelectProjectEnumNameSpec.m_Label = "New Name";
		m_SelectProjectEnumNameSpec.m_CurrentOption = "Empty";

		m_SelectProjectEnumLocationSpec.m_Label = "Location";
		m_SelectProjectEnumLocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveContext().GetProjectPaths().GetAssetDirectory();
		m_SelectProjectEnumLocationSpec.m_ConfirmAction = [&](std::string_view path)
			{
				if (!Utility::FileSystem::DoesPathContainSubPath(Projects::ProjectService::GetActiveContext().GetProjectPaths().GetAssetDirectory(), path))
				{
					KG_WARN("Cannot create an asset outside of the project's asset directory.");
					m_SelectProjectEnumLocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveContext().GetProjectPaths().GetAssetDirectory();
				}
			};

		m_CreateProjectEnumPopupSpec.m_Label = "Create Enum";
		m_CreateProjectEnumPopupSpec.m_ConfirmAction = [&]()
			{
				if (m_SelectProjectEnumNameSpec.m_CurrentOption == "")
				{
					return;
				}

				for (auto& [id, asset] : Assets::AssetService::GetProjectEnumRegistry())
				{
					if (asset.Data.GetSpecificMetaData<Assets::ProjectEnumMetaData>()->Name == m_SelectProjectEnumNameSpec.m_CurrentOption)
					{
						return;
					}
				}
				m_EditorProjectEnumHandle = Assets::AssetService::CreateProjectEnum(m_SelectProjectEnumNameSpec.m_CurrentOption.c_str(), m_SelectProjectEnumLocationSpec.m_CurrentOption);
				m_EditorProjectEnum = Assets::AssetService::GetProjectEnum(m_EditorProjectEnumHandle);
				m_MainHeader.m_EditColorActive = false;
				m_MainHeader.m_Label = Assets::AssetService::GetProjectEnumRegistry().at(
					m_EditorProjectEnumHandle).Data.FileLocation.filename().string();
				m_EnumDataTable.m_OnRefresh();
			};
		m_CreateProjectEnumPopupSpec.m_PopupContents = [&]()
			{
				m_SelectProjectEnumNameSpec.RenderText();
				m_SelectProjectEnumLocationSpec.RenderChooseDir();
			};
	}


	void ProjectEnumPanel::InitializeDisplayProjectEnumScreen()
	{
		// Header (Project Enum Name and Options)
		m_DeleteProjectEnumWarning.m_Label = "Delete Enum";
		m_DeleteProjectEnumWarning.m_ConfirmAction = [&]()
			{
				Assets::AssetService::DeleteProjectEnum(m_EditorProjectEnumHandle);
				m_EditorProjectEnumHandle = 0;
				m_EditorProjectEnum = nullptr;
			};
		m_DeleteProjectEnumWarning.m_PopupContents = [&]()
			{
				EditorUI::EditorUIContext::Text("Are you sure you want to delete this Project Enum object?");
			};

		m_CloseProjectEnumWarning.m_Label = "Close Enum";
		m_CloseProjectEnumWarning.m_ConfirmAction = [&]()
			{
				m_EditorProjectEnumHandle = 0;
				m_EditorProjectEnum = nullptr;
			};
		m_CloseProjectEnumWarning.m_PopupContents = [&]()
			{
				EditorUI::EditorUIContext::Text("Are you sure you want to close this enum object without saving?");
			};

		m_MainHeader.AddToSelectionList("Save", [&]()
			{
				Assets::AssetService::SaveProjectEnum(m_EditorProjectEnumHandle, m_EditorProjectEnum);
				m_MainHeader.m_EditColorActive = false;
			});
		m_MainHeader.AddToSelectionList("Close", [&]()
			{
				if (m_MainHeader.m_EditColorActive)
				{
					m_CloseProjectEnumWarning.m_OpenPopup = true;
				}
				else
				{
					m_EditorProjectEnumHandle = 0;
					m_EditorProjectEnum = nullptr;
				}
			});
		m_MainHeader.AddToSelectionList("Delete", [&]()
			{
				m_DeleteProjectEnumWarning.m_OpenPopup = true;
			});


		// Initialize enumeration table
		m_EnumDataTable.m_Label = "Identifiers";
		m_EnumDataTable.m_Expanded = true;
		m_EnumDataTable.m_OnRefresh = [&]()
		{
			m_EnumDataTable.ClearList();
			if (m_EditorProjectEnum)
			{
				size_t iteration{ 0 };
				for (FixedString32& enumeration : m_EditorProjectEnum->m_EnumIdentifiers)
				{
					m_EnumDataTable.InsertListEntry(enumeration.CString(),
						std::to_string(iteration),
						[&](EditorUI::ListEntry& entry, std::size_t listIteration)
						{
							UNREFERENCED_PARAMETER(entry);
							m_CurrentEnumeration = listIteration;

							// Initialize tooltip
							m_EnumTooltip.ClearEntries();
							
							// Initialize rename option
							EditorUI::TooltipEntry renameEntry{ "Rename", [&](EditorUI::TooltipEntry& entry) 
							{
								UNREFERENCED_PARAMETER(entry);

								m_EditIdentifierSpec.m_CurrentOption = m_EditorProjectEnum->m_EnumIdentifiers.at(m_CurrentEnumeration);

								m_EditIdentifierSpec.m_StartPopup = true;
							}};
							m_EnumTooltip.AddTooltipEntry(renameEntry);

							// Initialize Delete option
							EditorUI::TooltipEntry DeleteEntry{ "Delete", [&](EditorUI::TooltipEntry& entry)
							{
								UNREFERENCED_PARAMETER(entry);
								m_DeleteIdentifierWarning.m_OpenPopup = true;
							} };
							m_EnumTooltip.AddTooltipEntry(DeleteEntry);

							// Open tooltip
							m_EnumTooltip.m_TooltipActive = true;
						}, iteration);
					iteration++;
				}
			}
			};
		m_EnumDataTable.m_Column1Title = "Identifier";
		m_EnumDataTable.m_Column2Title = "Value";
		m_EnumDataTable.AddToSelectionList("Add New Identifier", [&]()
		{
			m_AddIdentifierSpec.m_StartPopup = true;
			m_AddIdentifierSpec.m_CurrentOption = "newIdentifier";
		});
		
		// Create enum slot widget
		m_AddIdentifierSpec.m_Label = "Add Identifier";
		m_AddIdentifierSpec.m_Flags |= EditorUI::EditTextFlags::EditText_PopupOnly;
		m_AddIdentifierSpec.m_ConfirmAction = [&](EditorUI::EditTextSpec& spec) 
		{
			// Ensure an identical enumeration name does not exist
			if (ProjectData::ProjectEnumService::DoesProjectEnumContainIdentifier(m_EditorProjectEnum.get(), spec.m_CurrentOption.c_str()))
			{
				KG_WARN("Duplicate enum identifier found");
				return;
			}

			// Add new item to enum at the end
			m_EditorProjectEnum->m_EnumIdentifiers.emplace_back(spec.m_CurrentOption.c_str());

			// Revalidate the table
			OnRefreshData();

			// Set the enum as modified
			m_MainHeader.m_EditColorActive = true;
		};

		// Create enum slot widget
		m_EditIdentifierSpec.m_Label = "Edit Identifier";
		m_EditIdentifierSpec.m_Flags |= EditorUI::EditTextFlags::EditText_PopupOnly;
		m_EditIdentifierSpec.m_ConfirmAction = [&](EditorUI::EditTextSpec& spec)
		{
			// Ensure an identical enumeration name does not exist
			if (!ProjectData::ProjectEnumService::RenameIdentifier(m_EditorProjectEnum.get(), m_CurrentEnumeration ,spec.m_CurrentOption.c_str()))
			{
				KG_WARN("Failed to rename identifier in ProjectEnumPanel");
				return;
			}

			// Revalidate the table
			OnRefreshData();

			// Set the enum as modified
			m_MainHeader.m_EditColorActive = true;
		};

		m_DeleteIdentifierWarning.m_Label = "Delete Identifier";
		m_DeleteIdentifierWarning.m_ConfirmAction = [&]()
		{
			// Ensure an identical enumeration name does not exist
			if (!ProjectData::ProjectEnumService::RemoveIdentifier(m_EditorProjectEnum.get(), m_CurrentEnumeration))
			{
				KG_WARN("Failed to delete identifier in ProjectEnumPanel");
				return;
			}

			// Revalidate the table
			OnRefreshData();

			// Set the enum as modified
			m_MainHeader.m_EditColorActive = true;
		};
		m_DeleteIdentifierWarning.m_PopupContents = [&]()
		{
			EditorUI::EditorUIContext::Text("Are you sure you want to delete this identifier?");
		};
	}

	ProjectEnumPanel::ProjectEnumPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
		s_MainWindow->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(ProjectEnumPanel::OnKeyPressedEditor));

		InitializeOpeningScreen();
		InitializeDisplayProjectEnumScreen();
	}
	void ProjectEnumPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIContext::StartRenderWindow(m_PanelName, &s_MainWindow->m_ShowProjectEnum);

		if (!EditorUI::EditorUIContext::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIContext::EndRenderWindow();
			return;
		}

		if (!m_EditorProjectEnum)
		{

			EditorUI::EditorUIContext::NewItemScreen("Open Existing Enum", KG_BIND_CLASS_FN(OnOpenProjectEnumDialog), "Create New Enum", KG_BIND_CLASS_FN(OnCreateProjectEnumDialog));
			m_CreateProjectEnumPopupSpec.RenderPopup();
			m_OpenProjectEnumPopupSpec.RenderOptions();
		}
		else
		{
			m_MainHeader.RenderHeader();
			m_DeleteProjectEnumWarning.RenderPopup();
			m_CloseProjectEnumWarning.RenderPopup();
			m_EnumDataTable.RenderList();
			m_AddIdentifierSpec.RenderText();
			m_EditIdentifierSpec.RenderText();
			m_DeleteIdentifierWarning.RenderPopup();
			m_EnumTooltip.RenderTooltip();
		}

		EditorUI::EditorUIContext::EndRenderWindow();
	}
	bool ProjectEnumPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
	bool ProjectEnumPanel::OnAssetEvent(Events::Event* event)
	{
		// Validate event type and asset type
		if (event->GetEventType() != Events::EventType::ManageAsset)
		{
			return false;
		}
		Events::ManageAsset* manageAsset = (Events::ManageAsset*)event;
		if (manageAsset->GetAssetType() != Assets::AssetType::ProjectEnum)
		{
			return false;
		}

		if (manageAsset->GetAssetID() != m_EditorProjectEnumHandle)
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
			// Update Project Enum header if necessary
			m_MainHeader.m_Label = Assets::AssetService::GetProjectEnumFileLocation(manageAsset->GetAssetID()).filename().string();

			return true;
		}
		return false;
	}
	void ProjectEnumPanel::ResetPanelResources()
	{
		m_EditorProjectEnum = nullptr;
		m_EditorProjectEnumHandle = Assets::EmptyHandle;
	}
	void ProjectEnumPanel::OpenCreateDialog(std::filesystem::path& createLocation)
	{

		// Open Project Enum Window
		s_MainWindow->m_ShowProjectEnum = true;
		EditorUI::EditorUIContext::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIContext::SetFocusedWindow(m_PanelName);

		if (!m_EditorProjectEnum)
		{
			// Open dialog to create editor Project Enum
			OnCreateProjectEnumDialog();
			m_SelectProjectEnumLocationSpec.m_CurrentOption = createLocation;
		}
		else
		{
			// Add warning to close active Project Enum before creating a new Project Enum
			s_MainWindow->OpenWarningMessage("A enum is already active inside the editor. Please close the current Project Enum before creating a new one.");
		}
	}

	void ProjectEnumPanel::OpenAssetInEditor(std::filesystem::path& assetLocation)
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
		Assets::AssetHandle assetHandle = Assets::AssetService::GetProjectEnumHandleFromFileLocation(relativePath);

		// Validate resulting handle
		if (!assetHandle)
		{
			KG_WARN("Could not open asset in editor. Provided path does not result in an asset inside the registry.");
			return;
		}

		// Open the editor panel to be visible
		s_MainWindow->m_ShowProjectEnum = true;
		EditorUI::EditorUIContext::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIContext::SetFocusedWindow(m_PanelName);

		// Early out if asset is already open
		if (m_EditorProjectEnumHandle == assetHandle)
		{
			return;
		}

		// Check if panel is already occupied by an asset
		if (!m_EditorProjectEnum)
		{
			OnOpenProjectEnum(assetHandle);
		}
		else
		{
			// Add warning to close active AI state before opening a new AIState
			s_MainWindow->OpenWarningMessage("An enum is already active inside the editor. Please close the current enum before opening a new one.");
		}
	}

	void ProjectEnumPanel::OnOpenProjectEnumDialog()
	{
		m_OpenProjectEnumPopupSpec.m_OpenPopup = true;
	}
	void ProjectEnumPanel::OnCreateProjectEnumDialog()
	{
		Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };
		m_SelectProjectEnumLocationSpec.m_CurrentOption = projectPaths.GetAssetDirectory();
		m_CreateProjectEnumPopupSpec.m_OpenPopup = true;
	}
	void ProjectEnumPanel::OnRefreshData()
	{
		m_EnumDataTable.m_OnRefresh();
	}
	void ProjectEnumPanel::OnOpenProjectEnum(Assets::AssetHandle newHandle)
	{
		m_EditorProjectEnum = Assets::AssetService::GetProjectEnum(newHandle);
		m_EditorProjectEnumHandle = newHandle;
		m_MainHeader.m_EditColorActive = false;
		m_MainHeader.m_Label = Assets::AssetService::GetProjectEnumRegistry().at(
			m_EditorProjectEnumHandle).Data.FileLocation.filename().string();
		OnRefreshData();
	}
}

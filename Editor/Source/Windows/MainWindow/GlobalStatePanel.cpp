#include "Windows/MainWindow/GlobalStatePanel.h"

#include "EditorApp.h"

static Kargono::EditorApp* s_EditorApp{ nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };

namespace Kargono::Panels
{
	void GlobalStatePanel::InitializeOpeningScreen()
	{
		m_OpenGlobalStatePopupSpec.m_Label = "Open Global State";
		m_OpenGlobalStatePopupSpec.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_OpenGlobalStatePopupSpec.m_Flags |= EditorUI::SelectOption_PopupOnly;
		m_OpenGlobalStatePopupSpec.m_PopupAction = [&](EditorUI::SelectOptionSpec& spec)
			{
				spec.GetAllOptions().clear();
				spec.m_CurrentOption = { "None", Assets::EmptyHandle };

				spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& [handle, asset] : Assets::AssetService::GetGlobalStateRegistry())
				{
					spec.AddToOptions("All Options", asset.Data.FileLocation.filename().string(), handle);
				}
			};

		m_OpenGlobalStatePopupSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
			{
				if (selection.m_Handle == Assets::EmptyHandle)
				{
					KG_WARN("No global state Selected");
					return;
				}
				if (!Assets::AssetService::GetGlobalStateRegistry().contains(selection.m_Handle))
				{
					KG_WARN("Could not find on global state in global state editor");
					return;
				}

				OnOpenGlobalState(selection.m_Handle);
			};

		m_SelectGlobalStateNameSpec.m_Label = "New Name";
		m_SelectGlobalStateNameSpec.m_CurrentOption = "Empty";

		m_SelectGlobalStateLocationSpec.m_Label = "Location";
		m_SelectGlobalStateLocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
		m_SelectGlobalStateLocationSpec.m_ConfirmAction = [&](std::string_view path)
			{
				if (!Utility::FileSystem::DoesPathContainSubPath(Projects::ProjectService::GetActiveAssetDirectory(), path))
				{
					KG_WARN("Cannot create an asset outside of the project's asset directory.");
					m_SelectGlobalStateLocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
				}
			};

		m_CreateGlobalStatePopupSpec.m_Label = "Create Global State";
		m_CreateGlobalStatePopupSpec.m_ConfirmAction = [&]()
			{
				if (m_SelectGlobalStateNameSpec.m_CurrentOption == "")
				{
					return;
				}

				for (auto& [id, asset] : Assets::AssetService::GetGlobalStateRegistry())
				{
					if (asset.Data.GetSpecificMetaData<Assets::GlobalStateMetaData>()->Name == m_SelectGlobalStateNameSpec.m_CurrentOption)
					{
						return;
					}
				}
				m_EditorGlobalStateHandle = Assets::AssetService::CreateGlobalState(m_SelectGlobalStateNameSpec.m_CurrentOption.c_str(), m_SelectGlobalStateLocationSpec.m_CurrentOption);
				m_EditorGlobalState = Assets::AssetService::GetGlobalState(m_EditorGlobalStateHandle);
				m_MainHeader.m_EditColorActive = false;
				m_MainHeader.m_Label = Assets::AssetService::GetGlobalStateRegistry().at(
					m_EditorGlobalStateHandle).Data.FileLocation.filename().string();
			};
		m_CreateGlobalStatePopupSpec.m_PopupContents = [&]()
			{
				EditorUI::EditorUIService::EditText(m_SelectGlobalStateNameSpec);
				EditorUI::EditorUIService::ChooseDirectory(m_SelectGlobalStateLocationSpec);
			};
	}


	void GlobalStatePanel::InitializeDisplayGlobalStateScreen()
	{
		// Header (global state Name and Options)
		m_DeleteGlobalStateWarning.m_Label = "Delete Global State";
		m_DeleteGlobalStateWarning.m_ConfirmAction = [&]()
			{
				Assets::AssetService::DeleteGlobalState(m_EditorGlobalStateHandle);
				m_EditorGlobalStateHandle = 0;
				m_EditorGlobalState = nullptr;
			};
		m_DeleteGlobalStateWarning.m_PopupContents = [&]()
			{
				EditorUI::EditorUIService::Text("Are you sure you want to delete this global state object?");
			};

		m_CloseGlobalStateWarning.m_Label = "Close Global State";
		m_CloseGlobalStateWarning.m_ConfirmAction = [&]()
			{
				m_EditorGlobalStateHandle = 0;
				m_EditorGlobalState = nullptr;
			};
		m_CloseGlobalStateWarning.m_PopupContents = [&]()
			{
				EditorUI::EditorUIService::Text("Are you sure you want to close this Global State object without saving?");
			};

		m_MainHeader.AddToSelectionList("Save", [&]()
			{
				Assets::AssetService::SaveGlobalState(m_EditorGlobalStateHandle, m_EditorGlobalState);
				m_MainHeader.m_EditColorActive = false;
			});
		m_MainHeader.AddToSelectionList("Close", [&]()
			{
				if (m_MainHeader.m_EditColorActive)
				{
					m_CloseGlobalStateWarning.m_OpenPopup = true;
				}
				else
				{
					m_EditorGlobalStateHandle = 0;
					m_EditorGlobalState = nullptr;
				}
			});
		m_MainHeader.AddToSelectionList("Delete", [&]()
			{
				m_DeleteGlobalStateWarning.m_OpenPopup = true;
			});
	}

	GlobalStatePanel::GlobalStatePanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
		s_MainWindow->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(GlobalStatePanel::OnKeyPressedEditor));

		InitializeOpeningScreen();
		InitializeDisplayGlobalStateScreen();
	}
	void GlobalStatePanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_MainWindow->m_ShowGlobalStateEditor);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		if (!m_EditorGlobalState)
		{

			EditorUI::EditorUIService::NewItemScreen("Open Existing Global State", KG_BIND_CLASS_FN(OnOpenGlobalStateDialog), "Create New Global State", KG_BIND_CLASS_FN(OnCreateGlobalStateDialog));
			EditorUI::EditorUIService::GenericPopup(m_CreateGlobalStatePopupSpec);
			EditorUI::EditorUIService::SelectOption(m_OpenGlobalStatePopupSpec);
		}
		else
		{
			EditorUI::EditorUIService::PanelHeader(m_MainHeader);
			EditorUI::EditorUIService::GenericPopup(m_DeleteGlobalStateWarning);
			EditorUI::EditorUIService::GenericPopup(m_CloseGlobalStateWarning);
		}

		EditorUI::EditorUIService::EndWindow();
	}
	bool GlobalStatePanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
	bool GlobalStatePanel::OnAssetEvent(Events::Event* event)
	{
		// Validate event type and asset type
		if (event->GetEventType() != Events::EventType::ManageAsset)
		{
			return false;
		}
		Events::ManageAsset* manageAsset = (Events::ManageAsset*)event;
		if (manageAsset->GetAssetType() != Assets::AssetType::GlobalState)
		{
			return false;
		}

		if (manageAsset->GetAssetID() != m_EditorGlobalStateHandle)
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
			// Update global state header if necessary
			m_MainHeader.m_Label = Assets::AssetService::GetGlobalStateFileLocation(manageAsset->GetAssetID()).filename().string();

			return true;
		}
		return false;
	}
	void GlobalStatePanel::ResetPanelResources()
	{
		m_EditorGlobalState = nullptr;
		m_EditorGlobalStateHandle = Assets::EmptyHandle;
	}
	void GlobalStatePanel::OpenCreateDialog(std::filesystem::path& createLocation)
	{
		// Open global state Window
		s_MainWindow->m_ShowGlobalStateEditor = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);

		if (!m_EditorGlobalState)
		{
			// Open dialog to create editor global state
			OnCreateGlobalStateDialog();
			m_SelectGlobalStateLocationSpec.m_CurrentOption = createLocation;
		}
		else
		{
			// Add warning to close active global state before creating a new global state
			s_MainWindow->OpenWarningMessage("A Global State is already active inside the editor. Please close the current global state before creating a new one.");
		}
	}

	void GlobalStatePanel::OpenAssetInEditor(std::filesystem::path& assetLocation)
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
		Assets::AssetHandle assetHandle = Assets::AssetService::GetGlobalStateHandleFromFileLocation(relativePath);

		// Validate resulting handle
		if (!assetHandle)
		{
			KG_WARN("Could not open asset in editor. Provided path does not result in an asset inside the registry.");
			return;
		}

		// Open the editor panel to be visible
		s_MainWindow->m_ShowGlobalStateEditor = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);

		// Early out if asset is already open
		if (m_EditorGlobalStateHandle == assetHandle)
		{
			return;
		}

		// Check if panel is already occupied by an asset
		if (!m_EditorGlobalState)
		{
			OnOpenGlobalState(assetHandle);
		}
		else
		{
			// Add warning to close active AI state before opening a new AIState
			s_MainWindow->OpenWarningMessage("An Global State is already active inside the editor. Please close the current Global State before opening a new one.");
		}
	}

	void GlobalStatePanel::OnOpenGlobalStateDialog()
	{
		m_OpenGlobalStatePopupSpec.m_OpenPopup = true;
	}
	void GlobalStatePanel::OnCreateGlobalStateDialog()
	{
		KG_ASSERT(Projects::ProjectService::GetActive());
		m_SelectGlobalStateLocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
		m_CreateGlobalStatePopupSpec.m_OpenPopup = true;
	}
	void GlobalStatePanel::OnRefreshData()
	{

	}
	void GlobalStatePanel::OnOpenGlobalState(Assets::AssetHandle newHandle)
	{
		m_EditorGlobalState = Assets::AssetService::GetGlobalState(newHandle);
		m_EditorGlobalStateHandle = newHandle;
		m_MainHeader.m_EditColorActive = false;
		m_MainHeader.m_Label = Assets::AssetService::GetGlobalStateRegistry().at(
			m_EditorGlobalStateHandle).Data.FileLocation.filename().string();
		OnRefreshData();
	}
}

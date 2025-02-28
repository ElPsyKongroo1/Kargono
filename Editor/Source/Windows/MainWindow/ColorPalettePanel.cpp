#include "Windows/MainWindow/ColorPalettePanel.h"

#include "EditorApp.h"

static Kargono::EditorApp* s_EditorApp{ nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };

namespace Kargono::Panels
{
	void ColorPalettePanel::InitializeOpeningScreen()
	{
		m_OpenColorPalettePopup.m_Label = "Open Color Palette";
		m_OpenColorPalettePopup.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_OpenColorPalettePopup.m_Flags |= EditorUI::SelectOption_PopupOnly;
		m_OpenColorPalettePopup.m_PopupAction = [&](EditorUI::SelectOptionSpec& spec)
			{
				spec.GetAllOptions().clear();
				spec.m_CurrentOption = { "None", Assets::EmptyHandle };

				spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& [handle, asset] : Assets::AssetService::GetColorPaletteRegistry())
				{
					spec.AddToOptions("All Options", asset.Data.FileLocation.filename().string(), handle);
				}
			};

		m_OpenColorPalettePopup.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
			{
				if (selection.m_Handle == Assets::EmptyHandle)
				{
					KG_WARN("No Color Palette Selected");
					return;
				}
				if (!Assets::AssetService::GetColorPaletteRegistry().contains(selection.m_Handle))
				{
					KG_WARN("Could not find on Color Palette in Color Palette editor");
					return;
				}

				OnOpenColorPalette(selection.m_Handle);
			};

		m_SelectColorPaletteNameSpec.m_Label = "New Name";
		m_SelectColorPaletteNameSpec.m_CurrentOption = "Empty";

		m_SelectColorPaletteLocationSpec.m_Label = "Location";
		m_SelectColorPaletteLocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
		m_SelectColorPaletteLocationSpec.m_ConfirmAction = [&](std::string_view path)
			{
				if (!Utility::FileSystem::DoesPathContainSubPath(Projects::ProjectService::GetActiveAssetDirectory(), path))
				{
					KG_WARN("Cannot create an asset outside of the project's asset directory.");
					m_SelectColorPaletteLocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
				}
			};

		m_CreateColorPalettePopup.m_Label = "Create Color Palette";
		m_CreateColorPalettePopup.m_ConfirmAction = [&]()
			{
				if (m_SelectColorPaletteNameSpec.m_CurrentOption == "")
				{
					return;
				}

				for (auto& [id, asset] : Assets::AssetService::GetColorPaletteRegistry())
				{
					if (asset.Data.GetSpecificMetaData<Assets::ColorPaletteMetaData>()->Name == m_SelectColorPaletteNameSpec.m_CurrentOption)
					{
						return;
					}
				}
				m_EditorColorPaletteHandle = Assets::AssetService::CreateColorPalette(m_SelectColorPaletteNameSpec.m_CurrentOption.c_str(), m_SelectColorPaletteLocationSpec.m_CurrentOption);
				m_EditorColorPalette = Assets::AssetService::GetColorPalette(m_EditorColorPaletteHandle);
				m_MainHeader.m_EditColorActive = false;
				m_MainHeader.m_Label = Assets::AssetService::GetColorPaletteRegistry().at(
					m_EditorColorPaletteHandle).Data.FileLocation.filename().string();
			};
		m_CreateColorPalettePopup.m_PopupContents = [&]()
			{
				EditorUI::EditorUIService::EditText(m_SelectColorPaletteNameSpec);
				EditorUI::EditorUIService::ChooseDirectory(m_SelectColorPaletteLocationSpec);
			};
	}


	void ColorPalettePanel::InitializeDisplayColorPaletteScreen()
	{
		// Header (Color Palette Name and Options)
		m_DeleteColorPaletteWarning.m_Label = "Delete Color Palette";
		m_DeleteColorPaletteWarning.m_ConfirmAction = [&]()
			{
				Assets::AssetService::DeleteColorPalette(m_EditorColorPaletteHandle);

				ResetPanelResources();
			};
		m_DeleteColorPaletteWarning.m_PopupContents = [&]()
			{
				EditorUI::EditorUIService::Text("Are you sure you want to delete this Color Palette object?");
			};

		m_CloseColorPaletteWarning.m_Label = "Close Color Palette";
		m_CloseColorPaletteWarning.m_ConfirmAction = [&]()
			{
				ResetPanelResources();
			};
		m_CloseColorPaletteWarning.m_PopupContents = [&]()
			{
				EditorUI::EditorUIService::Text("Are you sure you want to close this Color Palette object without saving?");
			};

		m_MainHeader.AddToSelectionList("Save", [&]()
			{
				Assets::AssetService::SaveColorPalette(m_EditorColorPaletteHandle, m_EditorColorPalette);
				m_MainHeader.m_EditColorActive = false;
			});
		m_MainHeader.AddToSelectionList("Close", [&]()
			{
				if (m_MainHeader.m_EditColorActive)
				{
					m_CloseColorPaletteWarning.m_OpenPopup = true;
				}
				else
				{
					ResetPanelResources();
				}
			});
		m_MainHeader.AddToSelectionList("Delete", [&]()
			{
				m_DeleteColorPaletteWarning.m_OpenPopup = true;
			});
	}

	ColorPalettePanel::ColorPalettePanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
		s_MainWindow->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(ColorPalettePanel::OnKeyPressedEditor));

		InitializeOpeningScreen();
		InitializeDisplayColorPaletteScreen();
	}
	void ColorPalettePanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_MainWindow->m_ShowColorPalette);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		if (!m_EditorColorPalette)
		{

			EditorUI::EditorUIService::NewItemScreen("Open Existing Color Palette", KG_BIND_CLASS_FN(OnOpenColorPaletteDialog), "Create New Color Palette", KG_BIND_CLASS_FN(OnCreateColorPaletteDialog));
			EditorUI::EditorUIService::GenericPopup(m_CreateColorPalettePopup);
			EditorUI::EditorUIService::SelectOption(m_OpenColorPalettePopup);
		}
		else
		{
			EditorUI::EditorUIService::PanelHeader(m_MainHeader);
			EditorUI::EditorUIService::GenericPopup(m_DeleteColorPaletteWarning);
			EditorUI::EditorUIService::GenericPopup(m_CloseColorPaletteWarning);
		}

		EditorUI::EditorUIService::EndWindow();
	}
	bool ColorPalettePanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
	bool ColorPalettePanel::OnAssetEvent(Events::Event* event)
	{
		// Validate event type and asset type
		if (event->GetEventType() != Events::EventType::ManageAsset)
		{
			return false;
		}
		Events::ManageAsset* manageAsset = (Events::ManageAsset*)event;
		if (manageAsset->GetAssetType() != Assets::AssetType::ColorPalette)
		{
			return false;
		}

		if (manageAsset->GetAssetID() != m_EditorColorPaletteHandle)
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
			// Update Color Palette header if necessary
			m_MainHeader.m_Label = Assets::AssetService::GetColorPaletteFileLocation(manageAsset->GetAssetID()).filename().string();

			return true;
		}
		return false;
	}
	void ColorPalettePanel::ResetPanelResources()
	{
		EngineService::SubmitToMainThread([&]()
			{
				m_EditorColorPalette = nullptr;
				m_EditorColorPaletteHandle = Assets::EmptyHandle;
			});
	}
	void ColorPalettePanel::OpenCreateDialog(std::filesystem::path& createLocation)
	{
		// Open Color Palette Window
		s_MainWindow->m_ShowColorPalette = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);

		if (!m_EditorColorPalette)
		{
			// Open dialog to create editor Color Palette
			OnCreateColorPaletteDialog();
			m_SelectColorPaletteLocationSpec.m_CurrentOption = createLocation;
		}
		else
		{
			// Add warning to close active Color Palette before creating a new Color Palette
			s_MainWindow->OpenWarningMessage("A Color Palette is already active inside the editor. Please close the current Color Palette before creating a new one.");
		}
	}

	void ColorPalettePanel::OpenAssetInEditor(std::filesystem::path& assetLocation)
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
		Assets::AssetHandle assetHandle = Assets::AssetService::GetColorPaletteHandleFromFileLocation(relativePath);

		// Validate resulting handle
		if (!assetHandle)
		{
			KG_WARN("Could not open asset in editor. Provided path does not result in an asset inside the registry.");
			return;
		}

		// Open the editor panel to be visible
		s_MainWindow->m_ShowColorPalette = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);

		// Early out if asset is already open
		if (m_EditorColorPaletteHandle == assetHandle)
		{
			return;
		}

		// Check if panel is already occupied by an asset
		if (!m_EditorColorPalette)
		{
			OnOpenColorPalette(assetHandle);
		}
		else
		{
			// Add warning to close active AI state before opening a new AIState
			s_MainWindow->OpenWarningMessage("An Color Palette is already active inside the editor. Please close the current Color Palette before opening a new one.");
		}
	}

	void ColorPalettePanel::DrawColorPaletteColors()
	{
	}

	void ColorPalettePanel::OnOpenColorPaletteDialog()
	{
		m_OpenColorPalettePopup.m_OpenPopup = true;
	}
	void ColorPalettePanel::OnCreateColorPaletteDialog()
	{
		KG_ASSERT(Projects::ProjectService::GetActive());
		m_SelectColorPaletteLocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
		m_CreateColorPalettePopup.m_OpenPopup = true;
	}
	void ColorPalettePanel::OnModifyColor(EditorUI::EditVec4Spec& spec)
	{
	}
	void ColorPalettePanel::OnRefreshData()
	{
	}
	void ColorPalettePanel::OnOpenColorPalette(Assets::AssetHandle newHandle)
	{
		m_EditorColorPalette = Assets::AssetService::GetColorPalette(newHandle);
		m_EditorColorPaletteHandle = newHandle;
		m_MainHeader.m_EditColorActive = false;
		m_MainHeader.m_Label = Assets::AssetService::GetColorPaletteRegistry().at(
			m_EditorColorPaletteHandle).Data.FileLocation.filename().string();
		OnRefreshData();
	}
}

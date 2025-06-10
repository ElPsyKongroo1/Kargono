#include "Windows/MainWindow/ColorPalettePanel.h"

#include "Kargono/Utility/Operations.h"

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
		m_OpenColorPalettePopup.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
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

		Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

		m_SelectColorPaletteNameSpec.m_Label = "New Name";
		m_SelectColorPaletteNameSpec.m_CurrentOption = "Empty";

		m_SelectColorPaletteLocationSpec.m_Label = "Location";
		m_SelectColorPaletteLocationSpec.m_CurrentOption = projectPaths.GetAssetDirectory();
		m_SelectColorPaletteLocationSpec.m_ConfirmAction = [&](std::string_view path)
		{
			if (!Utility::FileSystem::DoesPathContainSubPath(projectPaths.GetAssetDirectory(), path))
			{
				KG_WARN("Cannot create an asset outside of the project's asset directory.");
				m_SelectColorPaletteLocationSpec.m_CurrentOption = projectPaths.GetAssetDirectory();
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
			m_SelectColorPaletteNameSpec.RenderText();
			m_SelectColorPaletteLocationSpec.RenderChooseDir();
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

		m_MainHeader.AddToSelectionList("Add White", KG_BIND_CLASS_FN(OnAddWhite));
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
		m_EditColorName.m_Label = "Edit Color Name";
		m_EditColorName.m_Flags |= EditorUI::EditText_PopupOnly;
		m_EditColorName.m_CurrentOption = "None";
		m_EditColorName.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyColorName);
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
			m_CreateColorPalettePopup.RenderPopup();
			m_OpenColorPalettePopup.RenderOptions();
		}
		else
		{
			m_MainHeader.RenderHeader();
			DrawColorPaletteColors();
			m_EditColorName.RenderText();
			m_DeleteColorPaletteWarning.RenderPopup();
			m_CloseColorPaletteWarning.RenderPopup();
			m_LocalTooltip.RenderTooltip();
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
		EngineService::GetActiveEngine().GetThread().SubmitFunction([&]()
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
		Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

		// Ensure provided path is within the active asset directory
		std::filesystem::path activeAssetDirectory = projectPaths.GetAssetDirectory();
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
		for (EditorUI::EditVec4Widget& colorEditor : m_ColorEditorWidgets)
		{
			EditorUI::EditorUIService::EditVec4(colorEditor);
		}
	}

	void ColorPalettePanel::OnOpenColorPaletteDialog()
	{
		m_OpenColorPalettePopup.m_OpenPopup = true;
	}
	void ColorPalettePanel::OnCreateColorPaletteDialog()
	{
		Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

		m_SelectColorPaletteLocationSpec.m_CurrentOption = projectPaths.GetAssetDirectory();
		m_CreateColorPalettePopup.m_OpenPopup = true;
	}
	void ColorPalettePanel::OnModifyColor(EditorUI::EditVec4Widget& spec)
	{
		// Ensure the correct requirements are provided
		KG_ASSERT(m_EditorColorPalette);
		KG_ASSERT(spec.m_ProvidedData);

		// Get the color index
		size_t colorIndex = *(size_t*)spec.m_ProvidedData.get();

		// Ensure the index is valid
		KG_ASSERT(colorIndex < m_EditorColorPalette->m_Colors.size());

		// Get the indicated color object
		ProjectData::Color& indicatedColor = m_EditorColorPalette->m_Colors.at(colorIndex);

		// Modify the color value
		indicatedColor.m_HexCode = Utility::RGBAToHex(spec.m_CurrentVec4);

		m_MainHeader.m_EditColorActive = true;
	}
	void ColorPalettePanel::OnModifyColorName(EditorUI::EditTextSpec& spec)
	{
		// Ensure the correct requirements are provided
		KG_ASSERT(m_EditorColorPalette);
		KG_ASSERT(spec.m_ProvidedData);

		// Get the color index
		size_t colorIndex = *(size_t*)spec.m_ProvidedData.get();

		// Ensure the index is valid
		KG_ASSERT(colorIndex < m_EditorColorPalette->m_Colors.size());

		// Get the indicated color object
		ProjectData::Color& indicatedColor = m_EditorColorPalette->m_Colors.at(colorIndex);

		// Modify the color value
		indicatedColor.m_Name = spec.m_CurrentOption;

		// Update the table data
		OnRefreshData();
		m_MainHeader.m_EditColorActive = true;
	}
	void ColorPalettePanel::OnOpenEditTooltip(EditorUI::EditVec4Widget& spec)
	{
		m_LocalTooltip.ClearEntries();

		// Add entry to open an edit color name dialog
		EditorUI::TooltipEntry editColorName{ "Edit Name", [&](EditorUI::TooltipEntry& entry) 
		{
			// Get the location of the color in the color palette
			size_t colorLocation = entry.m_UserHandle;

			// Get the indicated color
			KG_ASSERT(colorLocation < m_EditorColorPalette->m_Colors.size());
			ProjectData::Color& color = m_EditorColorPalette->m_Colors[colorLocation];
			
			// Set-up the edit color text pop-up
			m_EditColorName.m_CurrentOption = color.m_Name;
			m_EditColorName.m_ProvidedData = CreateRef<size_t>(colorLocation);
			m_EditColorName.m_StartPopup = true;
		}};

		KG_ASSERT(spec.m_ProvidedData);
		editColorName.m_UserHandle = *(size_t*)spec.m_ProvidedData.get();
		m_LocalTooltip.AddTooltipEntry(editColorName);

		// Add entry to open modification of the color
		EditorUI::TooltipEntry editColor{ spec.m_Editing ? "Cancel Edit Color" : "Edit Color",
			[&](EditorUI::TooltipEntry& entry)
		{
			// Get the indicated color editing widget
			KG_ASSERT(entry.m_UserHandle < m_ColorEditorWidgets.size());
			EditorUI::EditVec4Widget& colorEditWidget = m_ColorEditorWidgets[entry.m_UserHandle];
			// Toggle the widget to allow/disallow editing of the vec4
			Utility::Operations::ToggleBoolean(colorEditWidget.m_Editing);
		} };
		KG_ASSERT(spec.m_ProvidedData);
		editColor.m_UserHandle = *(size_t*)spec.m_ProvidedData.get();
		m_LocalTooltip.AddTooltipEntry(editColor);

		// Add entry to delete a color
		EditorUI::TooltipEntry deleteColor{ "Delete Color",
			[&](EditorUI::TooltipEntry& entry)
		{
				// Get the indicated color editing widget
				KG_ASSERT(entry.m_UserHandle < m_ColorEditorWidgets.size());
				
				// Delete the color from the palette
				m_EditorColorPalette->m_Colors.erase(m_EditorColorPalette->m_Colors.begin() + entry.m_UserHandle);

				// Update the table data
				OnRefreshData();
				m_MainHeader.m_EditColorActive = true;
				
		}};
		KG_ASSERT(spec.m_ProvidedData);
		deleteColor.m_UserHandle = *(size_t*)spec.m_ProvidedData.get();
		m_LocalTooltip.AddTooltipEntry(deleteColor);


		// Activate the tooltip!
		m_LocalTooltip.m_TooltipActive = true;
	}
	void ColorPalettePanel::OnAddWhite()
	{
		KG_ASSERT(m_EditorColorPalette);

		// Add the new blank color
		m_EditorColorPalette->m_Colors.emplace_back("White", 0xFF'FF'FF'FF);

		// Refresh the widgets
		OnRefreshData();

		// Set the palette as modified
		m_MainHeader.m_EditColorActive = true;
	}
	void ColorPalettePanel::OnRefreshData()
	{
		KG_ASSERT(m_EditorColorPalette);

		// Reset the color editors
		m_ColorEditorWidgets.clear();

		// Revalidate the widgets
		size_t iteration{ 0 };
		for (const ProjectData::Color& color : m_EditorColorPalette->m_Colors)
		{
			// Create the new color editor
			EditorUI::EditVec4Widget& newColorEditor = m_ColorEditorWidgets.emplace_back();

			// Set up the color editor's values
			newColorEditor.m_Label = color.m_Name;
			newColorEditor.m_Bounds = {0.0f, 1.0f};
			newColorEditor.m_CurrentVec4 = Utility::HexToRGBA(color.m_HexCode);
			newColorEditor.m_OnEdit = KG_BIND_CLASS_FN(OnOpenEditTooltip);
			newColorEditor.m_Flags |= EditorUI::EditVec4_RGBA | 
				EditorUI::EditVec4_HandleEditButtonExternally;
			newColorEditor.m_ProvidedData = CreateRef<size_t>(iteration); // Use iteration to identify this color
			newColorEditor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyColor);

			// Continue to next color
			iteration++;
		}
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

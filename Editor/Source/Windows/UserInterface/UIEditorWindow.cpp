#include "Windows/UserInterface/UIEditorWindow.h"

#include "EditorApp.h"

#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp { nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };

namespace Kargono::Windows
{
	void UIEditorWindow::OpenCreateDialog(std::filesystem::path& createLocation)
	{
		// Open main user interface editor panel
		// TODO: Open UI EDITOR PANEL
		//s_MainWindow->m_ShowUserInterfaceEditor = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);

		// Check if panel is already occupied by an asset
		if (!m_EditorUI)
		{
			// Open dialog to create editor user interface
			OnCreateUIDialog();
			m_SelectUILocationSpec.m_CurrentOption = createLocation;
		}
		else
		{
			// Add warning to close active user interface before creating a new user interface
			s_MainWindow->OpenWarningMessage("A user interface is already active inside the editor. Please close the current user interface before creating a new one.");
		}
	}
	void UIEditorWindow::ResetPanelResources()
	{
		// Reset editor user interface
		m_EditorUIHandle = 0;
		m_EditorUI = nullptr;

		// Reset properties panel data
		ClearPropertiesPanelData();
	}

	void UIEditorWindow::OnOpenUIDialog()
	{
		// Set dialog popup to open on next frame
		m_OpenUIPopupSpec.m_OpenPopup = true;
	}
	void UIEditorWindow::OnCreateUIDialog()
	{
		// Set default values for new user interface creation location
		KG_ASSERT(Projects::ProjectService::GetActive());
		m_SelectUILocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();

		// Set dialog popup to open on next frame
		m_CreateUIPopupSpec.m_OpenPopup = true;
	}
	void UIEditorWindow::OnOpenUI(Assets::AssetHandle newHandle)
	{
		// Set new in editor user interface
		m_EditorUI = Assets::AssetService::GetUserInterface(newHandle);
		m_EditorUIHandle = newHandle;

		// Set default values for header
		m_MainHeader.m_EditColorActive = false;
		m_MainHeader.m_Label = Assets::AssetService::GetUserInterfaceRegistry().at(
			m_EditorUIHandle).Data.FileLocation.filename().string();

		// Refresh widget data in editor to use new user interface
		OnRefreshData();

		// Set editor user interface as active in runtime
		RuntimeUI::RuntimeUIService::SetActiveUI(m_EditorUI, m_EditorUIHandle);
	}
	void UIEditorWindow::OnRefreshData()
	{
		// Revalidate data with current user interface
		m_UITree.m_OnRefresh();
	}

	void UIEditorWindow::OnRefreshUITree()
	{
		// Ensure the editor UI is valid
		if (!m_EditorUI)
		{
			KG_WARN("Attempt to load table without valid m_EditorUI");
			return;
		}

		// Clear the tree before adding new entries
		m_UITree.ClearTree();

		// Add all windows and widgets from the editor UI to the tree
		std::size_t windowIterator{ 0 };
		for (RuntimeUI::Window& window : m_EditorUI->m_Windows)
		{
			// Create new window entry
			EditorUI::TreeEntry newEntry{};
			newEntry.m_Label = window.m_Tag;
			newEntry.m_IconHandle = EditorUI::EditorUIService::s_IconWindow;
			newEntry.m_Handle = windowIterator;

			// Add functions to call when interacting with window entry
			newEntry.m_OnLeftClick = KG_BIND_CLASS_FN(SelectWindow);
			newEntry.m_OnRightClickSelection.push_back({ "Delete Window", KG_BIND_CLASS_FN(DeleteWindow) });
			newEntry.m_OnRightClickSelection.push_back({ "Add Text Widget", KG_BIND_CLASS_FN(AddTextWidget) });

			// Add widgets to window entry
			std::size_t widgetIterator{ 0 };
			for (Ref<RuntimeUI::Widget> widget : window.m_Widgets)
			{
				// TODO: Note this only creates text widgets for now

				// Create new widget entry
				EditorUI::TreeEntry newWidgetEntry{};
				newWidgetEntry.m_Label = widget->m_Tag;
				newWidgetEntry.m_IconHandle = EditorUI::EditorUIService::s_IconTextWidget;
				newWidgetEntry.m_ProvidedData = CreateRef<uint32_t>((uint32_t)windowIterator);
				newWidgetEntry.m_Handle = widgetIterator;

				// Add functions to call when interacting with widget entry
				newWidgetEntry.m_OnLeftClick = KG_BIND_CLASS_FN(SelectTextWidget);
				newWidgetEntry.m_OnRightClickSelection.push_back({ "Delete Widget", KG_BIND_CLASS_FN(DeleteWidget) });

				// Add widget entry to window entry
				newEntry.m_SubEntries.push_back(newWidgetEntry);
				widgetIterator++;
			}

			// Add window entry to the user interface tree
			m_UITree.InsertEntry(newEntry);
			windowIterator++;
		}
	}

	void UIEditorWindow::ClearPropertiesPanelData()
	{
		m_ActiveWidget = nullptr;
		m_ActiveWindow = nullptr;
		m_CurrentDisplay = UIPropertiesDisplay::None;
	}

	UIEditorWindow::UIEditorWindow()
	{
		// Set up static editor app reference and register panel to editor app
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();

		// Initialize null state widget data
		InitializeOpeningScreen();

		// Initialize main editor panel resources
		InitializeUIHeader();
		InitializeMainContent();

		// Initialize properties panel widget resources
		InitializeWindowOptions();
		InitializeWidgetOptions();
	}

	void UIEditorWindow::InitPanels()
	{
	}

	bool UIEditorWindow::OnInputEvent(Events::Event* event)
	{
		// Handle key press events for the user interface editor panel
		if (event->GetEventType() == Events::EventType::KeyPressed)
		{
			return OnKeyPressedEditor(*(Events::KeyPressedEvent*)event);
		}

		// Default to not handling the event
		return false;
	}

	void UIEditorWindow::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();

		EditorUI::EditorUIService::StartRendering();
		EditorUI::EditorUIService::StartDockspaceWindow();

		// Set up Menu Toolbar
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::MenuItem("Back to Main Window"))
			{
				EngineService::SubmitToMainThread([]()
				{
					s_EditorApp->SetActiveEditorWindow(ActiveEditorUIWindow::MainWindow);
				});
			}

			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("Engine Docs"))
				{
					Utility::OSCommands::OpenWebURL("https://elpsykongroo1.github.io/Kargono/");
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		// Begin rendering the user interface editor panel
		EditorUI::EditorUIService::StartWindow(m_PanelName, nullptr);

		//// Early out if window is not visible
		//if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		//{
		//	EditorUI::EditorUIService::EndWindow();
		//	return;
		//}

		if (!m_EditorUI)
		{
			// Display opening screen for user interface editor
			EditorUI::EditorUIService::NewItemScreen("Open Existing User Interface", KG_BIND_CLASS_FN(OnOpenUIDialog), "Create New User Interface", KG_BIND_CLASS_FN(OnCreateUIDialog));
			EditorUI::EditorUIService::GenericPopup(m_CreateUIPopupSpec);
			EditorUI::EditorUIService::SelectOption(m_OpenUIPopupSpec);
		}
		else
		{
			// Display user interface editor panel main content
			EditorUI::EditorUIService::PanelHeader(m_MainHeader);
			EditorUI::EditorUIService::GenericPopup(m_DeleteUIWarning);
			EditorUI::EditorUIService::GenericPopup(m_CloseUIWarning);
			EditorUI::EditorUIService::Tree(m_UITree);
			EditorUI::EditorUIService::Tooltip(m_SelectScriptTooltip);
		}

		// Finish rendering the user interface editor panel
		EditorUI::EditorUIService::EndWindow();

		// Clean up dockspace window
		EditorUI::EditorUIService::EndDockspaceWindow();

		// Add highlighting around the focused window
		EditorUI::EditorUIService::HighlightFocusedWindow();

		// End Editor UI Rendering
		EditorUI::EditorUIService::EndRendering();
	}
	bool UIEditorWindow::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		// Handle varios key presses for the user interface editor panel
		switch (event.GetKeyCode())
		{
		// Clear selected entry if escape key is pressed
		case Key::Escape:
			m_UITree.m_SelectedEntry = {};
			m_CurrentDisplay = UIPropertiesDisplay::None;
			return true;
		default:
			return false;
		
		}
	}

	bool UIEditorWindow::OnAssetEvent(Events::Event* event)
	{
		Events::ManageAsset* manageAsset = (Events::ManageAsset*)event;

		// Clear script references from user interface widgets
		if (manageAsset->GetAssetType() == Assets::AssetType::Script &&
			manageAsset->GetAction() == Events::ManageAssetAction::Delete)
		{
			if (m_WidgetOnPress.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_WidgetOnPress.m_CurrentOption = { "None", Assets::EmptyHandle };
			}

			if (m_EditorUI)
			{
				Assets::AssetService::RemoveScriptFromUserInterface(m_EditorUI, manageAsset->GetAssetID());
			}
		}

		// Handle user interface deletion
		if (manageAsset->GetAssetType() == Assets::AssetType::UserInterface &&
			manageAsset->GetAction() == Events::ManageAssetAction::Delete)
		{
			// Check if editor needs modification
			if (manageAsset->GetAssetID() != m_EditorUIHandle)
			{
				return false;
			}

			// Handle deletion of asset
			ResetPanelResources();
			return true;
		}

		// Handle user interface name change
		if (manageAsset->GetAssetType() == Assets::AssetType::UserInterface &&
			manageAsset->GetAction() == Events::ManageAssetAction::UpdateAssetInfo)
		{
			// Check if editor needs modification
			if (manageAsset->GetAssetID() != m_EditorUIHandle)
			{
				return false;
			}

			// Update header name with new asset name
			m_MainHeader.m_Label = Assets::AssetService::GetUserInterfaceFileLocation(manageAsset->GetAssetID()).filename().string();
			return true;
		}

		return false;
	}

	void UIEditorWindow::OpenAssetInEditor(std::filesystem::path& assetLocation)
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
		Assets::AssetHandle assetHandle = Assets::AssetService::GetUserInterfaceHandleFromFileLocation(relativePath);

		// Validate resulting handle
		if (!assetHandle)
		{
			KG_WARN("Could not open asset in editor. Provided path does not result in an asset inside the registry.");
			return;
		}

		// Open the editor panel to be visible
		// TODO: MAKE VISIBLE
		//s_MainWindow->m_ShowUserInterfaceEditor = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);

		// Early out if asset is already open
		if (m_EditorUIHandle == assetHandle)
		{
			return;
		}

		// Check if panel is already occupied by an asset
		if (!m_EditorUI)
		{
			OnOpenUI(assetHandle);
		}
		else
		{
			// Add warning to close active AI state before opening a new AIState
			s_MainWindow->OpenWarningMessage("An user interface is already active inside the editor. Please close the current user interface before opening a new one.");
		}
	}

	void UIEditorWindow::DrawWindowOptions()
	{
		// Draw main header for window options
		EditorUI::EditorUIService::CollapsingHeader(m_WindowHeader);

		// Draw options to edit selected window
		if (m_WindowHeader.m_Expanded)
		{
			// Edit window tag
			m_WindowTag.m_CurrentOption = m_ActiveWindow->m_Tag;
			EditorUI::EditorUIService::EditText(m_WindowTag);

			// Edit default active widget
			std::size_t activeWidget = m_ActiveWindow->m_DefaultActiveWidget;
			m_WindowDefaultWidget.m_CurrentOption =
			{
				activeWidget == -1 ? "None" : m_ActiveWindow->m_Widgets.at(activeWidget)->m_Tag,
				(uint64_t)activeWidget
			};
			EditorUI::EditorUIService::SelectOption(m_WindowDefaultWidget);

			// Edit whether window is displayed
			m_WindowDisplay.m_CurrentBoolean = m_ActiveWindow->GetWindowDisplayed();
			EditorUI::EditorUIService::Checkbox(m_WindowDisplay);

			// Edit window location relative to screen
			m_WindowLocation.m_CurrentVec3 = m_ActiveWindow->m_ScreenPosition;
			EditorUI::EditorUIService::EditVec3(m_WindowLocation);

			// Edit window size relative to screen
			m_WindowSize.m_CurrentVec2 = m_ActiveWindow->m_Size;
			EditorUI::EditorUIService::EditVec2(m_WindowSize);

			// Edit window background color
			m_WindowBackgroundColor.m_CurrentVec4 = m_ActiveWindow->m_BackgroundColor;
			EditorUI::EditorUIService::EditVec4(m_WindowBackgroundColor);
		}
	}

	void UIEditorWindow::DrawWidgetOptions()
	{
		// Draw main header for widget options
		EditorUI::EditorUIService::CollapsingHeader(m_WidgetHeader);

		// Draw options to edit selected widget
		if (m_WidgetHeader.m_Expanded)
		{
			// Edit selected widget's tag
			m_WidgetTag.m_CurrentOption = m_ActiveWidget->m_Tag;
			EditorUI::EditorUIService::EditText(m_WidgetTag);

			// Edit selected widget's window location
			m_WidgetLocation.m_CurrentVec2 = m_ActiveWidget->m_WindowPosition;
			EditorUI::EditorUIService::EditVec2(m_WidgetLocation);

			// Edit selected widget's size relative to its window
			m_WidgetSize.m_CurrentVec2 = m_ActiveWidget->m_Size;
			EditorUI::EditorUIService::EditVec2(m_WidgetSize);
			
			// Edit selected widget's background color
			m_WidgetBackgroundColor.m_CurrentVec4 = m_ActiveWidget->m_DefaultBackgroundColor;
			EditorUI::EditorUIService::EditVec4(m_WidgetBackgroundColor);

			// Edit text widget specific options
			if (m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::TextWidget)
			{
				RuntimeUI::TextWidget& activeTextWidget = *(RuntimeUI::TextWidget*)m_ActiveWidget;

				// Edit selected text widget's on press script
				Assets::AssetHandle onPressHandle = activeTextWidget.m_FunctionPointers.m_OnPressHandle;
				m_WidgetOnPress.m_CurrentOption =
				{
					onPressHandle == Assets::EmptyHandle ? "None" : Assets::AssetService::GetScript(onPressHandle)->m_ScriptName,
					onPressHandle
				};
				EditorUI::EditorUIService::SelectOption(m_WidgetOnPress);

				// Edit selected text widget's text
				m_WidgetText.m_CurrentOption = activeTextWidget.m_Text;
				EditorUI::EditorUIService::EditText(m_WidgetText);

				// Edit selected text widget's text size relative to its window
				m_WidgetTextSize.m_CurrentFloat = activeTextWidget.m_TextSize;
				EditorUI::EditorUIService::EditFloat(m_WidgetTextSize);

				// Edit selected text widget's text color
				m_WidgetTextColor.m_CurrentVec4 = activeTextWidget.m_TextColor;
				EditorUI::EditorUIService::EditVec4(m_WidgetTextColor);

				// Edit selected text widget's text alignment
				m_WidgetCentered.m_CurrentBoolean = activeTextWidget.m_TextCentered;
				EditorUI::EditorUIService::Checkbox(m_WidgetCentered);
			}
			
		}
	}

	void UIEditorWindow::InitializeOpeningScreen()
	{
		// Initialize open existing user interface popup data
		m_OpenUIPopupSpec.m_Label = "Open User Interface";
		m_OpenUIPopupSpec.m_LineCount = 2;
		m_OpenUIPopupSpec.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_OpenUIPopupSpec.m_Flags |= EditorUI::SelectOption_PopupOnly;
		m_OpenUIPopupSpec.m_PopupAction = [&]()
		{
			m_OpenUIPopupSpec.GetAllOptions().clear();
			m_OpenUIPopupSpec.m_CurrentOption = { "None", Assets::EmptyHandle };

			m_OpenUIPopupSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetUserInterfaceRegistry())
			{
				m_OpenUIPopupSpec.AddToOptions("All Options", asset.Data.FileLocation.filename().string(), handle);
			}
		};
		m_OpenUIPopupSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.m_Handle == Assets::EmptyHandle)
			{
				KG_WARN("No User Interface Selected");
				return;
			}
			if (!Assets::AssetService::GetUserInterfaceRegistry().contains(selection.m_Handle))
			{
				KG_WARN("Could not find the user interface specified");
				return;
			}

			OnOpenUI(selection.m_Handle);
		};

		// Initialize create new user interface popup data
		m_CreateUIPopupSpec.m_Label = "Create User Interface";
		m_CreateUIPopupSpec.m_ConfirmAction = [&]()
		{
			if (m_SelectUINameSpec.m_CurrentOption == "")
			{
				return;
			}

			m_EditorUIHandle = Assets::AssetService::CreateUserInterface(m_SelectUINameSpec.m_CurrentOption.c_str(), m_SelectUILocationSpec.m_CurrentOption);
			if (m_EditorUIHandle == Assets::EmptyHandle)
			{
				KG_WARN("User Interface was not created");
				return;
			}
			m_EditorUI = Assets::AssetService::GetUserInterface(m_EditorUIHandle);
			m_MainHeader.m_EditColorActive = false;
			m_MainHeader.m_Label = Assets::AssetService::GetUserInterfaceRegistry().at(
				m_EditorUIHandle).Data.FileLocation.filename().string();
			OnRefreshData();
			RuntimeUI::RuntimeUIService::SetActiveUI(m_EditorUI, m_EditorUIHandle);
		};
		m_CreateUIPopupSpec.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_SelectUINameSpec);
			EditorUI::EditorUIService::ChooseDirectory(m_SelectUILocationSpec);
		};

		// Initialize widget for selecting user interface name
		m_SelectUINameSpec.m_Label = "New Name";
		m_SelectUINameSpec.m_CurrentOption = "Empty";

		// Initialize widget for selecting user interface location
		m_SelectUILocationSpec.m_Label = "Location";
		m_SelectUILocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
		m_SelectUILocationSpec.m_ConfirmAction = [&](const std::string& path)
		{
			if (!Utility::FileSystem::DoesPathContainSubPath(Projects::ProjectService::GetActiveAssetDirectory(), path))
			{
				KG_WARN("Cannot create an asset outside of the project's asset directory.");
				m_SelectUILocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
			}
		};

	}

	void UIEditorWindow::RecalculateTreeIndexData()
	{
		// Recalculate the handle for each window entry in the user interface tree
		m_UITree.EditDepth([&](EditorUI::TreeEntry& entry)
		{
			// Get tree path from entry reference
			EditorUI::TreePath entryPath = m_UITree.GetPathFromEntryReference(&entry);

			// Ensure the depth is 1, since this is the expected depth for the user interface tree
			if (entryPath.GetDepth() != 1)
			{
				KG_WARN("Invalid depth length when revalidating window index information");
				return;
			}

			// Get the window index from the path
			uint16_t currentWindow = entryPath.GetBack();

			// Update the handle for the window entry
			entry.m_Handle = (uint64_t)currentWindow;

		}, 0);


		// Recalculate the handle and provided data for each widget entry in the user interface tree
		m_UITree.EditDepth([&](EditorUI::TreeEntry& entry)
		{
			// Get tree path from entry reference
		 	EditorUI::TreePath entryPath = m_UITree.GetPathFromEntryReference(&entry);

			// Ensure the depth is 2, since this is the expected depth for the user interface tree
			if (entryPath.GetDepth() != 2)
			{
				KG_WARN("Invalid depth length when revalidating widget index information");
				return;
			}

			// Get the current widget and window index from the path
			uint16_t currentWidget = entryPath.GetBack();
			entryPath.PopBack();
			uint16_t currentWindow = entryPath.GetBack();

			// Update the handle and provided data for the widget entry
			entry.m_Handle = (uint64_t)currentWidget;
			entry.m_ProvidedData = CreateRef<uint32_t>((uint32_t)currentWindow);

		}, 1);
	}


	void UIEditorWindow::InitializeUIHeader()
	{
		// Header (Game State Name and Options)
		m_DeleteUIWarning.m_Label = "Delete User Interface";
		m_DeleteUIWarning.m_ConfirmAction = [&]()
		{
			// TODO: Remove UI from asset manager
			Assets::AssetService::DeleteUserInterface(m_EditorUIHandle);
			RuntimeUI::RuntimeUIService::ClearActiveUI();
			ResetPanelResources();
		};
		m_DeleteUIWarning.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to delete this user interface object?");
		};

		// Intialize widget data for closing the user interface warning popup
		m_CloseUIWarning.m_Label = "Close User Interface";
		m_CloseUIWarning.m_ConfirmAction = [&]()
		{
			ResetPanelResources();
			RuntimeUI::RuntimeUIService::ClearActiveUI();
		};
		m_CloseUIWarning.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to close this user interface object without saving?");
		};

		// Set up main header for user interface editor panel
		m_MainHeader.AddToSelectionList("Add Window", KG_BIND_CLASS_FN(AddWindow));
		m_MainHeader.AddToSelectionList("Save", [&]()
		{
			Assets::AssetService::SaveUserInterface(m_EditorUIHandle, m_EditorUI);
			m_MainHeader.m_EditColorActive = false;
		});
		m_MainHeader.AddToSelectionList("Close", [&]()
		{
			if (m_MainHeader.m_EditColorActive)
			{
				m_CloseUIWarning.m_OpenPopup = true;
			}
			else
			{
				ResetPanelResources();
				RuntimeUI::RuntimeUIService::ClearActiveUI();
			}
		});
		m_MainHeader.AddToSelectionList("Delete", [&]()
		{
			m_DeleteUIWarning.m_OpenPopup = true;
		});
	}

	void UIEditorWindow::InitializeMainContent()
	{
		m_UITree.m_Label = "User Interface Tree";
		m_UITree.m_OnRefresh = KG_BIND_CLASS_FN(OnRefreshUITree);
	}
	void UIEditorWindow::InitializeWindowOptions()
	{
		// Set up header for window options
		m_WindowHeader.m_Label = "Window Options";
		m_WindowHeader.m_Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		m_WindowHeader.m_Expanded = true;

		// Set up widget to modify the window's tag
		m_WindowTag.m_Label = "Tag";
		m_WindowTag.m_Flags |= EditorUI::EditText_Indented;
		m_WindowTag.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWindowTag);

		// Set up widget to modify the window's default widget
		m_WindowDefaultWidget.m_Label = "Default Widget";
		m_WindowDefaultWidget.m_Flags |= EditorUI::SelectOption_Indented;
		m_WindowDefaultWidget.m_PopupAction = KG_BIND_CLASS_FN(OnOpenWindowDefaultWidgetPopup);
		m_WindowDefaultWidget.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWindowDefaultWidget);

		// Set up widget to modify the window's display boolean
		m_WindowDisplay.m_Label = "Display Window";
		m_WindowDisplay.m_Flags |= EditorUI::Checkbox_Indented;
		m_WindowDisplay.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWindowDisplay);

		// Set up widget to modify the window's screen location
		m_WindowLocation.m_Label = "Screen Location";
		m_WindowLocation.m_Flags |= EditorUI::EditVec3_Indented;
		m_WindowLocation.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWindowLocation);

		// Set up widget to modify the window's screen size
		m_WindowSize.m_Label = "Screen Size";
		m_WindowSize.m_Flags |= EditorUI::EditVec2_Indented;
		m_WindowSize.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWindowSize);

		// Set up widget to modify the window's background color
		m_WindowBackgroundColor.m_Label = "Background Color";
		m_WindowBackgroundColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_WindowBackgroundColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWindowBackgroundColor);
	}
	void UIEditorWindow::InitializeWidgetOptions()
	{
		// Set up header for widget options
		m_WidgetHeader.m_Label = "Widget Options";
		m_WidgetHeader.m_Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		m_WidgetHeader.m_Expanded = true;

		// Set up widget to modify the widget's tag
		m_WidgetTag.m_Label = "Tag";
		m_WidgetTag.m_Flags |= EditorUI::EditText_Indented;
		m_WidgetTag.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWidgetTag);

		// Set up widget to modify the widget's location relative to its window
		m_WidgetLocation.m_Label = "Window Location";
		m_WidgetLocation.m_Flags |= EditorUI::EditVec2_Indented;
		m_WidgetLocation.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWidgetLocation);

		// Set up widget to modify the widget's size
		m_WidgetSize.m_Label = "Size";
		m_WidgetSize.m_Flags |= EditorUI::EditVec2_Indented;
		m_WidgetSize.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWidgetSize);

		// Set up widget to modify the widget's background color
		m_WidgetBackgroundColor.m_Label = "Background Color";
		m_WidgetBackgroundColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_WidgetBackgroundColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWidgetBackgroundColor);

		// Set up widget to modify the text widget's on press script
		m_WidgetOnPress.m_Label = "On Press";
		m_WidgetOnPress.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_WidgetOnPress.m_PopupAction = KG_BIND_CLASS_FN(OnOpenWidgetOnPressPopup);
		m_WidgetOnPress.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWidgetOnPress);
		m_WidgetOnPress.m_OnEdit = KG_BIND_CLASS_FN(OnOpenTooltipForWidgetOnPress);

		// Set up widget to modify the text widget's text
		m_WidgetText.m_Label = "Text";
		m_WidgetText.m_Flags |= EditorUI::EditText_Indented;
		m_WidgetText.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyTextWidgetText);

		// Set up widget to modify the text widget's text size
		m_WidgetTextSize.m_Label = "Text Size";
		m_WidgetTextSize.m_Flags |= EditorUI::EditFloat_Indented;
		m_WidgetTextSize.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyTextWidgetTextSize);

		// Set up widget to modify the text widget's text color
		m_WidgetTextColor.m_Label = "Text Color";
		m_WidgetTextColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_WidgetTextColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyTextWidgetTextColor);

		// Set up widget to modify the text widget's text alignment
		m_WidgetCentered.m_Label = "Centered";
		m_WidgetCentered.m_Flags |= EditorUI::Checkbox_Indented;
		m_WidgetCentered.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyTextWidgetCentered);
	}

	void UIEditorWindow::AddTextWidget(EditorUI::TreeEntry& windowEntry)
	{
		// Get window path from provided entry and ensure it is valid
		EditorUI::TreePath windowPath = m_UITree.GetPathFromEntryReference(&windowEntry);
		if (!windowPath)
		{
			KG_WARN("Could not locate window path inside m_UITree");
			return;
		}

		// Create Text Widget
		RuntimeUI::Window& window = m_EditorUI->m_Windows.at(windowEntry.m_Handle);
		Ref<RuntimeUI::TextWidget> newTextWidget = CreateRef<RuntimeUI::TextWidget>();

		// Create new widget entry for m_UITree
		EditorUI::TreeEntry newWidgetEntry {};
		newWidgetEntry.m_Label = newTextWidget->m_Tag;
		newWidgetEntry.m_IconHandle = EditorUI::EditorUIService::s_IconTextWidget;
		newWidgetEntry.m_ProvidedData = CreateRef<uint32_t>((uint32_t)windowEntry.m_Handle); ;
		newWidgetEntry.m_Handle = window.m_Widgets.size();

		// Add handlers for interacting with the tree entry
		newWidgetEntry.m_OnLeftClick = KG_BIND_CLASS_FN(SelectTextWidget);
		newWidgetEntry.m_OnRightClickSelection.push_back({ "Delete Widget", KG_BIND_CLASS_FN(DeleteWidget) });

		// Add Widget to RuntimeUI and EditorUI::Tree
		window.AddWidget(newTextWidget);
		windowEntry.m_SubEntries.push_back(newWidgetEntry);
	}

	void UIEditorWindow::SelectTextWidget(EditorUI::TreeEntry& entry)
	{
		m_ActiveWindow = &m_EditorUI->m_Windows.at(*(uint32_t*)entry.m_ProvidedData.get());
		m_ActiveWidget = m_ActiveWindow->m_Widgets.at(entry.m_Handle).get();
		m_CurrentDisplay = UIPropertiesDisplay::Widget;
		EditorUI::EditorUIService::BringWindowToFront(s_MainWindow->m_PropertiesPanel->m_PanelName);
		s_MainWindow->m_PropertiesPanel->m_ActiveParent = m_PanelName;
	}

	void UIEditorWindow::DeleteWindow(EditorUI::TreeEntry& entry)
	{
		// Get tree path from provided entry
		EditorUI::TreePath path = m_UITree.GetPathFromEntryReference(&entry);
		if (!path)
		{
			KG_WARN("Could not locate window path inside m_UITree");
			return;
		}

		// Remove window from active runtime UI and this panel's tree
		RuntimeUI::RuntimeUIService::DeleteActiveUIWindow(entry.m_Handle);
		m_UITree.RemoveEntry(path);

		// Reset properties panel and ensure tree index data is valid
		ClearPropertiesPanelData();
		RecalculateTreeIndexData();

		// Set the active editor UI as edited
		m_MainHeader.m_EditColorActive = true;

	}

	void UIEditorWindow::OnModifyWindowTag(EditorUI::EditTextSpec& spec)
	{
		// Ensure active window is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid window active when trying to update window tag");
			return;
		}

		// Ensure selected window path is valid
		if (!m_UITree.m_SelectedEntry)
		{
			KG_WARN("No valid selected window path available in m_UITree when trying to update window tag");
			return;
		}

		// Get the selected window entry and ensure it is valid
		EditorUI::TreeEntry* entry = m_UITree.GetEntryFromPath(m_UITree.m_SelectedEntry);
		if (!entry)
		{
			KG_WARN("No valid selected window active in m_UITree when trying to update window tag");
			return;
		}

		// Update the window tag and tree entry label
		entry->m_Label = m_WindowTag.m_CurrentOption;
		m_ActiveWindow->m_Tag = m_WindowTag.m_CurrentOption;

		// Set the active editor UI as edited
		m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorWindow::OnOpenWindowDefaultWidgetPopup()
	{
		// Clear existing options
		m_WindowDefaultWidget.ClearOptions();
		m_WindowDefaultWidget.AddToOptions("Clear", "None", (uint64_t)RuntimeUI::k_InvalidWidgetIndex);

		// Add all text widgets to the options
		std::size_t iteration{ 0 };
		for (Ref<RuntimeUI::Widget> widget : m_ActiveWindow->m_Widgets)
		{
			switch (widget->m_WidgetType)
			{
			case RuntimeUI::WidgetTypes::TextWidget:
				m_WindowDefaultWidget.AddToOptions("Text Widget", widget->m_Tag, iteration);
				break;
			default:
				KG_ERROR("Invalid widge type provided to UIEditorWindow");
				break;
			}
			iteration++;
		}
	}

	void UIEditorWindow::OnModifyWindowDisplay(EditorUI::CheckboxSpec& spec)
	{
		// Ensure active window is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid window active when trying to update window display option");
			return;
		}

		// Update the window display based on the checkbox value
		spec.m_CurrentBoolean ? m_ActiveWindow->DisplayWindow() : m_ActiveWindow->HideWindow();

		// Set the active editor UI as edited
		m_MainHeader.m_EditColorActive = true;
			
	}

	void UIEditorWindow::OnModifyWindowLocation(EditorUI::EditVec3Spec& spec)
	{
		// Ensure active window is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid window active when trying to update window location");
			return;
		}

		// Update the window location based on the widget value
		m_ActiveWindow->m_ScreenPosition = m_WindowLocation.m_CurrentVec3;

		// Set the active editor UI as edited
		m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorWindow::OnModifyWindowSize(EditorUI::EditVec2Spec& spec)
	{
		// Ensure active window is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid window active when trying to update window size");
			return;
		}

		// Update the window size based on the widget value
		m_ActiveWindow->m_Size = m_WindowSize.m_CurrentVec2;

		// Set the active editor UI as edited
		m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorWindow::OnModifyWindowBackgroundColor(EditorUI::EditVec4Spec& spec)
	{
		// Ensure active window is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid window active when trying to update window background color");
			return;
		}

		// Update the window background color based on the widget value
		m_ActiveWindow->m_BackgroundColor = m_WindowBackgroundColor.m_CurrentVec4;

		// Set the active editor UI as edited
		m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorWindow::OnModifyWindowDefaultWidget(const EditorUI::OptionEntry& entry)
	{
		// Clear the default active widget if the provided index is invalid
		if (entry.m_Handle == (uint64_t)RuntimeUI::k_InvalidWidgetIndex)
		{
			m_ActiveWindow->m_DefaultActiveWidget = RuntimeUI::k_InvalidWidgetIndex;
			m_ActiveWindow->m_DefaultActiveWidgetRef = nullptr;
			return;
		}
		// Ensure provided widget ID is valid
		if (entry.m_Handle >= m_ActiveWindow->m_Widgets.size())
		{
			KG_WARN("Invalid widget location provided when updating default active widget in window");
			return;
		}

		// Update the default active widget for the window
		m_ActiveWindow->m_DefaultActiveWidget = (std::size_t)entry.m_Handle;
		m_ActiveWindow->m_DefaultActiveWidgetRef = m_ActiveWindow->m_Widgets.at(entry.m_Handle);
	}

	void UIEditorWindow::DeleteWidget(EditorUI::TreeEntry& entry)
	{
		// Getpath from provided entry
		EditorUI::TreePath path = m_UITree.GetPathFromEntryReference(&entry);
		if (!path)
		{
			KG_WARN("Could not locate widget path inside m_UITree");
			return;
		}

		// Remove widget from RuntimeUI 
		bool success = RuntimeUI::RuntimeUIService::DeleteActiveUIWidget((size_t)*(uint32_t*)entry.m_ProvidedData.get(), entry.m_Handle);

		// Check if widget was successfully deleted
		if (!success)
		{
			KG_WARN("Could not delete widget from RuntimeUI");
			return;
		}

		// Remove widget from panel's tree widget
		m_UITree.RemoveEntry(path);

		// Reset properties panel and ensure tree index data is valid
		ClearPropertiesPanelData();
		RecalculateTreeIndexData();

		// Set the active editor UI as edited
		m_MainHeader.m_EditColorActive = true;

	}

	void UIEditorWindow::OnModifyWidgetTag(EditorUI::EditTextSpec& spec)
	{
		// Ensure active widget is valid and update the widget tag
		if (!m_ActiveWidget)
		{
			KG_WARN("No valid widget active when trying to update widget tag");
			return;
		}
		if (!m_UITree.m_SelectedEntry)
		{
			KG_WARN("No valid selected widget path available in m_UITree when trying to update widget tag");
			return;
		}

		// Get the selected widget entry and ensure it is valid
		EditorUI::TreeEntry* entry = m_UITree.GetEntryFromPath(m_UITree.m_SelectedEntry);
		if (!entry)
		{
			KG_WARN("No valid selected widget active in m_UITree when trying to update widget tag");
			return;
		}
		// Update the widget tag and tree entry label
		entry->m_Label = m_WidgetTag.m_CurrentOption;
		m_ActiveWidget->m_Tag = m_WidgetTag.m_CurrentOption;

		// Set the active editor UI as edited
		m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorWindow::OnModifyWidgetLocation(EditorUI::EditVec2Spec& spec)
	{
		// Ensure active widget is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget's window location");
			return;
		}

		// Update the widget location based on the editorUI widget value
		m_ActiveWidget->m_WindowPosition = m_WidgetLocation.m_CurrentVec2;

		// Set the active editor UI as edited
		m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorWindow::OnModifyWidgetSize(EditorUI::EditVec2Spec& spec)
	{
		// Ensure active widget is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget size");
			return;
		}

		// Update the widget size based on the editorUI widget value
		m_ActiveWidget->m_Size = m_WidgetSize.m_CurrentVec2;

		// Set the active editor UI as edited
		m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorWindow::OnModifyWidgetBackgroundColor(EditorUI::EditVec4Spec& spec)
	{
		// Ensure active widget is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget background color");
			return;
		}

		// Update the widget background color based on the editorUI widget value
		m_ActiveWidget->m_DefaultBackgroundColor = m_WidgetBackgroundColor.m_CurrentVec4;
		m_ActiveWidget->m_ActiveBackgroundColor = m_WidgetBackgroundColor.m_CurrentVec4;

		// Set the active editor UI as edited
		m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorWindow::OnModifyWidgetOnPress(const EditorUI::OptionEntry& entry)
	{
		// Clear the on press script if the provided handle is empty
		if (entry.m_Handle == Assets::EmptyHandle)
		{
			m_ActiveWidget->m_FunctionPointers.m_OnPress = nullptr;
			m_ActiveWidget->m_FunctionPointers.m_OnPressHandle = Assets::EmptyHandle;
			return;
		}

		// Set the on press script for the text widget
		m_ActiveWidget->m_FunctionPointers.m_OnPressHandle = entry.m_Handle;
		m_ActiveWidget->m_FunctionPointers.m_OnPress = Assets::AssetService::GetScript(entry.m_Handle);

		// Set the active editor UI as edited
		m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorWindow::OnOpenWidgetOnPressPopup()
	{
		// Clear existing options
		m_WidgetOnPress.ClearOptions();
		m_WidgetOnPress.AddToOptions("Clear", "None", Assets::EmptyHandle);

		// Add all compatible scripts to the select options
		for (auto& [handle, assetInfo] : Assets::AssetService::GetScriptRegistry())
		{
			// Get script from handle
			Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);

			// Ensure script is compatible with the text widget
			if (script->m_FuncType != WrappedFuncType::Void_None)
			{
				continue;
			}

			// Add script to the select options
			m_WidgetOnPress.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
		}
	}

	void UIEditorWindow::OnOpenTooltipForWidgetOnPress()
	{
		// Clear existing options
		m_SelectScriptTooltip.ClearEntries();

		// Add option to opening an existing script
		EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
		{
			m_WidgetOnPress.m_OpenPopup = true;
		} };
		m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

		// Add option or creating a new script from this usage point
		EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
		{
			// Open create script dialog in script editor
			s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_None, [&](Assets::AssetHandle scriptHandle)
			{
					// Ensure handle provides a script in the registry
					if (!Assets::AssetService::HasScript(scriptHandle))
					{
						KG_WARN("Could not find script");
						return;
					}

					// Ensure function type matches definition
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
					if (script->m_FuncType != WrappedFuncType::Void_None)
					{
						KG_WARN("Incorrect function type returned when linking script to usage point");
						return;
					}

					// Fill the new script handle
					m_ActiveWidget->m_FunctionPointers.m_OnPressHandle = scriptHandle;
					m_ActiveWidget->m_FunctionPointers.m_OnPress = script;
					m_WidgetOnPress.m_CurrentOption = { script->m_ScriptName, scriptHandle };

					// Set the active editor UI as edited
					m_MainHeader.m_EditColorActive = true;
				}, {});
		}   };
		m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

		// Open tooltip
		m_SelectScriptTooltip.m_TooltipActive = true;
	}

	void UIEditorWindow::OnModifyTextWidgetText(EditorUI::EditTextSpec& spec)
	{
		// Ensure active window is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget text");
			return;
		}

		// Ensure active widget is a valid type
		if (!m_ActiveWidget)
		{
			KG_WARN("No valid widget active when trying to update widget text");
			return;
		}

		// Ensure active widget is a valid type and get the text widget
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::TextWidget)
		{
			KG_WARN("Attempt to modify text widget member, however, active widget is an invalid type");
			return;
		}
		RuntimeUI::TextWidget& textWidget = *(RuntimeUI::TextWidget*)m_ActiveWidget;

		// Update the text widget text based on the editorUI widget's value
		textWidget.m_Text = m_WidgetText.m_CurrentOption;

		// Set the active editor UI as edited
		m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorWindow::OnModifyTextWidgetTextSize(EditorUI::EditFloatSpec& spec)
	{
		// Ensure active window is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget text size");
			return;
		}

		// Ensure active widget is a valid type
		if (!m_ActiveWidget)
		{
			KG_WARN("No valid widget active when trying to update widget text size");
			return;
		}

		// Ensure active widget is a valid type and get the text widget
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::TextWidget)
		{
			KG_WARN("Attempt to modify text widget member, however, active widget is an invalid type");
			return;
		}
		RuntimeUI::TextWidget& textWidget = *(RuntimeUI::TextWidget*)m_ActiveWidget;

		// Update the text widget text size based on the editorUI widget's value
		textWidget.m_TextSize = m_WidgetTextSize.m_CurrentFloat;

		// Set the active editor UI as edited
		m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorWindow::OnModifyTextWidgetTextColor(EditorUI::EditVec4Spec& spec)
	{
		// Ensure active window is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget text color");
			return;
		}

		if (!m_ActiveWidget)
		{
			KG_WARN("No valid widget active when trying to update widget text color");
			return;
		}

		// Ensure active widget is a valid type and get the text widget
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::TextWidget)
		{
			KG_WARN("Attempt to modify text widget member, however, active widget is an invalid type");
			return;
		}
		RuntimeUI::TextWidget& textWidget = *(RuntimeUI::TextWidget*)m_ActiveWidget;

		// Update the text widget text color based on the editorUI widget's value
		textWidget.m_TextColor = m_WidgetTextColor.m_CurrentVec4;

		// Set the active editor UI as edited
		m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorWindow::OnModifyTextWidgetCentered(EditorUI::CheckboxSpec& spec)
	{
		// Ensure active window is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget's TextCentered field");
			return;
		}

		// Ensure active widget is a valid type
		if (!m_ActiveWidget)
		{
			KG_WARN("No valid widget active when trying to update widget's TextCentered field");
			return;
		}

		// Ensure active widget is a valid type and get the text widget
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::TextWidget)
		{
			KG_WARN("Attempt to modify text widget member, however, active widget is an invalid type");
			return;
		}
		RuntimeUI::TextWidget& textWidget = *(RuntimeUI::TextWidget*)m_ActiveWidget;

		// Update the text widget text alignment based on the editorUI widget's value
		textWidget.m_TextCentered = spec.m_CurrentBoolean;

		// Set the active editor UI as edited
		m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorWindow::SelectWindow(EditorUI::TreeEntry& entry)
	{
		// Set selected window as active
		ClearPropertiesPanelData();
		m_ActiveWindow = &m_EditorUI->m_Windows.at(entry.m_Handle);
		m_CurrentDisplay = UIPropertiesDisplay::Window;

		// Display window properties in properties panel
		s_MainWindow->m_PropertiesPanel->m_ActiveParent = m_PanelName;

		// Bring properties panel to front
		EditorUI::EditorUIService::BringWindowToFront(s_MainWindow->m_PropertiesPanel->m_PanelName);
	}

	void UIEditorWindow::AddWindow()
	{
		// Create new window entry for m_UITree
		EditorUI::TreeEntry newEntry {};
		newEntry.m_Label = "None";
		newEntry.m_IconHandle = EditorUI::EditorUIService::s_IconWindow;
		newEntry.m_Handle = m_UITree.GetTreeEntries().size();

		// Add handlers for interacting with the tree entry
		newEntry.m_OnLeftClick = KG_BIND_CLASS_FN(SelectWindow);
		newEntry.m_OnRightClickSelection.push_back({ "Delete Window", KG_BIND_CLASS_FN(DeleteWindow) });
		newEntry.m_OnRightClickSelection.push_back({ "Add Text Widget", KG_BIND_CLASS_FN(AddTextWidget) });

		// Add new window to RuntimeUI and this panel's tree
		m_UITree.InsertEntry(newEntry);
		m_EditorUI->m_Windows.push_back({});
		
		// Select the newly created window
		SelectWindow(newEntry);

		// Set this UI as edited
		m_MainHeader.m_EditColorActive = true;
	}
}

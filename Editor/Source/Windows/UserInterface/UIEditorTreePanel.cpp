#include "Windows/UserInterface/UIEditorTreePanel.h"

#include "Kargono/Utility/Operations.h"

#include "EditorApp.h"

static Kargono::EditorApp* s_EditorApp{ nullptr };
static Kargono::Windows::UIEditorWindow* s_UIWindow{ nullptr };

namespace Kargono::Panels
{
	UIEditorTreePanel::UIEditorTreePanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_UIWindow = s_EditorApp->m_UIEditorWindow.get();

		// Initialize null state widget data
		InitializeOpeningScreen();

		// Initialize main table panel widget data
		InitializeUIHeader();
		InitializeMainContent();
	}
	void UIEditorTreePanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_UIWindow->m_ShowTree);

		// Early out if the window is not visible
		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		// Begin rendering the user interface editor panel
		if (!s_UIWindow->m_EditorUI)
		{
			// Display opening screen for user interface editor
			EditorUI::EditorUIService::NewItemScreen("Open Existing User Interface", KG_BIND_CLASS_FN(OnOpenUIDialog), "Create New User Interface", KG_BIND_CLASS_FN(OnCreateUIDialog));
			m_CreateUIPopupSpec.RenderPopup();
			m_OpenUIPopupSpec.RenderOptions();
		}
		else
		{
			// Display user interface editor panel main content
			m_MainHeader.RenderHeader();
			m_DeleteUIWarning.RenderPopup();
			m_CloseUIWarning.RenderPopup();
			m_UITree.RenderTree();
			m_SelectTooltip.RenderTooltip();
		}

		// End the window
		EditorUI::EditorUIService::EndWindow();
	}

	void UIEditorTreePanel::OnRefreshData()
	{
		OnRefreshUITree();
	}

	void UIEditorTreePanel::OnRefreshUITree()
	{
		// Ensure the editor UI is valid
		if (!s_UIWindow->m_EditorUI)
		{
			KG_WARN("Attempt to load table without valid m_EditorUI");
			return;
		}

		// Clear the tree before adding new entries
		m_UITree.ClearTree();

		// Create UI tree entry
		// Create new window entry
		EditorUI::TreeEntry uiEntry{};
		uiEntry.m_Label = Assets::AssetService::GetUserInterfaceRegistry().at(
			s_UIWindow->m_EditorUIHandle).Data.FileLocation.stem().string();
		uiEntry.m_IconHandle = EditorUI::EditorUIService::s_IconUserInterface2;
		uiEntry.m_Handle = s_UIWindow->m_EditorUIHandle;

		// Add functions to call when interacting with window entry
		uiEntry.m_OnLeftClick = KG_BIND_CLASS_FN(SelectUI);
		uiEntry.m_OnRightClick = KG_BIND_CLASS_FN(RightClickUIEntry);

		// Add all windows and widgets from the editor UI to the tree
		for (RuntimeUI::Window& window : s_UIWindow->m_EditorUI->m_WindowsState.m_Windows)
		{
			// Create new window entry
			EditorUI::TreeEntry windowEntry{};
			windowEntry.m_Label = window.m_Tag;
			windowEntry.m_IconHandle = EditorUI::EditorUIService::s_IconWindow;
			windowEntry.m_Handle = window.m_ID;

			// Add window selection options
			CreateWindowSelectionOptions(windowEntry);

			// Add widgets to window entry
			for (Ref<RuntimeUI::Widget> widget : window.m_Widgets)
			{
				CreateWidgetTreeEntry(windowEntry, widget);
			}

			// Add window entry to the user interface tree
			uiEntry.m_SubEntries.push_back(windowEntry);
		}

		m_UITree.InsertEntry(uiEntry);
		m_UITree.ExpandFirstLayer();
	}

	void UIEditorTreePanel::InitializeUIHeader()
	{
		// Header (Game State Name and Options)
		m_DeleteUIWarning.m_Label = "Delete User Interface";
		m_DeleteUIWarning.m_ConfirmAction = [&]()
			{
				// TODO: Remove UI from asset manager
				Assets::AssetService::DeleteUserInterface(s_UIWindow->m_EditorUIHandle);
				RuntimeUI::RuntimeUIService::GetActiveContext().ClearActiveUI();
				s_UIWindow->ResetWindowResources();
			};
		m_DeleteUIWarning.m_PopupContents = [&]()
			{
				EditorUI::EditorUIService::Text("Are you sure you want to delete this user interface object?");
			};

		// Intialize widget data for closing the user interface warning popup
		m_CloseUIWarning.m_Label = "Close User Interface";
		m_CloseUIWarning.m_ConfirmAction = [&]()
			{
				s_UIWindow->ResetWindowResources();
				RuntimeUI::RuntimeUIService::GetActiveContext().ClearActiveUI();
			};
		m_CloseUIWarning.m_PopupContents = [&]()
			{
				EditorUI::EditorUIService::Text("Are you sure you want to close this user interface object without saving?");
			};

		// Set up main header for user interface editor panel
		m_MainHeader.AddToSelectionList("Save", [&]()
			{
				Assets::AssetService::SaveUserInterface(s_UIWindow->m_EditorUIHandle, s_UIWindow->m_EditorUI);
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
					s_UIWindow->ResetWindowResources();
					RuntimeUI::RuntimeUIService::GetActiveContext().ClearActiveUI();
				}
			});
		m_MainHeader.AddToSelectionList("Delete", [&]()
			{
				m_DeleteUIWarning.m_OpenPopup = true;
			});
	}

	void UIEditorTreePanel::InitializeMainContent()
	{
		m_UITree.m_Label = "User Interface Tree";
		m_UITree.m_OnRefresh = KG_BIND_CLASS_FN(OnRefreshUITree);
	}

	void UIEditorTreePanel::InitializeOpeningScreen()
	{
		// Initialize open existing user interface popup data
		m_OpenUIPopupSpec.m_Label = "Open User Interface";
		m_OpenUIPopupSpec.m_LineCount = 2;
		m_OpenUIPopupSpec.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_OpenUIPopupSpec.m_Flags |= EditorUI::SelectOption_PopupOnly;
		m_OpenUIPopupSpec.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.GetAllOptions().clear();
			spec.m_CurrentOption = { "None", Assets::EmptyHandle };

			m_OpenUIPopupSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetUserInterfaceRegistry())
			{
				spec.AddToOptions("All Options", asset.Data.FileLocation.filename().string(), handle);
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

				s_UIWindow->m_EditorUIHandle = Assets::AssetService::CreateUserInterface(m_SelectUINameSpec.m_CurrentOption.c_str(), m_SelectUILocationSpec.m_CurrentOption);
				if (s_UIWindow->m_EditorUIHandle == Assets::EmptyHandle)
				{
					KG_WARN("User Interface was not created");
					return;
				}
				s_UIWindow->m_EditorUI = Assets::AssetService::GetUserInterface(s_UIWindow->m_EditorUIHandle);
				m_MainHeader.m_EditColorActive = false;
				m_MainHeader.m_Label = Assets::AssetService::GetUserInterfaceRegistry().at(
					s_UIWindow->m_EditorUIHandle).Data.FileLocation.filename().string();
				s_UIWindow->OnRefreshData();
				RuntimeUI::RuntimeUIService::GetActiveContext().SetActiveUI(s_UIWindow->m_EditorUI, s_UIWindow->m_EditorUIHandle);
			};
		m_CreateUIPopupSpec.m_PopupContents = [&]()
			{
				m_SelectUINameSpec.RenderText();
				m_SelectUILocationSpec.RenderChooseDir();
			};

		Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

		// Initialize widget for selecting user interface name
		m_SelectUINameSpec.m_Label = "New Name";
		m_SelectUINameSpec.m_CurrentOption = "Empty";

		// Initialize widget for selecting user interface location
		m_SelectUILocationSpec.m_Label = "Location";
		m_SelectUILocationSpec.m_CurrentOption = projectPaths.GetAssetDirectory();
		m_SelectUILocationSpec.m_ConfirmAction = [&](std::string_view path)
		{
			if (!Utility::FileSystem::DoesPathContainSubPath(projectPaths.GetAssetDirectory(), path))
			{
				KG_WARN("Cannot create an asset outside of the project's asset directory.");
				m_SelectUILocationSpec.m_CurrentOption = projectPaths.GetAssetDirectory();
			}
		};

	}

	void UIEditorTreePanel::OnOpenUIDialog()
	{
		// Set dialog popup to open on next frame
		m_OpenUIPopupSpec.m_OpenPopup = true;
	}
	void UIEditorTreePanel::OnCreateUIDialog()
	{
		Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

		// Set default values for new user interface creation location
		m_SelectUILocationSpec.m_CurrentOption = projectPaths.GetAssetDirectory();

		// Set dialog popup to open on next frame
		m_CreateUIPopupSpec.m_OpenPopup = true;
	}
	void UIEditorTreePanel::OnOpenUI(Assets::AssetHandle newHandle)
	{
		// Set new in editor user interface
		s_UIWindow->m_EditorUI = Assets::AssetService::GetUserInterface(newHandle);
		s_UIWindow->m_EditorUIHandle = newHandle;

		// Set default values for header
		m_MainHeader.m_EditColorActive = false;
		m_MainHeader.m_Label = Assets::AssetService::GetUserInterfaceRegistry().at(
			s_UIWindow->m_EditorUIHandle).Data.FileLocation.filename().string();

		// Handle opening the UI in viewport
		s_UIWindow->m_ViewportPanel->OnOpenUI();

		// Refresh widget data in editor to use new user interface
		s_UIWindow->OnRefreshData();

		// Set editor user interface as active in runtime
		RuntimeUI::RuntimeUIService::GetActiveContext().SetActiveUI(s_UIWindow->m_EditorUI, s_UIWindow->m_EditorUIHandle);
	}

	void UIEditorTreePanel::SelectUI(EditorUI::TreeEntry& entry)
	{
		UNREFERENCED_PARAMETER(entry);
		s_UIWindow->m_PropertiesPanel->ClearPanelData();
		s_UIWindow->m_PropertiesPanel->m_CurrentDisplay = UIPropertiesDisplay::UserInterface;
	}

	void UIEditorTreePanel::RightClickUIEntry(EditorUI::TreeEntry& entry)
	{
		// Reset the tooltip
		m_SelectTooltip.ClearEntries();

		// Create the add window entry
		EditorUI::TooltipEntry addWindowEntry{ "Window", KG_BIND_CLASS_FN(AddWindow) };
		addWindowEntry.m_ProvidedData = &entry;

		// Create the add menu and add it the the top-level tooltip
		EditorUI::TooltipEntry addMenu{ "Add", { addWindowEntry } };
		m_SelectTooltip.AddTooltipEntry(addMenu);

		// Active the tooltip
		m_SelectTooltip.m_TooltipActive = true;
	}

	void UIEditorTreePanel::RightClickWidgetEntry(EditorUI::TreeEntry& entry)
	{
		// Reset the tooltip
		m_SelectTooltip.ClearEntries();

		// Create the rename widget entry
		EditorUI::TooltipEntry renameWidgetEntry{ "Rename", KG_BIND_CLASS_FN(RenameWidget) };
		renameWidgetEntry.m_ProvidedData = &entry;

		// Create the delete widget entry
		EditorUI::TooltipEntry deleteWidgetEntry{ "Delete", KG_BIND_CLASS_FN(DeleteWidget) };
		deleteWidgetEntry.m_ProvidedData = &entry;

		// Create the edit menu and add it the the top-level tooltip
		EditorUI::TooltipEntry editMenu{ "Edit", { renameWidgetEntry, deleteWidgetEntry } };
		m_SelectTooltip.AddTooltipEntry(editMenu);

		// Create add options
		CreateAddWidgetsSelectionOptions(entry);

		// Active the tooltip
		m_SelectTooltip.m_TooltipActive = true;
	}

	void UIEditorTreePanel::AddTextWidget(EditorUI::TooltipEntry& entry)
	{
		// Get the widget entry
		EditorUI::TreeEntry* parentEntry = (EditorUI::TreeEntry*)entry.m_ProvidedData;
		KG_ASSERT(parentEntry);
		
		// Create new widget
		Ref<RuntimeUI::TextWidget> newWidget = CreateRef<RuntimeUI::TextWidget>(s_UIWindow->m_EditorUI.get());

		// Add widget to tree and runtime UI
		AddWidgetInternal(*parentEntry, newWidget, Utility::WidgetTypeToIcon(RuntimeUI::WidgetTypes::TextWidget));
	}

	void UIEditorTreePanel::AddButtonWidget(EditorUI::TooltipEntry& entry)
	{
		// Get the widget entry
		EditorUI::TreeEntry* parentEntry = (EditorUI::TreeEntry*)entry.m_ProvidedData;
		KG_ASSERT(parentEntry);

		// Create new widget
		Ref<RuntimeUI::ButtonWidget> newWidget = CreateRef<RuntimeUI::ButtonWidget>(s_UIWindow->m_EditorUI.get());
		AddWidgetInternal(*parentEntry, newWidget, Utility::WidgetTypeToIcon(RuntimeUI::WidgetTypes::ButtonWidget));
	}

	void UIEditorTreePanel::AddImageWidget(EditorUI::TooltipEntry& entry)
	{
		// Get the widget entry
		EditorUI::TreeEntry* parentEntry = (EditorUI::TreeEntry*)entry.m_ProvidedData;
		KG_ASSERT(parentEntry);

		// Create new widget
		Ref<RuntimeUI::ImageWidget> newWidget = CreateRef<RuntimeUI::ImageWidget>(s_UIWindow->m_EditorUI.get());
		AddWidgetInternal(*parentEntry, newWidget, Utility::WidgetTypeToIcon(RuntimeUI::WidgetTypes::ImageWidget));
	}

	void UIEditorTreePanel::AddImageButtonWidget(EditorUI::TooltipEntry& entry)
	{
		// Get the widget entry
		EditorUI::TreeEntry* parentEntry = (EditorUI::TreeEntry*)entry.m_ProvidedData;
		KG_ASSERT(parentEntry);

		// Create new widget
		Ref<RuntimeUI::ImageButtonWidget> newWidget = CreateRef<RuntimeUI::ImageButtonWidget>(s_UIWindow->m_EditorUI.get());
		AddWidgetInternal(*parentEntry, newWidget, Utility::WidgetTypeToIcon(RuntimeUI::WidgetTypes::ImageButtonWidget));
	}

	void UIEditorTreePanel::AddCheckboxWidget(EditorUI::TooltipEntry& entry)
	{
		// Get the widget entry
		EditorUI::TreeEntry* parentEntry = (EditorUI::TreeEntry*)entry.m_ProvidedData;
		KG_ASSERT(parentEntry);

		// Create new widget
		Ref<RuntimeUI::CheckboxWidget> newWidget = CreateRef<RuntimeUI::CheckboxWidget>(s_UIWindow->m_EditorUI.get());
		AddWidgetInternal(*parentEntry, newWidget, Utility::WidgetTypeToIcon(RuntimeUI::WidgetTypes::CheckboxWidget));
	}

	void UIEditorTreePanel::AddContainerWidget(EditorUI::TooltipEntry& entry)
	{
		// Get the widget entry
		EditorUI::TreeEntry* parentEntry = (EditorUI::TreeEntry*)entry.m_ProvidedData;
		KG_ASSERT(parentEntry);

		// Create new widget
		Ref<RuntimeUI::ContainerWidget> newWidget = CreateRef<RuntimeUI::ContainerWidget>(s_UIWindow->m_EditorUI.get());
		EditorUI::TreeEntry* newEntry = AddWidgetInternal(*parentEntry, newWidget, Utility::WidgetTypeToIcon(RuntimeUI::WidgetTypes::ContainerWidget));
		KG_ASSERT(newEntry);
	}

	void UIEditorTreePanel::AddHorizontalContainerWidget(EditorUI::TooltipEntry& entry)
	{
		// Get the widget entry
		EditorUI::TreeEntry* parentEntry = (EditorUI::TreeEntry*)entry.m_ProvidedData;
		KG_ASSERT(parentEntry);

		// Create new widget
		Ref<RuntimeUI::HorizontalContainerWidget> newWidget = CreateRef<RuntimeUI::HorizontalContainerWidget>(s_UIWindow->m_EditorUI.get());
		EditorUI::TreeEntry* newEntry = AddWidgetInternal(*parentEntry, newWidget, Utility::WidgetTypeToIcon(RuntimeUI::WidgetTypes::HorizontalContainerWidget));
		KG_ASSERT(newEntry);
	}

	void UIEditorTreePanel::AddVerticalContainerWidget(EditorUI::TooltipEntry& entry)
	{
		// Get the widget entry
		EditorUI::TreeEntry* parentEntry = (EditorUI::TreeEntry*)entry.m_ProvidedData;
		KG_ASSERT(parentEntry);

		// Create new widget
		Ref<RuntimeUI::VerticalContainerWidget> newWidget = CreateRef<RuntimeUI::VerticalContainerWidget>(s_UIWindow->m_EditorUI.get());
		EditorUI::TreeEntry* newEntry = AddWidgetInternal(*parentEntry, newWidget, Utility::WidgetTypeToIcon(RuntimeUI::WidgetTypes::VerticalContainerWidget));
		KG_ASSERT(newEntry);
	}

	void UIEditorTreePanel::AddInputTextWidget(EditorUI::TooltipEntry& entry)
	{
		// Get the widget entry
		EditorUI::TreeEntry* parentEntry = (EditorUI::TreeEntry*)entry.m_ProvidedData;
		KG_ASSERT(parentEntry);

		// Create new widget
		Ref<RuntimeUI::InputTextWidget> newWidget = CreateRef<RuntimeUI::InputTextWidget>(s_UIWindow->m_EditorUI.get());
		AddWidgetInternal(*parentEntry, newWidget, Utility::WidgetTypeToIcon(RuntimeUI::WidgetTypes::InputTextWidget));
	}

	void UIEditorTreePanel::AddSliderWidget(EditorUI::TooltipEntry& entry)
	{
		// Get the widget entry
		EditorUI::TreeEntry* parentEntry = (EditorUI::TreeEntry*)entry.m_ProvidedData;
		KG_ASSERT(parentEntry);

		// Create new widget
		Ref<RuntimeUI::SliderWidget> newWidget = CreateRef<RuntimeUI::SliderWidget>(s_UIWindow->m_EditorUI.get());
		AddWidgetInternal(*parentEntry, newWidget, Utility::WidgetTypeToIcon(RuntimeUI::WidgetTypes::SliderWidget));
	}

	void UIEditorTreePanel::AddDropDownWidget(EditorUI::TooltipEntry& entry)
	{
		// Get the widget entry
		EditorUI::TreeEntry* parentEntry = (EditorUI::TreeEntry*)entry.m_ProvidedData;
		KG_ASSERT(parentEntry);

		// Create new widget
		Ref<RuntimeUI::DropDownWidget> newWidget = CreateRef<RuntimeUI::DropDownWidget>(s_UIWindow->m_EditorUI.get());
		AddWidgetInternal(*parentEntry, newWidget, Utility::WidgetTypeToIcon(RuntimeUI::WidgetTypes::DropDownWidget));
	}

	void UIEditorTreePanel::SelectWidget(EditorUI::TreeEntry& entry)
	{
		// Get the current widget and its parent window
		RuntimeUI::Window& parentWindow = RuntimeUI::RuntimeUIService::GetActiveContext().m_ActiveUI->m_WindowsState.GetParentWindowFromWidgetID((int32_t)entry.m_Handle);
		Ref<RuntimeUI::Widget> currentWidget = RuntimeUI::RuntimeUIService::GetActiveContext().m_ActiveUI->m_WindowsState.GetWidgetFromID((int32_t)entry.m_Handle);
		KG_ASSERT(currentWidget);

		// Set the active window/widget
		s_UIWindow->m_PropertiesPanel->m_ActiveWindow = &parentWindow;
		s_UIWindow->m_PropertiesPanel->m_ActiveWidget = currentWidget.get();

		// Set the properties panel to display a widget
		s_UIWindow->m_PropertiesPanel->m_CurrentDisplay = UIPropertiesDisplay::Widget;
		s_UIWindow->m_PropertiesPanel->OnSelectWidget();
		EditorUI::TreePath entryPath = m_UITree.GetPathFromEntryReference(&entry);
		m_UITree.ExpandNodePath(entryPath);

		// TODO: Deal with local properties panel
		//EditorUI::EditorUIService::BringWindowToFront(s_MainWindow->m_PropertiesPanel->m_PanelName);
		//s_MainWindow->m_PropertiesPanel->m_ActiveParent = m_PanelName;
	}

	void UIEditorTreePanel::RenameWidget(EditorUI::TooltipEntry& entry)
	{
		// Get the tree entry
		EditorUI::TreeEntry* widgetTreeEntry = (EditorUI::TreeEntry*)entry.m_ProvidedData;
		KG_ASSERT(widgetTreeEntry);

		// Get tree path from provided entry
		EditorUI::TreePath path = m_UITree.GetPathFromEntryReference(widgetTreeEntry);
		if (!path)
		{
			KG_WARN("Could not locate widget path inside m_UITree");
			return;
		}

		// Select the tree entry
		SelectWidget(*widgetTreeEntry);

		// Open the rename dialog
		s_UIWindow->m_PropertiesPanel->OpenWidgetTagDialog();
	}

	void UIEditorTreePanel::DeleteWindow(EditorUI::TooltipEntry& entry)
	{
		// Get the tree entry
		EditorUI::TreeEntry* windowTreeEntry = (EditorUI::TreeEntry*)entry.m_ProvidedData;
		KG_ASSERT(windowTreeEntry);

		// Get tree path from provided entry
		EditorUI::TreePath path = m_UITree.GetPathFromEntryReference(windowTreeEntry);
		if (!path)
		{
			KG_WARN("Could not locate window path inside m_UITree");
			return;
		}

		// Remove window from active runtime UI and this panel's tree
		RuntimeUI::RuntimeUIService::GetActiveContext().m_ActiveUI->GetWindowsState().DeleteWindow((int32_t)windowTreeEntry->m_Handle);
		m_UITree.RemoveEntry(path);

		// Reset properties panel and ensure tree index data is valid
		s_UIWindow->m_PropertiesPanel->ClearPanelData();
		RecalculateTreeIndexData();

		// Set the active editor UI as edited
		m_MainHeader.m_EditColorActive = true;

	}

	void UIEditorTreePanel::RightClickWindowEntry(EditorUI::TreeEntry& entry)
	{
		// Reset the tooltip
		m_SelectTooltip.ClearEntries();

		// Create the toggle-visibility window entry
		EditorUI::TooltipEntry isVisibleWindowEntry{ "Toggle Visible", KG_BIND_CLASS_FN(ToggleWindowVisibility) };
		isVisibleWindowEntry.m_ProvidedData = &entry;

		// Create the rename window entry
		EditorUI::TooltipEntry renameWindowEntry{ "Rename", KG_BIND_CLASS_FN(RenameWindow) };
		renameWindowEntry.m_ProvidedData = &entry;

		// Create the delete window entry
		EditorUI::TooltipEntry deleteWindowEntry{ "Delete", KG_BIND_CLASS_FN(DeleteWindow) };
		deleteWindowEntry.m_ProvidedData = &entry;

		// Create the edit menu and edit it the the top-level tooltip
		EditorUI::TooltipEntry editMenu{ "Edit", { isVisibleWindowEntry, renameWindowEntry, deleteWindowEntry } };
		m_SelectTooltip.AddTooltipEntry(editMenu);

		// Create the add menu
		CreateAddWidgetsSelectionOptions(entry);

		// Active the tooltip
		m_SelectTooltip.m_TooltipActive = true;
	}

	void UIEditorTreePanel::SelectWindow(EditorUI::TreeEntry& entry)
	{
		// Get the runtimeUI window reference
		RuntimeUI::Window& currentWindow = RuntimeUI::RuntimeUIService::GetActiveContext().m_ActiveUI->m_WindowsState.GetWindowFromID((int32_t)entry.m_Handle);

		// Set current window as active and reset properties panel data
		s_UIWindow->m_PropertiesPanel->ClearPanelData();
		s_UIWindow->m_PropertiesPanel->m_ActiveWindow = &currentWindow;
		s_UIWindow->m_PropertiesPanel->m_CurrentDisplay = UIPropertiesDisplay::Window;


		// TODO: Deal with local properties panel
		// Display window properties in properties panel
		//s_MainWindow->m_PropertiesPanel->m_ActiveParent = m_PanelName;

		// Bring properties panel to front
		//EditorUI::EditorUIService::BringWindowToFront(s_MainWindow->m_PropertiesPanel->m_PanelName);
	}

	void UIEditorTreePanel::ToggleWindowVisibility(EditorUI::TooltipEntry& entry)
	{
		// Get the tree entry
		EditorUI::TreeEntry* windowTreeEntry = (EditorUI::TreeEntry*)entry.m_ProvidedData;
		KG_ASSERT(windowTreeEntry);

		// Get tree path from provided entry
		EditorUI::TreePath path = m_UITree.GetPathFromEntryReference(windowTreeEntry);
		if (!path)
		{
			KG_WARN("Could not locate window path inside m_UITree");
			return;
		}

		// Get the underlying window
		RuntimeUI::Window& window = RuntimeUI::RuntimeUIService::GetActiveContext().m_ActiveUI->m_WindowsState.GetWindowFromID((int32_t)windowTreeEntry->m_Handle);

		// Select the tree entry
		SelectWindow(*windowTreeEntry);

		// Toggle window visibility
		if (window.GetWindowDisplayed())
		{
			window.HideWindow();
		}
		else
		{
			window.DisplayWindow();
		}

	}

	void UIEditorTreePanel::RenameWindow(EditorUI::TooltipEntry& entry)
	{
		// Get the tree entry
		EditorUI::TreeEntry* windowTreeEntry = (EditorUI::TreeEntry*)entry.m_ProvidedData;
		KG_ASSERT(windowTreeEntry);

		// Get tree path from provided entry
		EditorUI::TreePath path = m_UITree.GetPathFromEntryReference(windowTreeEntry);
		if (!path)
		{
			KG_WARN("Could not locate window path inside m_UITree");
			return;
		}

		// Select the tree entry
		SelectWindow(*windowTreeEntry);

		// Open the rename dialog
		s_UIWindow->m_PropertiesPanel->OpenWindowTagDialog();
	}

	void UIEditorTreePanel::AddWindow(EditorUI::TooltipEntry& entry)
	{
		// Get the tree entry
		EditorUI::TreeEntry* uiTreeEntry = (EditorUI::TreeEntry*)entry.m_ProvidedData;
		KG_ASSERT(uiTreeEntry);

		// Get tree path from provided entry
		EditorUI::TreePath path = m_UITree.GetPathFromEntryReference(uiTreeEntry);
		if (!path)
		{
			KG_WARN("Could not locate UI path inside m_UITree");
			return;
		}

		// Create new window entry for m_UITree
		EditorUI::TreeEntry& newEntry = uiTreeEntry->m_SubEntries.emplace_back();
		newEntry.m_Label = "None";
		newEntry.m_IconHandle = EditorUI::EditorUIService::s_IconWindow;
		

		// Add window selection options
		CreateWindowSelectionOptions(newEntry);

		// Add new window to RuntimeUI and this panel's tree
		RuntimeUI::Window newWindow{s_UIWindow->m_EditorUI.get()};
		RuntimeUI::RuntimeUIService::GetActiveContext().m_ActiveUI->m_WindowsState.AddWindow(newWindow);
		newEntry.m_Handle = newWindow.m_ID;

		// Select the newly created window
		SelectWindow(newEntry);

		// Set this UI as edited
		m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorTreePanel::DeleteWidget(EditorUI::TooltipEntry& entry)
	{
		// Get the widget entry
		EditorUI::TreeEntry* widgetEntry = (EditorUI::TreeEntry*)entry.m_ProvidedData;
		KG_ASSERT(widgetEntry);

		// Getpath from provided entry
		EditorUI::TreePath path = m_UITree.GetPathFromEntryReference(widgetEntry);
		if (!path)
		{
			KG_WARN("Could not locate widget path inside m_UITree");
			return;
		}

		// Remove widget from RuntimeUI 
		bool success = RuntimeUI::RuntimeUIService::GetActiveContext().m_ActiveUI->m_WindowsState.DeleteWidget((int32_t)widgetEntry->m_Handle);

		// Check if widget was successfully deleted
		if (!success)
		{
			KG_WARN("Could not delete widget from RuntimeUI");
			return;
		}

		// Remove widget from panel's tree widget
		m_UITree.RemoveEntry(path);

		// Reset properties panel and ensure tree index data is valid
		s_UIWindow->m_PropertiesPanel->ClearPanelData();
		RecalculateTreeIndexData();

		// Set the active editor UI as edited
		m_MainHeader.m_EditColorActive = true;

	}

	EditorUI::TreeEntry* UIEditorTreePanel::AddWidgetInternal(EditorUI::TreeEntry& parentEntry, Ref<RuntimeUI::Widget> newWidget, Ref<Rendering::Texture2D> widgetIcon)
	{
		RuntimeUI::RuntimeUIContext& uiContext{ RuntimeUI::RuntimeUIService::GetActiveContext()};
		RuntimeUI::IDType parentType = uiContext.m_ActiveUI->m_WindowsState.CheckIDType((int32_t)parentEntry.m_Handle);

		switch (parentType)
		{
		case RuntimeUI::IDType::Window:
		{
			// Get the parent window
			RuntimeUI::Window& window = uiContext.m_ActiveUI->m_WindowsState.GetWindowFromID((int32_t)parentEntry.m_Handle);

			// Add the newWidget to the parent window
			window.AddWidget(newWidget);
			break;
		}
		case RuntimeUI::IDType::Widget:
		{
			// Get the parent widget
			Ref<RuntimeUI::Widget> parentWidget = uiContext.m_ActiveUI->m_WindowsState.GetWidgetFromID((int32_t)parentEntry.m_Handle);
			KG_ASSERT(parentWidget);

			// Get the parent widget's container data
			RuntimeUI::ContainerData* data = parentWidget->GetContainerData();
			KG_ASSERT(data);

			if (parentWidget->m_WidgetType == RuntimeUI::WidgetTypes::VerticalContainerWidget ||
				parentWidget->m_WidgetType == RuntimeUI::WidgetTypes::HorizontalContainerWidget)
			{
				// Set appropriate default values
				newWidget->m_PercentSize = { 1.0f, 1.0f };
			}
			
			// Add the new widget to the container data
			data->AddWidget(newWidget);
			break;
		}
		case RuntimeUI::IDType::None:
		default:
			KG_ERROR("Invalid IDType returned when creating a new widget");
			break;
		}

		// Create new widget entry for m_UITree
		EditorUI::TreeEntry& newWidgetEntry = parentEntry.m_SubEntries.emplace_back();

		newWidgetEntry.m_Label = newWidget->m_Tag;
		newWidgetEntry.m_IconHandle = widgetIcon;
		newWidgetEntry.m_Handle = (uint64_t)newWidget->m_ID;

		// Add handlers for interacting with the tree entry
		newWidgetEntry.m_OnLeftClick = KG_BIND_CLASS_FN(SelectWidget);
		newWidgetEntry.m_OnRightClick = KG_BIND_CLASS_FN(RightClickWidgetEntry);

		// Set the active editor UI as edited
		m_MainHeader.m_EditColorActive = true;

		return &newWidgetEntry;
	}

	void UIEditorTreePanel::CreateWindowSelectionOptions(EditorUI::TreeEntry& windowEntry)
	{
		// Add functions to call when interacting with window entry
		windowEntry.m_OnLeftClick = KG_BIND_CLASS_FN(SelectWindow);
		windowEntry.m_OnRightClick = KG_BIND_CLASS_FN(RightClickWindowEntry);
	}

	void UIEditorTreePanel::CreateAddWidgetsSelectionOptions(EditorUI::TreeEntry& entry)
	{
		RuntimeUI::RuntimeUIContext& uiContext{ RuntimeUI::RuntimeUIService::GetActiveContext() };
		RuntimeUI::IDType type = uiContext.m_ActiveUI->m_WindowsState.CheckIDType((int32_t)entry.m_Handle);

		KG_ASSERT(type != RuntimeUI::IDType::None);

		// Main vector of items
		std::vector<EditorUI::TooltipEntry> addItems;
		std::vector<EditorUI::TooltipEntry> subMenus;

		// Ensure the underlying widget is a container
		if (type == RuntimeUI::IDType::Widget)
		{
			Ref<RuntimeUI::Widget> widget = uiContext.m_ActiveUI->m_WindowsState.GetWidgetFromID((int32_t)entry.m_Handle);
			KG_ASSERT(widget);

			RuntimeUI::ContainerData* container = widget->GetContainerData();

			if (!container)
			{
				return;
			}
		}
		// Implicitly allowing windows to work

		// Add the text widget tooltip entry
		{
			// Create the tooltip entry
			EditorUI::TooltipEntry addWidget
			{
				Utility::WidgetTypeToDisplayString(RuntimeUI::WidgetTypes::TextWidget),
				KG_BIND_CLASS_FN(AddTextWidget)
			};
			addWidget.m_ProvidedData = &entry;

			// Add the entry to the tooltip menu
			addItems.push_back(addWidget);
		}

		// Add the InputText widget tooltip entry
		{
			// Create the tooltip entry
			EditorUI::TooltipEntry addWidget
			{
				Utility::WidgetTypeToDisplayString(RuntimeUI::WidgetTypes::InputTextWidget),
				KG_BIND_CLASS_FN(AddInputTextWidget)
			};
			addWidget.m_ProvidedData = &entry;

			// Add the entry to the tooltip menu
			addItems.push_back(addWidget);
		}

		// Create a sub-menu and add it to the main add items menu
		EditorUI::TooltipEntry textMenu{ "Text", addItems };
		subMenus.push_back(textMenu);
		addItems.clear();

		// Add the button widget tooltip entry
		{
			// Create the tooltip entry
			EditorUI::TooltipEntry addWidget
			{
				Utility::WidgetTypeToDisplayString(RuntimeUI::WidgetTypes::ButtonWidget),
				KG_BIND_CLASS_FN(AddButtonWidget)
			};
			addWidget.m_ProvidedData = &entry;

			// Add the entry to the tooltip menu
			addItems.push_back(addWidget);
		}

		// Add the ImageButton widget tooltip entry
		{
			// Create the tooltip entry
			EditorUI::TooltipEntry addWidget
			{
				Utility::WidgetTypeToDisplayString(RuntimeUI::WidgetTypes::ImageButtonWidget),
				KG_BIND_CLASS_FN(AddImageButtonWidget)
			};
			addWidget.m_ProvidedData = &entry;

			// Add the entry to the tooltip menu
			addItems.push_back(addWidget);
		}

		// Add the Checkbox widget tooltip entry
		{
			// Create the tooltip entry
			EditorUI::TooltipEntry addWidget
			{
				Utility::WidgetTypeToDisplayString(RuntimeUI::WidgetTypes::CheckboxWidget),
				KG_BIND_CLASS_FN(AddCheckboxWidget)
			};
			addWidget.m_ProvidedData = &entry;

			// Add the entry to the tooltip menu
			addItems.push_back(addWidget);
		}

		// Create a sub-menu and add it to the main add items menu
		EditorUI::TooltipEntry buttonMenu{ "Button", addItems };
		subMenus.push_back(buttonMenu);
		addItems.clear();

		// Add the Image widget tooltip entry
		{
			// Create the tooltip entry
			EditorUI::TooltipEntry addWidget
			{
				Utility::WidgetTypeToDisplayString(RuntimeUI::WidgetTypes::ImageWidget),
				KG_BIND_CLASS_FN(AddImageWidget)
			};
			addWidget.m_ProvidedData = &entry;

			// Add the entry to the tooltip menu
			addItems.push_back(addWidget);
		}
		
		// Create a sub-menu and add it to the main add items menu
		EditorUI::TooltipEntry imageMenu{ "Image", addItems };
		subMenus.push_back(imageMenu);
		addItems.clear();

		// Add the Container widget tooltip entry
		{
			// Create the tooltip entry
			EditorUI::TooltipEntry addWidget
			{
				Utility::WidgetTypeToDisplayString(RuntimeUI::WidgetTypes::ContainerWidget),
				KG_BIND_CLASS_FN(AddContainerWidget)
			};
			addWidget.m_ProvidedData = &entry;

			// Add the entry to the tooltip menu
			addItems.push_back(addWidget);
		}

		// Add the Horizontal Container widget tooltip entry
		{
			// Create the tooltip entry
			EditorUI::TooltipEntry addWidget
			{
				Utility::WidgetTypeToDisplayString(RuntimeUI::WidgetTypes::HorizontalContainerWidget),
				KG_BIND_CLASS_FN(AddHorizontalContainerWidget)
			};
			addWidget.m_ProvidedData = &entry;

			// Add the entry to the tooltip menu
			addItems.push_back(addWidget);
		}

		// Add the Vertical Container widget tooltip entry
		{
			// Create the tooltip entry
			EditorUI::TooltipEntry addWidget
			{
				Utility::WidgetTypeToDisplayString(RuntimeUI::WidgetTypes::VerticalContainerWidget),
				KG_BIND_CLASS_FN(AddVerticalContainerWidget)
			};
			addWidget.m_ProvidedData = &entry;

			// Add the entry to the tooltip menu
			addItems.push_back(addWidget);
		}

		// Create a sub-menu and add it to the main add items menu
		EditorUI::TooltipEntry containerMenu{ "Container", addItems };
		subMenus.push_back(containerMenu);
		addItems.clear();

		// Add the Slider widget tooltip entry
		{
			// Create the tooltip entry
			EditorUI::TooltipEntry addWidget
			{
				Utility::WidgetTypeToDisplayString(RuntimeUI::WidgetTypes::SliderWidget),
				KG_BIND_CLASS_FN(AddSliderWidget)
			};
			addWidget.m_ProvidedData = &entry;

			// Add the entry to the tooltip menu
			addItems.push_back(addWidget);
		}

		// Create a sub-menu and add it to the main add items menu
		EditorUI::TooltipEntry scrollMenu{ "Scroll", addItems };
		subMenus.push_back(scrollMenu);
		addItems.clear();

		// Add the DropDown widget tooltip entry
		{
			// Create the tooltip entry
			EditorUI::TooltipEntry addWidget
			{
				Utility::WidgetTypeToDisplayString(RuntimeUI::WidgetTypes::DropDownWidget),
				KG_BIND_CLASS_FN(AddDropDownWidget)
			};
			addWidget.m_ProvidedData = &entry;

			// Add the entry to the tooltip menu
			addItems.push_back(addWidget);
		}

		// Create a sub-menu and add it to the main add items menu
		EditorUI::TooltipEntry selectMenu{ "Select", addItems };
		subMenus.push_back(selectMenu);
		addItems.clear();

		// Create the add menu and add it the the top-level tooltip
		EditorUI::TooltipEntry addMenu{ "Add", subMenus};
		m_SelectTooltip.AddTooltipEntry(addMenu);
	}

	void UIEditorTreePanel::CreateWidgetSpecificSelectionOptions(EditorUI::TreeEntry& widgetEntry, RuntimeUI::WidgetTypes widgetType)
	{
		if (widgetType == RuntimeUI::WidgetTypes::ContainerWidget || 
			widgetType == RuntimeUI::WidgetTypes::HorizontalContainerWidget ||
			widgetType == RuntimeUI::WidgetTypes::VerticalContainerWidget)
		{
			// Add container widget options
			CreateAddWidgetsSelectionOptions(widgetEntry);
		}
	}

	void UIEditorTreePanel::CreateContainerDataWidgets(EditorUI::TreeEntry& parentEntry, RuntimeUI::ContainerData* container)
	{
		KG_ASSERT(container);

		// Add all of the widgets
		for (Ref<RuntimeUI::Widget> childWidget : container->m_ContainedWidgets)
		{
			// Add the widget to the parent entry
			CreateWidgetTreeEntry(parentEntry, childWidget);
		}
	}

	void UIEditorTreePanel::CreateWidgetTreeEntry(EditorUI::TreeEntry& parentEntry, Ref<RuntimeUI::Widget> currentWidget)
	{
		// Create new widget entry and add it to the parent's sub entries list
		EditorUI::TreeEntry& widgetEntry = parentEntry.m_SubEntries.emplace_back();
		
		// Add base widget information
		widgetEntry.m_Label = currentWidget->m_Tag;
		widgetEntry.m_IconHandle = Utility::WidgetTypeToIcon(currentWidget->m_WidgetType);
		widgetEntry.m_Handle = currentWidget->m_ID;

		// Add functions to call when interacting with widget entry
		widgetEntry.m_OnLeftClick = KG_BIND_CLASS_FN(SelectWidget);
		widgetEntry.m_OnRightClick = KG_BIND_CLASS_FN(RightClickWidgetEntry);

		// Check for a container widget
		RuntimeUI::ContainerData* containerData = currentWidget->GetContainerData();
		if (containerData)
		{
			CreateContainerDataWidgets(widgetEntry, containerData);
		}
	}

	void UIEditorTreePanel::RecalculateTreeIndexData()
	{
#if 0
		// Recalculate the handle for each window entry in the user interface tree
		m_UITree.EditDepth([&](EditorUI::TreeEntry& entry)
		{
			// Get tree path from entry reference
			EditorUI::TreePath entryPath = m_UITree.GetPathFromEntryReference(&entry);

			// Ensure the depth is 2, since this is the expected depth for the windows
			if (entryPath.GetDepth() != 2)
			{
				KG_WARN("Invalid depth length when revalidating window index information");
				return;
			}

			// Get the window index from the path
			uint16_t currentWindow = entryPath.GetBack();

			// Update the handle for the window entry
			entry.m_Handle = (uint64_t)currentWindow;

		}, 1);


		// Recalculate the handle and provided data for each widget entry in the user interface tree
		m_UITree.EditDepth([&](EditorUI::TreeEntry& entry)
		{
			// Get tree path from entry reference
			EditorUI::TreePath entryPath = m_UITree.GetPathFromEntryReference(&entry);

			// Ensure the depth is 2, since this is the expected depth for the user interface tree
			if (entryPath.GetDepth() != 3)
			{
				KG_WARN("Invalid depth length when revalidating widget index information");
				return;
			}

			// Get the current widget and window index from the path
			uint16_t currentWidget = entryPath.GetBack();
			entryPath.PopBack();
			uint16_t currentWindow = entryPath.GetBack();

			// Update the handle and provided data for the widget entry
			entry.m_ProvidedData = CreateRef<uint32_t>((uint32_t)currentWindow);

		}, 2);
#endif
	}
	void UIEditorTreePanel::SelectTreeNode(RuntimeUI::IDType idType, int32_t windowOrWidgetID)
	{
		// Ensure the id type provided is not invalid
		if (idType == RuntimeUI::IDType::None)
		{
			return;
		}

		// Get the location of the indicated widget/window inside the active UI
		std::vector<uint16_t>* locationInRuntimeUI = RuntimeUI::RuntimeUIService::GetActiveContext().m_ActiveUI->m_WindowsState.GetLocationFromID(windowOrWidgetID);
		KG_ASSERT(locationInRuntimeUI);
		KG_ASSERT(locationInRuntimeUI->size() > 0);

		uint16_t windowIndex = locationInRuntimeUI->at(0);
		EditorUI::TreePath newTreePath;

		// Add UI node
		newTreePath.PushBackNode(0);

		// Add window node
		newTreePath.PushBackNode(windowIndex);

		// Handle selecting a window node
		if (idType == RuntimeUI::IDType::Window)
		{
			m_UITree.ExpandFirstLayer();
			bool success = m_UITree.SelectEntry(newTreePath);
			KG_ASSERT(success);
			return;
		}

		// Loop through all remaining widget indices
		for (size_t uiLocationIndex{ 1 }; uiLocationIndex < locationInRuntimeUI->size(); uiLocationIndex++)
		{
			// Add the indicated widget index to the tree path
			uint16_t widgetIndex = locationInRuntimeUI->at(uiLocationIndex);
			newTreePath.PushBackNode(widgetIndex);
		}

		// Expand the newly created path and select the widget
		m_UITree.ExpandNodePath(newTreePath);
		bool success = m_UITree.SelectEntry(newTreePath);
		KG_ASSERT(success);
	}
}

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
			EditorUI::EditorUIService::Tooltip(m_SelectTooltip);
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
		uiEntry.m_OnRightClickSelection.push_back({ "Add Window", KG_BIND_CLASS_FN(AddWindow) });

		// Add all windows and widgets from the editor UI to the tree
		std::size_t windowIterator{ 0 };
		for (RuntimeUI::Window& window : s_UIWindow->m_EditorUI->m_Windows)
		{
			// Create new window entry
			EditorUI::TreeEntry windowEntry{};
			windowEntry.m_Label = window.m_Tag;
			windowEntry.m_IconHandle = EditorUI::EditorUIService::s_IconWindow;
			windowEntry.m_Handle = windowIterator;

			// Add window selection options
			CreateWindowSelectionOptions(windowEntry);

			// Add widgets to window entry
			std::size_t widgetIterator{ 0 };
			for (Ref<RuntimeUI::Widget> widget : window.m_Widgets)
			{
				// TODO: Note this only creates text widgets for now

				// Create new widget entry
				EditorUI::TreeEntry widgetEntry{};
				widgetEntry.m_Label = widget->m_Tag;
				widgetEntry.m_IconHandle = Utility::WidgetTypeToIcon(widget->m_WidgetType);

				// Provide widget/window ID's
				widgetEntry.m_ProvidedData = CreateRef<uint32_t>((uint32_t)windowIterator);
				widgetEntry.m_Handle = widgetIterator;

				// Add functions to call when interacting with widget entry
				widgetEntry.m_OnLeftClick = KG_BIND_CLASS_FN(SelectWidget);
				widgetEntry.m_OnRightClickSelection.push_back({ "Delete Widget", KG_BIND_CLASS_FN(DeleteWidget) });

				// Add widget entry to window entry
				windowEntry.m_SubEntries.push_back(widgetEntry);
				widgetIterator++;
			}

			// Add window entry to the user interface tree
			uiEntry.m_SubEntries.push_back(windowEntry);
			windowIterator++;
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
				RuntimeUI::RuntimeUIService::ClearActiveUI();
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
				RuntimeUI::RuntimeUIService::ClearActiveUI();
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
					RuntimeUI::RuntimeUIService::ClearActiveUI();
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
		m_OpenUIPopupSpec.m_PopupAction = [&](EditorUI::SelectOptionSpec& spec)
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
				RuntimeUI::RuntimeUIService::SetActiveUI(s_UIWindow->m_EditorUI, s_UIWindow->m_EditorUIHandle);
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
		m_SelectUILocationSpec.m_ConfirmAction = [&](std::string_view path)
		{
			if (!Utility::FileSystem::DoesPathContainSubPath(Projects::ProjectService::GetActiveAssetDirectory(), path))
			{
				KG_WARN("Cannot create an asset outside of the project's asset directory.");
				m_SelectUILocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
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
		// Set default values for new user interface creation location
		KG_ASSERT(Projects::ProjectService::GetActive());
		m_SelectUILocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();

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
		RuntimeUI::RuntimeUIService::SetActiveUI(s_UIWindow->m_EditorUI, s_UIWindow->m_EditorUIHandle);
	}

	void UIEditorTreePanel::SelectUI(EditorUI::TreeEntry& entry)
	{
		UNREFERENCED_PARAMETER(entry);
		s_UIWindow->m_PropertiesPanel->ClearPanelData();
		s_UIWindow->m_PropertiesPanel->m_CurrentDisplay = UIPropertiesDisplay::UserInterface;
	}

	void UIEditorTreePanel::AddTextWidget(EditorUI::TreeEntry& entry)
	{
		// Create new widget
		Ref<RuntimeUI::TextWidget> newWidget = CreateRef<RuntimeUI::TextWidget>();
		AddWidgetInternal(entry, newWidget, Utility::WidgetTypeToIcon(RuntimeUI::WidgetTypes::TextWidget));
	}

	void UIEditorTreePanel::AddButtonWidget(EditorUI::TreeEntry& entry)
	{
		// Create new widget
		Ref<RuntimeUI::ButtonWidget> newWidget = CreateRef<RuntimeUI::ButtonWidget>();
		AddWidgetInternal(entry, newWidget, Utility::WidgetTypeToIcon(RuntimeUI::WidgetTypes::ButtonWidget));
	}

	void UIEditorTreePanel::AddImageWidget(EditorUI::TreeEntry& entry)
	{
		// Create new widget
		Ref<RuntimeUI::ImageWidget> newWidget = CreateRef<RuntimeUI::ImageWidget>();
		AddWidgetInternal(entry, newWidget, Utility::WidgetTypeToIcon(RuntimeUI::WidgetTypes::ImageWidget));
	}

	void UIEditorTreePanel::AddImageButtonWidget(EditorUI::TreeEntry& entry)
	{
		// Create new widget
		Ref<RuntimeUI::ImageButtonWidget> newWidget = CreateRef<RuntimeUI::ImageButtonWidget>();
		AddWidgetInternal(entry, newWidget, Utility::WidgetTypeToIcon(RuntimeUI::WidgetTypes::ImageButtonWidget));
	}

	void UIEditorTreePanel::AddCheckboxWidget(EditorUI::TreeEntry& entry)
	{
		// Create new widget
		Ref<RuntimeUI::CheckboxWidget> newWidget = CreateRef<RuntimeUI::CheckboxWidget>();
		AddWidgetInternal(entry, newWidget, Utility::WidgetTypeToIcon(RuntimeUI::WidgetTypes::CheckboxWidget));
	}

	void UIEditorTreePanel::AddInputTextWidget(EditorUI::TreeEntry& entry)
	{
		// Create new widget
		Ref<RuntimeUI::InputTextWidget> newWidget = CreateRef<RuntimeUI::InputTextWidget>();
		AddWidgetInternal(entry, newWidget, Utility::WidgetTypeToIcon(RuntimeUI::WidgetTypes::InputTextWidget));
	}

	void UIEditorTreePanel::AddSliderWidget(EditorUI::TreeEntry& entry)
	{
		// Create new widget
		Ref<RuntimeUI::SliderWidget> newWidget = CreateRef<RuntimeUI::SliderWidget>();
		AddWidgetInternal(entry, newWidget, Utility::WidgetTypeToIcon(RuntimeUI::WidgetTypes::SliderWidget));
	}

	void UIEditorTreePanel::AddDropDownWidget(EditorUI::TreeEntry& entry)
	{
		// Create new widget
		Ref<RuntimeUI::DropDownWidget> newWidget = CreateRef<RuntimeUI::DropDownWidget>();
		AddWidgetInternal(entry, newWidget, Utility::WidgetTypeToIcon(RuntimeUI::WidgetTypes::DropDownWidget));
	}

	void UIEditorTreePanel::SelectWidget(EditorUI::TreeEntry& entry)
	{
		s_UIWindow->m_PropertiesPanel->m_ActiveWindow = &s_UIWindow->m_EditorUI->m_Windows.at(*(uint32_t*)entry.m_ProvidedData.get());
		s_UIWindow->m_PropertiesPanel->m_ActiveWidget = s_UIWindow->m_PropertiesPanel->m_ActiveWindow->m_Widgets.at(entry.m_Handle).get();
		s_UIWindow->m_PropertiesPanel->m_CurrentDisplay = UIPropertiesDisplay::Widget;
		s_UIWindow->m_PropertiesPanel->OnSelectWidget();

		// TODO: Deal with local properties panel
		//EditorUI::EditorUIService::BringWindowToFront(s_MainWindow->m_PropertiesPanel->m_PanelName);
		//s_MainWindow->m_PropertiesPanel->m_ActiveParent = m_PanelName;
	}

	void UIEditorTreePanel::DeleteWindow(EditorUI::TreeEntry& entry)
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
		s_UIWindow->m_PropertiesPanel->ClearPanelData();
		RecalculateTreeIndexData();

		// Set the active editor UI as edited
		m_MainHeader.m_EditColorActive = true;

	}

	void UIEditorTreePanel::SelectWindow(EditorUI::TreeEntry& entry)
	{
		// Set selected window as active
		s_UIWindow->m_PropertiesPanel->ClearPanelData();
		s_UIWindow->m_PropertiesPanel->m_ActiveWindow = &s_UIWindow->m_EditorUI->m_Windows.at(entry.m_Handle);
		s_UIWindow->m_PropertiesPanel->m_CurrentDisplay = UIPropertiesDisplay::Window;


		// TODO: Deal with local properties panel
		// Display window properties in properties panel
		//s_MainWindow->m_PropertiesPanel->m_ActiveParent = m_PanelName;

		// Bring properties panel to front
		//EditorUI::EditorUIService::BringWindowToFront(s_MainWindow->m_PropertiesPanel->m_PanelName);
	}

	void UIEditorTreePanel::AddWindow(EditorUI::TreeEntry& entry)
	{
		// Get tree path from provided entry
		EditorUI::TreePath path = m_UITree.GetPathFromEntryReference(&entry);
		if (!path)
		{
			KG_WARN("Could not locate UI path inside m_UITree");
			return;
		}

		// Create new window entry for m_UITree
		EditorUI::TreeEntry newEntry{};
		newEntry.m_Label = "None";
		newEntry.m_IconHandle = EditorUI::EditorUIService::s_IconWindow;
		newEntry.m_Handle = entry.m_SubEntries.size();

		// Add window selection options
		CreateWindowSelectionOptions(newEntry);

		// Add new window to RuntimeUI and this panel's tree
		entry.m_SubEntries.push_back(newEntry);
		RuntimeUI::Window newWindow{};
		RuntimeUI::RuntimeUIService::AddActiveWindow(newWindow);

		// Select the newly created window
		SelectWindow(newEntry);

		// Set this UI as edited
		m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorTreePanel::DeleteWidget(EditorUI::TreeEntry& entry)
	{
		// Getpath from provided entry
		EditorUI::TreePath path = m_UITree.GetPathFromEntryReference(&entry);
		if (!path)
		{
			KG_WARN("Could not locate widget path inside m_UITree");
			return;
		}

		// Remove widget from RuntimeUI 
		bool success = RuntimeUI::RuntimeUIService::DeleteActiveUIWidget((size_t) * (uint32_t*)entry.m_ProvidedData.get(), entry.m_Handle);

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

	void UIEditorTreePanel::AddWidgetInternal(EditorUI::TreeEntry& entry, Ref<RuntimeUI::Widget> newWidget, Ref<Rendering::Texture2D> widgetIcon)
	{
		// Get window path from provided entry and ensure it is valid
		EditorUI::TreePath windowPath = m_UITree.GetPathFromEntryReference(&entry);
		if (!windowPath)
		{
			KG_WARN("Could not locate window path inside m_UITree");
			return;
		}
		RuntimeUI::Window& window = s_UIWindow->m_EditorUI->m_Windows.at(entry.m_Handle);

		// Create new widget entry for m_UITree
		EditorUI::TreeEntry newWidgetEntry{};
		newWidgetEntry.m_Label = newWidget->m_Tag;
		newWidgetEntry.m_IconHandle = widgetIcon;
		newWidgetEntry.m_ProvidedData = CreateRef<uint32_t>((uint32_t)entry.m_Handle); ;
		newWidgetEntry.m_Handle = window.m_Widgets.size();

		// Add handlers for interacting with the tree entry
		newWidgetEntry.m_OnLeftClick = KG_BIND_CLASS_FN(SelectWidget);
		newWidgetEntry.m_OnRightClickSelection.push_back({ "Delete Widget", KG_BIND_CLASS_FN(DeleteWidget) });

		// Add Widget to RuntimeUI and EditorUI::Tree
		window.AddWidget(newWidget);
		entry.m_SubEntries.push_back(newWidgetEntry);

		// Set the active editor UI as edited
		m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorTreePanel::CreateWindowSelectionOptions(EditorUI::TreeEntry& windowEntry)
	{
		// Add functions to call when interacting with window entry
		windowEntry.m_OnLeftClick = KG_BIND_CLASS_FN(SelectWindow);
		windowEntry.m_OnRightClickSelection.push_back
		({
			std::string("Add ") + Utility::WidgetTypeToDisplayString(RuntimeUI::WidgetTypes::TextWidget),
			KG_BIND_CLASS_FN(AddTextWidget)
			});
		windowEntry.m_OnRightClickSelection.push_back
		({
			std::string("Add ") + Utility::WidgetTypeToDisplayString(RuntimeUI::WidgetTypes::ButtonWidget),
			KG_BIND_CLASS_FN(AddButtonWidget)
			});
		windowEntry.m_OnRightClickSelection.push_back
		({
			std::string("Add ") + Utility::WidgetTypeToDisplayString(RuntimeUI::WidgetTypes::ImageWidget),
			KG_BIND_CLASS_FN(AddImageWidget)
			});
		windowEntry.m_OnRightClickSelection.push_back
		({
			std::string("Add ") + Utility::WidgetTypeToDisplayString(RuntimeUI::WidgetTypes::ImageButtonWidget),
			KG_BIND_CLASS_FN(AddImageButtonWidget)
			});
		windowEntry.m_OnRightClickSelection.push_back
		({
			std::string("Add ") + Utility::WidgetTypeToDisplayString(RuntimeUI::WidgetTypes::CheckboxWidget),
			KG_BIND_CLASS_FN(AddCheckboxWidget)
			});
		windowEntry.m_OnRightClickSelection.push_back
		({
			std::string("Add ") + Utility::WidgetTypeToDisplayString(RuntimeUI::WidgetTypes::InputTextWidget),
			KG_BIND_CLASS_FN(AddInputTextWidget)
			});
		windowEntry.m_OnRightClickSelection.push_back
		({
			std::string("Add ") + Utility::WidgetTypeToDisplayString(RuntimeUI::WidgetTypes::SliderWidget),
			KG_BIND_CLASS_FN(AddSliderWidget)
			});
		windowEntry.m_OnRightClickSelection.push_back
		({
			std::string("Add ") + Utility::WidgetTypeToDisplayString(RuntimeUI::WidgetTypes::DropDownWidget),
			KG_BIND_CLASS_FN(AddDropDownWidget)
			});
		windowEntry.m_OnRightClickSelection.push_back({ "Delete Window", KG_BIND_CLASS_FN(DeleteWindow) });
	}

	void UIEditorTreePanel::RecalculateTreeIndexData()
	{
		// Recalculate the handle for each window entry in the user interface tree
		m_UITree.EditDepth([&](EditorUI::TreeEntry& entry)
		{
			// Get tree path from entry reference
			EditorUI::TreePath entryPath = m_UITree.GetPathFromEntryReference(&entry);

			// Ensure the depth is 1, since this is the expected depth for the user interface tree
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
			entry.m_Handle = (uint64_t)currentWidget;
			entry.m_ProvidedData = CreateRef<uint32_t>((uint32_t)currentWindow);

		}, 2);
	}
	void UIEditorTreePanel::SelectTreeNode(uint16_t windowID, uint16_t widgetID)
	{
		if (windowID == RuntimeUI::k_InvalidWindowIndex)
		{
			return;
		}
		
		EditorUI::TreePath path;
		bool success{ false };

		// Add UI node
		path.AddNode(0);

		// Add window node
		path.AddNode(windowID);

		if (widgetID == RuntimeUI::k_InvalidWidgetIndex)
		{
			m_UITree.ExpandFirstLayer();
			success = m_UITree.SelectEntry(path);
		}
		else
		{
			m_UITree.ExpandFirstLayer();
			m_UITree.ExpandNodePath(path);
			path.AddNode(widgetID);
			success = m_UITree.SelectEntry(path);
		}

		if (!success)
		{
			KG_WARN("Failed to select window/widget with ID {} and {}", windowID, widgetID);
		}
	}
}

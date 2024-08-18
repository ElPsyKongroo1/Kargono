#include "Panels/UIEditorPanel.h"

#include "EditorApp.h"

#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{

	// Opening Panel w/ Popups
	static EditorUI::SelectOptionSpec s_OpenUIPopupSpec {};
	static EditorUI::GenericPopupSpec s_CreateUIPopupSpec {};
	static EditorUI::EditTextSpec s_SelectUINameSpec {};

	// User Interface Header
	static EditorUI::PanelHeaderSpec s_MainHeader {};
	static EditorUI::GenericPopupSpec s_DeleteUIWarning {};
	static EditorUI::GenericPopupSpec s_CloseUIWarning {};

	// Edit Window Options
	static EditorUI::CollapsingHeaderSpec s_WindowHeader{};
	static EditorUI::EditTextSpec s_WindowTag{};
	static EditorUI::SelectOptionSpec s_WindowDefaultWidget{};
	static EditorUI::CheckboxSpec s_WindowDisplay{};
	static EditorUI::EditVec3Spec s_WindowLocation{};
	static EditorUI::EditVec2Spec s_WindowSize{};
	static EditorUI::EditVec4Spec s_WindowBackgroundColor{};

	// Edit Widget Options
	static EditorUI::CollapsingHeaderSpec s_WidgetHeader{};
	static EditorUI::EditTextSpec s_WidgetTag{};
	static EditorUI::EditVec2Spec s_WidgetLocation{};
	static EditorUI::EditVec4Spec s_WidgetBackgroundColor{};
	static EditorUI::SelectOptionSpec s_WidgetOnPress{};
	static EditorUI::EditTextSpec s_WidgetText{};
	static EditorUI::EditFloatSpec s_WidgetTextSize{};
	static EditorUI::EditVec4Spec s_WidgetTextColor{};
	static EditorUI::CheckboxSpec s_WidgetCentered{};

	// Main Content
	static EditorUI::TreeSpec s_UITree {};

	// Reusable Functions
	static void OnOpenUI()
	{
		s_OpenUIPopupSpec.PopupActive = true;
	}
	static void OnCreateUI()
	{
		s_CreateUIPopupSpec.PopupActive = true;
	}
	static void OnRefreshData()
	{
		s_UITree.OnRefresh();
	}

	UIEditorPanel::UIEditorPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName,
			KG_BIND_CLASS_FN(UIEditorPanel::OnKeyPressedEditor));
		InitializeOpeningScreen();
		InitializeUIHeader();
		InitializeMainContent();
		InitializeWindowOptions();
		InitializeWidgetOptions();
	}

	void UIEditorPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();

		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowUserInterfaceEditor);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		if (!m_EditorUI)
		{
			// Opening/Null State Screen
			EditorUI::EditorUIService::NewItemScreen("Open Existing User Interface", OnOpenUI, "Create New User Interface", OnCreateUI);
			EditorUI::EditorUIService::GenericPopup(s_CreateUIPopupSpec);
			EditorUI::EditorUIService::SelectOption(s_OpenUIPopupSpec);
		}
		else
		{
			// Header
			EditorUI::EditorUIService::PanelHeader(s_MainHeader);
			EditorUI::EditorUIService::GenericPopup(s_DeleteUIWarning);
			EditorUI::EditorUIService::GenericPopup(s_CloseUIWarning);

			// Main Content
			EditorUI::EditorUIService::Tree(s_UITree);
		}

		EditorUI::EditorUIService::EndWindow();
	}
	bool UIEditorPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}

	void UIEditorPanel::DrawWindowOptions()
	{

		EditorUI::EditorUIService::CollapsingHeader(s_WindowHeader);

		if (s_WindowHeader.Expanded)
		{
			EditorUI::EditorUIService::EditText(s_WindowTag);
			EditorUI::EditorUIService::SelectOption(s_WindowDefaultWidget);
			EditorUI::EditorUIService::Checkbox(s_WindowDisplay);
			EditorUI::EditorUIService::EditVec3(s_WindowLocation);
			EditorUI::EditorUIService::EditVec2(s_WindowSize);
			EditorUI::EditorUIService::EditVec4(s_WindowBackgroundColor);
		}
	}

	void UIEditorPanel::DrawWidgetOptions()
	{
		EditorUI::EditorUIService::CollapsingHeader(s_WindowHeader);
		if (s_WindowHeader.Expanded)
		{
			EditorUI::EditorUIService::EditText(s_WindowTag);
			EditorUI::EditorUIService::SelectOption(s_WindowDefaultWidget);
			EditorUI::EditorUIService::Checkbox(s_WindowDisplay);
			EditorUI::EditorUIService::EditVec3(s_WindowLocation);
			EditorUI::EditorUIService::EditVec2(s_WindowSize);
			EditorUI::EditorUIService::EditVec4(s_WindowBackgroundColor);
		}

		EditorUI::EditorUIService::CollapsingHeader(s_WidgetHeader);
		if (s_WidgetHeader.Expanded)
		{
			EditorUI::EditorUIService::EditText(s_WidgetTag);
			EditorUI::EditorUIService::EditVec2(s_WidgetLocation);
			EditorUI::EditorUIService::EditVec4(s_WidgetBackgroundColor);
			EditorUI::EditorUIService::SelectOption(s_WidgetOnPress);
			EditorUI::EditorUIService::EditText(s_WidgetText);
			EditorUI::EditorUIService::EditFloat(s_WidgetTextSize);
			EditorUI::EditorUIService::EditVec4(s_WidgetTextColor);
			EditorUI::EditorUIService::Checkbox(s_WidgetCentered);
		}
	}

	void UIEditorPanel::InitializeOpeningScreen()
	{
		s_OpenUIPopupSpec.Label = "Open User Interface";
		s_OpenUIPopupSpec.LineCount = 2;
		s_OpenUIPopupSpec.CurrentOption = { "None", Assets::EmptyHandle };
		s_OpenUIPopupSpec.Flags |= EditorUI::SelectOption_PopupOnly;
		s_OpenUIPopupSpec.PopupAction = [&]()
		{
			s_OpenUIPopupSpec.GetAllOptions().clear();
			s_OpenUIPopupSpec.CurrentOption = { "None", Assets::EmptyHandle };

			s_OpenUIPopupSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetManager::GetUserInterfaceRegistry())
			{
				s_OpenUIPopupSpec.AddToOptions("All Options", asset.Data.IntermediateLocation.string(), handle);
			}
		};

		s_OpenUIPopupSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.Handle == Assets::EmptyHandle)
			{
				KG_WARN("No User Interface Selected");
				return;
			}
			if (!Assets::AssetManager::GetUserInterfaceRegistry().contains(selection.Handle))
			{
				KG_WARN("Could not find the user interface specified");
				return;
			}

			m_EditorUI = Assets::AssetManager::GetUserInterface(selection.Handle);
			m_EditorUIHandle = selection.Handle;
			s_MainHeader.EditColorActive = false;
			s_MainHeader.Label = Assets::AssetManager::GetUserInterfaceRegistry().at(
				m_EditorUIHandle).Data.IntermediateLocation.string();
			OnRefreshData();
			RuntimeUI::RuntimeUIService::SetActiveUI(m_EditorUI, m_EditorUIHandle);
		};

		s_SelectUINameSpec.Label = "New Name";
		s_SelectUINameSpec.CurrentOption = "Empty";

		s_CreateUIPopupSpec.Label = "Create User Interface";
		s_CreateUIPopupSpec.PopupWidth = 420.0f;
		s_CreateUIPopupSpec.ConfirmAction = [&]()
		{
			if (s_SelectUINameSpec.CurrentOption == "")
			{
				return;
			}

			m_EditorUIHandle = Assets::AssetManager::CreateNewUserInterface(s_SelectUINameSpec.CurrentOption);
			if (m_EditorUIHandle == Assets::EmptyHandle)
			{
				KG_WARN("User Interface was not created");
				return;
			}
			m_EditorUI = Assets::AssetManager::GetUserInterface(m_EditorUIHandle);
			s_MainHeader.EditColorActive = false;
			s_MainHeader.Label = Assets::AssetManager::GetUserInterfaceRegistry().at(
				m_EditorUIHandle).Data.IntermediateLocation.string();
			OnRefreshData();
			RuntimeUI::RuntimeUIService::SetActiveUI(m_EditorUI, m_EditorUIHandle);
		};
		s_CreateUIPopupSpec.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(s_SelectUINameSpec);
		};
	}

	static void RecalculateTreeIterators()
	{
		uint32_t iterator{ 0 };
		for (auto& entry : s_UITree.GetTreeEntries())
		{
			entry.Handle = iterator;
			uint32_t iteratorTwo{};
			for (auto& subEntry : entry.SubEntries)
			{
				subEntry.Handle = iteratorTwo;
				subEntry.ProvidedData = CreateRef<uint32_t>(iterator);
				iteratorTwo++;
			}
			iterator++;
		}
	}

	void UIEditorPanel::InitializeUIHeader()
	{
		// Header (Game State Name and Options)
		s_DeleteUIWarning.Label = "Delete User Interface";
		s_DeleteUIWarning.ConfirmAction = [&]()
		{
			// TODO: Remove UI from asset manager
			Assets::AssetManager::DeleteUserInterface(m_EditorUIHandle);
			m_EditorUIHandle = 0;
			m_EditorUI = nullptr;
			RuntimeUI::RuntimeUIService::ClearActiveUI();
			m_ActiveWindow = nullptr;
			m_ActiveWidget = nullptr;
			m_CurrentDisplay = UIPropertiesDisplay::None;
		};
		s_DeleteUIWarning.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to delete this user interface object?");
		};

		s_CloseUIWarning.Label = "Close User Interface";
		s_CloseUIWarning.ConfirmAction = [&]()
		{
			m_EditorUIHandle = 0;
			m_EditorUI = nullptr;
			RuntimeUI::RuntimeUIService::ClearActiveUI();
			m_ActiveWindow = nullptr;
			m_ActiveWidget = nullptr;
			m_CurrentDisplay = UIPropertiesDisplay::None;
		};
		s_CloseUIWarning.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to close this user interface object without saving?");
		};

		s_MainHeader.AddToSelectionList("Add Window", [&]()
		{
			
			EditorUI::TreeEntry newEntry {};
			newEntry.Label = "None";
			newEntry.IconHandle = EditorUI::EditorUIService::s_IconWindow;
			newEntry.Handle = s_UITree.GetTreeEntries().size();
			newEntry.OnLeftClick = [&](EditorUI::TreeEntry& entry)
			{
				m_ActiveWindow = &m_EditorUI->m_Windows.at(entry.Handle);
				m_CurrentDisplay = UIPropertiesDisplay::Window;
				EditorUI::EditorUIService::BringWindowToFront(s_EditorApp->m_PropertiesPanel->m_PanelName);
				s_EditorApp->m_PropertiesPanel->m_ActiveParent = m_PanelName;
			};

			newEntry.OnRightClickSelection.push_back({ "Delete Window", [&](EditorUI::TreeEntry& entry)
			{
				EditorUI::TreePath path = s_UITree.GetPathFromEntryReference(&entry);
				if (!path)
				{
					KG_WARN("Could not locate window path inside s_UITree");
					return;
				}
				auto& windows = s_EditorApp->m_UIEditorPanel->m_EditorUI->m_Windows;
				windows.erase(windows.begin() + entry.Handle);
				s_UITree.RemoveEntry(path);
				s_EditorApp->m_UIEditorPanel->m_ActiveWidget = nullptr;
				s_EditorApp->m_UIEditorPanel->m_ActiveWindow = nullptr;
				s_EditorApp->m_UIEditorPanel->m_CurrentDisplay = UIPropertiesDisplay::None;

				s_MainHeader.EditColorActive = true;
				RecalculateTreeIterators();

			} });

			s_UITree.InsertEntry(newEntry);
			s_EditorApp->m_UIEditorPanel->m_EditorUI->m_Windows.push_back({});
			s_MainHeader.EditColorActive = true;
		});

		s_MainHeader.AddToSelectionList("Save", [&]()
		{
			Assets::AssetManager::SaveUserInterface(m_EditorUIHandle, m_EditorUI);
			s_MainHeader.EditColorActive = false;
		});
		s_MainHeader.AddToSelectionList("Close", [&]()
		{
			if (s_MainHeader.EditColorActive)
			{
				s_CloseUIWarning.PopupActive = true;
			}
			else
			{
				m_EditorUIHandle = 0;
				m_EditorUI = nullptr;
				RuntimeUI::RuntimeUIService::ClearActiveUI();
				m_ActiveWindow = nullptr;
				m_ActiveWidget = nullptr;
				m_CurrentDisplay = UIPropertiesDisplay::None;
			}
		});
		s_MainHeader.AddToSelectionList("Delete", [&]()
		{
			s_DeleteUIWarning.PopupActive = true;
		});
	}

	void UIEditorPanel::InitializeMainContent()
	{
		s_UITree.Label = "User Interface Tree";
		s_UITree.OnRefresh = [&]()
		{
			if (!m_EditorUI)
			{
				KG_WARN("Attempt to load table without valid m_EditorUI");
				return;
			}
			s_UITree.ClearTree();
			uint32_t iteratorOne{ 0 };
			for (auto& window : m_EditorUI->m_Windows)
			{
				EditorUI::TreeEntry newEntry {};
				newEntry.Label = window.Tag;
				newEntry.IconHandle = EditorUI::EditorUIService::s_IconWindow;
				newEntry.Handle = iteratorOne;
				newEntry.OnLeftClick = [&](EditorUI::TreeEntry& entry)
				{
					m_ActiveWindow = &m_EditorUI->m_Windows.at(entry.Handle);
					m_CurrentDisplay = UIPropertiesDisplay::Window;
					EditorUI::EditorUIService::BringWindowToFront(s_EditorApp->m_PropertiesPanel->m_PanelName);
					s_EditorApp->m_PropertiesPanel->m_ActiveParent = m_PanelName;
				};


				newEntry.OnRightClickSelection.push_back({ "Delete Window", [&](EditorUI::TreeEntry& entry)
				{
					EditorUI::TreePath path = s_UITree.GetPathFromEntryReference(&entry);
					if (!path)
					{
						KG_WARN("Could not locate window path inside s_UITree");
						return;
					}
					auto& windows = s_EditorApp->m_UIEditorPanel->m_EditorUI->m_Windows;
					windows.erase(windows.begin() + entry.Handle);
					s_UITree.RemoveEntry(path);
					s_EditorApp->m_UIEditorPanel->m_ActiveWidget = nullptr;
					s_EditorApp->m_UIEditorPanel->m_ActiveWindow = nullptr;
					s_EditorApp->m_UIEditorPanel->m_CurrentDisplay = UIPropertiesDisplay::None;

					s_MainHeader.EditColorActive = true;
					RecalculateTreeIterators();

				} });

				uint32_t iteratorTwo{ 0 };
				for (auto widget : window.Widgets)
				{
					EditorUI::TreeEntry newWidgetEntry {};
					newWidgetEntry.Label = widget->Tag;
					newWidgetEntry.IconHandle = EditorUI::EditorUIService::s_IconTextWidget;
					newWidgetEntry.ProvidedData = CreateRef<uint32_t>(iteratorOne);
					newWidgetEntry.Handle = iteratorTwo;
					newWidgetEntry.OnLeftClick = [&](EditorUI::TreeEntry& entry)
					{
						m_ActiveWindow = &m_EditorUI->m_Windows.at(*(uint32_t*)entry.ProvidedData.get());
						m_ActiveWidget = m_ActiveWindow->Widgets.at(entry.Handle).get();
						m_CurrentDisplay = UIPropertiesDisplay::Widget;
						EditorUI::EditorUIService::BringWindowToFront(s_EditorApp->m_PropertiesPanel->m_PanelName);
						s_EditorApp->m_PropertiesPanel->m_ActiveParent = m_PanelName;
					};
					newWidgetEntry.OnRightClickSelection.push_back({ "Delete Widget", [&](EditorUI::TreeEntry& entry)
					{
						EditorUI::TreePath path = s_UITree.GetPathFromEntryReference(&entry);
						if (!path)
						{
							KG_WARN("Could not locate widget path inside s_UITree");
							return;
						}
						auto& windows = s_EditorApp->m_UIEditorPanel->m_EditorUI->m_Windows;
						auto& widgets = windows.at(*(uint32_t*)entry.ProvidedData.get()).Widgets;
						
						widgets.erase(widgets.begin() + entry.Handle);
						s_UITree.RemoveEntry(path);
						s_EditorApp->m_UIEditorPanel->m_ActiveWidget = nullptr;
						s_EditorApp->m_UIEditorPanel->m_ActiveWindow = nullptr;
						s_EditorApp->m_UIEditorPanel->m_CurrentDisplay = UIPropertiesDisplay::None;

						s_MainHeader.EditColorActive = true;
						RecalculateTreeIterators();

					} });

					newEntry.SubEntries.push_back(newWidgetEntry);
					iteratorTwo++;
				}

				s_UITree.InsertEntry(newEntry);
				iteratorOne++;
			}
		};
	}
	void UIEditorPanel::InitializeWindowOptions()
	{
		s_WindowHeader.Label = "Window Options";
		s_WindowHeader.Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		s_WindowHeader.Expanded = true;

		s_WindowTag.Label = "Tag";
		s_WindowTag.Flags |= EditorUI::EditText_Indented;

		s_WindowDefaultWidget.Label = "Default Widget";
		s_WindowDefaultWidget.Flags |= EditorUI::SelectOption_Indented;

		s_WindowDisplay.Label = "Display Window";
		s_WindowDisplay.Flags |= EditorUI::Checkbox_Indented;

		s_WindowLocation.Label = "Screen Location";
		s_WindowLocation.Flags |= EditorUI::EditVec3_Indented;

		s_WindowSize.Label = "Screen Size";
		s_WindowSize.Flags |= EditorUI::EditVec2_Indented;

		s_WindowBackgroundColor.Label = "Background Color";
		s_WindowBackgroundColor.Flags |= EditorUI::EditVec4_Indented;
	}
	void UIEditorPanel::InitializeWidgetOptions()
	{
		s_WidgetHeader.Label = "Widget Options";
		s_WidgetHeader.Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		s_WidgetHeader.Expanded = true;

		s_WidgetTag.Label = "Tag";
		s_WidgetTag.Flags |= EditorUI::EditText_Indented;

		s_WidgetLocation.Label = "Window Location";
		s_WidgetLocation.Flags |= EditorUI::EditVec2_Indented;

		s_WidgetBackgroundColor.Label = "Background Color";
		s_WidgetBackgroundColor.Flags |= EditorUI::EditVec4_Indented;

		s_WidgetOnPress.Label = "On Press";
		s_WidgetOnPress.Flags |= EditorUI::SelectOption_Indented;

		s_WidgetText.Label = "Text";
		s_WidgetText.Flags |= EditorUI::EditText_Indented;

		s_WidgetTextSize.Label = "Text Size";
		s_WidgetTextSize.Flags |= EditorUI::EditFloat_Indented;

		s_WidgetTextColor.Label = "Text Color";
		s_WidgetTextColor.Flags |= EditorUI::EditVec4_Indented;

		s_WidgetCentered.Label = "Centered";
		s_WidgetCentered.Flags |= EditorUI::Checkbox_Indented;
	}
}

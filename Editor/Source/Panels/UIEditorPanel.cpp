#include "Panels/UIEditorPanel.h"

#include "EditorApp.h"

#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{
	void UIEditorPanel::OpenCreateDialog(std::filesystem::path& createLocation)
	{
		// Open user interface Window
		s_EditorApp->m_ShowUserInterfaceEditor = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);

		if (!m_EditorUI)
		{
			// Open dialog to create editor user interface
			OnCreateUI();
			m_SelectUILocationSpec.CurrentOption = createLocation;
		}
		else
		{
			// Add warning to close active user interface before creating a new user interface
			s_EditorApp->OpenWarningMessage("A user interface is already active inside the editor. Please close the current user interface before creating a new one.");
		}
	}
	// Reusable Functions
	void UIEditorPanel::OnOpenUI()
	{
		m_OpenUIPopupSpec.PopupActive = true;
	}
	void UIEditorPanel::OnCreateUI()
	{
		KG_ASSERT(Projects::ProjectService::GetActive());
		m_SelectUILocationSpec.CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
		m_CreateUIPopupSpec.PopupActive = true;
	}
	void UIEditorPanel::OnRefreshData()
	{
		m_UITree.OnRefresh();
	}

	UIEditorPanel::UIEditorPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
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
			EditorUI::EditorUIService::NewItemScreen("Open Existing User Interface", KG_BIND_CLASS_FN(OnOpenUI), "Create New User Interface", KG_BIND_CLASS_FN(OnCreateUI));
			EditorUI::EditorUIService::GenericPopup(m_CreateUIPopupSpec);
			EditorUI::EditorUIService::SelectOption(m_OpenUIPopupSpec);
		}
		else
		{
			// Header
			EditorUI::EditorUIService::PanelHeader(m_MainHeader);
			EditorUI::EditorUIService::GenericPopup(m_DeleteUIWarning);
			EditorUI::EditorUIService::GenericPopup(m_CloseUIWarning);

			// Main Content
			EditorUI::EditorUIService::Tree(m_UITree);
		}

		EditorUI::EditorUIService::EndWindow();
	}
	bool UIEditorPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		switch (event.GetKeyCode())
		{
		case Key::Escape:
			m_UITree.SelectedEntry = {};
			m_CurrentDisplay = UIPropertiesDisplay::None;
			return true;
		
		default:
			return false;
		
		}
	}

	void UIEditorPanel::DrawWindowOptions()
	{

		EditorUI::EditorUIService::CollapsingHeader(m_WindowHeader);

		if (m_WindowHeader.Expanded)
		{
			m_WindowTag.CurrentOption = m_ActiveWindow->Tag;
			EditorUI::EditorUIService::EditText(m_WindowTag);

			int32_t activeWidget = m_ActiveWindow->DefaultActiveWidget;
			m_WindowDefaultWidget.CurrentOption =
			{
				activeWidget == -1 ? "None" : m_ActiveWindow->Widgets.at(activeWidget)->Tag,
				(uint64_t)activeWidget
			};
			EditorUI::EditorUIService::SelectOption(m_WindowDefaultWidget);

			m_WindowDisplay.CurrentBoolean = m_ActiveWindow->GetWindowDisplayed();
			EditorUI::EditorUIService::Checkbox(m_WindowDisplay);

			m_WindowLocation.CurrentVec3 = m_ActiveWindow->ScreenPosition;
			EditorUI::EditorUIService::EditVec3(m_WindowLocation);

			m_WindowSize.CurrentVec2 = m_ActiveWindow->Size;
			EditorUI::EditorUIService::EditVec2(m_WindowSize);

			m_WindowBackgroundColor.CurrentVec4 = m_ActiveWindow->BackgroundColor;
			EditorUI::EditorUIService::EditVec4(m_WindowBackgroundColor);
		}
	}

	void UIEditorPanel::DrawWidgetOptions()
	{
		EditorUI::EditorUIService::CollapsingHeader(m_WidgetHeader);
		if (m_WidgetHeader.Expanded)
		{
			m_WidgetTag.CurrentOption = m_ActiveWidget->Tag;
			EditorUI::EditorUIService::EditText(m_WidgetTag);

			m_WidgetLocation.CurrentVec2 = m_ActiveWidget->WindowPosition;
			EditorUI::EditorUIService::EditVec2(m_WidgetLocation);

			m_WidgetSize.CurrentVec2 = m_ActiveWidget->Size;
			EditorUI::EditorUIService::EditVec2(m_WidgetSize);
			
			m_WidgetBackgroundColor.CurrentVec4 = m_ActiveWidget->DefaultBackgroundColor;
			EditorUI::EditorUIService::EditVec4(m_WidgetBackgroundColor);

			if (m_ActiveWidget->WidgetType == RuntimeUI::WidgetTypes::TextWidget)
			{
				RuntimeUI::TextWidget& activeTextWidget = *(RuntimeUI::TextWidget*)m_ActiveWidget;

				Assets::AssetHandle onPressHandle = activeTextWidget.FunctionPointers.OnPressHandle;
				m_WidgetOnPress.CurrentOption =
				{
					onPressHandle == Assets::EmptyHandle ? "None" : Utility::ScriptToString(Assets::AssetService::GetScript(onPressHandle)),
					onPressHandle
				};
				EditorUI::EditorUIService::SelectOption(m_WidgetOnPress);

				m_WidgetText.CurrentOption = activeTextWidget.Text;
				EditorUI::EditorUIService::EditText(m_WidgetText);

				m_WidgetTextSize.CurrentFloat = activeTextWidget.TextSize;
				EditorUI::EditorUIService::EditFloat(m_WidgetTextSize);

				m_WidgetTextColor.CurrentVec4 = activeTextWidget.TextColor;
				EditorUI::EditorUIService::EditVec4(m_WidgetTextColor);

				m_WidgetCentered.CurrentBoolean = activeTextWidget.TextCentered;
				EditorUI::EditorUIService::Checkbox(m_WidgetCentered);
			}
			
		}
	}

	void UIEditorPanel::InitializeOpeningScreen()
	{
		m_OpenUIPopupSpec.Label = "Open User Interface";
		m_OpenUIPopupSpec.LineCount = 2;
		m_OpenUIPopupSpec.CurrentOption = { "None", Assets::EmptyHandle };
		m_OpenUIPopupSpec.Flags |= EditorUI::SelectOption_PopupOnly;
		m_OpenUIPopupSpec.PopupAction = [&]()
		{
			m_OpenUIPopupSpec.GetAllOptions().clear();
			m_OpenUIPopupSpec.CurrentOption = { "None", Assets::EmptyHandle };

			m_OpenUIPopupSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetUserInterfaceRegistry())
			{
				m_OpenUIPopupSpec.AddToOptions("All Options", asset.Data.FileLocation.string(), handle);
			}
		};

		m_OpenUIPopupSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.Handle == Assets::EmptyHandle)
			{
				KG_WARN("No User Interface Selected");
				return;
			}
			if (!Assets::AssetService::GetUserInterfaceRegistry().contains(selection.Handle))
			{
				KG_WARN("Could not find the user interface specified");
				return;
			}

			m_EditorUI = Assets::AssetService::GetUserInterface(selection.Handle);
			m_EditorUIHandle = selection.Handle;
			m_MainHeader.EditColorActive = false;
			m_MainHeader.Label = Assets::AssetService::GetUserInterfaceRegistry().at(
				m_EditorUIHandle).Data.FileLocation.string();
			OnRefreshData();
			RuntimeUI::RuntimeUIService::SetActiveUI(m_EditorUI, m_EditorUIHandle);
		};

		m_SelectUINameSpec.Label = "New Name";
		m_SelectUINameSpec.CurrentOption = "Empty";

		m_SelectUILocationSpec.Label = "Location";
		m_SelectUILocationSpec.CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
		m_SelectUILocationSpec.ConfirmAction = [&](const std::string& path)
		{
			if (!Utility::FileSystem::DoesPathContainSubPath(Projects::ProjectService::GetActiveAssetDirectory(), path))
			{
				KG_WARN("Cannot create an asset outside of the project's asset directory.");
				m_SelectUILocationSpec.CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
			}
		};

		m_CreateUIPopupSpec.Label = "Create User Interface";
		m_CreateUIPopupSpec.ConfirmAction = [&]()
		{
			if (m_SelectUINameSpec.CurrentOption == "")
			{
				return;
			}

			m_EditorUIHandle = Assets::AssetService::CreateUserInterface(m_SelectUINameSpec.CurrentOption.c_str(), m_SelectUILocationSpec.CurrentOption);
			if (m_EditorUIHandle == Assets::EmptyHandle)
			{
				KG_WARN("User Interface was not created");
				return;
			}
			m_EditorUI = Assets::AssetService::GetUserInterface(m_EditorUIHandle);
			m_MainHeader.EditColorActive = false;
			m_MainHeader.Label = Assets::AssetService::GetUserInterfaceRegistry().at(
				m_EditorUIHandle).Data.FileLocation.string();
			OnRefreshData();
			RuntimeUI::RuntimeUIService::SetActiveUI(m_EditorUI, m_EditorUIHandle);
		};
		m_CreateUIPopupSpec.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_SelectUINameSpec);
			EditorUI::EditorUIService::ChooseDirectory(m_SelectUILocationSpec);
		};
	}

	void UIEditorPanel::RecalculateTreeIterators()
	{
		uint32_t iterator{ 0 };
		for (auto& entry : m_UITree.GetTreeEntries())
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
		m_DeleteUIWarning.Label = "Delete User Interface";
		m_DeleteUIWarning.ConfirmAction = [&]()
		{
			// TODO: Remove UI from asset manager
			Assets::AssetService::DeleteUserInterface(m_EditorUIHandle);
			m_EditorUIHandle = 0;
			m_EditorUI = nullptr;
			RuntimeUI::RuntimeUIService::ClearActiveUI();
			m_ActiveWindow = nullptr;
			m_ActiveWidget = nullptr;
			m_CurrentDisplay = UIPropertiesDisplay::None;
		};
		m_DeleteUIWarning.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to delete this user interface object?");
		};

		m_CloseUIWarning.Label = "Close User Interface";
		m_CloseUIWarning.ConfirmAction = [&]()
		{
			m_EditorUIHandle = 0;
			m_EditorUI = nullptr;
			RuntimeUI::RuntimeUIService::ClearActiveUI();
			m_ActiveWindow = nullptr;
			m_ActiveWidget = nullptr;
			m_CurrentDisplay = UIPropertiesDisplay::None;
		};
		m_CloseUIWarning.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to close this user interface object without saving?");
		};

		m_MainHeader.AddToSelectionList("Add Window", KG_BIND_CLASS_FN(AddWindow));

		m_MainHeader.AddToSelectionList("Save", [&]()
		{
			Assets::AssetService::SaveUserInterface(m_EditorUIHandle, m_EditorUI);
			m_MainHeader.EditColorActive = false;
		});
		m_MainHeader.AddToSelectionList("Close", [&]()
		{
			if (m_MainHeader.EditColorActive)
			{
				m_CloseUIWarning.PopupActive = true;
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
		m_MainHeader.AddToSelectionList("Delete", [&]()
		{
			m_DeleteUIWarning.PopupActive = true;
		});
	}

	void UIEditorPanel::InitializeMainContent()
	{
		m_UITree.Label = "User Interface Tree";
		m_UITree.OnRefresh = [&]()
		{
			if (!m_EditorUI)
			{
				KG_WARN("Attempt to load table without valid m_EditorUI");
				return;
			}
			m_UITree.ClearTree();
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


				newEntry.OnRightClickSelection.push_back({ "Delete Window", KG_BIND_CLASS_FN(DeleteWindow) });

				newEntry.OnRightClickSelection.push_back({ "Add Text Widget", KG_BIND_CLASS_FN(AddTextWidget) });

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
					newWidgetEntry.OnRightClickSelection.push_back({ "Delete Widget", KG_BIND_CLASS_FN(DeleteWidget) });

					newEntry.SubEntries.push_back(newWidgetEntry);
					iteratorTwo++;
				}

				m_UITree.InsertEntry(newEntry);
				iteratorOne++;
			}
		};
	}
	void UIEditorPanel::InitializeWindowOptions()
	{
		m_WindowHeader.Label = "Window Options";
		m_WindowHeader.Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		m_WindowHeader.Expanded = true;

		m_WindowTag.Label = "Tag";
		m_WindowTag.Flags |= EditorUI::EditText_Indented;
		m_WindowTag.ConfirmAction = [&](EditorUI::EditTextSpec& spec)
		{
			if (!m_ActiveWindow)
			{
				KG_WARN("No valid window active when trying to update window tag");
				return;
			}
			if (!m_UITree.SelectedEntry)
			{
				KG_WARN("No valid selected window path available in m_UITree when trying to update window tag");
				return;
			}

			EditorUI::TreeEntry* entry = m_UITree.GetEntryFromPath(m_UITree.SelectedEntry);
			if (!entry)
			{
				KG_WARN("No valid selected window active in m_UITree when trying to update window tag");
				return;
			}

			entry->Label = m_WindowTag.CurrentOption;
			m_ActiveWindow->Tag = m_WindowTag.CurrentOption;

			m_MainHeader.EditColorActive = true;
		};

		m_WindowDefaultWidget.Label = "Default Widget";
		m_WindowDefaultWidget.Flags |= EditorUI::SelectOption_Indented;
		m_WindowDefaultWidget.PopupAction = [&]() 
		{
			m_WindowDefaultWidget.ClearOptions();
			m_WindowDefaultWidget.AddToOptions("Clear", "None", (uint64_t)-1);
			uint32_t iteration{ 0 };
			for (auto& widget : m_ActiveWindow->Widgets)
			{
				switch (widget->WidgetType)
				{
				case RuntimeUI::WidgetTypes::TextWidget:
					m_WindowDefaultWidget.AddToOptions("Text Widget", widget->Tag, iteration);
					break;
				default:
					KG_ERROR("Invalid widge type provided to UIEditorPanel");
					break;
				}
				iteration++;
			}
		};

		m_WindowDefaultWidget.ConfirmAction = [&](const EditorUI::OptionEntry& entry) 
		{

			if (entry.Handle == (uint64_t)-1)
			{
				m_ActiveWindow->DefaultActiveWidget = -1;
				m_ActiveWindow->DefaultActiveWidgetRef = nullptr;
				return;
			}

			if (entry.Handle > m_ActiveWindow->Widgets.size())
			{
				KG_WARN("Invalid widget location provided when updating default active widget in window");
				return;
			}

			m_ActiveWindow->DefaultActiveWidget = (int32_t)entry.Handle;
			m_ActiveWindow->DefaultActiveWidgetRef = m_ActiveWindow->Widgets.at(entry.Handle);
		};

		m_WindowDisplay.Label = "Display Window";
		m_WindowDisplay.Flags |= EditorUI::Checkbox_Indented;
		m_WindowDisplay.ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
		{
			if (!m_ActiveWindow)
			{
				KG_WARN("No valid window active when trying to update window display option");
				return;
			}
			spec.CurrentBoolean ? m_ActiveWindow->DisplayWindow() : m_ActiveWindow->HideWindow();
			m_MainHeader.EditColorActive = true;
		};

		m_WindowLocation.Label = "Screen Location";
		m_WindowLocation.Flags |= EditorUI::EditVec3_Indented;
		m_WindowLocation.ConfirmAction = [&](EditorUI::EditVec3Spec& spec)
		{
			if (!m_ActiveWindow)
			{
				KG_WARN("No valid window active when trying to update window location");
				return;
			}
			m_ActiveWindow->ScreenPosition = m_WindowLocation.CurrentVec3;
			m_MainHeader.EditColorActive = true;
		};

		m_WindowSize.Label = "Screen Size";
		m_WindowSize.Flags |= EditorUI::EditVec2_Indented;
		m_WindowSize.ConfirmAction = [&](EditorUI::EditVec2Spec& spec)
		{
			if (!m_ActiveWindow)
			{
				KG_WARN("No valid window active when trying to update window size");
				return;
			}
			m_ActiveWindow->Size = m_WindowSize.CurrentVec2;
			m_MainHeader.EditColorActive = true;
		};

		m_WindowBackgroundColor.Label = "Background Color";
		m_WindowBackgroundColor.Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_WindowBackgroundColor.ConfirmAction = [&](EditorUI::EditVec4Spec& spec)
		{
			if (!m_ActiveWindow)
			{
				KG_WARN("No valid window active when trying to update window background color");
				return;
			}
			m_ActiveWindow->BackgroundColor = m_WindowBackgroundColor.CurrentVec4;
			m_MainHeader.EditColorActive = true;
		};
	}
	void UIEditorPanel::InitializeWidgetOptions()
	{
		m_WidgetHeader.Label = "Widget Options";
		m_WidgetHeader.Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		m_WidgetHeader.Expanded = true;

		m_WidgetTag.Label = "Tag";
		m_WidgetTag.Flags |= EditorUI::EditText_Indented;
		m_WidgetTag.ConfirmAction = [&](EditorUI::EditTextSpec& spec)
		{
			if (!m_ActiveWidget)
			{
				KG_WARN("No valid widget active when trying to update widget tag");
				return;
			}
			if (!m_UITree.SelectedEntry)
			{
				KG_WARN("No valid selected widget path available in m_UITree when trying to update widget tag");
				return;
			}

			EditorUI::TreeEntry* entry = m_UITree.GetEntryFromPath(m_UITree.SelectedEntry);
			if (!entry)
			{
				KG_WARN("No valid selected widget active in m_UITree when trying to update widget tag");
				return;
			}

			entry->Label = m_WidgetTag.CurrentOption;
			m_ActiveWidget->Tag = m_WidgetTag.CurrentOption;
			
			m_MainHeader.EditColorActive = true;
		};

		m_WidgetLocation.Label = "Window Location";
		m_WidgetLocation.Flags |= EditorUI::EditVec2_Indented;
		m_WidgetLocation.ConfirmAction = [&](EditorUI::EditVec2Spec& spec)
		{
			if (!m_ActiveWindow)
			{
				KG_WARN("No valid widget active when trying to update widget's window location");
				return;
			}

			m_ActiveWidget->WindowPosition = m_WidgetLocation.CurrentVec2;
			m_MainHeader.EditColorActive = true;
		};

		m_WidgetSize.Label = "Size";
		m_WidgetSize.Flags |= EditorUI::EditVec2_Indented;
		m_WidgetSize.ConfirmAction = [&](EditorUI::EditVec2Spec& spec)
		{
			if (!m_ActiveWindow)
			{
				KG_WARN("No valid widget active when trying to update widget size");
				return;
			}

			m_ActiveWidget->Size = m_WidgetSize.CurrentVec2;
			m_MainHeader.EditColorActive = true;
		};

		m_WidgetBackgroundColor.Label = "Background Color";
		m_WidgetBackgroundColor.Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_WidgetBackgroundColor.ConfirmAction = [&](EditorUI::EditVec4Spec& spec)
		{
			if (!m_ActiveWindow)
			{
				KG_WARN("No valid widget active when trying to update widget background color");
				return;
			}

			m_ActiveWidget->DefaultBackgroundColor = m_WidgetBackgroundColor.CurrentVec4;
			m_ActiveWidget->ActiveBackgroundColor = m_WidgetBackgroundColor.CurrentVec4;
			m_MainHeader.EditColorActive = true;
		};

		m_WidgetOnPress.Label = "On Press";
		m_WidgetOnPress.Flags |= EditorUI::SelectOption_Indented;
		m_WidgetOnPress.PopupAction = [&]()
		{
			m_WidgetOnPress.ClearOptions();
			m_WidgetOnPress.AddToOptions("Clear", "None", Assets::EmptyHandle);

			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_FuncType != WrappedFuncType::Void_None)
				{
					continue;
				}
				m_WidgetOnPress.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType) +
					"::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}
		};

		m_WidgetOnPress.ConfirmAction = [&](const EditorUI::OptionEntry& entry)
		{

			if (entry.Handle == Assets::EmptyHandle)
			{
				m_ActiveWidget->FunctionPointers.OnPress = nullptr;
				m_ActiveWidget->FunctionPointers.OnPressHandle = Assets::EmptyHandle;
				return;
			}

			m_ActiveWidget->FunctionPointers.OnPressHandle = entry.Handle;
			m_ActiveWidget->FunctionPointers.OnPress = Assets::AssetService::GetScript(entry.Handle);
			
			m_MainHeader.EditColorActive = true;
		};

		m_WidgetText.Label = "Text";
		m_WidgetText.Flags |= EditorUI::EditText_Indented;
		m_WidgetText.ConfirmAction = [&](EditorUI::EditTextSpec& spec)
		{
			if (m_ActiveWidget->WidgetType != RuntimeUI::WidgetTypes::TextWidget)
			{
				KG_WARN("Attempt to modify text widget member, however, active widget is an invalid type");
				return;
			}
			RuntimeUI::TextWidget& textWidget = *(RuntimeUI::TextWidget*)m_ActiveWidget;

			if (!m_ActiveWindow)
			{
				KG_WARN("No valid widget active when trying to update widget text");
				return;
			}

			textWidget.Text = m_WidgetText.CurrentOption;
			m_MainHeader.EditColorActive = true;
		};

		m_WidgetTextSize.Label = "Text Size";
		m_WidgetTextSize.Flags |= EditorUI::EditFloat_Indented;
		m_WidgetTextSize.ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			if (m_ActiveWidget->WidgetType != RuntimeUI::WidgetTypes::TextWidget)
			{
				KG_WARN("Attempt to modify text widget member, however, active widget is an invalid type");
				return;
			}
			RuntimeUI::TextWidget& textWidget = *(RuntimeUI::TextWidget*)m_ActiveWidget;

			if (!m_ActiveWindow)
			{
				KG_WARN("No valid widget active when trying to update widget text size");
				return;
			}

			textWidget.TextSize = m_WidgetTextSize.CurrentFloat;
			m_MainHeader.EditColorActive = true;
		};

		m_WidgetTextColor.Label = "Text Color";
		m_WidgetTextColor.Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_WidgetTextColor.ConfirmAction = [&](EditorUI::EditVec4Spec& spec)
		{
			if (m_ActiveWidget->WidgetType != RuntimeUI::WidgetTypes::TextWidget)
			{
				KG_WARN("Attempt to modify text widget member, however, active widget is an invalid type");
				return;
			}
			RuntimeUI::TextWidget& textWidget = *(RuntimeUI::TextWidget*)m_ActiveWidget;

			if (!m_ActiveWindow)
			{
				KG_WARN("No valid widget active when trying to update widget text color");
				return;
			}

			textWidget.TextColor = m_WidgetTextColor.CurrentVec4;
			m_MainHeader.EditColorActive = true;
		};

		m_WidgetCentered.Label = "Centered";
		m_WidgetCentered.Flags |= EditorUI::Checkbox_Indented;
		m_WidgetCentered.ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
		{
			if (m_ActiveWidget->WidgetType != RuntimeUI::WidgetTypes::TextWidget)
			{
				KG_WARN("Attempt to modify text widget member, however, active widget is an invalid type");
				return;
			}
			RuntimeUI::TextWidget& textWidget = *(RuntimeUI::TextWidget*)m_ActiveWidget;

			if (!m_ActiveWindow)
			{
				KG_WARN("No valid widget active when trying to update widget's TextCentered field");
				return;
			}

			textWidget.TextCentered = spec.CurrentBoolean;
			m_MainHeader.EditColorActive = true;
		};
	}

	void UIEditorPanel::AddTextWidget(EditorUI::TreeEntry& windowEntry)
	{
		UIEditorPanel& panel = *(s_EditorApp->m_UIEditorPanel.get());
		EditorUI::TreePath windowPath = m_UITree.GetPathFromEntryReference(&windowEntry);
		if (!windowPath)
		{
			KG_WARN("Could not locate window path inside m_UITree");
			return;
		}

		// Create Text Widget
		auto& window = panel.m_EditorUI->m_Windows.at(windowEntry.Handle);
		Ref<RuntimeUI::TextWidget> newTextWidget = CreateRef<RuntimeUI::TextWidget>();

		// Create new widget entry for m_UITree
		EditorUI::TreeEntry newWidgetEntry {};
		newWidgetEntry.Label = newTextWidget->Tag;
		newWidgetEntry.IconHandle = EditorUI::EditorUIService::s_IconTextWidget;
		newWidgetEntry.ProvidedData = CreateRef<uint32_t>((uint32_t)windowEntry.Handle); ;
		newWidgetEntry.Handle = window.Widgets.size();
		newWidgetEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
		{
			UIEditorPanel& panel = *(s_EditorApp->m_UIEditorPanel.get());
			panel.m_ActiveWindow = &panel.m_EditorUI->m_Windows.at(*(uint32_t*)entry.ProvidedData.get());
			panel.m_ActiveWidget = panel.m_ActiveWindow->Widgets.at(entry.Handle).get();
			panel.m_CurrentDisplay = UIPropertiesDisplay::Widget;
			EditorUI::EditorUIService::BringWindowToFront(s_EditorApp->m_PropertiesPanel->m_PanelName);
			s_EditorApp->m_PropertiesPanel->m_ActiveParent = panel.m_PanelName;
		};
		newWidgetEntry.OnRightClickSelection.push_back({ "Delete Widget", KG_BIND_CLASS_FN(DeleteWidget) });

		// Add Widget to RuntimeUI and EditorUI::Tree
		window.AddWidget(newTextWidget);
		windowEntry.SubEntries.push_back(newWidgetEntry);
	}

	void UIEditorPanel::DeleteWindow(EditorUI::TreeEntry& entry)
	{
		UIEditorPanel& panel = *(s_EditorApp->m_UIEditorPanel.get());
		EditorUI::TreePath path = m_UITree.GetPathFromEntryReference(&entry);
		if (!path)
		{
			KG_WARN("Could not locate window path inside m_UITree");
			return;
		}
		auto& windows = panel.m_EditorUI->m_Windows;
		windows.erase(windows.begin() + entry.Handle);
		m_UITree.RemoveEntry(path);
		panel.m_ActiveWidget = nullptr;
		panel.m_ActiveWindow = nullptr;
		panel.m_CurrentDisplay = UIPropertiesDisplay::None;

		m_MainHeader.EditColorActive = true;
		RecalculateTreeIterators();

	}

	void UIEditorPanel::DeleteWidget(EditorUI::TreeEntry& entry)
	{
		UIEditorPanel& panel = *(s_EditorApp->m_UIEditorPanel.get());
		EditorUI::TreePath path = m_UITree.GetPathFromEntryReference(&entry);
		if (!path)
		{
			KG_WARN("Could not locate widget path inside m_UITree");
			return;
		}
		auto& windows = panel.m_EditorUI->m_Windows;
		auto& widgets = windows.at(*(uint32_t*)entry.ProvidedData.get()).Widgets;

		widgets.erase(widgets.begin() + entry.Handle);
		m_UITree.RemoveEntry(path);
		panel.m_ActiveWidget = nullptr;
		panel.m_ActiveWindow = nullptr;
		panel.m_CurrentDisplay = UIPropertiesDisplay::None;

		m_MainHeader.EditColorActive = true;
		RecalculateTreeIterators();

	}

	void UIEditorPanel::AddWindow()
	{
		UIEditorPanel& panel = *(s_EditorApp->m_UIEditorPanel.get());
		EditorUI::TreeEntry newEntry {};
		newEntry.Label = "None";
		newEntry.IconHandle = EditorUI::EditorUIService::s_IconWindow;
		newEntry.Handle = m_UITree.GetTreeEntries().size();
		newEntry.OnLeftClick = [&](EditorUI::TreeEntry& entry)
		{
			UIEditorPanel& panel = *(s_EditorApp->m_UIEditorPanel.get());
			panel.m_ActiveWindow = &panel.m_EditorUI->m_Windows.at(entry.Handle);
			panel.m_CurrentDisplay = UIPropertiesDisplay::Window;
			EditorUI::EditorUIService::BringWindowToFront(s_EditorApp->m_PropertiesPanel->m_PanelName);
			s_EditorApp->m_PropertiesPanel->m_ActiveParent = panel.m_PanelName;
		};

		newEntry.OnRightClickSelection.push_back({ "Delete Window", KG_BIND_CLASS_FN(DeleteWindow) });

		newEntry.OnRightClickSelection.push_back({ "Add Text Widget", KG_BIND_CLASS_FN(AddTextWidget) });

		m_UITree.InsertEntry(newEntry);
		panel.m_EditorUI->m_Windows.push_back({});
		m_MainHeader.EditColorActive = true;
	}
}

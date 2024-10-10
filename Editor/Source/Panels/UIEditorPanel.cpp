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
	static EditorUI::EditVec2Spec s_WidgetSize{};
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
		switch (event.GetKeyCode())
		{
		case Key::Escape:
			s_UITree.SelectedEntry = {};
			m_CurrentDisplay = UIPropertiesDisplay::None;
			return true;
		
		default:
			return false;
		
		}
	}

	void UIEditorPanel::DrawWindowOptions()
	{

		EditorUI::EditorUIService::CollapsingHeader(s_WindowHeader);

		if (s_WindowHeader.Expanded)
		{
			s_WindowTag.CurrentOption = m_ActiveWindow->Tag;
			EditorUI::EditorUIService::EditText(s_WindowTag);

			int32_t activeWidget = m_ActiveWindow->DefaultActiveWidget;
			s_WindowDefaultWidget.CurrentOption =
			{
				activeWidget == -1 ? "None" : m_ActiveWindow->Widgets.at(activeWidget)->Tag,
				(uint64_t)activeWidget
			};
			EditorUI::EditorUIService::SelectOption(s_WindowDefaultWidget);

			s_WindowDisplay.CurrentBoolean = m_ActiveWindow->GetWindowDisplayed();
			EditorUI::EditorUIService::Checkbox(s_WindowDisplay);

			s_WindowLocation.CurrentVec3 = m_ActiveWindow->ScreenPosition;
			EditorUI::EditorUIService::EditVec3(s_WindowLocation);

			s_WindowSize.CurrentVec2 = m_ActiveWindow->Size;
			EditorUI::EditorUIService::EditVec2(s_WindowSize);

			s_WindowBackgroundColor.CurrentVec4 = m_ActiveWindow->BackgroundColor;
			EditorUI::EditorUIService::EditVec4(s_WindowBackgroundColor);
		}
	}

	void UIEditorPanel::DrawWidgetOptions()
	{
		EditorUI::EditorUIService::CollapsingHeader(s_WidgetHeader);
		if (s_WidgetHeader.Expanded)
		{
			s_WidgetTag.CurrentOption = m_ActiveWidget->Tag;
			EditorUI::EditorUIService::EditText(s_WidgetTag);

			s_WidgetLocation.CurrentVec2 = m_ActiveWidget->WindowPosition;
			EditorUI::EditorUIService::EditVec2(s_WidgetLocation);

			s_WidgetSize.CurrentVec2 = m_ActiveWidget->Size;
			EditorUI::EditorUIService::EditVec2(s_WidgetSize);
			
			s_WidgetBackgroundColor.CurrentVec4 = m_ActiveWidget->DefaultBackgroundColor;
			EditorUI::EditorUIService::EditVec4(s_WidgetBackgroundColor);

			if (m_ActiveWidget->WidgetType == RuntimeUI::WidgetTypes::TextWidget)
			{
				RuntimeUI::TextWidget& activeTextWidget = *(RuntimeUI::TextWidget*)m_ActiveWidget;

				Assets::AssetHandle onPressHandle = activeTextWidget.FunctionPointers.OnPressHandle;
				s_WidgetOnPress.CurrentOption =
				{
					onPressHandle == Assets::EmptyHandle ? "None" : Utility::ScriptToString(Assets::AssetService::GetScript(onPressHandle)),
					onPressHandle
				};
				EditorUI::EditorUIService::SelectOption(s_WidgetOnPress);

				s_WidgetText.CurrentOption = activeTextWidget.Text;
				EditorUI::EditorUIService::EditText(s_WidgetText);

				s_WidgetTextSize.CurrentFloat = activeTextWidget.TextSize;
				EditorUI::EditorUIService::EditFloat(s_WidgetTextSize);

				s_WidgetTextColor.CurrentVec4 = activeTextWidget.TextColor;
				EditorUI::EditorUIService::EditVec4(s_WidgetTextColor);

				s_WidgetCentered.CurrentBoolean = activeTextWidget.TextCentered;
				EditorUI::EditorUIService::Checkbox(s_WidgetCentered);
			}
			
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
			for (auto& [handle, asset] : Assets::AssetService::GetUserInterfaceRegistry())
			{
				s_OpenUIPopupSpec.AddToOptions("All Options", asset.Data.FileLocation.string(), handle);
			}
		};

		s_OpenUIPopupSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
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
			s_MainHeader.EditColorActive = false;
			s_MainHeader.Label = Assets::AssetService::GetUserInterfaceRegistry().at(
				m_EditorUIHandle).Data.FileLocation.string();
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

			m_EditorUIHandle = Assets::AssetService::CreateUserInterface(s_SelectUINameSpec.CurrentOption);
			if (m_EditorUIHandle == Assets::EmptyHandle)
			{
				KG_WARN("User Interface was not created");
				return;
			}
			m_EditorUI = Assets::AssetService::GetUserInterface(m_EditorUIHandle);
			s_MainHeader.EditColorActive = false;
			s_MainHeader.Label = Assets::AssetService::GetUserInterfaceRegistry().at(
				m_EditorUIHandle).Data.FileLocation.string();
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
			Assets::AssetService::DeleteUserInterface(m_EditorUIHandle);
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

		s_MainHeader.AddToSelectionList("Add Window", AddWindow);

		s_MainHeader.AddToSelectionList("Save", [&]()
		{
			Assets::AssetService::SaveUserInterface(m_EditorUIHandle, m_EditorUI);
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


				newEntry.OnRightClickSelection.push_back({ "Delete Window", DeleteWindow });

				newEntry.OnRightClickSelection.push_back({ "Add Text Widget", AddTextWidget });

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
					newWidgetEntry.OnRightClickSelection.push_back({ "Delete Widget", DeleteWidget });

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
		s_WindowTag.ConfirmAction = [&](EditorUI::EditTextSpec& spec)
		{
			if (!m_ActiveWindow)
			{
				KG_WARN("No valid window active when trying to update window tag");
				return;
			}
			if (!s_UITree.SelectedEntry)
			{
				KG_WARN("No valid selected window path available in s_UITree when trying to update window tag");
				return;
			}

			EditorUI::TreeEntry* entry = s_UITree.GetEntryFromPath(s_UITree.SelectedEntry);
			if (!entry)
			{
				KG_WARN("No valid selected window active in s_UITree when trying to update window tag");
				return;
			}

			entry->Label = s_WindowTag.CurrentOption;
			m_ActiveWindow->Tag = s_WindowTag.CurrentOption;

			s_MainHeader.EditColorActive = true;
		};

		s_WindowDefaultWidget.Label = "Default Widget";
		s_WindowDefaultWidget.Flags |= EditorUI::SelectOption_Indented;
		s_WindowDefaultWidget.PopupAction = [&]() 
		{
			s_WindowDefaultWidget.ClearOptions();
			s_WindowDefaultWidget.AddToOptions("Clear", "None", (uint64_t)-1);
			uint32_t iteration{ 0 };
			for (auto& widget : m_ActiveWindow->Widgets)
			{
				switch (widget->WidgetType)
				{
				case RuntimeUI::WidgetTypes::TextWidget:
					s_WindowDefaultWidget.AddToOptions("Text Widget", widget->Tag, iteration);
					break;
				default:
					KG_ERROR("Invalid widge type provided to UIEditorPanel");
					break;
				}
				iteration++;
			}
		};

		s_WindowDefaultWidget.ConfirmAction = [&](const EditorUI::OptionEntry& entry) 
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

		s_WindowDisplay.Label = "Display Window";
		s_WindowDisplay.Flags |= EditorUI::Checkbox_Indented;
		s_WindowDisplay.ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
		{
			if (!m_ActiveWindow)
			{
				KG_WARN("No valid window active when trying to update window display option");
				return;
			}
			spec.CurrentBoolean ? m_ActiveWindow->DisplayWindow() : m_ActiveWindow->HideWindow();
			s_MainHeader.EditColorActive = true;
		};

		s_WindowLocation.Label = "Screen Location";
		s_WindowLocation.Flags |= EditorUI::EditVec3_Indented;
		s_WindowLocation.ConfirmAction = [&](EditorUI::EditVec3Spec& spec)
		{
			if (!m_ActiveWindow)
			{
				KG_WARN("No valid window active when trying to update window location");
				return;
			}
			m_ActiveWindow->ScreenPosition = s_WindowLocation.CurrentVec3;
			s_MainHeader.EditColorActive = true;
		};

		s_WindowSize.Label = "Screen Size";
		s_WindowSize.Flags |= EditorUI::EditVec2_Indented;
		s_WindowSize.ConfirmAction = [&](EditorUI::EditVec2Spec& spec)
		{
			if (!m_ActiveWindow)
			{
				KG_WARN("No valid window active when trying to update window size");
				return;
			}
			m_ActiveWindow->Size = s_WindowSize.CurrentVec2;
			s_MainHeader.EditColorActive = true;
		};

		s_WindowBackgroundColor.Label = "Background Color";
		s_WindowBackgroundColor.Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		s_WindowBackgroundColor.ConfirmAction = [&](EditorUI::EditVec4Spec& spec)
		{
			if (!m_ActiveWindow)
			{
				KG_WARN("No valid window active when trying to update window background color");
				return;
			}
			m_ActiveWindow->BackgroundColor = s_WindowBackgroundColor.CurrentVec4;
			s_MainHeader.EditColorActive = true;
		};
	}
	void UIEditorPanel::InitializeWidgetOptions()
	{
		s_WidgetHeader.Label = "Widget Options";
		s_WidgetHeader.Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		s_WidgetHeader.Expanded = true;

		s_WidgetTag.Label = "Tag";
		s_WidgetTag.Flags |= EditorUI::EditText_Indented;
		s_WidgetTag.ConfirmAction = [&](EditorUI::EditTextSpec& spec)
		{
			if (!m_ActiveWidget)
			{
				KG_WARN("No valid widget active when trying to update widget tag");
				return;
			}
			if (!s_UITree.SelectedEntry)
			{
				KG_WARN("No valid selected widget path available in s_UITree when trying to update widget tag");
				return;
			}

			EditorUI::TreeEntry* entry = s_UITree.GetEntryFromPath(s_UITree.SelectedEntry);
			if (!entry)
			{
				KG_WARN("No valid selected widget active in s_UITree when trying to update widget tag");
				return;
			}

			entry->Label = s_WidgetTag.CurrentOption;
			m_ActiveWidget->Tag = s_WidgetTag.CurrentOption;
			
			s_MainHeader.EditColorActive = true;
		};

		s_WidgetLocation.Label = "Window Location";
		s_WidgetLocation.Flags |= EditorUI::EditVec2_Indented;
		s_WidgetLocation.ConfirmAction = [&](EditorUI::EditVec2Spec& spec)
		{
			if (!m_ActiveWindow)
			{
				KG_WARN("No valid widget active when trying to update widget's window location");
				return;
			}

			m_ActiveWidget->WindowPosition = s_WidgetLocation.CurrentVec2;
			s_MainHeader.EditColorActive = true;
		};

		s_WidgetSize.Label = "Size";
		s_WidgetSize.Flags |= EditorUI::EditVec2_Indented;
		s_WidgetSize.ConfirmAction = [&](EditorUI::EditVec2Spec& spec)
		{
			if (!m_ActiveWindow)
			{
				KG_WARN("No valid widget active when trying to update widget size");
				return;
			}

			m_ActiveWidget->Size = s_WidgetSize.CurrentVec2;
			s_MainHeader.EditColorActive = true;
		};

		s_WidgetBackgroundColor.Label = "Background Color";
		s_WidgetBackgroundColor.Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		s_WidgetBackgroundColor.ConfirmAction = [&](EditorUI::EditVec4Spec& spec)
		{
			if (!m_ActiveWindow)
			{
				KG_WARN("No valid widget active when trying to update widget background color");
				return;
			}

			m_ActiveWidget->DefaultBackgroundColor = s_WidgetBackgroundColor.CurrentVec4;
			m_ActiveWidget->ActiveBackgroundColor = s_WidgetBackgroundColor.CurrentVec4;
			s_MainHeader.EditColorActive = true;
		};

		s_WidgetOnPress.Label = "On Press";
		s_WidgetOnPress.Flags |= EditorUI::SelectOption_Indented;
		s_WidgetOnPress.PopupAction = [&]()
		{
			s_WidgetOnPress.ClearOptions();
			s_WidgetOnPress.AddToOptions("Clear", "None", Assets::EmptyHandle);

			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_ScriptType == Scripting::ScriptType::Class)
				{
					if (script->m_FuncType != WrappedFuncType::Void_UInt64)
					{
						continue;
					}
					s_WidgetOnPress.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType) + 
						"::" + script->m_SectionLabel, script->m_ScriptName, handle);
				}
				if (script->m_ScriptType == Scripting::ScriptType::Global || script->m_ScriptType == Scripting::ScriptType::Engine)
				{
					if (script->m_FuncType != WrappedFuncType::Void_None)
					{
						continue;
					}
					s_WidgetOnPress.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType) +
						"::" + script->m_SectionLabel, script->m_ScriptName, handle);
				}
			}
		};

		s_WidgetOnPress.ConfirmAction = [&](const EditorUI::OptionEntry& entry)
		{

			if (entry.Handle == Assets::EmptyHandle)
			{
				m_ActiveWidget->FunctionPointers.OnPress = nullptr;
				m_ActiveWidget->FunctionPointers.OnPressHandle = Assets::EmptyHandle;
				return;
			}

			m_ActiveWidget->FunctionPointers.OnPressHandle = entry.Handle;
			m_ActiveWidget->FunctionPointers.OnPress = Assets::AssetService::GetScript(entry.Handle);
			
			s_MainHeader.EditColorActive = true;
		};

		s_WidgetText.Label = "Text";
		s_WidgetText.Flags |= EditorUI::EditText_Indented;
		s_WidgetText.ConfirmAction = [&](EditorUI::EditTextSpec& spec)
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

			textWidget.Text = s_WidgetText.CurrentOption;
			s_MainHeader.EditColorActive = true;
		};

		s_WidgetTextSize.Label = "Text Size";
		s_WidgetTextSize.Flags |= EditorUI::EditFloat_Indented;
		s_WidgetTextSize.ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
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

			textWidget.TextSize = s_WidgetTextSize.CurrentFloat;
			s_MainHeader.EditColorActive = true;
		};

		s_WidgetTextColor.Label = "Text Color";
		s_WidgetTextColor.Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		s_WidgetTextColor.ConfirmAction = [&](EditorUI::EditVec4Spec& spec)
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

			textWidget.TextColor = s_WidgetTextColor.CurrentVec4;
			s_MainHeader.EditColorActive = true;
		};

		s_WidgetCentered.Label = "Centered";
		s_WidgetCentered.Flags |= EditorUI::Checkbox_Indented;
		s_WidgetCentered.ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
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
			s_MainHeader.EditColorActive = true;
		};
	}

	void UIEditorPanel::AddTextWidget(EditorUI::TreeEntry& windowEntry)
	{
		UIEditorPanel& panel = *(s_EditorApp->m_UIEditorPanel.get());
		EditorUI::TreePath windowPath = s_UITree.GetPathFromEntryReference(&windowEntry);
		if (!windowPath)
		{
			KG_WARN("Could not locate window path inside s_UITree");
			return;
		}

		// Create Text Widget
		auto& window = panel.m_EditorUI->m_Windows.at(windowEntry.Handle);
		Ref<RuntimeUI::TextWidget> newTextWidget = CreateRef<RuntimeUI::TextWidget>();

		// Create new widget entry for s_UITree
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
		newWidgetEntry.OnRightClickSelection.push_back({ "Delete Widget", DeleteWidget });

		// Add Widget to RuntimeUI and EditorUI::Tree
		window.AddWidget(newTextWidget);
		windowEntry.SubEntries.push_back(newWidgetEntry);
	}

	void UIEditorPanel::DeleteWindow(EditorUI::TreeEntry& entry)
	{
		UIEditorPanel& panel = *(s_EditorApp->m_UIEditorPanel.get());
		EditorUI::TreePath path = s_UITree.GetPathFromEntryReference(&entry);
		if (!path)
		{
			KG_WARN("Could not locate window path inside s_UITree");
			return;
		}
		auto& windows = panel.m_EditorUI->m_Windows;
		windows.erase(windows.begin() + entry.Handle);
		s_UITree.RemoveEntry(path);
		panel.m_ActiveWidget = nullptr;
		panel.m_ActiveWindow = nullptr;
		panel.m_CurrentDisplay = UIPropertiesDisplay::None;

		s_MainHeader.EditColorActive = true;
		RecalculateTreeIterators();

	}

	void UIEditorPanel::DeleteWidget(EditorUI::TreeEntry& entry)
	{
		UIEditorPanel& panel = *(s_EditorApp->m_UIEditorPanel.get());
		EditorUI::TreePath path = s_UITree.GetPathFromEntryReference(&entry);
		if (!path)
		{
			KG_WARN("Could not locate widget path inside s_UITree");
			return;
		}
		auto& windows = panel.m_EditorUI->m_Windows;
		auto& widgets = windows.at(*(uint32_t*)entry.ProvidedData.get()).Widgets;

		widgets.erase(widgets.begin() + entry.Handle);
		s_UITree.RemoveEntry(path);
		panel.m_ActiveWidget = nullptr;
		panel.m_ActiveWindow = nullptr;
		panel.m_CurrentDisplay = UIPropertiesDisplay::None;

		s_MainHeader.EditColorActive = true;
		RecalculateTreeIterators();

	}

	void UIEditorPanel::AddWindow()
	{
		UIEditorPanel& panel = *(s_EditorApp->m_UIEditorPanel.get());
		EditorUI::TreeEntry newEntry {};
		newEntry.Label = "None";
		newEntry.IconHandle = EditorUI::EditorUIService::s_IconWindow;
		newEntry.Handle = s_UITree.GetTreeEntries().size();
		newEntry.OnLeftClick = [&](EditorUI::TreeEntry& entry)
		{
			UIEditorPanel& panel = *(s_EditorApp->m_UIEditorPanel.get());
			panel.m_ActiveWindow = &panel.m_EditorUI->m_Windows.at(entry.Handle);
			panel.m_CurrentDisplay = UIPropertiesDisplay::Window;
			EditorUI::EditorUIService::BringWindowToFront(s_EditorApp->m_PropertiesPanel->m_PanelName);
			s_EditorApp->m_PropertiesPanel->m_ActiveParent = panel.m_PanelName;
		};

		newEntry.OnRightClickSelection.push_back({ "Delete Window", DeleteWindow });

		newEntry.OnRightClickSelection.push_back({ "Add Text Widget", AddTextWidget });

		s_UITree.InsertEntry(newEntry);
		panel.m_EditorUI->m_Windows.push_back({});
		s_MainHeader.EditColorActive = true;
	}
}

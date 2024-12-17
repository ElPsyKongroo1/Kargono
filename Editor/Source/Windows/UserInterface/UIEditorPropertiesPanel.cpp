#include "Windows/UserInterface/UIEditorPropertiesPanel.h"

#include "EditorApp.h"
#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp{ nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };
static Kargono::Windows::UIEditorWindow* s_UIWindow{ nullptr };

namespace Kargono::Panels
{
	UIEditorPropertiesPanel::UIEditorPropertiesPanel()
	{
		// Get the main window and UI window
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
		s_UIWindow = s_EditorApp->m_UIEditorWindow.get();

		// Initialize properties panel widget resources
		InitializeWindowOptions();
		InitializeWidgetGeneralOptions();
		InitializeWidgetLocationOptions();
	}
	void UIEditorPropertiesPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_UIWindow->m_ShowProperties);

		// Early out if the window is not visible
		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		// Display the properties of the selected window/widget
		switch (m_CurrentDisplay)
		{
		case UIPropertiesDisplay::None:
			break;
		case UIPropertiesDisplay::Window:
			DrawWindowOptions();
			break;
		case UIPropertiesDisplay::Widget:
			DrawWidgetOptions();
			break;
		}

		// End the window
		EditorUI::EditorUIService::EndWindow();
	}

	void UIEditorPropertiesPanel::DrawWindowOptions()
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

	void UIEditorPropertiesPanel::DrawWidgetOptions()
	{
		// Draw main header for widget options
		EditorUI::EditorUIService::CollapsingHeader(m_WidgetGeneralHeader);
		// Draw options to edit selected widget
		if (m_WidgetGeneralHeader.m_Expanded)
		{
			// Edit selected widget's tag
			m_WidgetTag.m_CurrentOption = m_ActiveWidget->m_Tag;
			EditorUI::EditorUIService::EditText(m_WidgetTag);

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

		// Draw location header for widget options and display options to edit selected widget's location
		EditorUI::EditorUIService::CollapsingHeader(m_WidgetLocationHeader);
		if (m_WidgetLocationHeader.m_Expanded)
		{
			// Modify the X location metric
			m_WidgetXRelOrAbs.m_SelectedOption = (uint16_t)m_ActiveWidget->m_XRelativeOrAbsolute;
			EditorUI::EditorUIService::RadioSelector(m_WidgetXRelOrAbs);
			if (m_WidgetXRelOrAbs.m_SelectedOption == (uint16_t)RuntimeUI::RelativeOrAbsolute::Relative)
			{
				m_WidgetXConstraint.m_CurrentOption = { Utility::ConstraintToString(m_ActiveWidget->m_XConstraint) , (uint16_t)m_ActiveWidget->m_XConstraint};
				EditorUI::EditorUIService::SelectOption(m_WidgetXConstraint);
			}
			m_WidgetXPixelOrPercent.m_SelectedOption = (uint16_t)m_ActiveWidget->m_XPositionType;
			EditorUI::EditorUIService::RadioSelector(m_WidgetXPixelOrPercent);
			if (m_WidgetXPixelOrPercent.m_SelectedOption == (uint16_t)RuntimeUI::PixelOrPercent::Pixel)
			{
				m_WidgetXPixelLocation.m_CurrentInteger = m_ActiveWidget->m_PixelPosition.x;
				EditorUI::EditorUIService::EditInteger(m_WidgetXPixelLocation);
			}
			else
			{
				m_WidgetXPercentLocation.m_CurrentFloat = m_ActiveWidget->m_PercentPosition.x;
				EditorUI::EditorUIService::EditFloat(m_WidgetXPercentLocation);
			}
			
			EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

			// Modify the Y location metric
			m_WidgetYRelOrAbs.m_SelectedOption = (uint16_t)m_ActiveWidget->m_YRelativeOrAbsolute;
			EditorUI::EditorUIService::RadioSelector(m_WidgetYRelOrAbs);
			if (m_WidgetYRelOrAbs.m_SelectedOption == (uint16_t)RuntimeUI::RelativeOrAbsolute::Relative)
			{
				m_WidgetYConstraint.m_CurrentOption = { Utility::ConstraintToString(m_ActiveWidget->m_YConstraint) , (uint16_t)m_ActiveWidget->m_YConstraint };
				EditorUI::EditorUIService::SelectOption(m_WidgetYConstraint);
			}
			m_WidgetYPixelOrPercent.m_SelectedOption = (uint16_t)m_ActiveWidget->m_YPositionType;
			EditorUI::EditorUIService::RadioSelector(m_WidgetYPixelOrPercent);
			if (m_WidgetYPixelOrPercent.m_SelectedOption == (uint16_t)RuntimeUI::PixelOrPercent::Pixel)
			{
				m_WidgetYPixelLocation.m_CurrentInteger = m_ActiveWidget->m_PixelPosition.y;
				EditorUI::EditorUIService::EditInteger(m_WidgetYPixelLocation);
			}
			else
			{
				m_WidgetYPercentLocation.m_CurrentFloat = m_ActiveWidget->m_PercentPosition.y;
				EditorUI::EditorUIService::EditFloat(m_WidgetYPercentLocation);
			}
		}
	}

	void UIEditorPropertiesPanel::ClearPanelData()
	{
		m_ActiveWidget = nullptr;
		m_ActiveWindow = nullptr;
		m_CurrentDisplay = UIPropertiesDisplay::None;
	}

	void UIEditorPropertiesPanel::InitializeWindowOptions()
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
	void UIEditorPropertiesPanel::InitializeWidgetGeneralOptions()
	{
		// Set up header for widget options
		m_WidgetGeneralHeader.m_Label = "Widget General Options";
		m_WidgetGeneralHeader.m_Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		m_WidgetGeneralHeader.m_Expanded = true;

		// Set up widget to modify the widget's tag
		m_WidgetTag.m_Label = "Tag";
		m_WidgetTag.m_Flags |= EditorUI::EditText_Indented;
		m_WidgetTag.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWidgetTag);

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

	void UIEditorPropertiesPanel::InitializeWidgetLocationOptions()
	{
		// Set up location collapsing header
		m_WidgetLocationHeader.m_Label = "Widget Location Options";
		m_WidgetLocationHeader.m_Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		m_WidgetLocationHeader.m_Expanded = true;

		// Set up widgets for selecting a constraint type
		m_WidgetXConstraint.m_Label = "X Constraint";
		m_WidgetXConstraint.m_Flags |= EditorUI::SelectOption_Indented;
		m_WidgetXConstraint.m_CurrentOption = {"None", (uint64_t)RuntimeUI::Constraint::None};
		m_WidgetXConstraint.m_PopupAction = KG_BIND_CLASS_FN(OnOpenWidgetXConstraint);
		m_WidgetXConstraint.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWidgetXConstraint);

		m_WidgetYConstraint.m_Label = "Y Constraint";
		m_WidgetYConstraint.m_Flags |= EditorUI::SelectOption_Indented;
		m_WidgetYConstraint.m_CurrentOption = { "None", (uint64_t)RuntimeUI::Constraint::None };
		m_WidgetYConstraint.m_PopupAction = KG_BIND_CLASS_FN(OnOpenWidgetYConstraint);
		m_WidgetYConstraint.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWidgetYConstraint);

		// Set up widgets to select between relative and absolute location
		m_WidgetXRelOrAbs.m_Label = "X Spacing Mode";
		m_WidgetXRelOrAbs.m_Flags |= EditorUI::SelectOption_Indented;
		m_WidgetXRelOrAbs.m_FirstOptionLabel = "Relative";
		m_WidgetXRelOrAbs.m_SecondOptionLabel = "Absolute";
		m_WidgetXRelOrAbs.m_SelectedOption = (uint16_t)RuntimeUI::RelativeOrAbsolute::Absolute;
		m_WidgetXRelOrAbs.m_SelectAction = KG_BIND_CLASS_FN(OnModifyWidgetXLocationRelOrAbs);

		m_WidgetYRelOrAbs.m_Label = "Y Spacing Mode";
		m_WidgetYRelOrAbs.m_Flags |= EditorUI::SelectOption_Indented;
		m_WidgetYRelOrAbs.m_FirstOptionLabel = "Relative";
		m_WidgetYRelOrAbs.m_SecondOptionLabel = "Absolute";
		m_WidgetYRelOrAbs.m_SelectedOption = (uint16_t)RuntimeUI::RelativeOrAbsolute::Absolute;
		m_WidgetYRelOrAbs.m_SelectAction = KG_BIND_CLASS_FN(OnModifyWidgetYLocationRelOrAbs);

		// Set up widgets to select between pixel and percent location
		m_WidgetXPixelOrPercent.m_Label = "X Position Metric";
		m_WidgetXPixelOrPercent.m_Flags |= EditorUI::SelectOption_Indented;
		m_WidgetXPixelOrPercent.m_FirstOptionLabel = "Pixels";
		m_WidgetXPixelOrPercent.m_SecondOptionLabel = "Percent";
		m_WidgetXPixelOrPercent.m_SelectedOption = (uint16_t)RuntimeUI::PixelOrPercent::Percent;
		m_WidgetXPixelOrPercent.m_SelectAction = KG_BIND_CLASS_FN(OnModifyWidgetXLocationPixelOrPercent);

		m_WidgetYPixelOrPercent.m_Label = "Y Position Metric";
		m_WidgetYPixelOrPercent.m_Flags |= EditorUI::SelectOption_Indented;
		m_WidgetYPixelOrPercent.m_FirstOptionLabel = "Pixels";
		m_WidgetYPixelOrPercent.m_SecondOptionLabel = "Percent";
		m_WidgetYPixelOrPercent.m_SelectedOption = (uint16_t)RuntimeUI::PixelOrPercent::Percent;
		m_WidgetYPixelOrPercent.m_SelectAction = KG_BIND_CLASS_FN(OnModifyWidgetYLocationPixelOrPercent);

		// Set up widget to modify the widget's location based on the pixel value
		m_WidgetXPixelLocation.m_Label = "Pixel X Location";
		m_WidgetXPixelLocation.m_Flags |= EditorUI::EditInteger_Indented;
		m_WidgetXPixelLocation.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWidgetXPixelLocation);

		m_WidgetYPixelLocation.m_Label = "Pixel Y Location";
		m_WidgetYPixelLocation.m_Flags |= EditorUI::EditInteger_Indented;
		m_WidgetYPixelLocation.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWidgetYPixelLocation);

		// Set up widget to modify the widget's location relative to its window
		m_WidgetXPercentLocation.m_Label = "Percent X Location";
		m_WidgetXPercentLocation.m_Flags |= EditorUI::EditFloat_Indented;
		m_WidgetXPercentLocation.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWidgetXPercentLocation);

		m_WidgetYPercentLocation.m_Label = "Percent Y Location";
		m_WidgetYPercentLocation.m_Flags |= EditorUI::EditFloat_Indented;
		m_WidgetYPercentLocation.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWidgetYPercentLocation);
	}

	void UIEditorPropertiesPanel::OnModifyWindowTag(EditorUI::EditTextSpec& spec)
	{
		// Ensure active window is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid window active when trying to update window tag");
			return;
		}

		// Ensure selected window path is valid
		if (!s_UIWindow->m_TreePanel->m_UITree.m_SelectedEntry)
		{
			KG_WARN("No valid selected window path available in m_UITree when trying to update window tag");
			return;
		}

		// Get the selected window entry and ensure it is valid
		EditorUI::TreeEntry* entry = s_UIWindow->m_TreePanel->m_UITree.GetEntryFromPath(s_UIWindow->m_TreePanel->m_UITree.m_SelectedEntry);
		if (!entry)
		{
			KG_WARN("No valid selected window active in m_UITree when trying to update window tag");
			return;
		}

		// Update the window tag and tree entry label
		entry->m_Label = m_WindowTag.m_CurrentOption;
		m_ActiveWindow->m_Tag = m_WindowTag.m_CurrentOption;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnOpenWindowDefaultWidgetPopup()
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

	void UIEditorPropertiesPanel::OnModifyWindowDisplay(EditorUI::CheckboxSpec& spec)
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
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;

	}

	void UIEditorPropertiesPanel::OnModifyWindowLocation(EditorUI::EditVec3Spec& spec)
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
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWindowSize(EditorUI::EditVec2Spec& spec)
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
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWindowBackgroundColor(EditorUI::EditVec4Spec& spec)
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
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWindowDefaultWidget(const EditorUI::OptionEntry& entry)
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

	void UIEditorPropertiesPanel::OnModifyWidgetTag(EditorUI::EditTextSpec& spec)
	{
		// Ensure active widget is valid and update the widget tag
		if (!m_ActiveWidget)
		{
			KG_WARN("No valid widget active when trying to update widget tag");
			return;
		}
		if (!s_UIWindow->m_TreePanel->m_UITree.m_SelectedEntry)
		{
			KG_WARN("No valid selected widget path available in m_UITree when trying to update widget tag");
			return;
		}

		// Get the selected widget entry and ensure it is valid
		EditorUI::TreeEntry* entry = s_UIWindow->m_TreePanel->m_UITree.GetEntryFromPath(s_UIWindow->m_TreePanel->m_UITree.m_SelectedEntry);
		if (!entry)
		{
			KG_WARN("No valid selected widget active in m_UITree when trying to update widget tag");
			return;
		}
		// Update the widget tag and tree entry label
		entry->m_Label = m_WidgetTag.m_CurrentOption;
		m_ActiveWidget->m_Tag = m_WidgetTag.m_CurrentOption;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetXLocationPixelOrPercent()
	{
		// Ensure active widget is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget location metric");
			return;
		}

		// Update the widget location metric based on the radio selector value
		m_ActiveWidget->m_XPositionType = (RuntimeUI::PixelOrPercent)m_WidgetXPixelOrPercent.m_SelectedOption;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetYLocationPixelOrPercent()
	{
		// Ensure active widget is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget location metric");
			return;
		}

		// Update the widget location metric based on the radio selector value
		m_ActiveWidget->m_YPositionType = (RuntimeUI::PixelOrPercent)m_WidgetYPixelOrPercent.m_SelectedOption;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetXConstraint(const EditorUI::OptionEntry& entry)
	{
		// Ensure active widget is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget location metric");
			return;
		}

		// Update the widget location metric based on the radio selector value
		m_ActiveWidget->m_XConstraint = (RuntimeUI::Constraint)(uint16_t)entry.m_Handle;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetYConstraint(const EditorUI::OptionEntry& entry)
	{
		// Ensure active widget is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget location metric");
			return;
		}

		// Update the widget location metric based on the radio selector value
		m_ActiveWidget->m_YConstraint = (RuntimeUI::Constraint)(uint16_t)entry.m_Handle;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnOpenWidgetXConstraint()
	{
		m_WidgetXConstraint.ClearOptions();
		m_WidgetXConstraint.AddToOptions("Clear", "None", (uint64_t)RuntimeUI::Constraint::None);
		m_WidgetXConstraint.AddToOptions("All Options", "Left", (uint64_t)RuntimeUI::Constraint::Left);
		m_WidgetXConstraint.AddToOptions("All Options", "Right", (uint64_t)RuntimeUI::Constraint::Right);
		m_WidgetXConstraint.AddToOptions("All Options", "Center", (uint64_t)RuntimeUI::Constraint::Center);
	}

	void UIEditorPropertiesPanel::OnOpenWidgetYConstraint()
	{
		m_WidgetYConstraint.ClearOptions();
		m_WidgetYConstraint.AddToOptions("Clear", "None", (uint64_t)RuntimeUI::Constraint::None);
		m_WidgetYConstraint.AddToOptions("All Options", "Top", (uint64_t)RuntimeUI::Constraint::Top);
		m_WidgetYConstraint.AddToOptions("All Options", "Bottom", (uint64_t)RuntimeUI::Constraint::Bottom);
		m_WidgetYConstraint.AddToOptions("All Options", "Center", (uint64_t)RuntimeUI::Constraint::Center);
	}

	void UIEditorPropertiesPanel::OnModifyWidgetXPixelLocation(EditorUI::EditIntegerSpec& spec)
	{
		// Ensure active widget is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget's window location");
			return;
		}

		// Update the widget location based on the editorUI widget value
		m_ActiveWidget->m_PixelPosition.x = spec.m_CurrentInteger;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetYPixelLocation(EditorUI::EditIntegerSpec& spec)
	{
		// Ensure active widget is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget's window location");
			return;
		}

		// Update the widget location based on the editorUI widget value
		m_ActiveWidget->m_PixelPosition.y = spec.m_CurrentInteger;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetXPercentLocation(EditorUI::EditFloatSpec& spec)
	{
		// Ensure active widget is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget's window location");
			return;
		}

		// Update the widget location based on the editorUI widget value
		m_ActiveWidget->m_PercentPosition.x = spec.m_CurrentFloat;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetYPercentLocation(EditorUI::EditFloatSpec& spec)
	{
		// Ensure active widget is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget's window location");
			return;
		}

		// Update the widget location based on the editorUI widget value
		m_ActiveWidget->m_PercentPosition.y = spec.m_CurrentFloat;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetSize(EditorUI::EditVec2Spec& spec)
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
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetBackgroundColor(EditorUI::EditVec4Spec& spec)
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
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetOnPress(const EditorUI::OptionEntry& entry)
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
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnOpenWidgetOnPressPopup()
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

	void UIEditorPropertiesPanel::OnOpenTooltipForWidgetOnPress()
	{
		// Clear existing options
		s_UIWindow->m_TreePanel->m_SelectScriptTooltip.ClearEntries();

		// Add option to opening an existing script
		EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
		{
			m_WidgetOnPress.m_OpenPopup = true;
		} };
		s_UIWindow->m_TreePanel->m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

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
					s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
				}, {});
			} };
		s_UIWindow->m_TreePanel->m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

		// Open tooltip
		s_UIWindow->m_TreePanel->m_SelectScriptTooltip.m_TooltipActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetXLocationRelOrAbs()
	{
		// Ensure active widget is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget location metric");
			return;
		}

		// Update the widget location metric based on the radio selector value
		m_ActiveWidget->m_XRelativeOrAbsolute = (RuntimeUI::RelativeOrAbsolute)m_WidgetXRelOrAbs.m_SelectedOption;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetYLocationRelOrAbs()
	{
		// Ensure active widget is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget location metric");
			return;
		}

		// Update the widget location metric based on the radio selector value
		m_ActiveWidget->m_YRelativeOrAbsolute = (RuntimeUI::RelativeOrAbsolute)m_WidgetYRelOrAbs.m_SelectedOption;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyTextWidgetText(EditorUI::EditTextSpec& spec)
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
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyTextWidgetTextSize(EditorUI::EditFloatSpec& spec)
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
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyTextWidgetTextColor(EditorUI::EditVec4Spec& spec)
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
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyTextWidgetCentered(EditorUI::CheckboxSpec& spec)
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
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
}

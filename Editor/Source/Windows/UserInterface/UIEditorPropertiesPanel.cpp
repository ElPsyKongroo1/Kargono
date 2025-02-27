#include "Windows/UserInterface/UIEditorPropertiesPanel.h"

#include "EditorApp.h"

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
		InitializeUIOptions();
		InitializeWindowOptions();
		InitializeWidgetGeneralOptions();
		InitializeTextWidgetOptions();
		InitializeButtonWidgetOptions();
		InitializeImageButtonWidgetOptions();
		InitializeImageWidgetOptions();
		InitializeCheckboxWidgetOptions();
		InitializeContainerWidgetOptions();
		InitializeInputTextWidgetOptions();
		InitializeSliderWidgetOptions();
		InitializeDropDownWidgetOptions();
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
			DrawGeneralWidgetOptions();
			DrawSpecificWidgetOptions();
			DrawWidgetOptions();
			break;
		case UIPropertiesDisplay::UserInterface:
			DrawUIOptions();
			break;
		}

		s_EditorApp->m_MainWindow->m_ScriptEditorPanel->DrawOnCreatePopup();

		// End the window
		EditorUI::EditorUIService::EndWindow();
	}

	void UIEditorPropertiesPanel::DrawUIOptions()
	{
		// Draw main header for UI options
		EditorUI::EditorUIService::CollapsingHeader(m_UIHeader);

		// Draw options for UI
		if (m_UIHeader.m_Expanded)
		{
			// Edit font for current UI
			Assets::AssetHandle fontHandle = s_UIWindow->m_EditorUI->m_FontHandle;
			m_UISelectFont.m_CurrentOption =
			{
				fontHandle == Assets::EmptyHandle ? "None" : Assets::AssetService::GetFontInfo(fontHandle).Data.FileLocation.stem().string(),
				fontHandle
			};
			EditorUI::EditorUIService::SelectOption(m_UISelectFont);

			// Edit on move for current UI
			Assets::AssetHandle onMoveHandle = s_UIWindow->m_EditorUI->m_FunctionPointers.m_OnMoveHandle;
			m_UIOnMove.m_CurrentOption =
			{
				onMoveHandle == Assets::EmptyHandle ? "None" : Assets::AssetService::GetScriptInfo(onMoveHandle).Data.FileLocation.stem().string(),
				onMoveHandle
			};
			EditorUI::EditorUIService::SelectOption(m_UIOnMove);

			// Edit on Hover for current UI
			Assets::AssetHandle onHoverHandle = s_UIWindow->m_EditorUI->m_FunctionPointers.m_OnHoverHandle;
			m_UIOnHover.m_CurrentOption =
			{
				onHoverHandle == Assets::EmptyHandle ? "None" : Assets::AssetService::GetScriptInfo(onHoverHandle).Data.FileLocation.stem().string(),
				onHoverHandle
			};
			EditorUI::EditorUIService::SelectOption(m_UIOnHover);

			// Edit UI's selection color
			m_UISelectionColor.m_CurrentVec4 = s_UIWindow->m_EditorUI->m_SelectColor;
			EditorUI::EditorUIService::EditVec4(m_UISelectionColor);

			// Edit UI's hovered color
			m_UIHoveredColor.m_CurrentVec4 = s_UIWindow->m_EditorUI->m_HoveredColor;
			EditorUI::EditorUIService::EditVec4(m_UIHoveredColor);

			// Edit UI's editing color
			m_UIEditingColor.m_CurrentVec4 = s_UIWindow->m_EditorUI->m_EditingColor;
			EditorUI::EditorUIService::EditVec4(m_UIEditingColor);
		}
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
			Ref<RuntimeUI::Widget> activeWidget = m_ActiveWindow->m_DefaultActiveWidgetRef;
			m_WindowDefaultWidget.m_CurrentOption =
			{
				activeWidget ? activeWidget->m_Tag : "None",
				(uint64_t)m_ActiveWindow->m_DefaultActiveWidget
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
		// Draw location header for widget options and display options to edit selected widget's location
		EditorUI::EditorUIService::CollapsingHeader(m_WidgetLocationHeader);
		if (m_WidgetLocationHeader.m_Expanded)
		{
			// Modify the X location metric
			m_WidgetXRelOrAbsLocation.m_SelectedOption = (uint16_t)m_ActiveWidget->m_XRelativeOrAbsolute;
			EditorUI::EditorUIService::RadioSelector(m_WidgetXRelOrAbsLocation);
			if (m_WidgetXRelOrAbsLocation.m_SelectedOption == (uint16_t)RuntimeUI::RelativeOrAbsolute::Relative)
			{
				m_WidgetXConstraintLocation.m_CurrentOption = { Utility::ConstraintToString(m_ActiveWidget->m_XConstraint) , (uint16_t)m_ActiveWidget->m_XConstraint};
				EditorUI::EditorUIService::SelectOption(m_WidgetXConstraintLocation);
			}
			m_WidgetXPixelOrPercentLocation.m_SelectedOption = (uint16_t)m_ActiveWidget->m_XPositionType;
			EditorUI::EditorUIService::RadioSelector(m_WidgetXPixelOrPercentLocation);
			if (m_WidgetXPixelOrPercentLocation.m_SelectedOption == (uint16_t)RuntimeUI::PixelOrPercent::Pixel)
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
			m_WidgetYRelOrAbsLocation.m_SelectedOption = (uint16_t)m_ActiveWidget->m_YRelativeOrAbsolute;
			EditorUI::EditorUIService::RadioSelector(m_WidgetYRelOrAbsLocation);
			if (m_WidgetYRelOrAbsLocation.m_SelectedOption == (uint16_t)RuntimeUI::RelativeOrAbsolute::Relative)
			{
				m_WidgetYConstraintLocation.m_CurrentOption = { Utility::ConstraintToString(m_ActiveWidget->m_YConstraint) , (uint16_t)m_ActiveWidget->m_YConstraint };
				EditorUI::EditorUIService::SelectOption(m_WidgetYConstraintLocation);
			}
			m_WidgetYPixelOrPercentLocation.m_SelectedOption = (uint16_t)m_ActiveWidget->m_YPositionType;
			EditorUI::EditorUIService::RadioSelector(m_WidgetYPixelOrPercentLocation);
			if (m_WidgetYPixelOrPercentLocation.m_SelectedOption == (uint16_t)RuntimeUI::PixelOrPercent::Pixel)
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

	void UIEditorPropertiesPanel::DrawGeneralWidgetOptions()
	{
		// Draw main header for widget options
		EditorUI::EditorUIService::CollapsingHeader(m_WidgetGeneralHeader);
		// Draw options to edit selected widget
		if (m_WidgetGeneralHeader.m_Expanded)
		{
			// Edit selected widget's tag
			m_WidgetTag.m_CurrentOption = m_ActiveWidget->m_Tag;
			EditorUI::EditorUIService::EditText(m_WidgetTag);

			m_WidgetPixelOrPercentSize.m_SelectedOption = (uint16_t)m_ActiveWidget->m_SizeType;
			EditorUI::EditorUIService::RadioSelector(m_WidgetPixelOrPercentSize);
			if (m_WidgetPixelOrPercentSize.m_SelectedOption == (uint16_t)RuntimeUI::PixelOrPercent::Pixel)
			{
				// Edit selected widget's size in pixels
				m_WidgetPixelSize.m_CurrentIVec2 = m_ActiveWidget->m_PixelSize;
				EditorUI::EditorUIService::EditIVec2(m_WidgetPixelSize);
			}
			else
			{
				// Edit selected widget's size relative to its window
				m_WidgetPercentSize.m_CurrentVec2 = m_ActiveWidget->m_PercentSize;
				EditorUI::EditorUIService::EditVec2(m_WidgetPercentSize);
			}
		}
	}

	void UIEditorPropertiesPanel::DrawTextWidgetOptions()
	{
		// Draw main header for text widget options
		EditorUI::EditorUIService::CollapsingHeader(m_TextWidgetHeader);
		if (m_TextWidgetHeader.m_Expanded)
		{
			// Draw options to edit selected text widget
			RuntimeUI::TextWidget& activeTextWidget = *(RuntimeUI::TextWidget*)m_ActiveWidget;

			// Edit selected text widget's text
			m_TextWidgetText.m_CurrentOption = activeTextWidget.m_TextData.m_Text;
			EditorUI::EditorUIService::EditMultiLineText(m_TextWidgetText);

			// Edit selected text widget's text size relative to its window
			m_TextWidgetTextSize.m_CurrentFloat = activeTextWidget.m_TextData.m_TextSize;
			EditorUI::EditorUIService::EditFloat(m_TextWidgetTextSize);

			// Edit selected text widget's text color
			m_TextWidgetTextColor.m_CurrentVec4 = activeTextWidget.m_TextData.m_TextColor;
			EditorUI::EditorUIService::EditVec4(m_TextWidgetTextColor);

			// Edit selected text widget's text alignment
			m_TextWidgetTextAlignment.m_CurrentOption = { Utility::ConstraintToString(activeTextWidget.m_TextData.m_TextAlignment) , (uint64_t)activeTextWidget.m_TextData.m_TextAlignment };
			EditorUI::EditorUIService::SelectOption(m_TextWidgetTextAlignment);

			// Edit selected text widget's wrapped alignment
			m_TextWidgetTextWrapped.m_CurrentBoolean = activeTextWidget.m_TextData.m_TextWrapped;
			EditorUI::EditorUIService::Checkbox(m_TextWidgetTextWrapped);
		}
	}

	void UIEditorPropertiesPanel::DrawButtonWidgetOptions()
	{
		// Draw main header for button widget options
		EditorUI::EditorUIService::CollapsingHeader(m_ButtonWidgetHeader);
		if (m_ButtonWidgetHeader.m_Expanded)
		{
			// Draw options to edit selected button widget
			RuntimeUI::ButtonWidget& activeButtonWidget = *(RuntimeUI::ButtonWidget*)m_ActiveWidget;

			// Edit selected text widget's wrapped alignment
			m_ButtonWidgetSelectable.m_CurrentBoolean = activeButtonWidget.m_SelectionData.m_Selectable;
			EditorUI::EditorUIService::Checkbox(m_ButtonWidgetSelectable);

			// Edit selected button widget's text
			m_ButtonWidgetText.m_CurrentOption = activeButtonWidget.m_TextData.m_Text;
			EditorUI::EditorUIService::EditText(m_ButtonWidgetText);

			// Edit selected button widget's text size relative to its window
			m_ButtonWidgetTextSize.m_CurrentFloat = activeButtonWidget.m_TextData.m_TextSize;
			EditorUI::EditorUIService::EditFloat(m_ButtonWidgetTextSize);

			// Edit selected button widget's text color
			m_ButtonWidgetTextColor.m_CurrentVec4 = activeButtonWidget.m_TextData.m_TextColor;
			EditorUI::EditorUIService::EditVec4(m_ButtonWidgetTextColor);

			// Edit selected button widget's text alignment
			m_ButtonWidgetTextAlignment.m_CurrentOption = { Utility::ConstraintToString(activeButtonWidget.m_TextData.m_TextAlignment) , (uint64_t)activeButtonWidget.m_TextData.m_TextAlignment };
			EditorUI::EditorUIService::SelectOption(m_ButtonWidgetTextAlignment);

			// Edit selected widget's background color
			m_ButtonWidgetBackgroundColor.m_CurrentVec4 = activeButtonWidget.m_SelectionData.m_DefaultBackgroundColor;
			EditorUI::EditorUIService::EditVec4(m_ButtonWidgetBackgroundColor);

			// Edit selected widget's on press script
			Assets::AssetHandle onPressHandle = activeButtonWidget.m_SelectionData.m_FunctionPointers.m_OnPressHandle;
			m_ButtonWidgetOnPress.m_CurrentOption =
			{
				onPressHandle == Assets::EmptyHandle ? "None" : Assets::AssetService::GetScript(onPressHandle)->m_ScriptName,
				onPressHandle
			};
			EditorUI::EditorUIService::SelectOption(m_ButtonWidgetOnPress);
		}
	}

	void UIEditorPropertiesPanel::DrawImageWidgetOptions()
	{
		// Draw main header for Image widget options
		EditorUI::EditorUIService::CollapsingHeader(m_ImageWidgetHeader);
		if (m_ImageWidgetHeader.m_Expanded)
		{
			// Draw options to edit selected Image widget
			RuntimeUI::ImageWidget& activeImageWidget = *(RuntimeUI::ImageWidget*)m_ActiveWidget;

			// Edit selected widget's image handle
			Assets::AssetHandle imageHandle = activeImageWidget.m_ImageData.m_ImageHandle;
			m_ImageWidgetImage.m_CurrentOption =
			{
				imageHandle == Assets::EmptyHandle ? "None" : 
				Assets::AssetService::GetTexture2DInfo(imageHandle).Data.FileLocation.stem().string(),
				imageHandle
			};
			EditorUI::EditorUIService::SelectOption(m_ImageWidgetImage);

			// Edit selected widget's fixed aspect ratio usage
			m_ImageWidgetFixedAspectRatio.m_CurrentBoolean = activeImageWidget.m_ImageData.m_FixedAspectRatio;
			EditorUI::EditorUIService::Checkbox(m_ImageWidgetFixedAspectRatio);
		}
	}

	void UIEditorPropertiesPanel::DrawImageButtonWidgetOptions()
	{
		// Draw main header for image button widget options
		EditorUI::EditorUIService::CollapsingHeader(m_ImageButtonWidgetHeader);
		if (m_ImageButtonWidgetHeader.m_Expanded)
		{
			// Draw options to edit selected Image widget
			RuntimeUI::ImageButtonWidget& activeImageButtonWidget = *(RuntimeUI::ImageButtonWidget*)m_ActiveWidget;

			// Edit selected widget's image handle
			Assets::AssetHandle imageHandle = activeImageButtonWidget.m_ImageData.m_ImageHandle;
			m_ImageButtonWidgetImage.m_CurrentOption =
			{
				imageHandle == Assets::EmptyHandle ? "None" :
				Assets::AssetService::GetTexture2DInfo(imageHandle).Data.FileLocation.stem().string(),
				imageHandle
			};
			EditorUI::EditorUIService::SelectOption(m_ImageButtonWidgetImage);

			// Edit selected widget's fixed aspect ratio usage
			m_ImageButtonWidgetFixedAspectRatio.m_CurrentBoolean = activeImageButtonWidget.m_ImageData.m_FixedAspectRatio;
			EditorUI::EditorUIService::Checkbox(m_ImageButtonWidgetFixedAspectRatio);

			// Edit selected text widget's wrapped alignment
			m_ImageButtonWidgetSelectable.m_CurrentBoolean = activeImageButtonWidget.m_SelectionData.m_Selectable;
			EditorUI::EditorUIService::Checkbox(m_ImageButtonWidgetSelectable);

			// Edit selected widget's background color
			m_ImageButtonWidgetBackgroundColor.m_CurrentVec4 = activeImageButtonWidget.m_SelectionData.m_DefaultBackgroundColor;
			EditorUI::EditorUIService::EditVec4(m_ImageButtonWidgetBackgroundColor);

			// Edit selected widget's on press script
			Assets::AssetHandle onPressHandle = activeImageButtonWidget.m_SelectionData.m_FunctionPointers.m_OnPressHandle;
			m_ImageButtonWidgetOnPress.m_CurrentOption =
			{
				onPressHandle == Assets::EmptyHandle ? "None" : Assets::AssetService::GetScript(onPressHandle)->m_ScriptName,
				onPressHandle
			};
			EditorUI::EditorUIService::SelectOption(m_ImageButtonWidgetOnPress);
		}
	}

	void UIEditorPropertiesPanel::DrawCheckboxWidgetOptions()
	{
		// Draw main header for checkbox widget options
		EditorUI::EditorUIService::CollapsingHeader(m_CheckboxWidgetHeader);
		if (m_CheckboxWidgetHeader.m_Expanded)
		{
			// Draw options to edit selected checkbox widget
			RuntimeUI::CheckboxWidget& activeCheckboxWidget = *(RuntimeUI::CheckboxWidget*)m_ActiveWidget;

			// Edit widgets's checked status
			m_CheckboxWidgetChecked.m_CurrentBoolean = activeCheckboxWidget.m_Checked;
			EditorUI::EditorUIService::Checkbox(m_CheckboxWidgetChecked);

			// Edit selected widget's image handle
			Assets::AssetHandle checkedImage = activeCheckboxWidget.m_ImageChecked.m_ImageHandle;
			m_CheckboxWidgetCheckedImage.m_CurrentOption =
			{
				checkedImage == Assets::EmptyHandle ? "None" :
				Assets::AssetService::GetTexture2DInfo(checkedImage).Data.FileLocation.stem().string(),
				checkedImage
			};
			EditorUI::EditorUIService::SelectOption(m_CheckboxWidgetCheckedImage);

			// Edit selected widget's image handle
			Assets::AssetHandle unCheckedImage = activeCheckboxWidget.m_ImageUnChecked.m_ImageHandle;
			m_CheckboxWidgetUnCheckedImage.m_CurrentOption =
			{
				unCheckedImage == Assets::EmptyHandle ? "None" :
				Assets::AssetService::GetTexture2DInfo(unCheckedImage).Data.FileLocation.stem().string(),
				unCheckedImage
			};
			EditorUI::EditorUIService::SelectOption(m_CheckboxWidgetUnCheckedImage);

			// Edit selected widget's fixed aspect ratio usage
			m_CheckboxWidgetFixedAspectRatio.m_CurrentBoolean = activeCheckboxWidget.m_ImageChecked.m_FixedAspectRatio;
			EditorUI::EditorUIService::Checkbox(m_CheckboxWidgetFixedAspectRatio);

			// Edit selected text widget's wrapped alignment
			m_CheckboxWidgetSelectable.m_CurrentBoolean = activeCheckboxWidget.m_SelectionData.m_Selectable;
			EditorUI::EditorUIService::Checkbox(m_CheckboxWidgetSelectable);

			// Edit selected widget's background color
			m_CheckboxWidgetBackgroundColor.m_CurrentVec4 = activeCheckboxWidget.m_SelectionData.m_DefaultBackgroundColor;
			EditorUI::EditorUIService::EditVec4(m_CheckboxWidgetBackgroundColor);

			// Edit selected widget's on press script
			Assets::AssetHandle onPressHandle = activeCheckboxWidget.m_SelectionData.m_FunctionPointers.m_OnPressHandle;
			m_CheckboxWidgetOnPress.m_CurrentOption =
			{
				onPressHandle == Assets::EmptyHandle ? "None" : Assets::AssetService::GetScript(onPressHandle)->m_ScriptName,
				onPressHandle
			};
			EditorUI::EditorUIService::SelectOption(m_CheckboxWidgetOnPress);
		}
	}

	void UIEditorPropertiesPanel::DrawContainerWidgetOptions()
	{
		// Draw main header for container widget options
		EditorUI::EditorUIService::CollapsingHeader(m_ContainerWidgetHeader);
		if (m_ContainerWidgetHeader.m_Expanded)
		{
			// Draw options to edit selected container widget
			RuntimeUI::ContainerWidget& activeContainerWidget = *(RuntimeUI::ContainerWidget*)m_ActiveWidget;

			// Edit selected widget's container background color
			m_ContainerWidgetBackground.m_CurrentVec4 = activeContainerWidget.m_ContainerData.m_BackgroundColor;
			EditorUI::EditorUIService::EditVec4(m_ContainerWidgetBackground);
		}
	}

	void UIEditorPropertiesPanel::DrawInputTextWidgetOptions()
	{
		// Draw main header for input text widget options
		EditorUI::EditorUIService::CollapsingHeader(m_InputTextWidgetHeader);
		if (m_InputTextWidgetHeader.m_Expanded)
		{
			// Draw options to edit selected InputText widget
			RuntimeUI::InputTextWidget& activeInputTextWidget = *(RuntimeUI::InputTextWidget*)m_ActiveWidget;

			// Edit selected text widget's wrapped alignment
			m_InputTextWidgetSelectable.m_CurrentBoolean = activeInputTextWidget.m_SelectionData.m_Selectable;
			EditorUI::EditorUIService::Checkbox(m_InputTextWidgetSelectable);

			// Edit selected InputText widget's text
			m_InputTextWidgetText.m_CurrentOption = activeInputTextWidget.m_TextData.m_Text;
			EditorUI::EditorUIService::EditText(m_InputTextWidgetText);

			// Edit selected InputText widget's text size relative to its window
			m_InputTextWidgetTextSize.m_CurrentFloat = activeInputTextWidget.m_TextData.m_TextSize;
			EditorUI::EditorUIService::EditFloat(m_InputTextWidgetTextSize);

			// Edit selected InputText widget's text color
			m_InputTextWidgetTextColor.m_CurrentVec4 = activeInputTextWidget.m_TextData.m_TextColor;
			EditorUI::EditorUIService::EditVec4(m_InputTextWidgetTextColor);

			// Edit selected InputText widget's text alignment
			m_InputTextWidgetTextAlignment.m_CurrentOption = { Utility::ConstraintToString(activeInputTextWidget.m_TextData.m_TextAlignment) , (uint64_t)activeInputTextWidget.m_TextData.m_TextAlignment };
			EditorUI::EditorUIService::SelectOption(m_InputTextWidgetTextAlignment);

			// Edit selected widget's background color
			m_InputTextWidgetBackgroundColor.m_CurrentVec4 = activeInputTextWidget.m_SelectionData.m_DefaultBackgroundColor;
			EditorUI::EditorUIService::EditVec4(m_InputTextWidgetBackgroundColor);

			// Edit selected widget's on press script
			Assets::AssetHandle onPressHandle = activeInputTextWidget.m_SelectionData.m_FunctionPointers.m_OnPressHandle;
			m_InputTextWidgetOnPress.m_CurrentOption =
			{
				onPressHandle == Assets::EmptyHandle ? "None" : Assets::AssetService::GetScript(onPressHandle)->m_ScriptName,
				onPressHandle
			};
			EditorUI::EditorUIService::SelectOption(m_InputTextWidgetOnPress);

			// Edit selected widget's on move cursor script
			Assets::AssetHandle onMoveCursorHandle = activeInputTextWidget.m_OnMoveCursorHandle;
			m_InputTextWidgetOnMoveCursor.m_CurrentOption =
			{
				onMoveCursorHandle == Assets::EmptyHandle ? "None" : Assets::AssetService::GetScript(onMoveCursorHandle)->m_ScriptName,
				onMoveCursorHandle
			};
			EditorUI::EditorUIService::SelectOption(m_InputTextWidgetOnMoveCursor);
		}
	}

	void UIEditorPropertiesPanel::DrawSliderWidgetOptions()
	{
		// Draw main header for Slider widget options
		EditorUI::EditorUIService::CollapsingHeader(m_SliderWidgetHeader);
		if (m_SliderWidgetHeader.m_Expanded)
		{
			// Draw options to edit selected Slider widget
			RuntimeUI::SliderWidget& activeSliderWidget = *(RuntimeUI::SliderWidget*)m_ActiveWidget;

			// Edit selected text widget's wrapped alignment
			m_SliderWidgetSelectable.m_CurrentBoolean = activeSliderWidget.m_SelectionData.m_Selectable;
			EditorUI::EditorUIService::Checkbox(m_SliderWidgetSelectable);

			// Edit selected widget's size relative to its window
			m_SliderWidgetBounds.m_CurrentVec2 = activeSliderWidget.m_Bounds;
			EditorUI::EditorUIService::EditVec2(m_SliderWidgetBounds);

			// Edit selected widget's Slider color
			m_SliderWidgetSliderColor.m_CurrentVec4 = activeSliderWidget.m_SliderColor;
			EditorUI::EditorUIService::EditVec4(m_SliderWidgetSliderColor);

			// Edit selected widget's Line color
			m_SliderWidgetLineColor.m_CurrentVec4 = activeSliderWidget.m_LineColor;
			EditorUI::EditorUIService::EditVec4(m_SliderWidgetLineColor);

			//// Edit selected widget's background color
			//m_SliderWidgetBackgroundColor.m_CurrentVec4 = activeSliderWidget.m_SelectionData.m_DefaultBackgroundColor;
			//EditorUI::EditorUIService::EditVec4(m_SliderWidgetBackgroundColor);

			// Edit selected widget's on press script
			Assets::AssetHandle onPressHandle = activeSliderWidget.m_SelectionData.m_FunctionPointers.m_OnPressHandle;
			m_SliderWidgetOnPress.m_CurrentOption =
			{
				onPressHandle == Assets::EmptyHandle ? "None" : Assets::AssetService::GetScript(onPressHandle)->m_ScriptName,
				onPressHandle
			};
			EditorUI::EditorUIService::SelectOption(m_SliderWidgetOnPress);

			// Edit selected widget's on move slider script
			Assets::AssetHandle onMoveSliderHandle = activeSliderWidget.m_OnMoveSliderHandle;
			m_SliderWidgetOnMoveSlider.m_CurrentOption =
			{
				onMoveSliderHandle == Assets::EmptyHandle ? "None" : Assets::AssetService::GetScript(onMoveSliderHandle)->m_ScriptName,
				onMoveSliderHandle
			};
			EditorUI::EditorUIService::SelectOption(m_SliderWidgetOnMoveSlider);
		}
	}

	void UIEditorPropertiesPanel::DrawDropDownWidgetOptions()
	{
		// Draw main header for input text widget options
		EditorUI::EditorUIService::CollapsingHeader(m_DropDownWidgetHeader);
		if (m_DropDownWidgetHeader.m_Expanded)
		{
			// Draw options to edit selected DropDown widget
			RuntimeUI::DropDownWidget& activeDropDownWidget = *(RuntimeUI::DropDownWidget*)m_ActiveWidget;
			
			// Edit the drop-down open boolean
			m_DropDownWidgetDropdownOpen.m_CurrentBoolean = activeDropDownWidget.m_DropDownOpen;
			EditorUI::EditorUIService::Checkbox(m_DropDownWidgetDropdownOpen);

			// Only display the current option if an options list exists
			if (activeDropDownWidget.m_DropDownOptions.size() > 0)
			{
				// Edit the drop-down's current option
				size_t currentOption = activeDropDownWidget.m_CurrentOption;
				bool validOption{ currentOption < activeDropDownWidget.m_DropDownOptions.size() };
				m_DropDownWidgetCurrentOption.m_CurrentOption =
				{
					validOption ? activeDropDownWidget.m_DropDownOptions[currentOption].m_Text : "",
					currentOption
				};
				EditorUI::EditorUIService::SelectOption(m_DropDownWidgetCurrentOption);
			}

			// Display all drop-down options
			EditorUI::EditorUIService::List(m_DropDownWidgetOptionsList);

			// Edit selected drop-down's wrapped alignment
			m_DropDownWidgetSelectable.m_CurrentBoolean = activeDropDownWidget.m_SelectionData.m_Selectable;
			EditorUI::EditorUIService::Checkbox(m_DropDownWidgetSelectable);

#if 0 // TODO: COME BACK AND FIX PLEASE
			// Edit selected DropDown widget's text size relative to its window
			m_DropDownWidgetTextSize.m_CurrentFloat = activeDropDownWidget.m_TextData.m_TextSize;
			EditorUI::EditorUIService::EditFloat(m_DropDownWidgetTextSize);

			// Edit selected DropDown widget's text color
			m_DropDownWidgetTextColor.m_CurrentVec4 = activeDropDownWidget.m_TextData.m_TextColor;
			EditorUI::EditorUIService::EditVec4(m_DropDownWidgetTextColor);

			// Edit selected DropDown widget's text alignment
			m_DropDownWidgetTextAlignment.m_CurrentOption = { Utility::ConstraintToString(activeDropDownWidget.m_TextData.m_TextAlignment) , (uint64_t)activeDropDownWidget.m_TextData.m_TextAlignment };
			EditorUI::EditorUIService::SelectOption(m_DropDownWidgetTextAlignment);
#endif

			// Edit selected widget's background color
			m_DropDownWidgetBackgroundColor.m_CurrentVec4 = activeDropDownWidget.m_SelectionData.m_DefaultBackgroundColor;
			EditorUI::EditorUIService::EditVec4(m_DropDownWidgetBackgroundColor);

			// Edit selected widget's background color
			m_DropDownWidgetOptionBackgroundColor.m_CurrentVec4 = activeDropDownWidget.m_DropDownBackground;
			EditorUI::EditorUIService::EditVec4(m_DropDownWidgetOptionBackgroundColor);

			// Edit selected widget's on press script
			Assets::AssetHandle onPressHandle = activeDropDownWidget.m_SelectionData.m_FunctionPointers.m_OnPressHandle;
			m_DropDownWidgetOnPress.m_CurrentOption =
			{
				onPressHandle == Assets::EmptyHandle ? "None" : Assets::AssetService::GetScript(onPressHandle)->m_ScriptName,
				onPressHandle
			};
			EditorUI::EditorUIService::SelectOption(m_DropDownWidgetOnPress);

			// Edit selected widget's on select option script
			Assets::AssetHandle OnSelectOptionHandle = activeDropDownWidget.m_OnSelectOptionHandle;
			m_DropDownWidgetOnSelectOption.m_CurrentOption =
			{
				OnSelectOptionHandle == Assets::EmptyHandle ? "None" : Assets::AssetService::GetScript(OnSelectOptionHandle)->m_ScriptName,
				OnSelectOptionHandle
			};
			EditorUI::EditorUIService::SelectOption(m_DropDownWidgetOnSelectOption);
		}
		EditorUI::EditorUIService::EditText(m_DropDownWidgetOptionsListAddEntry);
		EditorUI::EditorUIService::EditText(m_DropDownWidgetEditEntry);
		EditorUI::EditorUIService::GenericPopup(m_DropDownWidgetDeleteEntryWarning);
	}

	void UIEditorPropertiesPanel::DrawSpecificWidgetOptions()
	{
		switch (m_ActiveWidget->m_WidgetType)
		{
		case RuntimeUI::WidgetTypes::TextWidget:
			DrawTextWidgetOptions();
			break;
		case RuntimeUI::WidgetTypes::ButtonWidget:
			DrawButtonWidgetOptions();
			break;
		case RuntimeUI::WidgetTypes::ImageWidget:
			DrawImageWidgetOptions();
			break;
		case RuntimeUI::WidgetTypes::ImageButtonWidget:
			DrawImageButtonWidgetOptions();
			break;
		case RuntimeUI::WidgetTypes::CheckboxWidget:
			DrawCheckboxWidgetOptions();
			break;
		case RuntimeUI::WidgetTypes::ContainerWidget:
			DrawContainerWidgetOptions();
			break;
		case RuntimeUI::WidgetTypes::InputTextWidget:
			DrawInputTextWidgetOptions();
			break;
		case RuntimeUI::WidgetTypes::SliderWidget:
			DrawSliderWidgetOptions();
			break;
		case RuntimeUI::WidgetTypes::DropDownWidget:
			DrawDropDownWidgetOptions();
			break;
		default:
			KG_ERROR("Invalid widget type attempted to be drawn");
			break;
		}
	}

	void UIEditorPropertiesPanel::OnSelectWidget()
	{
		// Ensure active window and widget are valid
		KG_ASSERT(ValidateActiveWindowAndWidget());

		// Handle refreshing specific widget data
		if (m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::DropDownWidget)
		{
			KG_ASSERT(m_DropDownWidgetOptionsList.m_OnRefresh);
			m_DropDownWidgetOptionsList.m_OnRefresh();
		}
	}

	void UIEditorPropertiesPanel::OpenWidgetTagDialog()
	{
		m_WidgetTag.m_StartPopup = true;
	}

	void UIEditorPropertiesPanel::OpenWindowTagDialog()
	{
		m_WindowTag.m_StartPopup = true;
	}

	void UIEditorPropertiesPanel::ClearPanelData()
	{
		m_ActiveWidget = nullptr;
		m_ActiveWindow = nullptr;
		m_CurrentDisplay = UIPropertiesDisplay::None;
	}

	void UIEditorPropertiesPanel::OnModifyUIFont(const EditorUI::OptionEntry& entry)
	{
		// Check for empty case
		if (entry.m_Handle == Assets::EmptyHandle)
		{
			s_UIWindow->m_EditorUI->m_FontHandle = Assets::EmptyHandle;
			s_UIWindow->m_EditorUI->m_Font = nullptr;

			// Set the active editor UI as edited
			s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
			return;
		}

		Ref<RuntimeUI::Font> fontRef{ Assets::AssetService::GetFont(entry.m_Handle) };

		// Ensure returned font is valid
		KG_ASSERT(fontRef);

		// Update UI font to new type
		RuntimeUI::RuntimeUIService::SetActiveFont(fontRef, entry.m_Handle);

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnOpenUIFontPopup(EditorUI::SelectOptionSpec& spec)
	{
		// Clear existing options
		spec.ClearOptions();
		spec.AddToOptions("Clear", "None", Assets::EmptyHandle);

		// Add all font options
		for (auto& [fontHandle, fontInfo] : Assets::AssetService::GetFontRegistry())
		{
			spec.AddToOptions("All Options", fontInfo.Data.FileLocation.stem().string(), fontHandle);
		}
	}

	void UIEditorPropertiesPanel::OnModifyUIOnMove(const EditorUI::OptionEntry& entry)
	{
		// Clear the on move script if the provided handle is empty
		if (entry.m_Handle == Assets::EmptyHandle)
		{
			RuntimeUI::RuntimeUIService::SetActiveOnMove(Assets::EmptyHandle, nullptr);

			// Set the active editor UI as edited
			s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
			return;
		}

		// Get script and ensure valid
		Ref<Scripting::Script> script = Assets::AssetService::GetScript(entry.m_Handle);
		KG_ASSERT(script);

		// Set the on move script for the UI
		RuntimeUI::RuntimeUIService::SetActiveOnMove(entry.m_Handle, script);

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnOpenUIOnMovePopup(EditorUI::SelectOptionSpec& spec)
	{
		// Clear existing options
		spec.ClearOptions();
		spec.AddToOptions("Clear", "None", Assets::EmptyHandle);

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
			spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
		}
	}

	void UIEditorPropertiesPanel::OnOpenTooltipForUIOnMove(EditorUI::SelectOptionSpec& spec)
	{
		UNREFERENCED_PARAMETER(spec);
		// Clear existing options
		s_UIWindow->m_TreePanel->m_SelectTooltip.ClearEntries();

		// Add option to opening an existing script
		EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
		{
			UNREFERENCED_PARAMETER(entry);
			m_UIOnMove.m_OpenPopup = true;
		} };
		s_UIWindow->m_TreePanel->m_SelectTooltip.AddTooltipEntry(openScriptOptions);

		// Add option or creating a new script from this usage point
		EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
		{
			UNREFERENCED_PARAMETER(entry);
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

					// Set the on move script for the UI and editor
					RuntimeUI::RuntimeUIService::SetActiveOnMove(scriptHandle, script);
					m_UIOnMove.m_CurrentOption = { script->m_ScriptName, scriptHandle };

					// Set the active editor UI as edited
					s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
				}, {});
			} 
		};
		s_UIWindow->m_TreePanel->m_SelectTooltip.AddTooltipEntry(createScriptOptions);

		// Open tooltip
		s_UIWindow->m_TreePanel->m_SelectTooltip.m_TooltipActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyUIOnHover(const EditorUI::OptionEntry& entry)
	{
		// Clear the on Hover script if the provided handle is empty
		if (entry.m_Handle == Assets::EmptyHandle)
		{
			RuntimeUI::RuntimeUIService::SetActiveOnHover(Assets::EmptyHandle, nullptr);

			// Set the active editor UI as edited
			s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
			return;
		}

		// Get script and ensure valid
		Ref<Scripting::Script> script = Assets::AssetService::GetScript(entry.m_Handle);
		KG_ASSERT(script);

		// Set the on Hover script for the UI
		RuntimeUI::RuntimeUIService::SetActiveOnHover(entry.m_Handle, script);

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnOpenUIOnHoverPopup(EditorUI::SelectOptionSpec& spec)
	{
		// Clear existing options
		spec.ClearOptions();
		spec.AddToOptions("Clear", "None", Assets::EmptyHandle);

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
			spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
		}
	}

	void UIEditorPropertiesPanel::OnOpenTooltipForUIOnHover(EditorUI::SelectOptionSpec& spec)
	{
		UNREFERENCED_PARAMETER(spec);
		// Clear existing options
		s_UIWindow->m_TreePanel->m_SelectTooltip.ClearEntries();

		// Add option to opening an existing script
		EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
		{
			UNREFERENCED_PARAMETER(entry);
			m_UIOnHover.m_OpenPopup = true;
		} };
		s_UIWindow->m_TreePanel->m_SelectTooltip.AddTooltipEntry(openScriptOptions);

		// Add option or creating a new script from this usage point
		EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
		{
				UNREFERENCED_PARAMETER(entry);
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

						// Set the on Hover script for the UI and editor
						RuntimeUI::RuntimeUIService::SetActiveOnHover(scriptHandle, script);
						m_UIOnHover.m_CurrentOption = { script->m_ScriptName, scriptHandle };

						// Set the active editor UI as edited
						s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
					}, {});
				}
		};
		s_UIWindow->m_TreePanel->m_SelectTooltip.AddTooltipEntry(createScriptOptions);

		// Open tooltip
		s_UIWindow->m_TreePanel->m_SelectTooltip.m_TooltipActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyUISelectionColor(EditorUI::EditVec4Spec& spec)
	{
		// Update the UI's selection color
		s_UIWindow->m_EditorUI->m_SelectColor = spec.m_CurrentVec4;
		RuntimeUI::RuntimeUIService::SetSelectedWidgetColor(spec.m_CurrentVec4);

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyUIHoveredColor(EditorUI::EditVec4Spec& spec)
	{
		// Update the UI's selection color
		s_UIWindow->m_EditorUI->m_HoveredColor = spec.m_CurrentVec4;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyUIEditingColor(EditorUI::EditVec4Spec& spec)
	{
		// Update the UI's selection color
		s_UIWindow->m_EditorUI->m_EditingColor = spec.m_CurrentVec4;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	bool UIEditorPropertiesPanel::ValidateActiveWindowAndWidget()
	{
		// Ensure active window is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update a field");
			return false;
		}

		// Ensure active widget is a valid type
		if (!m_ActiveWidget)
		{
			KG_WARN("No valid widget active when trying to update a field");
			return false;
		}

		return true;
	}

	bool UIEditorPropertiesPanel::ValidateActiveWindow()
	{
		// Ensure active window is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update a field");
			return false;
		}
		return true;
	}

	void UIEditorPropertiesPanel::InitializeUIOptions()
	{
		// Set up header for window options
		m_UIHeader.m_Label = "User Interface Options";
		m_UIHeader.m_Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		m_UIHeader.m_Expanded = true;

		// Set up widget to modify the UI's font
		m_UISelectFont.m_Label = "Font";
		m_UISelectFont.m_Flags |= EditorUI::SelectOption_Indented;
		m_UISelectFont.m_PopupAction = KG_BIND_CLASS_FN(OnOpenUIFontPopup);
		m_UISelectFont.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyUIFont);

		// Set up widget to modify the UI's OnMove functions
		m_UIOnMove.m_Label = "On Move";
		m_UIOnMove.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_UIOnMove.m_PopupAction = KG_BIND_CLASS_FN(OnOpenUIOnMovePopup);
		m_UIOnMove.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyUIOnMove);
		m_UIOnMove.m_OnEdit = KG_BIND_CLASS_FN(OnOpenTooltipForUIOnMove);

		// Set up widget to modify the UI's OnHover functions
		m_UIOnHover.m_Label = "On Hover";
		m_UIOnHover.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_UIOnHover.m_PopupAction = KG_BIND_CLASS_FN(OnOpenUIOnHoverPopup);
		m_UIOnHover.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyUIOnHover);
		m_UIOnHover.m_OnEdit = KG_BIND_CLASS_FN(OnOpenTooltipForUIOnHover);

		// Set up widget to modify the UI's selection background color
		m_UISelectionColor.m_Label = "Selection Color";
		m_UISelectionColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_UISelectionColor.m_Bounds = { 0.0f, 1.0f };
		m_UISelectionColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyUISelectionColor);

		// Set up widget to modify the UI's hovered background color
		m_UIHoveredColor.m_Label = "Hovered Color";
		m_UIHoveredColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_UIHoveredColor.m_Bounds = { 0.0f, 1.0f };
		m_UIHoveredColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyUIHoveredColor);

		// Set up widget to modify the UI's Editing background color
		m_UIEditingColor.m_Label = "Editing Color";
		m_UIEditingColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_UIEditingColor.m_Bounds = { 0.0f, 1.0f };
		m_UIEditingColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyUIEditingColor);
	}

	void UIEditorPropertiesPanel::InitializeWindowOptions()
	{
		// Set up header for window options
		m_WindowHeader.m_Label = "General Window Options";
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
		m_WindowSize.m_Bounds = { 0.0f, 10'000 };
		m_WindowSize.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWindowSize);

		// Set up widget to modify the window's background color
		m_WindowBackgroundColor.m_Label = "Background Color";
		m_WindowBackgroundColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_WindowBackgroundColor.m_Bounds = { 0.0f, 1.0f };
		m_WindowBackgroundColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWindowBackgroundColor);
	}
	void UIEditorPropertiesPanel::InitializeWidgetGeneralOptions()
	{
		// Set up header for widget options
		m_WidgetGeneralHeader.m_Label = "General Widget Options";
		m_WidgetGeneralHeader.m_Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		m_WidgetGeneralHeader.m_Expanded = true;

		// Set up widget to modify the widget's tag
		m_WidgetTag.m_Label = "Tag";
		m_WidgetTag.m_Flags |= EditorUI::EditText_Indented;
		m_WidgetTag.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWidgetTag);

		// Set up widgets to select between relative and absolute location
		m_WidgetPixelOrPercentSize.m_Label = "Sizing Metric";
		m_WidgetPixelOrPercentSize.m_Flags |= EditorUI::SelectOption_Indented;
		m_WidgetPixelOrPercentSize.m_FirstOptionLabel = "Pixel";
		m_WidgetPixelOrPercentSize.m_SecondOptionLabel = "Percent";
		m_WidgetPixelOrPercentSize.m_SelectedOption = (uint16_t)RuntimeUI::RelativeOrAbsolute::Relative;
		m_WidgetPixelOrPercentSize.m_SelectAction = KG_BIND_CLASS_FN(OnModifyWidgetSizePixelOrPercent);

		// Set up widget to modify the widget's size
		m_WidgetPercentSize.m_Label = "Percent Size";
		m_WidgetPercentSize.m_Flags |= EditorUI::EditVec2_Indented;
		m_WidgetPercentSize.m_Bounds = { 0, 10'000 };
		m_WidgetPercentSize.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWidgetPercentSize);

		// Set up widget to modify the widget's size
		m_WidgetPixelSize.m_Label = "Pixel Size";
		m_WidgetPixelSize.m_Flags |= EditorUI::EditIVec2_Indented;
		m_WidgetPixelSize.m_Bounds = { 0, 10'000 };
		m_WidgetPixelSize.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWidgetPixelSize);
	}

	void UIEditorPropertiesPanel::InitializeWidgetLocationOptions()
	{
		// Set up location collapsing header
		m_WidgetLocationHeader.m_Label = "Widget Location Options";
		m_WidgetLocationHeader.m_Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		m_WidgetLocationHeader.m_Expanded = true;

		// Set up widgets for selecting a constraint type
		m_WidgetXConstraintLocation.m_Label = "X Constraint";
		m_WidgetXConstraintLocation.m_Flags |= EditorUI::SelectOption_Indented;
		m_WidgetXConstraintLocation.m_CurrentOption = { "None", (uint64_t)RuntimeUI::Constraint::None };
		m_WidgetXConstraintLocation.m_PopupAction = KG_BIND_CLASS_FN(OnOpenWidgetXConstraint);
		m_WidgetXConstraintLocation.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWidgetXConstraint);

		m_WidgetYConstraintLocation.m_Label = "Y Constraint";
		m_WidgetYConstraintLocation.m_Flags |= EditorUI::SelectOption_Indented;
		m_WidgetYConstraintLocation.m_CurrentOption = { "None", (uint64_t)RuntimeUI::Constraint::None };
		m_WidgetYConstraintLocation.m_PopupAction = KG_BIND_CLASS_FN(OnOpenWidgetYConstraint);
		m_WidgetYConstraintLocation.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWidgetYConstraint);

		// Set up widgets to select between relative and absolute location
		m_WidgetXRelOrAbsLocation.m_Label = "X Spacing Mode";
		m_WidgetXRelOrAbsLocation.m_Flags |= EditorUI::SelectOption_Indented;
		m_WidgetXRelOrAbsLocation.m_FirstOptionLabel = "Relative";
		m_WidgetXRelOrAbsLocation.m_SecondOptionLabel = "Absolute";
		m_WidgetXRelOrAbsLocation.m_SelectedOption = (uint16_t)RuntimeUI::RelativeOrAbsolute::Absolute;
		m_WidgetXRelOrAbsLocation.m_SelectAction = KG_BIND_CLASS_FN(OnModifyWidgetXLocationRelOrAbs);

		m_WidgetYRelOrAbsLocation.m_Label = "Y Spacing Mode";
		m_WidgetYRelOrAbsLocation.m_Flags |= EditorUI::SelectOption_Indented;
		m_WidgetYRelOrAbsLocation.m_FirstOptionLabel = "Relative";
		m_WidgetYRelOrAbsLocation.m_SecondOptionLabel = "Absolute";
		m_WidgetYRelOrAbsLocation.m_SelectedOption = (uint16_t)RuntimeUI::RelativeOrAbsolute::Absolute;
		m_WidgetYRelOrAbsLocation.m_SelectAction = KG_BIND_CLASS_FN(OnModifyWidgetYLocationRelOrAbs);

		// Set up widgets to select between pixel and percent location
		m_WidgetXPixelOrPercentLocation.m_Label = "X Position Metric";
		m_WidgetXPixelOrPercentLocation.m_Flags |= EditorUI::SelectOption_Indented;
		m_WidgetXPixelOrPercentLocation.m_FirstOptionLabel = "Pixels";
		m_WidgetXPixelOrPercentLocation.m_SecondOptionLabel = "Percent";
		m_WidgetXPixelOrPercentLocation.m_SelectedOption = (uint16_t)RuntimeUI::PixelOrPercent::Percent;
		m_WidgetXPixelOrPercentLocation.m_SelectAction = KG_BIND_CLASS_FN(OnModifyWidgetXLocationPixelOrPercent);

		m_WidgetYPixelOrPercentLocation.m_Label = "Y Position Metric";
		m_WidgetYPixelOrPercentLocation.m_Flags |= EditorUI::SelectOption_Indented;
		m_WidgetYPixelOrPercentLocation.m_FirstOptionLabel = "Pixels";
		m_WidgetYPixelOrPercentLocation.m_SecondOptionLabel = "Percent";
		m_WidgetYPixelOrPercentLocation.m_SelectedOption = (uint16_t)RuntimeUI::PixelOrPercent::Percent;
		m_WidgetYPixelOrPercentLocation.m_SelectAction = KG_BIND_CLASS_FN(OnModifyWidgetYLocationPixelOrPercent);

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

	void UIEditorPropertiesPanel::InitializeTextWidgetOptions()
	{
		// Set up header for text widget options
		m_TextWidgetHeader.m_Label = "Text Widget Options";
		m_TextWidgetHeader.m_Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		m_TextWidgetHeader.m_Expanded = true;

		// Set up widget to modify the text widget's text
		m_TextWidgetText.m_Label = "Text";
		m_TextWidgetText.m_Flags |= EditorUI::EditMultiLineText_Indented;
		m_TextWidgetText.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyMultiLineDataText);

		// Set up widget to modify the text widget's text size
		m_TextWidgetTextSize.m_Label = "Text Size";
		m_TextWidgetTextSize.m_Flags |= EditorUI::EditFloat_Indented;
		m_TextWidgetTextSize.m_Bounds = { 0.0f, 10'000.0f };
		m_TextWidgetTextSize.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyTextDataTextSize);

		// Set up widget to modify the text widget's text color
		m_TextWidgetTextColor.m_Label = "Text Color";
		m_TextWidgetTextColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_TextWidgetTextColor.m_Bounds = { 0.0f, 1.0f };
		m_TextWidgetTextColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyTextDataTextColor);

		// Set up widget to modify the text widget's text alignment
		m_TextWidgetTextAlignment.m_Label = "Text Alignment";
		m_TextWidgetTextAlignment.m_Flags |= EditorUI::SelectOption_Indented;
		m_TextWidgetTextAlignment.m_PopupAction = KG_BIND_CLASS_FN(OnOpenTextDataAlignmentPopup);
		m_TextWidgetTextAlignment.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyTextDataAlignment);

		// Set up widget to modify if the text widget is wrapped
		m_TextWidgetTextWrapped.m_Label = "Text Wrapped";
		m_TextWidgetTextWrapped.m_Flags |= EditorUI::Checkbox_Indented;
		m_TextWidgetTextWrapped.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyTextDataWrapped);
	}

	void UIEditorPropertiesPanel::InitializeButtonWidgetOptions()
	{
		// Set up header for button widget options
		m_ButtonWidgetHeader.m_Label = "Button Widget Options";
		m_ButtonWidgetHeader.m_Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		m_ButtonWidgetHeader.m_Expanded = true;

		// Set up widget to modify the text widget's text alignment
		m_ButtonWidgetSelectable.m_Label = "Selectable";
		m_ButtonWidgetSelectable.m_Flags |= EditorUI::Checkbox_Indented;
		m_ButtonWidgetSelectable.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySelectionDataSelectable);

		// Set up widget to modify the button widget's text
		m_ButtonWidgetText.m_Label = "Text";
		m_ButtonWidgetText.m_Flags |= EditorUI::EditText_Indented;
		m_ButtonWidgetText.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySingleLineDataText);

		// Set up widget to modify the button widget's text size
		m_ButtonWidgetTextSize.m_Label = "Text Size";
		m_ButtonWidgetTextSize.m_Flags |= EditorUI::EditFloat_Indented;
		m_ButtonWidgetTextSize.m_Bounds = { 0.0f, 10'000.0f };
		m_ButtonWidgetTextSize.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyTextDataTextSize);

		// Set up widget to modify the button widget's text color
		m_ButtonWidgetTextColor.m_Label = "Text Color";
		m_ButtonWidgetTextColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_ButtonWidgetTextColor.m_Bounds = { 0.0f, 1.0f };
		m_ButtonWidgetTextColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyTextDataTextColor);

		// Set up widget to modify the button widget's text alignment
		m_ButtonWidgetTextAlignment.m_Label = "Text Alignment";
		m_ButtonWidgetTextAlignment.m_Flags |= EditorUI::SelectOption_Indented;
		m_ButtonWidgetTextAlignment.m_PopupAction = KG_BIND_CLASS_FN(OnOpenTextDataAlignmentPopup);
		m_ButtonWidgetTextAlignment.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyTextDataAlignment);

		// Set up widget to modify the button widget's background color
		m_ButtonWidgetBackgroundColor.m_Label = "Background Color";
		m_ButtonWidgetBackgroundColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_ButtonWidgetBackgroundColor.m_Bounds = { 0.0f, 1.0f };
		m_ButtonWidgetBackgroundColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySelectionDataBackgroundColor);

		// Set up widget to modify the button widget's on press script
		m_ButtonWidgetOnPress.m_Label = "On Press";
		m_ButtonWidgetOnPress.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_ButtonWidgetOnPress.m_ProvidedData = CreateRef<WrappedFuncType>(WrappedFuncType::Void_None);
		m_ButtonWidgetOnPress.m_PopupAction = KG_BIND_CLASS_FN(OnOpenSelectionDataOnPressPopup);
		m_ButtonWidgetOnPress.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySelectionDataOnPress);
		m_ButtonWidgetOnPress.m_OnEdit = KG_BIND_CLASS_FN(OnOpenTooltipForSelectionDataOnPress);
	}

	void UIEditorPropertiesPanel::InitializeImageWidgetOptions()
	{
		// Set up header for button widget options
		m_ImageWidgetHeader.m_Label = "Image Widget Options";
		m_ImageWidgetHeader.m_Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		m_ImageWidgetHeader.m_Expanded = true;

		// Set up widget to modify the image widget's image
		m_ImageWidgetImage.m_Label = "Image";
		m_ImageWidgetImage.m_Flags |= EditorUI::SelectOption_Indented;
		m_ImageWidgetImage.m_PopupAction = KG_BIND_CLASS_FN(OnOpenImageDataImagePopup);
		m_ImageWidgetImage.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyImageDataImage);

		// Set up widget to modify the image widget's fixed aspect ratio display
		m_ImageWidgetFixedAspectRatio.m_Label = "Fixed Aspect Ratio";
		m_ImageWidgetFixedAspectRatio.m_Flags |= EditorUI::Checkbox_Indented;
		m_ImageWidgetFixedAspectRatio.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyImageDataFixedAspectRatio);
	}

	void UIEditorPropertiesPanel::InitializeImageButtonWidgetOptions()
	{
		// Set up header for button widget options
		m_ImageButtonWidgetHeader.m_Label = "Image Button Widget Options";
		m_ImageButtonWidgetHeader.m_Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		m_ImageButtonWidgetHeader.m_Expanded = true;

		// Set up widget to modify the button widget's on press script
		m_ImageButtonWidgetImage.m_Label = "Image";
		m_ImageButtonWidgetImage.m_Flags |= EditorUI::SelectOption_Indented;
		m_ImageButtonWidgetImage.m_PopupAction = KG_BIND_CLASS_FN(OnOpenImageDataImagePopup);
		m_ImageButtonWidgetImage.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyImageDataImage);

		// Set up widget to modify the image widget's fixed aspect ratio display
		m_ImageButtonWidgetFixedAspectRatio.m_Label = "Fixed Aspect Ratio";
		m_ImageButtonWidgetFixedAspectRatio.m_Flags |= EditorUI::Checkbox_Indented;
		m_ImageButtonWidgetFixedAspectRatio.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyImageDataFixedAspectRatio);

		// Set up widget to modify the text widget's text alignment
		m_ImageButtonWidgetSelectable.m_Label = "Selectable";
		m_ImageButtonWidgetSelectable.m_Flags |= EditorUI::Checkbox_Indented;
		m_ImageButtonWidgetSelectable.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySelectionDataSelectable);

		// Set up widget to modify the button widget's background color
		m_ImageButtonWidgetBackgroundColor.m_Label = "Background Color";
		m_ImageButtonWidgetBackgroundColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_ImageButtonWidgetBackgroundColor.m_Bounds = { 0.0f, 1.0f };
		m_ImageButtonWidgetBackgroundColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySelectionDataBackgroundColor);

		// Set up widget to modify the button widget's on press script
		m_ImageButtonWidgetOnPress.m_Label = "On Press";
		m_ImageButtonWidgetOnPress.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_ImageButtonWidgetOnPress.m_ProvidedData = CreateRef<WrappedFuncType>(WrappedFuncType::Void_None);
		m_ImageButtonWidgetOnPress.m_PopupAction = KG_BIND_CLASS_FN(OnOpenSelectionDataOnPressPopup);
		m_ImageButtonWidgetOnPress.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySelectionDataOnPress);
		m_ImageButtonWidgetOnPress.m_OnEdit = KG_BIND_CLASS_FN(OnOpenTooltipForSelectionDataOnPress);

	}

	void UIEditorPropertiesPanel::InitializeCheckboxWidgetOptions()
	{
		// Set up header for checkbox widget options
		m_CheckboxWidgetHeader.m_Label = "Checkbox Widget Options";
		m_CheckboxWidgetHeader.m_Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		m_CheckboxWidgetHeader.m_Expanded = true;

		// Set up widget to modify the checkbox widget's checked status
		m_CheckboxWidgetChecked.m_Label = "Checked";
		m_CheckboxWidgetChecked.m_Flags |= EditorUI::Checkbox_Indented;
		m_CheckboxWidgetChecked.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyCheckboxWidgetChecked);

		// Set up widget to modify the button widget's on press script
		m_CheckboxWidgetCheckedImage.m_Label = "Checked Image";
		m_CheckboxWidgetCheckedImage.m_Flags |= EditorUI::SelectOption_Indented;
		m_CheckboxWidgetCheckedImage.m_PopupAction = KG_BIND_CLASS_FN(OnOpenCheckboxWidgetCheckedImagePopup);
		m_CheckboxWidgetCheckedImage.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyCheckboxWidgetCheckedImage);

		// Set up widget to modify the button widget's on press script
		m_CheckboxWidgetUnCheckedImage.m_Label = "UnChecked Image";
		m_CheckboxWidgetUnCheckedImage.m_Flags |= EditorUI::SelectOption_Indented;
		m_CheckboxWidgetUnCheckedImage.m_PopupAction = KG_BIND_CLASS_FN(OnOpenCheckboxWidgetUnCheckedImagePopup);
		m_CheckboxWidgetUnCheckedImage.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyCheckboxWidgetUnCheckedImage);

		// Set up widget to modify the image widget's fixed aspect ratio display
		m_CheckboxWidgetFixedAspectRatio.m_Label = "Fixed Aspect Ratio";
		m_CheckboxWidgetFixedAspectRatio.m_Flags |= EditorUI::Checkbox_Indented;
		m_CheckboxWidgetFixedAspectRatio.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyCheckboxWidgetFixedAspectRatio);

		// Set up widget to modify the text widget's text alignment
		m_CheckboxWidgetSelectable.m_Label = "Selectable";
		m_CheckboxWidgetSelectable.m_Flags |= EditorUI::Checkbox_Indented;
		m_CheckboxWidgetSelectable.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySelectionDataSelectable);

		// Set up widget to modify the button widget's background color
		m_CheckboxWidgetBackgroundColor.m_Label = "Background Color";
		m_CheckboxWidgetBackgroundColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_CheckboxWidgetBackgroundColor.m_Bounds = { 0.0f, 1.0f };
		m_CheckboxWidgetBackgroundColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySelectionDataBackgroundColor);

		// Set up widget to modify the button widget's on press script
		m_CheckboxWidgetOnPress.m_Label = "On Press";
		m_CheckboxWidgetOnPress.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_CheckboxWidgetOnPress.m_ProvidedData = CreateRef<WrappedFuncType>(WrappedFuncType::Void_Bool);
		m_CheckboxWidgetOnPress.m_PopupAction = KG_BIND_CLASS_FN(OnOpenSelectionDataOnPressPopup);
		m_CheckboxWidgetOnPress.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySelectionDataOnPress);
		m_CheckboxWidgetOnPress.m_OnEdit = KG_BIND_CLASS_FN(OnOpenTooltipForSelectionDataOnPress);
	}

	void UIEditorPropertiesPanel::InitializeContainerWidgetOptions()
	{
		// Set up header for Container widget options
		m_ContainerWidgetHeader.m_Label = "Container Widget Options";
		m_ContainerWidgetHeader.m_Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		m_ContainerWidgetHeader.m_Expanded = true;

		// Set up widget to modify the containers background color
		m_ContainerWidgetBackground.m_Label = "Background Color";
		m_ContainerWidgetBackground.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_ContainerWidgetBackground.m_Bounds = { 0.0f, 1.0f };
		m_ContainerWidgetBackground.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyContainerDataBackgroundColor);
	}

	void UIEditorPropertiesPanel::InitializeInputTextWidgetOptions()
	{
		// Set up header for input text widget options
		m_InputTextWidgetHeader.m_Label = "Input Text Widget Options";
		m_InputTextWidgetHeader.m_Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		m_InputTextWidgetHeader.m_Expanded = true;

		// Set up widget to modify the text widget's text alignment
		m_InputTextWidgetSelectable.m_Label = "Selectable";
		m_InputTextWidgetSelectable.m_Flags |= EditorUI::Checkbox_Indented;
		m_InputTextWidgetSelectable.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySelectionDataSelectable);

		// Set up widget to modify the InputText widget's text
		m_InputTextWidgetText.m_Label = "Text";
		m_InputTextWidgetText.m_Flags |= EditorUI::EditText_Indented;
		m_InputTextWidgetText.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySingleLineDataText);

		// Set up widget to modify the InputText widget's text size
		m_InputTextWidgetTextSize.m_Label = "Text Size";
		m_InputTextWidgetTextSize.m_Flags |= EditorUI::EditFloat_Indented;
		m_InputTextWidgetTextSize.m_Bounds = { 0.0f, 10'000.0f };
		m_InputTextWidgetTextSize.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyTextDataTextSize);

		// Set up widget to modify the InputText widget's text color
		m_InputTextWidgetTextColor.m_Label = "Text Color";
		m_InputTextWidgetTextColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_InputTextWidgetTextColor.m_Bounds = { 0.0f, 1.0f };
		m_InputTextWidgetTextColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyTextDataTextColor);

		// Set up widget to modify the InputText widget's text alignment
		m_InputTextWidgetTextAlignment.m_Label = "Text Alignment";
		m_InputTextWidgetTextAlignment.m_Flags |= EditorUI::SelectOption_Indented;
		m_InputTextWidgetTextAlignment.m_PopupAction = KG_BIND_CLASS_FN(OnOpenTextDataAlignmentPopup);
		m_InputTextWidgetTextAlignment.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyTextDataAlignment);

		// Set up widget to modify the InputText widget's background color
		m_InputTextWidgetBackgroundColor.m_Label = "Background Color";
		m_InputTextWidgetBackgroundColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_InputTextWidgetBackgroundColor.m_Bounds = { 0.0f, 1.0f };
		m_InputTextWidgetBackgroundColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySelectionDataBackgroundColor);

		// Set up widget to modify the InputText widget's on press script
		m_InputTextWidgetOnPress.m_Label = "On Press";
		m_InputTextWidgetOnPress.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_InputTextWidgetOnPress.m_ProvidedData = CreateRef<WrappedFuncType>(WrappedFuncType::Void_None);
		m_InputTextWidgetOnPress.m_PopupAction = KG_BIND_CLASS_FN(OnOpenSelectionDataOnPressPopup);
		m_InputTextWidgetOnPress.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySelectionDataOnPress);
		m_InputTextWidgetOnPress.m_OnEdit = KG_BIND_CLASS_FN(OnOpenTooltipForSelectionDataOnPress);

		// Set up widget to modify the InputText widget's on press script
		m_InputTextWidgetOnMoveCursor.m_Label = "On Move Cursor";
		m_InputTextWidgetOnMoveCursor.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_InputTextWidgetOnMoveCursor.m_PopupAction = KG_BIND_CLASS_FN(OnOpenInputTextOnMoveCursorPopup);
		m_InputTextWidgetOnMoveCursor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyInputTextOnMoveCursor);
		m_InputTextWidgetOnMoveCursor.m_OnEdit = KG_BIND_CLASS_FN(OnOpenTooltipForInputTextWidgetOnMoveCursor);
	}

	void UIEditorPropertiesPanel::InitializeSliderWidgetOptions()
	{
		// Set up header for Slider widget options
		m_SliderWidgetHeader.m_Label = "Slider Widget Options";
		m_SliderWidgetHeader.m_Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		m_SliderWidgetHeader.m_Expanded = true;

		// Set up widget to modify the text widget's text alignment
		m_SliderWidgetSelectable.m_Label = "Selectable";
		m_SliderWidgetSelectable.m_Flags |= EditorUI::Checkbox_Indented;
		m_SliderWidgetSelectable.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySelectionDataSelectable);

		// Set up widget to modify the widget's size
		m_SliderWidgetBounds.m_Label = "Bounds";
		m_SliderWidgetBounds.m_Flags |= EditorUI::EditVec2_Indented;
		m_SliderWidgetBounds.m_Bounds = { 0, 10'000 };
		m_SliderWidgetBounds.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySliderWidgetBounds);

		// Set up widget to modify the Slider widget's color
		m_SliderWidgetSliderColor.m_Label = "Slider Color";
		m_SliderWidgetSliderColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_SliderWidgetSliderColor.m_Bounds = { 0.0f, 1.0f };
		m_SliderWidgetSliderColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySliderWidgetSliderColor);

		// Set up widget to modify the Slider widget's color
		m_SliderWidgetLineColor.m_Label = "Line Color";
		m_SliderWidgetLineColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_SliderWidgetLineColor.m_Bounds = { 0.0f, 1.0f };
		m_SliderWidgetLineColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySliderWidgetLineColor);

		// Set up widget to modify the Slider widget's background color
		m_SliderWidgetBackgroundColor.m_Label = "Background Color";
		m_SliderWidgetBackgroundColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_SliderWidgetBackgroundColor.m_Bounds = { 0.0f, 1.0f };
		m_SliderWidgetBackgroundColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySelectionDataBackgroundColor);

		// Set up widget to modify the Slider widget's on press script
		m_SliderWidgetOnPress.m_Label = "On Press";
		m_SliderWidgetOnPress.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_SliderWidgetOnPress.m_ProvidedData = CreateRef<WrappedFuncType>(WrappedFuncType::Void_None);
		m_SliderWidgetOnPress.m_PopupAction = KG_BIND_CLASS_FN(OnOpenSelectionDataOnPressPopup);
		m_SliderWidgetOnPress.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySelectionDataOnPress);
		m_SliderWidgetOnPress.m_OnEdit = KG_BIND_CLASS_FN(OnOpenTooltipForSelectionDataOnPress);

		// Set up widget to modify the Slider widget's on press script
		m_SliderWidgetOnMoveSlider.m_Label = "On Move Slider";
		m_SliderWidgetOnMoveSlider.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_SliderWidgetOnMoveSlider.m_PopupAction = KG_BIND_CLASS_FN(OnOpenSliderWidgetOnMoveSliderPopup);
		m_SliderWidgetOnMoveSlider.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySliderWidgetOnMoveSlider);
		m_SliderWidgetOnMoveSlider.m_OnEdit = KG_BIND_CLASS_FN(OnOpenTooltipForSliderWidgetOnMoveSlider);
	}

	void UIEditorPropertiesPanel::InitializeDropDownWidgetOptions()
	{
		// Set up header for Drop-Down widget options
		m_DropDownWidgetHeader.m_Label = "Drop-Down Widget Options";
		m_DropDownWidgetHeader.m_Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		m_DropDownWidgetHeader.m_Expanded = true;

		// Set up widget to modify the text widget's text alignment
		m_DropDownWidgetSelectable.m_Label = "Selectable";
		m_DropDownWidgetSelectable.m_Flags |= EditorUI::Checkbox_Indented;
		m_DropDownWidgetSelectable.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySelectionDataSelectable);

		// Set up widget to modify the DropDown widget's text size
		m_DropDownWidgetTextSize.m_Label = "Text Size";
		m_DropDownWidgetTextSize.m_Flags |= EditorUI::EditFloat_Indented;
		m_DropDownWidgetTextSize.m_Bounds = { 0.0f, 10'000.0f };
		m_DropDownWidgetTextSize.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyDropDownTextSize);

		// Set up widget to modify the DropDown widget's text color
		m_DropDownWidgetTextColor.m_Label = "Text Color";
		m_DropDownWidgetTextColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_DropDownWidgetTextColor.m_Bounds = { 0.0f, 1.0f };
		m_DropDownWidgetTextColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyDropDownTextColor);

		// Set up widget to modify the DropDown widget's text alignment
		m_DropDownWidgetTextAlignment.m_Label = "Text Alignment";
		m_DropDownWidgetTextAlignment.m_Flags |= EditorUI::SelectOption_Indented;
		m_DropDownWidgetTextAlignment.m_PopupAction = KG_BIND_CLASS_FN(OnOpenTextDataAlignmentPopup);
		m_DropDownWidgetTextAlignment.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyDropDownAlignment);

		// Set up widget to modify the DropDown widget's background color
		m_DropDownWidgetBackgroundColor.m_Label = "Current Option Color";
		m_DropDownWidgetBackgroundColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_DropDownWidgetBackgroundColor.m_Bounds = { 0.0f, 1.0f };
		m_DropDownWidgetBackgroundColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySelectionDataBackgroundColor);

		// Set up widget to modify the DropDown widget's background color
		m_DropDownWidgetOptionBackgroundColor.m_Label = "Drop Down Color";
		m_DropDownWidgetOptionBackgroundColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_DropDownWidgetOptionBackgroundColor.m_Bounds = { 0.0f, 1.0f };
		m_DropDownWidgetOptionBackgroundColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyDropDownWidgetOptionBackgroundColor);

		// Set up widget to modify the DropDown widget's on press script
		m_DropDownWidgetOnPress.m_Label = "On Press";
		m_DropDownWidgetOnPress.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_DropDownWidgetOnPress.m_ProvidedData = CreateRef<WrappedFuncType>(WrappedFuncType::Void_None);
		m_DropDownWidgetOnPress.m_PopupAction = KG_BIND_CLASS_FN(OnOpenSelectionDataOnPressPopup);
		m_DropDownWidgetOnPress.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySelectionDataOnPress);
		m_DropDownWidgetOnPress.m_OnEdit = KG_BIND_CLASS_FN(OnOpenTooltipForSelectionDataOnPress);

		// Set up widget to modify the DropDown widget's on press script
		m_DropDownWidgetOnSelectOption.m_Label = "On Select Option";
		m_DropDownWidgetOnSelectOption.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_DropDownWidgetOnSelectOption.m_PopupAction = KG_BIND_CLASS_FN(OnOpenDropDownWidgetOnSelectOptionPopup);
		m_DropDownWidgetOnSelectOption.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyDropDownWidgetOnSelectOption);
		m_DropDownWidgetOnSelectOption.m_OnEdit = KG_BIND_CLASS_FN(OnOpenTooltipForDropDownWidgetOnSelectOption);

		// Set up editorUI widget to modify the DropDown widget's open boolean
		m_DropDownWidgetDropdownOpen.m_Label = "Drop-Down Open";
		m_DropDownWidgetDropdownOpen.m_Flags |= EditorUI::Checkbox_Indented;
		m_DropDownWidgetDropdownOpen.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyDropDownWidgetOpen);

		// Set up widget to modify the DropDown widget's current option
		m_DropDownWidgetCurrentOption.m_Label = "Current Option";
		m_DropDownWidgetCurrentOption.m_Flags |= EditorUI::SelectOption_Indented;
		m_DropDownWidgetCurrentOption.m_PopupAction = KG_BIND_CLASS_FN(OnOpenDropDownWidgetCurrentOptionPopup);
		m_DropDownWidgetCurrentOption.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyDropDownWidgetCurrentOption);

		// Set up editorUI widget to view the DropDown widget's options list
		m_DropDownWidgetOptionsList.m_Label = "Drop-Down Options";
		m_DropDownWidgetOptionsList.m_Column1Title = "Option Name";
		m_DropDownWidgetOptionsList.m_Expanded = true;
		m_DropDownWidgetOptionsList.m_Flags |= EditorUI::List_Indented | EditorUI::List_RegularSizeTitle;
		m_DropDownWidgetOptionsList.m_OnRefresh = KG_BIND_CLASS_FN(OnRefreshDropDownWidgetOptionsList);
		m_DropDownWidgetOptionsList.AddToSelectionList("Add Option", KG_BIND_CLASS_FN(OnDropDownWidgetAddEntryDialog));

		m_DropDownWidgetOptionsListAddEntry.m_Label = "Add New Entry";
		m_DropDownWidgetOptionsListAddEntry.m_Flags |= EditorUI::EditText_PopupOnly;
		m_DropDownWidgetOptionsListAddEntry.m_CurrentOption = "New Entry";
		m_DropDownWidgetOptionsListAddEntry.m_ConfirmAction = KG_BIND_CLASS_FN(OnDropDownWidgetAddEntry);

		m_DropDownWidgetDeleteEntryWarning.m_Label = "Delete Drop-Down Option";
		m_DropDownWidgetDeleteEntryWarning.m_PopupContents = []() 
		{
			EditorUI::EditorUIService::Text("Are you sure you want to delete this option?");
		};
		m_DropDownWidgetDeleteEntryWarning.m_ConfirmAction = KG_BIND_CLASS_FN(OnDropDownWidgetDeleteEntry);

		m_DropDownWidgetEditEntry.m_Label = "Edit Entry";
		m_DropDownWidgetEditEntry.m_Flags |= EditorUI::EditText_PopupOnly;
		m_DropDownWidgetEditEntry.m_CurrentOption = "New Entry";
		m_DropDownWidgetEditEntry.m_ConfirmAction = KG_BIND_CLASS_FN(OnDropDownWidgetEditEntry);

	}

	void UIEditorPropertiesPanel::OnModifyWindowTag(EditorUI::EditTextSpec& spec)
	{
		UNREFERENCED_PARAMETER(spec);
		// Ensure active window is valid
		if (!ValidateActiveWindow())
		{
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

	void UIEditorPropertiesPanel::OnOpenWindowDefaultWidgetPopup(EditorUI::SelectOptionSpec& spec)
	{
		// Clear existing options
		spec.ClearOptions();
		spec.AddToOptions("Clear", "None", (uint64_t)RuntimeUI::k_InvalidWidgetID);

		// Add all text widgets to the options
		std::size_t iteration{ 0 };
		for (Ref<RuntimeUI::Widget> widget : m_ActiveWindow->GetAllChildWidgets())
		{
			if (!widget->Selectable())
			{
				continue;
			}
			spec.AddToOptions(Utility::WidgetTypeToDisplayString(widget->m_WidgetType), widget->m_Tag, (uint64_t)widget->m_ID);

			iteration++;
		}
	}

	void UIEditorPropertiesPanel::OnModifyWindowDisplay(EditorUI::CheckboxSpec& spec)
	{
		// Ensure active window is valid
		if (!ValidateActiveWindow())
		{
			return;
		}

		// Update the window display based on the checkbox value
		spec.m_CurrentBoolean ? m_ActiveWindow->DisplayWindow() : m_ActiveWindow->HideWindow();

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;

	}

	void UIEditorPropertiesPanel::OnModifyWindowLocation(EditorUI::EditVec3Spec& spec)
	{
		UNREFERENCED_PARAMETER(spec);
		// Ensure active window is valid
		if (!ValidateActiveWindow())
		{
			return;
		}

		// Update the window location based on the widget value
		m_ActiveWindow->m_ScreenPosition = m_WindowLocation.m_CurrentVec3;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWindowSize(EditorUI::EditVec2Spec& spec)
	{
		UNREFERENCED_PARAMETER(spec);
		// Ensure active window is valid
		if (!ValidateActiveWindow())
		{
			return;
		}

		// Update the window size based on the widget value
		m_ActiveWindow->m_Size = m_WindowSize.m_CurrentVec2;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWindowBackgroundColor(EditorUI::EditVec4Spec& spec)
	{
		UNREFERENCED_PARAMETER(spec);
		// Ensure active window is valid
		if (!ValidateActiveWindow())
		{
			return;
		}

		// Update the window background color based on the widget value
		m_ActiveWindow->m_BackgroundColor = m_WindowBackgroundColor.m_CurrentVec4;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWindowDefaultWidget(const EditorUI::OptionEntry& entry)
	{
		// Ensure active window is valid
		if (!ValidateActiveWindow())
		{
			return;
		}

		// Clear the default active widget if the provided index is invalid
		if (entry.m_Handle == (uint64_t)RuntimeUI::k_InvalidWidgetID)
		{
			m_ActiveWindow->m_DefaultActiveWidget = RuntimeUI::k_InvalidWidgetID;
			m_ActiveWindow->m_DefaultActiveWidgetRef = nullptr;
			return;
		}

		// Get the new default widget
		Ref<RuntimeUI::Widget> newDefaultWidget = RuntimeUI::RuntimeUIService::GetWidgetFromID((int32_t)entry.m_Handle);
		KG_ASSERT(newDefaultWidget);

		// Update the default active widget for the window
		m_ActiveWindow->m_DefaultActiveWidget = (int32_t)entry.m_Handle;
		m_ActiveWindow->m_DefaultActiveWidgetRef = newDefaultWidget;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetTag(EditorUI::EditTextSpec& spec)
	{
		UNREFERENCED_PARAMETER(spec);
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		//
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
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Update the widget location metric based on the radio selector value
		m_ActiveWidget->m_XPositionType = (RuntimeUI::PixelOrPercent)m_WidgetXPixelOrPercentLocation.m_SelectedOption;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetYLocationPixelOrPercent()
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Update the widget location metric based on the radio selector value
		m_ActiveWidget->m_YPositionType = (RuntimeUI::PixelOrPercent)m_WidgetYPixelOrPercentLocation.m_SelectedOption;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetXConstraint(const EditorUI::OptionEntry& entry)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Update the widget location metric based on the radio selector value
		m_ActiveWidget->m_XConstraint = (RuntimeUI::Constraint)(uint16_t)entry.m_Handle;

		// Reset the pixel and percent positions if the constraint is set
		m_ActiveWidget->m_PixelPosition.x = 0;
		m_ActiveWidget->m_PercentPosition.x = 0;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetYConstraint(const EditorUI::OptionEntry& entry)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Update the widget location metric based on the radio selector value
		m_ActiveWidget->m_YConstraint = (RuntimeUI::Constraint)(uint16_t)entry.m_Handle;

		// Reset the pixel and percent positions if the constraint is set
		m_ActiveWidget->m_PixelPosition.y = 0;
		m_ActiveWidget->m_PercentPosition.y = 0;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnOpenWidgetXConstraint(EditorUI::SelectOptionSpec& spec)
	{
		spec.ClearOptions();
		spec.AddToOptions("Clear", "None", (uint64_t)RuntimeUI::Constraint::None);
		spec.AddToOptions("All Options", "Left", (uint64_t)RuntimeUI::Constraint::Left);
		spec.AddToOptions("All Options", "Right", (uint64_t)RuntimeUI::Constraint::Right);
		spec.AddToOptions("All Options", "Center", (uint64_t)RuntimeUI::Constraint::Center);
	}

	void UIEditorPropertiesPanel::OnOpenWidgetYConstraint(EditorUI::SelectOptionSpec& spec)
	{
		spec.ClearOptions();
		spec.AddToOptions("Clear", "None", (uint64_t)RuntimeUI::Constraint::None);
		spec.AddToOptions("All Options", "Top", (uint64_t)RuntimeUI::Constraint::Top);
		spec.AddToOptions("All Options", "Bottom", (uint64_t)RuntimeUI::Constraint::Bottom);
		spec.AddToOptions("All Options", "Center", (uint64_t)RuntimeUI::Constraint::Center);
	}

	void UIEditorPropertiesPanel::OnModifyWidgetXPixelLocation(EditorUI::EditIntegerSpec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Update the widget location based on the editorUI widget value
		m_ActiveWidget->m_PixelPosition.x = spec.m_CurrentInteger;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetYPixelLocation(EditorUI::EditIntegerSpec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Update the widget location based on the editorUI widget value
		m_ActiveWidget->m_PixelPosition.y = spec.m_CurrentInteger;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetXPercentLocation(EditorUI::EditFloatSpec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Update the widget location based on the editorUI widget value
		m_ActiveWidget->m_PercentPosition.x = spec.m_CurrentFloat;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetYPercentLocation(EditorUI::EditFloatSpec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Update the widget location based on the editorUI widget value
		m_ActiveWidget->m_PercentPosition.y = spec.m_CurrentFloat;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetSizePixelOrPercent()
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Update the widget location metric based on the radio selector value
		m_ActiveWidget->m_SizeType = (RuntimeUI::PixelOrPercent)m_WidgetPixelOrPercentSize.m_SelectedOption;
		RuntimeUI::RuntimeUIService::RecalculateTextData(m_ActiveWidget);

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetPercentSize(EditorUI::EditVec2Spec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		bool useXAsBasis = std::fabs(spec.m_CurrentVec2.x - m_ActiveWidget->m_PercentSize.x) >
			std::fabs(spec.m_CurrentVec2.y - m_ActiveWidget->m_PercentSize.y);

		// Update the widget size based on the editorUI widget value
		m_ActiveWidget->m_PercentSize = m_WidgetPercentSize.m_CurrentVec2;
		RuntimeUI::RuntimeUIService::RecalculateTextData( m_ActiveWidget);

		// Revalidate widget size if fixed aspect ratio is specified
		RuntimeUI::ImageData* imageData = RuntimeUI::RuntimeUIService::GetImageDataFromWidget(m_ActiveWidget);
		if (imageData && imageData->m_FixedAspectRatio)
		{
			ViewportData& currentViewport = s_UIWindow->m_ViewportPanel->m_ViewportData;
			RuntimeUI::RuntimeUIService::CalculateFixedAspectRatioSize
			(
				m_ActiveWidget,
				currentViewport.m_Width,
				currentViewport.m_Height,
				useXAsBasis
			);
		}

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetPixelSize(EditorUI::EditIVec2Spec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		bool useXAsBasis = std::abs(spec.m_CurrentIVec2.x - m_ActiveWidget->m_PixelSize.x) >=
			std::abs(spec.m_CurrentIVec2.y - m_ActiveWidget->m_PixelSize.y);

		// Update the widget size based on the editorUI widget value
		m_ActiveWidget->m_PixelSize = spec.m_CurrentIVec2;
		RuntimeUI::RuntimeUIService::RecalculateTextData(m_ActiveWidget);

		// Revalidate widget size if fixed aspect ratio is specified
		RuntimeUI::ImageData* imageData = RuntimeUI::RuntimeUIService::GetImageDataFromWidget(m_ActiveWidget);
		if (imageData && imageData->m_FixedAspectRatio)
		{
			ViewportData& currentViewport = s_UIWindow->m_ViewportPanel->m_ViewportData;
			RuntimeUI::RuntimeUIService::CalculateFixedAspectRatioSize
			(
				m_ActiveWidget,
				currentViewport.m_Width,
				currentViewport.m_Height,
				useXAsBasis
			);
		}

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetXLocationRelOrAbs()
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Update the widget location metric based on the radio selector value
		m_ActiveWidget->m_XRelativeOrAbsolute = (RuntimeUI::RelativeOrAbsolute)m_WidgetXRelOrAbsLocation.m_SelectedOption;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetYLocationRelOrAbs()
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Update the widget location metric based on the radio selector value
		m_ActiveWidget->m_YRelativeOrAbsolute = (RuntimeUI::RelativeOrAbsolute)m_WidgetYRelOrAbsLocation.m_SelectedOption;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	
	void UIEditorPropertiesPanel::OnModifyCheckboxWidgetChecked(EditorUI::CheckboxSpec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Ensure active widget is a valid type and get the button widget
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::CheckboxWidget)
		{
			KG_WARN("Attempt to modify checkbox widget member, however, active widget is an invalid type");
			return;
		}
		RuntimeUI::CheckboxWidget& checkboxWidget = *(RuntimeUI::CheckboxWidget*)m_ActiveWidget;

		// Update the checkbox widget selectable value
		checkboxWidget.m_Checked = spec.m_CurrentBoolean;
		if (checkboxWidget.m_ImageChecked.m_FixedAspectRatio)
		{
			ViewportData& currentViewport = s_UIWindow->m_ViewportPanel->m_ViewportData;
			RuntimeUI::RuntimeUIService::CalculateFixedAspectRatioSize
			(
				m_ActiveWidget,
				currentViewport.m_Width,
				currentViewport.m_Height,
				true
			);
		}

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnModifyCheckboxWidgetCheckedImage(const EditorUI::OptionEntry& entry)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Get the active widget as a checkbox widget
		KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::CheckboxWidget);
		RuntimeUI::CheckboxWidget& activeCheckboxWidget = *(RuntimeUI::CheckboxWidget*)m_ActiveWidget;

		// Clear the image reference if an empty entry is provided
		if (entry.m_Handle == Assets::EmptyHandle)
		{
			activeCheckboxWidget.m_ImageChecked.m_ImageRef = nullptr;
			activeCheckboxWidget.m_ImageChecked.m_ImageHandle = Assets::EmptyHandle;

			// Set the active editor UI as edited
			s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
			return;
		}

		// Set the texture reference for the checkbox widget
		activeCheckboxWidget.m_ImageChecked.m_ImageRef = Assets::AssetService::GetTexture2D(entry.m_Handle);
		activeCheckboxWidget.m_ImageChecked.m_ImageHandle = entry.m_Handle;

		// Resize to fit new image aspect ratio if necessary
		if (activeCheckboxWidget.m_ImageChecked.m_FixedAspectRatio)
		{
			ViewportData& currentViewport = s_UIWindow->m_ViewportPanel->m_ViewportData;
			RuntimeUI::RuntimeUIService::CalculateFixedAspectRatioSize
			(
				m_ActiveWidget,
				currentViewport.m_Width,
				currentViewport.m_Height,
				true
			);
		}

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnOpenCheckboxWidgetCheckedImagePopup(EditorUI::SelectOptionSpec& spec)
	{
		// Clear existing options
		spec.ClearOptions();
		spec.AddToOptions("Clear", "None", Assets::EmptyHandle);

		// Add all compatible textures to the select options
		for (auto& [handle, assetInfo] : Assets::AssetService::GetTexture2DRegistry())
		{
			// Get texture from handle
			Ref<Rendering::Texture2D> script = Assets::AssetService::GetTexture2D(handle);

			// Add texture to the select options
			spec.AddToOptions("All Options", assetInfo.Data.FileLocation.stem().string(), handle);
		}
	}
	void UIEditorPropertiesPanel::OnModifyCheckboxWidgetUnCheckedImage(const EditorUI::OptionEntry& entry)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Get the active widget as a checkbox widget
		KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::CheckboxWidget);
		RuntimeUI::CheckboxWidget& activeCheckboxWidget = *(RuntimeUI::CheckboxWidget*)m_ActiveWidget;

		// Clear the on press script if the provided handle is empty
		if (entry.m_Handle == Assets::EmptyHandle)
		{
			activeCheckboxWidget.m_ImageUnChecked.m_ImageRef = nullptr;
			activeCheckboxWidget.m_ImageUnChecked.m_ImageHandle = Assets::EmptyHandle;

			// Set the active editor UI as edited
			s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
			return;
		}

		// Set the texture reference for the checkbox widget
		activeCheckboxWidget.m_ImageUnChecked.m_ImageRef = Assets::AssetService::GetTexture2D(entry.m_Handle);
		activeCheckboxWidget.m_ImageUnChecked.m_ImageHandle = entry.m_Handle;

		// Resize to fit new image aspect ratio if necessary
		if (activeCheckboxWidget.m_ImageChecked.m_FixedAspectRatio) // Note that m_Checked here is intentional!!! (and kinda silly tbh)
		{
			ViewportData& currentViewport = s_UIWindow->m_ViewportPanel->m_ViewportData;
			RuntimeUI::RuntimeUIService::CalculateFixedAspectRatioSize
			(
				m_ActiveWidget,
				currentViewport.m_Width,
				currentViewport.m_Height,
				true
			);
		}

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnOpenCheckboxWidgetUnCheckedImagePopup(EditorUI::SelectOptionSpec& spec)
	{
		// Clear existing options
		spec.ClearOptions();
		spec.AddToOptions("Clear", "None", Assets::EmptyHandle);

		// Add all compatible textures to the select options
		for (auto& [handle, assetInfo] : Assets::AssetService::GetTexture2DRegistry())
		{
			// Get texture from handle
			Ref<Rendering::Texture2D> script = Assets::AssetService::GetTexture2D(handle);

			// Add texture to the select options
			spec.AddToOptions("All Options", assetInfo.Data.FileLocation.stem().string(), handle);
		}
	}
	void UIEditorPropertiesPanel::OnModifyCheckboxWidgetFixedAspectRatio(EditorUI::CheckboxSpec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Ensure active widget is a valid type and get the button widget
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::CheckboxWidget)
		{
			KG_WARN("Attempt to modify an checkbox widget member, however, active widget is an invalid type");
			return;
		}
		RuntimeUI::CheckboxWidget& CheckboxWidget = *(RuntimeUI::CheckboxWidget*)m_ActiveWidget;

		// Update the text widget text alignment based on the editorUI widget's value
		CheckboxWidget.m_ImageChecked.m_FixedAspectRatio = spec.m_CurrentBoolean;
		CheckboxWidget.m_ImageUnChecked.m_FixedAspectRatio = spec.m_CurrentBoolean;

		// Calculate navigation links
		RuntimeUI::NavigationLinksCalculator newCalculator;
		newCalculator.CalculateNavigationLinks(RuntimeUI::RuntimeUIService::GetActiveUI(),
			EngineService::GetActiveWindow().GetActiveViewport());

		if (spec.m_CurrentBoolean)
		{
			ViewportData& currentViewport = s_UIWindow->m_ViewportPanel->m_ViewportData;
			RuntimeUI::RuntimeUIService::CalculateFixedAspectRatioSize
			(
				m_ActiveWidget,
				currentViewport.m_Width,
				currentViewport.m_Height,
				true
			);
		}

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyInputTextOnMoveCursor(const EditorUI::OptionEntry& entry)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Ensure this is the correct widget type
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::InputTextWidget)
		{
			KG_WARN("Invalid widget type provided when modifying widget");
			return;
		}

		// Get the underlying widget
		RuntimeUI::InputTextWidget* activeInputTextWidget = (RuntimeUI::InputTextWidget*)m_ActiveWidget;

		// Clear the widget's script if the provided handle is empty
		if (entry.m_Handle == Assets::EmptyHandle)
		{
			activeInputTextWidget->m_OnMoveCursor = nullptr;
			activeInputTextWidget->m_OnMoveCursorHandle = Assets::EmptyHandle;

			// Set the active editor UI as edited
			s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
			return;
		}

		// Set the script for the widget
		activeInputTextWidget->m_OnMoveCursorHandle = entry.m_Handle;
		activeInputTextWidget->m_OnMoveCursor = Assets::AssetService::GetScript(entry.m_Handle);

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnOpenInputTextOnMoveCursorPopup(EditorUI::SelectOptionSpec& spec)
	{
		// Clear existing options
		spec.ClearOptions();
		spec.AddToOptions("Clear", "None", Assets::EmptyHandle);

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
			spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
		}
	}

	void UIEditorPropertiesPanel::OnOpenTooltipForInputTextWidgetOnMoveCursor(EditorUI::SelectOptionSpec& spec)
	{
		UNREFERENCED_PARAMETER(spec);
		// Clear existing options
		s_UIWindow->m_TreePanel->m_SelectTooltip.ClearEntries();

		// Add option to opening an existing script
		EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
		{
			UNREFERENCED_PARAMETER(entry);
			m_InputTextWidgetOnMoveCursor.m_OpenPopup = true;
		} };
		s_UIWindow->m_TreePanel->m_SelectTooltip.AddTooltipEntry(openScriptOptions);

		// Add option or creating a new script from this usage point
		EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
		{
				UNREFERENCED_PARAMETER(entry);
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

						// Get the active widget as its underlying widget type
						KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::InputTextWidget);
						RuntimeUI::InputTextWidget& activeInputTextWidget = *(RuntimeUI::InputTextWidget*)m_ActiveWidget;

						// Fill the new script handle
						activeInputTextWidget.m_OnMoveCursorHandle = scriptHandle;
						activeInputTextWidget.m_OnMoveCursor = Assets::AssetService::GetScript(scriptHandle);
						m_InputTextWidgetOnMoveCursor.m_CurrentOption = { script->m_ScriptName, scriptHandle };

						// Set the active editor UI as edited
						s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
					}, {});
				} };
		s_UIWindow->m_TreePanel->m_SelectTooltip.AddTooltipEntry(createScriptOptions);

		// Open tooltip
		s_UIWindow->m_TreePanel->m_SelectTooltip.m_TooltipActive = true;
	}

	void UIEditorPropertiesPanel::OnModifySliderWidgetBounds(EditorUI::EditVec2Spec& spec)
	{
		// Ensure active window is valid
		if (!ValidateActiveWindow())
		{
			return;
		}

		// Ensure we have the correct widget type
		KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::SliderWidget);

		// Get the underlying widget type
		RuntimeUI::SliderWidget* activeSlider = (RuntimeUI::SliderWidget*)m_ActiveWidget;

		// Update the widget value
		activeSlider->m_Bounds = spec.m_CurrentVec2;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifySliderWidgetSliderColor(EditorUI::EditVec4Spec& spec)
	{
		// Ensure active window is valid
		if (!ValidateActiveWindow())
		{
			return;
		}

		// Ensure we have the correct widget type
		KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::SliderWidget);

		// Get the underlying widget type
		RuntimeUI::SliderWidget* activeSlider = (RuntimeUI::SliderWidget*)m_ActiveWidget;

		// Update the widget value
		activeSlider->m_SliderColor = spec.m_CurrentVec4;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifySliderWidgetLineColor(EditorUI::EditVec4Spec& spec)
	{
		// Ensure active window is valid
		if (!ValidateActiveWindow())
		{
			return;
		}

		// Ensure we have the correct widget type
		KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::SliderWidget);

		// Get the underlying widget type
		RuntimeUI::SliderWidget* activeSlider = (RuntimeUI::SliderWidget*)m_ActiveWidget;

		// Update the widget value
		activeSlider->m_LineColor = spec.m_CurrentVec4;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifySliderWidgetOnMoveSlider(const EditorUI::OptionEntry& entry)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Ensure this is the correct widget type
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::SliderWidget)
		{
			KG_WARN("Invalid widget type provided when modifying widget");
			return;
		}

		// Get the underlying widget
		RuntimeUI::SliderWidget* activeSliderWidget = (RuntimeUI::SliderWidget*)m_ActiveWidget;

		// Clear the widget's script if the provided handle is empty
		if (entry.m_Handle == Assets::EmptyHandle)
		{
			activeSliderWidget->m_OnMoveSlider = nullptr;
			activeSliderWidget->m_OnMoveSliderHandle = Assets::EmptyHandle;

			// Set the active editor UI as edited
			s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
			return;
		}

		// Set the script for the widget
		activeSliderWidget->m_OnMoveSliderHandle = entry.m_Handle;
		activeSliderWidget->m_OnMoveSlider = Assets::AssetService::GetScript(entry.m_Handle);

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnOpenSliderWidgetOnMoveSliderPopup(EditorUI::SelectOptionSpec& spec)
	{
		// Clear existing options
		spec.ClearOptions();
		spec.AddToOptions("Clear", "None", Assets::EmptyHandle);

		// Add all compatible scripts to the select options
		for (auto& [handle, assetInfo] : Assets::AssetService::GetScriptRegistry())
		{
			// Get script from handle
			Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);

			// Ensure script is compatible with the text widget
			if (script->m_FuncType != WrappedFuncType::Void_Float)
			{
				continue;
			}

			// Add script to the select options
			spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
		}
	}

	void UIEditorPropertiesPanel::OnOpenTooltipForSliderWidgetOnMoveSlider(EditorUI::SelectOptionSpec& spec)
	{
		UNREFERENCED_PARAMETER(spec);
		// Clear existing options
		s_UIWindow->m_TreePanel->m_SelectTooltip.ClearEntries();

		// Add option to opening an existing script
		EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
		{
			UNREFERENCED_PARAMETER(entry);
			m_SliderWidgetOnMoveSlider.m_OpenPopup = true;
		} };
		s_UIWindow->m_TreePanel->m_SelectTooltip.AddTooltipEntry(openScriptOptions);

		// Add option or creating a new script from this usage point
		EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
		{
				UNREFERENCED_PARAMETER(entry);
				// Open create script dialog in script editor
				s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_Float, [&](Assets::AssetHandle scriptHandle)
				{
						// Ensure handle provides a script in the registry
						if (!Assets::AssetService::HasScript(scriptHandle))
						{
							KG_WARN("Could not find script");
							return;
						}

						// Ensure function type matches definition
						Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
						if (script->m_FuncType != WrappedFuncType::Void_Float)
						{
							KG_WARN("Incorrect function type returned when linking script to usage point");
							return;
						}

						// Get the active widget as its underlying widget type
						KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::SliderWidget);
						RuntimeUI::SliderWidget& activeSliderWidget = *(RuntimeUI::SliderWidget*)m_ActiveWidget;

						// Fill the new script handle
						activeSliderWidget.m_OnMoveSliderHandle = scriptHandle;
						activeSliderWidget.m_OnMoveSlider = Assets::AssetService::GetScript(scriptHandle);
						m_SliderWidgetOnMoveSlider.m_CurrentOption = { script->m_ScriptName, scriptHandle };

						// Set the active editor UI as edited
						s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
					}, {"sliderValue"});
				} };
		s_UIWindow->m_TreePanel->m_SelectTooltip.AddTooltipEntry(createScriptOptions);

		// Open tooltip
		s_UIWindow->m_TreePanel->m_SelectTooltip.m_TooltipActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyDropDownTextSize(EditorUI::EditFloatSpec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Ensure we have the correct widget type
		KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::DropDownWidget);

		// Get the underlying widget type
		RuntimeUI::DropDownWidget* activeDropDownWidget = (RuntimeUI::DropDownWidget*)m_ActiveWidget;
		KG_ASSERT(activeDropDownWidget);

		// Update the text size for all options
		for (RuntimeUI::SingleLineTextData& currentTextData : activeDropDownWidget->m_DropDownOptions)
		{
			currentTextData.m_TextSize = spec.m_CurrentFloat;
		}

		// Update the text widget text size based on the editorUI widget's value
		RuntimeUI::RuntimeUIService::RecalculateTextData( m_ActiveWidget);

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyDropDownTextColor(EditorUI::EditVec4Spec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Ensure we have the correct widget type
		KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::DropDownWidget);

		// Get the underlying widget type
		RuntimeUI::DropDownWidget* activeDropDownWidget = (RuntimeUI::DropDownWidget*)m_ActiveWidget;
		KG_ASSERT(activeDropDownWidget);

		// Update the text color for all options
		for (RuntimeUI::SingleLineTextData& currentTextData : activeDropDownWidget->m_DropDownOptions)
		{
			currentTextData.m_TextColor = spec.m_CurrentVec4;
		}

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyDropDownAlignment(const EditorUI::OptionEntry& entry)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Ensure we have the correct widget type
		KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::DropDownWidget);

		// Get the underlying widget type
		RuntimeUI::DropDownWidget* activeDropDownWidget = (RuntimeUI::DropDownWidget*)m_ActiveWidget;
		KG_ASSERT(activeDropDownWidget);

		// Update the text color for all options
		for (RuntimeUI::SingleLineTextData& currentTextData : activeDropDownWidget->m_DropDownOptions)
		{
			currentTextData.m_TextAlignment = (RuntimeUI::Constraint)(uint64_t)entry.m_Handle;
		}

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyDropDownWidgetOptionBackgroundColor(EditorUI::EditVec4Spec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Ensure we have the correct widget type
		KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::DropDownWidget);

		// Get the underlying widget type
		RuntimeUI::DropDownWidget* activeDropDownWidget = (RuntimeUI::DropDownWidget*)m_ActiveWidget;
		KG_ASSERT(activeDropDownWidget);

		// Update the widget's drop down background color
		activeDropDownWidget->m_DropDownBackground = spec.m_CurrentVec4;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyDropDownWidgetOnSelectOption(const EditorUI::OptionEntry& entry)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Ensure this is the correct widget type
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::DropDownWidget)
		{
			KG_WARN("Invalid widget type provided when modifying widget");
			return;
		}

		// Get the underlying widget
		RuntimeUI::DropDownWidget* activeDropDownWidget = (RuntimeUI::DropDownWidget*)m_ActiveWidget;

		// Clear the widget's script if the provided handle is empty
		if (entry.m_Handle == Assets::EmptyHandle)
		{
			activeDropDownWidget->m_OnSelectOption = nullptr;
			activeDropDownWidget->m_OnSelectOptionHandle = Assets::EmptyHandle;

			// Set the active editor UI as edited
			s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
			return;
		}

		// Set the script for the widget
		activeDropDownWidget->m_OnSelectOptionHandle = entry.m_Handle;
		activeDropDownWidget->m_OnSelectOption = Assets::AssetService::GetScript(entry.m_Handle);

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnOpenDropDownWidgetOnSelectOptionPopup(EditorUI::SelectOptionSpec& spec)
	{
		// Clear existing options
		spec.ClearOptions();
		spec.AddToOptions("Clear", "None", Assets::EmptyHandle);

		// Add all compatible scripts to the select options
		for (auto& [handle, assetInfo] : Assets::AssetService::GetScriptRegistry())
		{
			// Get script from handle
			Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);

			// Ensure script is compatible with the widget
			if (script->m_FuncType != WrappedFuncType::Void_String)
			{
				continue;
			}

			// Add script to the select options
			spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
		}
	}

	void UIEditorPropertiesPanel::OnOpenTooltipForDropDownWidgetOnSelectOption(EditorUI::SelectOptionSpec& spec)
	{
		UNREFERENCED_PARAMETER(spec);
		// Clear existing options
		s_UIWindow->m_TreePanel->m_SelectTooltip.ClearEntries();

		// Add option to opening an existing script
		EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
		{
			UNREFERENCED_PARAMETER(entry);
			m_DropDownWidgetOnSelectOption.m_OpenPopup = true;
		} };
		s_UIWindow->m_TreePanel->m_SelectTooltip.AddTooltipEntry(openScriptOptions);

		// Add option or creating a new script from this usage point
		EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
		{
				UNREFERENCED_PARAMETER(entry);
				// Open create script dialog in script editor
				s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_String, [&](Assets::AssetHandle scriptHandle)
				{
						// Ensure handle provides a script in the registry
						if (!Assets::AssetService::HasScript(scriptHandle))
						{
							KG_WARN("Could not find script");
							return;
						}

						// Ensure function type matches definition
						Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
						if (script->m_FuncType != WrappedFuncType::Void_String)
						{
							KG_WARN("Incorrect function type returned when linking script to usage point");
							return;
						}

						// Get the active widget as its underlying widget type
						KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::DropDownWidget);
						RuntimeUI::DropDownWidget& activeDropDownWidget = *(RuntimeUI::DropDownWidget*)m_ActiveWidget;

						// Fill the new script handle
						activeDropDownWidget.m_OnSelectOptionHandle = scriptHandle;
						activeDropDownWidget.m_OnSelectOption = Assets::AssetService::GetScript(scriptHandle);
						m_DropDownWidgetOnSelectOption.m_CurrentOption = { script->m_ScriptName, scriptHandle };

						// Set the active editor UI as edited
						s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
					}, {"selectedOption"});
				} };
		s_UIWindow->m_TreePanel->m_SelectTooltip.AddTooltipEntry(createScriptOptions);

		// Open tooltip
		s_UIWindow->m_TreePanel->m_SelectTooltip.m_TooltipActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyDropDownWidgetOpen(EditorUI::CheckboxSpec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Ensure we have the correct widget type
		KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::DropDownWidget);

		// Get the underlying widget type
		RuntimeUI::DropDownWidget* activeDropDownWidget = (RuntimeUI::DropDownWidget*)m_ActiveWidget;
		KG_ASSERT(activeDropDownWidget);

		// Update the widget's open boolean
		activeDropDownWidget->m_DropDownOpen = spec.m_CurrentBoolean;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyDropDownWidgetCurrentOption(const EditorUI::OptionEntry& entry)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Ensure this is the correct widget type
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::DropDownWidget)
		{
			KG_WARN("Invalid widget type provided when modifying widget");
			return;
		}

		// Get the underlying widget
		RuntimeUI::DropDownWidget* activeDropDownWidget = (RuntimeUI::DropDownWidget*)m_ActiveWidget;

		// Ensure the widget is valid and is within correct bounds
		KG_ASSERT(activeDropDownWidget);
		KG_ASSERT(entry.m_Handle < activeDropDownWidget->m_DropDownOptions.size());

		// Modify the current option
		activeDropDownWidget->m_CurrentOption = entry.m_Handle;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnOpenDropDownWidgetCurrentOptionPopup(EditorUI::SelectOptionSpec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Ensure this is the correct widget type
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::DropDownWidget)
		{
			KG_WARN("Invalid widget type provided when modifying widget");
			return;
		}

		// Get the underlying widget
		RuntimeUI::DropDownWidget* activeDropDownWidget = (RuntimeUI::DropDownWidget*)m_ActiveWidget;
		KG_ASSERT(activeDropDownWidget);

		// Clear existing options
		spec.ClearOptions();

		// Add all compatible scripts to the select options
		size_t iteration{ 0 };
		for (const RuntimeUI::SingleLineTextData& textData : activeDropDownWidget->m_DropDownOptions)
		{

			// Add script to the select options
			spec.AddToOptions("All Options", textData.m_Text, iteration);
			iteration++;
		}
	}

	void UIEditorPropertiesPanel::OnRefreshDropDownWidgetOptionsList()
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Ensure we have the correct widget type
		KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::DropDownWidget);

		// Get the underlying widget type
		RuntimeUI::DropDownWidget* activeDropDownWidget = (RuntimeUI::DropDownWidget*)m_ActiveWidget;
		KG_ASSERT(activeDropDownWidget);

		// Ensure the list starts off as empty
		m_DropDownWidgetOptionsList.ClearList();

		// Add all the options to the list
		size_t iteration{ 0 };
		for (RuntimeUI::SingleLineTextData& textData : activeDropDownWidget->m_DropDownOptions)
		{
			m_DropDownWidgetOptionsList.InsertListEntry(textData.m_Text, "", KG_BIND_CLASS_FN(OnDropDownWidgetEditEntryTooltip), iteration);
			iteration++;
		}
	}

	void UIEditorPropertiesPanel::OnDropDownWidgetAddEntryDialog()
	{
		// Open the add entry dialog
		m_DropDownWidgetOptionsListAddEntry.m_CurrentOption = "New Entry";
		m_DropDownWidgetOptionsListAddEntry.m_StartPopup = true;
		
	}

	void UIEditorPropertiesPanel::OnDropDownWidgetEditEntryTooltip(EditorUI::ListEntry& entry, size_t iteration)
	{
		UNREFERENCED_PARAMETER(entry);
		UNREFERENCED_PARAMETER(iteration);
		// Clear existing options
		s_UIWindow->m_TreePanel->m_SelectTooltip.ClearEntries();

		// Add option to opening an existing script
		EditorUI::TooltipEntry editOptionTooltip{ "Edit Option", KG_BIND_CLASS_FN(OnDropDownWidgetEditEntryDialog) };
		editOptionTooltip.m_UserHandle = iteration;
		s_UIWindow->m_TreePanel->m_SelectTooltip.AddTooltipEntry(editOptionTooltip);

		// Add option to opening an existing script
		EditorUI::TooltipEntry deleteOptionTooltip{ "Delete Option", 
			KG_BIND_CLASS_FN(OnDropDownWidgetDeleteEntryDialog) };
		deleteOptionTooltip.m_UserHandle = iteration;
		s_UIWindow->m_TreePanel->m_SelectTooltip.AddTooltipEntry(deleteOptionTooltip);

		// Open tooltip
		s_UIWindow->m_TreePanel->m_SelectTooltip.m_TooltipActive = true;
	}

	void UIEditorPropertiesPanel::OnDropDownWidgetEditEntryDialog(EditorUI::TooltipEntry& entry)
	{
		// TODO: This hurts me. Need some refactoring here

		// Open an on-edit popup
		m_DropDownWidgetEditEntry.m_StartPopup = true;

		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Ensure this is the correct widget type
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::DropDownWidget)
		{
			KG_WARN("Invalid widget type provided when modifying widget");
			return;
		}

		// Get the underlying widget and optionsList
		RuntimeUI::DropDownWidget* activeDropDownWidget = (RuntimeUI::DropDownWidget*)m_ActiveWidget;
		KG_ASSERT(activeDropDownWidget);
		std::vector<RuntimeUI::SingleLineTextData>& optionsList = activeDropDownWidget->m_DropDownOptions;

		// Ensure that the index is valid
		if (m_ActiveDropDownOption >= optionsList.size())
		{
			KG_WARN("Active index is out of bounds for the drop-down widget's options");
			return;
		}
		// Modify the name to match the current entry
		RuntimeUI::SingleLineTextData& textData = optionsList.at(m_ActiveDropDownOption);
		m_DropDownWidgetEditEntry.m_CurrentOption = textData.m_Text;
		//textData.m_Text = spec.m_CurrentOption;

		// Store the indicated entry
		m_ActiveDropDownOption = entry.m_UserHandle;
	}

	void UIEditorPropertiesPanel::OnDropDownWidgetDeleteEntryDialog(EditorUI::TooltipEntry& entry)
	{
		// Open an on-delete popup
		m_DropDownWidgetDeleteEntryWarning.m_OpenPopup = true;

		// Store the indicated entry
		m_ActiveDropDownOption = entry.m_UserHandle;
	}

	void UIEditorPropertiesPanel::OnDropDownWidgetAddEntry(EditorUI::EditTextSpec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Ensure this is the correct widget type
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::DropDownWidget)
		{
			KG_WARN("Invalid widget type provided when modifying widget");
			return;
		}

		// Get the underlying widget
		RuntimeUI::DropDownWidget* activeDropDownWidget = (RuntimeUI::DropDownWidget*)m_ActiveWidget;
		// Add the entry to the underlying runtime UI widget
		RuntimeUI::SingleLineTextData& newDropDown = activeDropDownWidget->m_DropDownOptions.emplace_back();
		newDropDown.m_Text = spec.m_CurrentOption;

		// Revalidate the text data
		RuntimeUI::RuntimeUIService::RecalculateTextData( m_ActiveWidget);

		// Refresh the table
		KG_ASSERT(m_DropDownWidgetOptionsList.m_OnRefresh);
		m_DropDownWidgetOptionsList.m_OnRefresh();

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnDropDownWidgetEditEntry(EditorUI::EditTextSpec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Ensure this is the correct widget type
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::DropDownWidget)
		{
			KG_WARN("Invalid widget type provided when modifying widget");
			return;
		}

		// Get the underlying widget and optionsList
		RuntimeUI::DropDownWidget* activeDropDownWidget = (RuntimeUI::DropDownWidget*)m_ActiveWidget;
		KG_ASSERT(activeDropDownWidget);
		std::vector<RuntimeUI::SingleLineTextData>& optionsList = activeDropDownWidget->m_DropDownOptions;

		// Ensure that the index is valid
		if (m_ActiveDropDownOption >= optionsList.size())
		{
			KG_WARN("Active index is out of bounds for the drop-down widget's options");
			return;
		}

		// Get the indicated single line text
		RuntimeUI::SingleLineTextData& textData = optionsList.at(m_ActiveDropDownOption);
		textData.m_Text = spec.m_CurrentOption;

		// Revalidate the text data
		RuntimeUI::RuntimeUIService::RecalculateTextData( m_ActiveWidget);

		// Refresh the table
		KG_ASSERT(m_DropDownWidgetOptionsList.m_OnRefresh);
		m_DropDownWidgetOptionsList.m_OnRefresh();

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnDropDownWidgetDeleteEntry()
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Ensure this is the correct widget type
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::DropDownWidget)
		{
			KG_WARN("Invalid widget type provided when modifying widget");
			return;
		}

		// Get the underlying widget and optionsList
		RuntimeUI::DropDownWidget* activeDropDownWidget = (RuntimeUI::DropDownWidget*)m_ActiveWidget;
		KG_ASSERT(activeDropDownWidget);
		std::vector<RuntimeUI::SingleLineTextData>& optionsList = activeDropDownWidget->m_DropDownOptions;

		// Ensure that the index is valid
		if (m_ActiveDropDownOption >= optionsList.size())
		{
			KG_WARN("Active index is out of bounds for the drop-down widget's options");
			return;
		}

		// Delete the indicated entry
		optionsList.erase(optionsList.begin() + m_ActiveDropDownOption);

		// Revalidate the text data
		RuntimeUI::RuntimeUIService::RecalculateTextData( m_ActiveWidget);

		// Revalidate the current option
		if (activeDropDownWidget->m_CurrentOption >= optionsList.size())
		{
			activeDropDownWidget->m_CurrentOption = 0;
		}

		// Refresh the table
		KG_ASSERT(m_DropDownWidgetOptionsList.m_OnRefresh);
		m_DropDownWidgetOptionsList.m_OnRefresh();

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	
	void UIEditorPropertiesPanel::OnModifySingleLineDataText(EditorUI::EditTextSpec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Get the widget's text data
		RuntimeUI::SingleLineTextData* textData = RuntimeUI::RuntimeUIService::GetSingleLineTextDataFromWidget(m_ActiveWidget);
		if (!textData)
		{
			KG_WARN("Attempt to modify widget's text data, but none could be found.");
			return;
		}

		// Update text data and recalculate text metrics
		textData->m_Text = spec.m_CurrentOption;
		RuntimeUI::RuntimeUIService::RecalculateTextData( m_ActiveWidget);

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnModifyTextDataTextSize(EditorUI::EditFloatSpec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Get the widget's text data
		RuntimeUI::SingleLineTextData* singleLineData = RuntimeUI::RuntimeUIService::GetSingleLineTextDataFromWidget(m_ActiveWidget);
		if (singleLineData)
		{
			// Update the text widget text size based on the editorUI widget's value
			singleLineData->m_TextSize = spec.m_CurrentFloat;
			RuntimeUI::RuntimeUIService::RecalculateTextData( m_ActiveWidget);

			// Set the active editor UI as edited
			s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
			return;
		}

		// Get the widget's text data
		RuntimeUI::MultiLineTextData* multiLineData = RuntimeUI::RuntimeUIService::GetMultiLineTextDataFromWidget(m_ActiveWidget);
		if (multiLineData)
		{
			// Update the text widget text size based on the editorUI widget's value
			multiLineData->m_TextSize = spec.m_CurrentFloat;
			RuntimeUI::RuntimeUIService::RecalculateTextData(m_ActiveWidget);

			// Set the active editor UI as edited
			s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
			return;
		}

		KG_WARN("Attempt to modify widget's text data, but none could be found.");
	}
	void UIEditorPropertiesPanel::OnModifyTextDataTextColor(EditorUI::EditVec4Spec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Get the widget's text data
		RuntimeUI::SingleLineTextData* singleLineData = RuntimeUI::RuntimeUIService::GetSingleLineTextDataFromWidget(m_ActiveWidget);
		if (singleLineData)
		{
			// Update the text widget text size based on the editorUI widget's value
			singleLineData->m_TextColor = spec.m_CurrentVec4;

			// Set the active editor UI as edited
			s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
			return;
		}

		// Get the widget's text data
		RuntimeUI::MultiLineTextData* multiLineData = RuntimeUI::RuntimeUIService::GetMultiLineTextDataFromWidget(m_ActiveWidget);
		if (multiLineData)
		{
			// Update the text widget text size based on the editorUI widget's value
			multiLineData->m_TextColor = spec.m_CurrentVec4;

			// Set the active editor UI as edited
			s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
			return;
		}

		KG_WARN("Attempt to modify widget's text data, but none could be found.");
	}
	void UIEditorPropertiesPanel::OnModifyTextDataAlignment(const EditorUI::OptionEntry& entry)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Get the widget's text data
		RuntimeUI::SingleLineTextData* singleLineData = RuntimeUI::RuntimeUIService::GetSingleLineTextDataFromWidget(m_ActiveWidget);
		if (singleLineData)
		{
			// Update the text widget text size based on the editorUI widget's value
			singleLineData->m_TextAlignment = (RuntimeUI::Constraint)(uint64_t)entry.m_Handle;

			// Set the active editor UI as edited
			s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
			return;
		}

		// Get the widget's text data
		RuntimeUI::MultiLineTextData* multiLineData = RuntimeUI::RuntimeUIService::GetMultiLineTextDataFromWidget(m_ActiveWidget);
		if (multiLineData)
		{
			// Update the text widget text size based on the editorUI widget's value
			multiLineData->m_TextAlignment = (RuntimeUI::Constraint)(uint64_t)entry.m_Handle;

			// Set the active editor UI as edited
			s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
			return;
		}

		KG_WARN("Attempt to modify widget's text data, but none could be found.");


	}
	void UIEditorPropertiesPanel::OnOpenTextDataAlignmentPopup(EditorUI::SelectOptionSpec& spec)
	{
		// Clear existing options
		spec.ClearOptions();

		// Add left, right, and center alignment options
		spec.AddToOptions
		(
			"All Options",
			Utility::ConstraintToString(RuntimeUI::Constraint::Left),
			(uint64_t)RuntimeUI::Constraint::Left
		);

		spec.AddToOptions
		(
			"All Options",
			Utility::ConstraintToString(RuntimeUI::Constraint::Right),
			(uint64_t)RuntimeUI::Constraint::Right
		);
		spec.AddToOptions
		(
			"All Options",
			Utility::ConstraintToString(RuntimeUI::Constraint::Center),
			(uint64_t)RuntimeUI::Constraint::Center
		);
	}
	void UIEditorPropertiesPanel::OnModifyMultiLineDataText(EditorUI::EditMultiLineTextSpec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Get the widget's text data
		RuntimeUI::MultiLineTextData* textData = RuntimeUI::RuntimeUIService::GetMultiLineTextDataFromWidget(m_ActiveWidget);
		if (!textData)
		{
			KG_WARN("Attempt to modify widget's text data, but none could be found.");
			return;
		}

		// Update text data and recalculate text metrics
		textData->m_Text = spec.m_CurrentOption;
		RuntimeUI::RuntimeUIService::RecalculateTextData(m_ActiveWidget);

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnModifyTextDataWrapped(EditorUI::CheckboxSpec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Get the widget's text data
		RuntimeUI::MultiLineTextData* textData = RuntimeUI::RuntimeUIService::GetMultiLineTextDataFromWidget(m_ActiveWidget);
		if (!textData)
		{
			KG_WARN("Attempt to modify widget's text data, but none could be found.");
			return;
		}

		// Update text data and recalculate text metrics
		textData->m_TextWrapped = spec.m_CurrentBoolean;
		RuntimeUI::RuntimeUIService::RecalculateTextData(m_ActiveWidget);

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnModifySelectionDataSelectable(EditorUI::CheckboxSpec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Get the widget's selection data
		RuntimeUI::SelectionData* selectionData = RuntimeUI::RuntimeUIService::GetSelectionDataFromWidget(m_ActiveWidget);
		if (!selectionData)
		{
			KG_WARN("Attempt to modify widget's selection data, but none could be found.");
			return;
		}

		// Update the text widget text alignment based on the editorUI widget's value
		selectionData->m_Selectable = spec.m_CurrentBoolean;

		// Calculate navigation links
		RuntimeUI::NavigationLinksCalculator newCalculator;
		newCalculator.CalculateNavigationLinks(RuntimeUI::RuntimeUIService::GetActiveUI(),
			EngineService::GetActiveWindow().GetActiveViewport());

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnModifySelectionDataOnPress(const EditorUI::OptionEntry& entry)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Get the widget's selection data
		RuntimeUI::SelectionData* selectionData = RuntimeUI::RuntimeUIService::GetSelectionDataFromWidget(m_ActiveWidget);
		if (!selectionData)
		{
			KG_WARN("Attempt to modify widget's selection data, but none could be found.");
			return;
		}

		// Clear the on press script if the provided handle is empty
		if (entry.m_Handle == Assets::EmptyHandle)
		{
			selectionData->m_FunctionPointers.m_OnPress = nullptr;
			selectionData->m_FunctionPointers.m_OnPressHandle = Assets::EmptyHandle;

			// Set the active editor UI as edited
			s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
			return;
		}

		// Set the on press script for the text widget
		selectionData->m_FunctionPointers.m_OnPressHandle = entry.m_Handle;
		selectionData->m_FunctionPointers.m_OnPress = Assets::AssetService::GetScript(entry.m_Handle);

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnOpenSelectionDataOnPressPopup(EditorUI::SelectOptionSpec& spec)
	{
		// Get the current func type
		WrappedFuncType currentFuncType{ *(WrappedFuncType*)spec.m_ProvidedData.get() };
		// Clear existing options
		spec.ClearOptions();
		spec.AddToOptions("Clear", "None", Assets::EmptyHandle);

		// Add all compatible scripts to the select options
		for (auto& [handle, assetInfo] : Assets::AssetService::GetScriptRegistry())
		{
			// Get script from handle
			Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);

			// Ensure script is compatible with the text widget
			if (script->m_FuncType != currentFuncType)
			{
				continue;
			}

			// Add script to the select options
			spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
		}
	}
	void UIEditorPropertiesPanel::OnOpenTooltipForSelectionDataOnPress(EditorUI::SelectOptionSpec& spec)
	{
		// Store the current select option spec, function type, and parameter names
		static EditorUI::SelectOptionSpec* s_CurrentSpec{ nullptr };
		static WrappedFuncType s_CurrentFuncType{ WrappedFuncType::None };
		static std::vector<FixedString32> s_ParameterNames;
		s_CurrentSpec = &spec;
		s_CurrentFuncType = *(WrappedFuncType*)s_CurrentSpec->m_ProvidedData.get();

		switch (s_CurrentFuncType)
		{
		case WrappedFuncType::Void_None:
			s_ParameterNames = {};
			break;
		case WrappedFuncType::Void_Bool:
			s_ParameterNames = { "isChecked" };
			break;
		default:
			KG_ERROR("Invalid function type provided!");
		}

		// Clear existing options
		s_UIWindow->m_TreePanel->m_SelectTooltip.ClearEntries();

		// Add option to opening an existing script
		EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
		{
			UNREFERENCED_PARAMETER(entry);
			s_CurrentSpec->m_OpenPopup = true;
		} };
		s_UIWindow->m_TreePanel->m_SelectTooltip.AddTooltipEntry(openScriptOptions);

		// Add option or creating a new script from this usage point
		EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
		{
			UNREFERENCED_PARAMETER(entry);
			// Open create script dialog in script editor
			s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(
				s_CurrentFuncType, [&](Assets::AssetHandle scriptHandle)
			{
					// Ensure handle provides a script in the registry
					if (!Assets::AssetService::HasScript(scriptHandle))
					{
						KG_WARN("Could not find script");
						return;
					}

					// Ensure function type matches definition
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
					if (script->m_FuncType != s_CurrentFuncType)
					{
						KG_WARN("Incorrect function type returned when linking script to usage point");
						return;
					}

					// Get the selection data associated with the active widget
					RuntimeUI::SelectionData* selectionData = RuntimeUI::RuntimeUIService::GetSelectionDataFromWidget(m_ActiveWidget);
					KG_ASSERT(selectionData);


					// Fill the new script handle
					selectionData->m_FunctionPointers.m_OnPressHandle = scriptHandle;
					selectionData->m_FunctionPointers.m_OnPress = script;
					s_CurrentSpec->m_CurrentOption = { script->m_ScriptName, scriptHandle };

					// Set the active editor UI as edited
					s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
				}, s_ParameterNames);
			} };
		s_UIWindow->m_TreePanel->m_SelectTooltip.AddTooltipEntry(createScriptOptions);

		// Open tooltip
		s_UIWindow->m_TreePanel->m_SelectTooltip.m_TooltipActive = true;
	}
	void UIEditorPropertiesPanel::OnModifySelectionDataBackgroundColor(EditorUI::EditVec4Spec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Get the widget's selection data
		RuntimeUI::SelectionData* selectionData = RuntimeUI::RuntimeUIService::GetSelectionDataFromWidget(m_ActiveWidget);
		if (!selectionData)
		{
			KG_WARN("Attempt to modify widget's selection data, but none could be found.");
			return;
		}

		// Update the widget background color based on the editorUI widget value
		selectionData->m_DefaultBackgroundColor = spec.m_CurrentVec4;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnModifyImageDataImage(const EditorUI::OptionEntry& entry)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Get the widget's image data
		RuntimeUI::ImageData* imageData = RuntimeUI::RuntimeUIService::GetImageDataFromWidget(m_ActiveWidget);
		if (!imageData)
		{
			KG_WARN("Attempt to modify widget's image data, but none could be found.");
			return;
		}

		// Clear the on press script if the provided handle is empty
		if (entry.m_Handle == Assets::EmptyHandle)
		{
			imageData->m_ImageRef = nullptr;
			imageData->m_ImageHandle = Assets::EmptyHandle;

			// Set the active editor UI as edited
			s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
			return;
		}

		// Set the texture reference for the image widget
		imageData->m_ImageRef = Assets::AssetService::GetTexture2D(entry.m_Handle);
		imageData->m_ImageHandle = entry.m_Handle;

		if (imageData->m_FixedAspectRatio)
		{
			ViewportData& currentViewport = s_UIWindow->m_ViewportPanel->m_ViewportData;
			RuntimeUI::RuntimeUIService::CalculateFixedAspectRatioSize
			(
				m_ActiveWidget,
				currentViewport.m_Width,
				currentViewport.m_Height,
				true
			);
		}

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnOpenImageDataImagePopup(EditorUI::SelectOptionSpec& spec)
	{
		// Clear existing options
		spec.ClearOptions();
		spec.AddToOptions("Clear", "None", Assets::EmptyHandle);

		// Add all compatible textures to the select options
		for (auto& [handle, assetInfo] : Assets::AssetService::GetTexture2DRegistry())
		{
			// Get texture from handle
			Ref<Rendering::Texture2D> script = Assets::AssetService::GetTexture2D(handle);

			// Add texture to the select options
			spec.AddToOptions("All Options", assetInfo.Data.FileLocation.stem().string(), handle);
		}
	}
	void UIEditorPropertiesPanel::OnModifyImageDataFixedAspectRatio(EditorUI::CheckboxSpec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Get the widget's image data
		RuntimeUI::ImageData* imageData = RuntimeUI::RuntimeUIService::GetImageDataFromWidget(m_ActiveWidget);
		if (!imageData)
		{
			KG_WARN("Attempt to modify widget's image data, but none could be found.");
			return;
		}

		// Update the text widget text alignment based on the editorUI widget's value
		imageData->m_FixedAspectRatio = spec.m_CurrentBoolean;

		// Calculate navigation links
		RuntimeUI::NavigationLinksCalculator newCalculator;
		newCalculator.CalculateNavigationLinks(RuntimeUI::RuntimeUIService::GetActiveUI(),
			EngineService::GetActiveWindow().GetActiveViewport());
		if (spec.m_CurrentBoolean)
		{
			ViewportData& currentViewport = s_UIWindow->m_ViewportPanel->m_ViewportData;
			RuntimeUI::RuntimeUIService::CalculateFixedAspectRatioSize
			(
				m_ActiveWidget,
				currentViewport.m_Width,
				currentViewport.m_Height,
				true
			);
		}

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnModifyContainerDataBackgroundColor(EditorUI::EditVec4Spec& spec)
	{
		// Ensure active window and widget are valid
		if (!ValidateActiveWindowAndWidget())
		{
			return;
		}

		// Get the widget's container data
		RuntimeUI::ContainerData* containerData = RuntimeUI::RuntimeUIService::GetContainerDataFromWidget(m_ActiveWidget);
		if (!containerData)
		{
			KG_WARN("Attempt to modify widget's container data, but none could be found.");
			return;
		}

		// Update the widget background color based on the editorUI widget value
		containerData->m_BackgroundColor = spec.m_CurrentVec4;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
}

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

			// Edit UI's selection color
			m_UISelectionColor.m_CurrentVec4 = s_UIWindow->m_EditorUI->m_SelectColor;
			EditorUI::EditorUIService::EditVec4(m_UISelectionColor);
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

	void UIEditorPropertiesPanel::DrawTextWidgetOptions()
	{
		// Draw main header for text widget options
		EditorUI::EditorUIService::CollapsingHeader(m_TextWidgetHeader);
		if (m_TextWidgetHeader.m_Expanded)
		{
			// Draw options to edit selected text widget
			RuntimeUI::TextWidget& activeTextWidget = *(RuntimeUI::TextWidget*)m_ActiveWidget;

			// Edit selected text widget's text
			m_TextWidgetText.m_CurrentOption = activeTextWidget.m_Text;
			EditorUI::EditorUIService::EditMultiLineText(m_TextWidgetText);

			// Edit selected text widget's text size relative to its window
			m_TextWidgetTextSize.m_CurrentFloat = activeTextWidget.m_TextSize;
			EditorUI::EditorUIService::EditFloat(m_TextWidgetTextSize);

			// Edit selected text widget's text color
			m_TextWidgetTextColor.m_CurrentVec4 = activeTextWidget.m_TextColor;
			EditorUI::EditorUIService::EditVec4(m_TextWidgetTextColor);

			// Edit selected text widget's text alignment
			m_TextWidgetTextAlignment.m_CurrentOption = { Utility::ConstraintToString(activeTextWidget.m_TextAlignment) , (uint64_t)activeTextWidget.m_TextAlignment };
			EditorUI::EditorUIService::SelectOption(m_TextWidgetTextAlignment);

			// Edit selected text widget's wrapped alignment
			m_TextWidgetTextWrapped.m_CurrentBoolean = activeTextWidget.m_TextWrapped;
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
			m_ButtonWidgetText.m_CurrentOption = activeButtonWidget.m_Text;
			EditorUI::EditorUIService::EditText(m_ButtonWidgetText);

			// Edit selected button widget's text size relative to its window
			m_ButtonWidgetTextSize.m_CurrentFloat = activeButtonWidget.m_TextSize;
			EditorUI::EditorUIService::EditFloat(m_ButtonWidgetTextSize);

			// Edit selected button widget's text color
			m_ButtonWidgetTextColor.m_CurrentVec4 = activeButtonWidget.m_TextColor;
			EditorUI::EditorUIService::EditVec4(m_ButtonWidgetTextColor);

			// Edit selected button widget's text alignment
			m_ButtonWidgetTextAlignment.m_CurrentOption = { Utility::ConstraintToString(activeButtonWidget.m_TextAlignment) , (uint64_t)activeButtonWidget.m_TextAlignment };
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
			Assets::AssetHandle imageHandle = activeImageWidget.m_ImageHandle;
			m_ImageWidgetImage.m_CurrentOption =
			{
				imageHandle == Assets::EmptyHandle ? "None" : 
				Assets::AssetService::GetTexture2DInfo(imageHandle).Data.FileLocation.stem().string(),
				imageHandle
			};
			EditorUI::EditorUIService::SelectOption(m_ImageWidgetImage);

			// Edit selected widget's fixed aspect ratio usage
			m_ImageWidgetFixedAspectRatio.m_CurrentBoolean = activeImageWidget.m_FixedAspectRatio;
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
			Assets::AssetHandle imageHandle = activeImageButtonWidget.m_ImageHandle;
			m_ImageButtonWidgetImage.m_CurrentOption =
			{
				imageHandle == Assets::EmptyHandle ? "None" :
				Assets::AssetService::GetTexture2DInfo(imageHandle).Data.FileLocation.stem().string(),
				imageHandle
			};
			EditorUI::EditorUIService::SelectOption(m_ImageButtonWidgetImage);

			// Edit selected widget's fixed aspect ratio usage
			m_ImageButtonWidgetFixedAspectRatio.m_CurrentBoolean = activeImageButtonWidget.m_FixedAspectRatio;
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
		default:
			KG_ERROR("Invalid widget type attempted to be drawn");
			break;
		}
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

	void UIEditorPropertiesPanel::OnOpenUIFontPopup()
	{
		// Clear existing options
		m_UISelectFont.ClearOptions();
		m_UISelectFont.AddToOptions("Clear", "None", Assets::EmptyHandle);

		// Add all font options
		for (auto& [fontHandle, fontInfo] : Assets::AssetService::GetFontRegistry())
		{
			m_UISelectFont.AddToOptions("All Options", fontInfo.Data.FileLocation.stem().string(), fontHandle);
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

	void UIEditorPropertiesPanel::OnOpenUIOnMovePopup()
	{
		// Clear existing options
		m_UIOnMove.ClearOptions();
		m_UIOnMove.AddToOptions("Clear", "None", Assets::EmptyHandle);

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
			m_UIOnMove.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
		}
	}

	void UIEditorPropertiesPanel::OnOpenTooltipForUIOnMove()
	{
		// Clear existing options
		s_UIWindow->m_TreePanel->m_SelectScriptTooltip.ClearEntries();

		// Add option to opening an existing script
		EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
		{
			m_UIOnMove.m_OpenPopup = true;
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

					// Set the on move script for the UI and editor
					RuntimeUI::RuntimeUIService::SetActiveOnMove(scriptHandle, script);
					m_UIOnMove.m_CurrentOption = { script->m_ScriptName, scriptHandle };

					// Set the active editor UI as edited
					s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
				}, {});
			} 
		};
		s_UIWindow->m_TreePanel->m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

		// Open tooltip
		s_UIWindow->m_TreePanel->m_SelectScriptTooltip.m_TooltipActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyUISelectionColor(EditorUI::EditVec4Spec& spec)
	{
		// Update the UI's selection color
		s_UIWindow->m_EditorUI->m_SelectColor = spec.m_CurrentVec4;
		RuntimeUI::RuntimeUIService::SetSelectedWidgetColor(spec.m_CurrentVec4);

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
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

		// Set up widget to modify the UI's selection background color
		m_UISelectionColor.m_Label = "Selection Color";
		m_UISelectionColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_UISelectionColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyUISelectionColor);

		
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
		m_WindowSize.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyWindowSize);

		// Set up widget to modify the window's background color
		m_WindowBackgroundColor.m_Label = "Background Color";
		m_WindowBackgroundColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
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
		m_WidgetPixelOrPercentSize.m_Label = "Sizing Mode";
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
		m_TextWidgetText.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyTextWidgetText);

		// Set up widget to modify the text widget's text size
		m_TextWidgetTextSize.m_Label = "Text Size";
		m_TextWidgetTextSize.m_Flags |= EditorUI::EditFloat_Indented;
		m_TextWidgetTextSize.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyTextWidgetTextSize);

		// Set up widget to modify the text widget's text color
		m_TextWidgetTextColor.m_Label = "Text Color";
		m_TextWidgetTextColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_TextWidgetTextColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyTextWidgetTextColor);

		// Set up widget to modify the text widget's text alignment
		m_TextWidgetTextAlignment.m_Label = "Text Alignment";
		m_TextWidgetTextAlignment.m_Flags |= EditorUI::SelectOption_Indented;
		m_TextWidgetTextAlignment.m_PopupAction = KG_BIND_CLASS_FN(OnOpenTextWidgetAlignmentPopup);
		m_TextWidgetTextAlignment.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyTextWidgetAlignment);

		// Set up widget to modify if the text widget is wrapped
		m_TextWidgetTextWrapped.m_Label = "Text Wrapped";
		m_TextWidgetTextWrapped.m_Flags |= EditorUI::Checkbox_Indented;
		m_TextWidgetTextWrapped.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyTextWidgetWrapped);
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
		m_ButtonWidgetSelectable.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyButtonWidgetSelectable);

		// Set up widget to modify the button widget's text
		m_ButtonWidgetText.m_Label = "Text";
		m_ButtonWidgetText.m_Flags |= EditorUI::EditText_Indented;
		m_ButtonWidgetText.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyButtonWidgetText);

		// Set up widget to modify the button widget's text size
		m_ButtonWidgetTextSize.m_Label = "Text Size";
		m_ButtonWidgetTextSize.m_Flags |= EditorUI::EditFloat_Indented;
		m_ButtonWidgetTextSize.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyButtonWidgetButtonTextSize);

		// Set up widget to modify the button widget's text color
		m_ButtonWidgetTextColor.m_Label = "Text Color";
		m_ButtonWidgetTextColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_ButtonWidgetTextColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyButtonWidgetButtonTextColor);

		// Set up widget to modify the button widget's text alignment
		m_ButtonWidgetTextAlignment.m_Label = "Text Alignment";
		m_ButtonWidgetTextAlignment.m_Flags |= EditorUI::SelectOption_Indented;
		m_ButtonWidgetTextAlignment.m_PopupAction = KG_BIND_CLASS_FN(OnOpenButtonWidgetTextAlignmentPopup);
		m_ButtonWidgetTextAlignment.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyButtonWidgetTextAlignment);

		// Set up widget to modify the button widget's background color
		m_ButtonWidgetBackgroundColor.m_Label = "Background Color";
		m_ButtonWidgetBackgroundColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_ButtonWidgetBackgroundColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyButtonWidgetBackgroundColor);

		// Set up widget to modify the button widget's on press script
		m_ButtonWidgetOnPress.m_Label = "On Press";
		m_ButtonWidgetOnPress.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_ButtonWidgetOnPress.m_PopupAction = KG_BIND_CLASS_FN(OnOpenButtonWidgetOnPressPopup);
		m_ButtonWidgetOnPress.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyButtonWidgetOnPress);
		m_ButtonWidgetOnPress.m_OnEdit = KG_BIND_CLASS_FN(OnOpenTooltipForButtonWidgetOnPress);
	}

	void UIEditorPropertiesPanel::InitializeImageWidgetOptions()
	{
		// Set up header for button widget options
		m_ImageWidgetHeader.m_Label = "Button Widget Options";
		m_ImageWidgetHeader.m_Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		m_ImageWidgetHeader.m_Expanded = true;

		// Set up widget to modify the image widget's image
		m_ImageWidgetImage.m_Label = "Image";
		m_ImageWidgetImage.m_Flags |= EditorUI::SelectOption_Indented;
		m_ImageWidgetImage.m_PopupAction = KG_BIND_CLASS_FN(OnOpenImageWidgetImagePopup);
		m_ImageWidgetImage.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyImageWidgetImage);

		// Set up widget to modify the image widget's fixed aspect ratio display
		m_ImageWidgetFixedAspectRatio.m_Label = "Fixed Aspect Ratio";
		m_ImageWidgetFixedAspectRatio.m_Flags |= EditorUI::Checkbox_Indented;
		m_ImageWidgetFixedAspectRatio.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyImageWidgetFixedAspectRatio);
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
		m_ImageButtonWidgetImage.m_PopupAction = KG_BIND_CLASS_FN(OnOpenImageButtonWidgetImagePopup);
		m_ImageButtonWidgetImage.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyImageButtonWidgetImage);

		// Set up widget to modify the image widget's fixed aspect ratio display
		m_ImageButtonWidgetFixedAspectRatio.m_Label = "Fixed Aspect Ratio";
		m_ImageButtonWidgetFixedAspectRatio.m_Flags |= EditorUI::Checkbox_Indented;
		m_ImageButtonWidgetFixedAspectRatio.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyImageButtonWidgetFixedAspectRatio);

		// Set up widget to modify the text widget's text alignment
		m_ImageButtonWidgetSelectable.m_Label = "Selectable";
		m_ImageButtonWidgetSelectable.m_Flags |= EditorUI::Checkbox_Indented;
		m_ImageButtonWidgetSelectable.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyImageButtonWidgetSelectable);

		// Set up widget to modify the button widget's background color
		m_ImageButtonWidgetBackgroundColor.m_Label = "Background Color";
		m_ImageButtonWidgetBackgroundColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_ImageButtonWidgetBackgroundColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyImageButtonWidgetBackgroundColor);

		// Set up widget to modify the button widget's on press script
		m_ImageButtonWidgetOnPress.m_Label = "On Press";
		m_ImageButtonWidgetOnPress.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_ImageButtonWidgetOnPress.m_PopupAction = KG_BIND_CLASS_FN(OnOpenImageButtonWidgetOnPressPopup);
		m_ImageButtonWidgetOnPress.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyImageButtonWidgetOnPress);
		m_ImageButtonWidgetOnPress.m_OnEdit = KG_BIND_CLASS_FN(OnOpenTooltipForImageButtonWidgetOnPress);

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
			case RuntimeUI::WidgetTypes::ButtonWidget:
				m_WindowDefaultWidget.AddToOptions("Button Widget", widget->m_Tag, iteration);
				break;
			case RuntimeUI::WidgetTypes::ImageWidget:
				m_WindowDefaultWidget.AddToOptions("Image Widget", widget->m_Tag, iteration);
				break;
			case RuntimeUI::WidgetTypes::ImageButtonWidget:
				m_WindowDefaultWidget.AddToOptions("Image Button Widget", widget->m_Tag, iteration);
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
		m_ActiveWidget->m_XPositionType = (RuntimeUI::PixelOrPercent)m_WidgetXPixelOrPercentLocation.m_SelectedOption;

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
		m_ActiveWidget->m_YPositionType = (RuntimeUI::PixelOrPercent)m_WidgetYPixelOrPercentLocation.m_SelectedOption;

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

		// Reset the pixel and percent positions if the constraint is set
		m_ActiveWidget->m_PixelPosition.x = 0;
		m_ActiveWidget->m_PercentPosition.x = 0;

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

		// Reset the pixel and percent positions if the constraint is set
		m_ActiveWidget->m_PixelPosition.y = 0;
		m_ActiveWidget->m_PercentPosition.y = 0;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnOpenWidgetXConstraint()
	{
		m_WidgetXConstraintLocation.ClearOptions();
		m_WidgetXConstraintLocation.AddToOptions("Clear", "None", (uint64_t)RuntimeUI::Constraint::None);
		m_WidgetXConstraintLocation.AddToOptions("All Options", "Left", (uint64_t)RuntimeUI::Constraint::Left);
		m_WidgetXConstraintLocation.AddToOptions("All Options", "Right", (uint64_t)RuntimeUI::Constraint::Right);
		m_WidgetXConstraintLocation.AddToOptions("All Options", "Center", (uint64_t)RuntimeUI::Constraint::Center);
	}

	void UIEditorPropertiesPanel::OnOpenWidgetYConstraint()
	{
		m_WidgetYConstraintLocation.ClearOptions();
		m_WidgetYConstraintLocation.AddToOptions("Clear", "None", (uint64_t)RuntimeUI::Constraint::None);
		m_WidgetYConstraintLocation.AddToOptions("All Options", "Top", (uint64_t)RuntimeUI::Constraint::Top);
		m_WidgetYConstraintLocation.AddToOptions("All Options", "Bottom", (uint64_t)RuntimeUI::Constraint::Bottom);
		m_WidgetYConstraintLocation.AddToOptions("All Options", "Center", (uint64_t)RuntimeUI::Constraint::Center);
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

	void UIEditorPropertiesPanel::OnModifyWidgetSizePixelOrPercent()
	{
		// Ensure active widget is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget size metric");
			return;
		}

		// Update the widget location metric based on the radio selector value
		m_ActiveWidget->m_SizeType = (RuntimeUI::PixelOrPercent)m_WidgetPixelOrPercentSize.m_SelectedOption;
		RuntimeUI::RuntimeUIService::RecalculateTextData(m_ActiveWindow, m_ActiveWidget);

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetPercentSize(EditorUI::EditVec2Spec& spec)
	{
		// Ensure active widget is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget size");
			return;
		}

		// Update the widget size based on the editorUI widget value
		m_ActiveWidget->m_PercentSize = m_WidgetPercentSize.m_CurrentVec2;
		RuntimeUI::RuntimeUIService::RecalculateTextData(m_ActiveWindow, m_ActiveWidget);

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyWidgetPixelSize(EditorUI::EditIVec2Spec& spec)
	{
		// Ensure active widget is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget size");
			return;
		}

		// Update the widget size based on the editorUI widget value
		m_ActiveWidget->m_PixelSize = spec.m_CurrentIVec2;
		RuntimeUI::RuntimeUIService::RecalculateTextData(m_ActiveWindow, m_ActiveWidget);

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyButtonWidgetBackgroundColor(EditorUI::EditVec4Spec& spec)
	{
		// Ensure active widget is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget background color");
			return;
		}

		// Get the active widget as a button widget
		KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::ButtonWidget);
		RuntimeUI::ButtonWidget& activeButton = *(RuntimeUI::ButtonWidget*)m_ActiveWidget;

		// Update the widget background color based on the editorUI widget value
		activeButton.m_SelectionData.m_DefaultBackgroundColor = m_ButtonWidgetBackgroundColor.m_CurrentVec4;
		activeButton.m_SelectionData.m_ActiveBackgroundColor = m_ButtonWidgetBackgroundColor.m_CurrentVec4;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyButtonWidgetOnPress(const EditorUI::OptionEntry& entry)
	{
		// Get the active widget as a button widget
		KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::ButtonWidget);
		RuntimeUI::ButtonWidget& activeButton = *(RuntimeUI::ButtonWidget*)m_ActiveWidget;

		// Clear the on press script if the provided handle is empty
		if (entry.m_Handle == Assets::EmptyHandle)
		{
			activeButton.m_SelectionData.m_FunctionPointers.m_OnPress = nullptr;
			activeButton.m_SelectionData.m_FunctionPointers.m_OnPressHandle = Assets::EmptyHandle;

			// Set the active editor UI as edited
			s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
			return;
		}

		// Set the on press script for the text widget
		activeButton.m_SelectionData.m_FunctionPointers.m_OnPressHandle = entry.m_Handle;
		activeButton.m_SelectionData.m_FunctionPointers.m_OnPress = Assets::AssetService::GetScript(entry.m_Handle);

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnOpenButtonWidgetOnPressPopup()
	{
		// Clear existing options
		m_ButtonWidgetOnPress.ClearOptions();
		m_ButtonWidgetOnPress.AddToOptions("Clear", "None", Assets::EmptyHandle);

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
			m_ButtonWidgetOnPress.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
		}
	}

	void UIEditorPropertiesPanel::OnOpenTooltipForButtonWidgetOnPress()
	{
		// Clear existing options
		s_UIWindow->m_TreePanel->m_SelectScriptTooltip.ClearEntries();

		// Add option to opening an existing script
		EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
		{
			m_ButtonWidgetOnPress.m_OpenPopup = true;
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

					// Get the active widget as a button widget
					KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::ButtonWidget);
					RuntimeUI::ButtonWidget& activeButton = *(RuntimeUI::ButtonWidget*)m_ActiveWidget;

					// Fill the new script handle
					activeButton.m_SelectionData.m_FunctionPointers.m_OnPressHandle = scriptHandle;
					activeButton.m_SelectionData.m_FunctionPointers.m_OnPress = script;
					m_ButtonWidgetOnPress.m_CurrentOption = { script->m_ScriptName, scriptHandle };

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
		m_ActiveWidget->m_XRelativeOrAbsolute = (RuntimeUI::RelativeOrAbsolute)m_WidgetXRelOrAbsLocation.m_SelectedOption;

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
		m_ActiveWidget->m_YRelativeOrAbsolute = (RuntimeUI::RelativeOrAbsolute)m_WidgetYRelOrAbsLocation.m_SelectedOption;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyTextWidgetText(EditorUI::EditMultiLineTextSpec& spec)
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
		textWidget.SetText(m_TextWidgetText.m_CurrentOption, m_ActiveWindow);

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
		textWidget.m_TextSize = m_TextWidgetTextSize.m_CurrentFloat;
		textWidget.CalculateTextMetadata(m_ActiveWindow);

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
		textWidget.m_TextColor = m_TextWidgetTextColor.m_CurrentVec4;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnModifyTextWidgetAlignment(const EditorUI::OptionEntry& entry)
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

		// Change the alignment option for the text widget
		textWidget.m_TextAlignment = (RuntimeUI::Constraint)(uint64_t)entry.m_Handle;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}

	void UIEditorPropertiesPanel::OnOpenTextWidgetAlignmentPopup()
	{
		// Clear existing options
		m_TextWidgetTextAlignment.ClearOptions();

		// Add left, right, and center alignment options
		m_TextWidgetTextAlignment.AddToOptions
		(
			"All Options", 
			Utility::ConstraintToString(RuntimeUI::Constraint::Left),
			(uint64_t)RuntimeUI::Constraint::Left
		);

		m_TextWidgetTextAlignment.AddToOptions
		(
			"All Options",
			Utility::ConstraintToString(RuntimeUI::Constraint::Right),
			(uint64_t)RuntimeUI::Constraint::Right
		);
		m_TextWidgetTextAlignment.AddToOptions
		(
			"All Options",
			Utility::ConstraintToString(RuntimeUI::Constraint::Center),
			(uint64_t)RuntimeUI::Constraint::Center
		);
	}

	void UIEditorPropertiesPanel::OnModifyTextWidgetWrapped(EditorUI::CheckboxSpec& spec)
	{
		// Ensure active window is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget's text alignment field");
			return;
		}

		// Ensure active widget is a valid type
		if (!m_ActiveWidget)
		{
			KG_WARN("No valid widget active when trying to update widget's text alignment field");
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
		textWidget.m_TextWrapped = spec.m_CurrentBoolean;
		textWidget.CalculateTextMetadata(m_ActiveWindow);

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnModifyButtonWidgetText(EditorUI::EditTextSpec& spec)
	{
		// Ensure active window is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update the button widget's text");
			return;
		}

		// Ensure active widget is a valid type
		if (!m_ActiveWidget)
		{
			KG_WARN("No valid widget active when trying to update the button widget's text");
			return;
		}

		// Ensure active widget is a valid type and get the button widget
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::ButtonWidget)
		{
			KG_WARN("Attempt to modify text widget member, however, active widget is an invalid type");
			return;
		}
		RuntimeUI::ButtonWidget& buttonWidget = *(RuntimeUI::ButtonWidget*)m_ActiveWidget;

		// Update the button widget text based on the editorUI widget's value
		buttonWidget.SetText(spec.m_CurrentOption);

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnModifyButtonWidgetButtonTextSize(EditorUI::EditFloatSpec& spec)
	{
		// Ensure active window is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update button widget's text size");
			return;
		}

		// Ensure active widget is a valid type
		if (!m_ActiveWidget)
		{
			KG_WARN("No valid widget active when trying to update the button widget's text size");
			return;
		}

		// Ensure active widget is a valid type and get the button widget
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::ButtonWidget)
		{
			KG_WARN("Attempt to modify text widget member, however, active widget is an invalid type");
			return;
		}
		RuntimeUI::ButtonWidget& buttonWidget = *(RuntimeUI::ButtonWidget*)m_ActiveWidget;

		// Update the button widget text size based on the editorUI widget's value
		buttonWidget.m_TextSize = spec.m_CurrentFloat;
		buttonWidget.CalculateTextSize();

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnModifyButtonWidgetButtonTextColor(EditorUI::EditVec4Spec& spec)
	{
		// Ensure active window is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update button widget's text color");
			return;
		}

		if (!m_ActiveWidget)
		{
			KG_WARN("No valid widget active when trying to update button widget's text color");
			return;
		}

		// Ensure active widget is a valid type and get the button widget
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::ButtonWidget)
		{
			KG_WARN("Attempt to modify button widget member, however, active widget is an invalid type");
			return;
		}
		RuntimeUI::ButtonWidget& buttonWidget = *(RuntimeUI::ButtonWidget*)m_ActiveWidget;

		// Update the button widget text color based on the editorUI widget's value
		buttonWidget.m_TextColor = spec.m_CurrentVec4;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnModifyButtonWidgetTextAlignment(const EditorUI::OptionEntry& entry)
	{
		// Ensure active window is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update button widget's text alignment");
			return;
		}

		// Ensure active widget is a valid type
		if (!m_ActiveWidget)
		{
			KG_WARN("No valid widget active when trying to update the button widget's text alignment");
			return;
		}

		// Ensure active widget is a valid type and get the button widget
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::ButtonWidget)
		{
			KG_WARN("Attempt to modify button widget member, however, active widget is an invalid type");
			return;
		}
		RuntimeUI::ButtonWidget& buttonWidget = *(RuntimeUI::ButtonWidget*)m_ActiveWidget;

		// Change the alignment option for the button widget
		buttonWidget.m_TextAlignment = (RuntimeUI::Constraint)(uint64_t)entry.m_Handle;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnOpenButtonWidgetTextAlignmentPopup()
	{
		// Clear existing options
		m_ButtonWidgetTextAlignment.ClearOptions();

		// Add left, right, and center alignment options
		m_ButtonWidgetTextAlignment.AddToOptions
		(
			"All Options",
			Utility::ConstraintToString(RuntimeUI::Constraint::Left),
			(uint64_t)RuntimeUI::Constraint::Left
		);

		m_ButtonWidgetTextAlignment.AddToOptions
		(
			"All Options",
			Utility::ConstraintToString(RuntimeUI::Constraint::Right),
			(uint64_t)RuntimeUI::Constraint::Right
		);
		m_ButtonWidgetTextAlignment.AddToOptions
		(
			"All Options",
			Utility::ConstraintToString(RuntimeUI::Constraint::Center),
			(uint64_t)RuntimeUI::Constraint::Center
		);
	}
	void UIEditorPropertiesPanel::OnModifyButtonWidgetSelectable(EditorUI::CheckboxSpec& spec)
	{
		// Ensure active window is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget's text alignment field");
			return;
		}

		// Ensure active widget is a valid type
		if (!m_ActiveWidget)
		{
			KG_WARN("No valid widget active when trying to update widget's text alignment field");
			return;
		}

		// Ensure active widget is a valid type and get the button widget
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::ButtonWidget)
		{
			KG_WARN("Attempt to modify button widget member, however, active widget is an invalid type");
			return;
		}
		RuntimeUI::ButtonWidget& buttonWidget = *(RuntimeUI::ButtonWidget*)m_ActiveWidget;

		// Update the text widget text alignment based on the editorUI widget's value
		buttonWidget.m_SelectionData.m_Selectable = spec.m_CurrentBoolean;
		RuntimeUI::RuntimeUIService::CalculateWindowNavigationLinks();

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnModifyImageWidgetImage(const EditorUI::OptionEntry& entry)
	{
		// Get the active widget as a image widget
		KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::ImageWidget);
		RuntimeUI::ImageWidget& activeImageWidget = *(RuntimeUI::ImageWidget*)m_ActiveWidget;

		// Clear the on press script if the provided handle is empty
		if (entry.m_Handle == Assets::EmptyHandle)
		{
			activeImageWidget.m_ImageRef = nullptr;
			activeImageWidget.m_ImageHandle = Assets::EmptyHandle;

			// Set the active editor UI as edited
			s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
			return;
		}

		// Set the texture reference for the image widget
		activeImageWidget.m_ImageRef = Assets::AssetService::GetTexture2D(entry.m_Handle);
		activeImageWidget.m_ImageHandle = entry.m_Handle;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnOpenImageWidgetImagePopup()
	{
		// Clear existing options
		m_ImageWidgetImage.ClearOptions();
		m_ImageWidgetImage.AddToOptions("Clear", "None", Assets::EmptyHandle);

		// Add all compatible textures to the select options
		for (auto& [handle, assetInfo] : Assets::AssetService::GetTexture2DRegistry())
		{
			// Get texture from handle
			Ref<Rendering::Texture2D> script = Assets::AssetService::GetTexture2D(handle);

			// Add texture to the select options
			m_ImageWidgetImage.AddToOptions("All Options", assetInfo.Data.FileLocation.stem().string(), handle);
		}
	}
	void UIEditorPropertiesPanel::OnModifyImageWidgetFixedAspectRatio(EditorUI::CheckboxSpec& spec)
	{
		// Ensure active widget is a valid type and get the iamge widget
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::ImageWidget)
		{
			KG_WARN("Attempt to modify an image widget member, however, active widget is an invalid type");
			return;
		}
		RuntimeUI::ImageWidget& imageWidget = *(RuntimeUI::ImageWidget*)m_ActiveWidget;

		// Update the 
		imageWidget.m_FixedAspectRatio = spec.m_CurrentBoolean;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnModifyImageButtonWidgetImage(const EditorUI::OptionEntry& entry)
	{
		// Get the active widget as a image button widget
		KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::ImageButtonWidget);
		RuntimeUI::ImageButtonWidget& activeImageButtonWidget = *(RuntimeUI::ImageButtonWidget*)m_ActiveWidget;

		// Clear the on press script if the provided handle is empty
		if (entry.m_Handle == Assets::EmptyHandle)
		{
			activeImageButtonWidget.m_ImageRef = nullptr;
			activeImageButtonWidget.m_ImageHandle = Assets::EmptyHandle;

			// Set the active editor UI as edited
			s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
			return;
		}

		// Set the texture reference for the image button widget
		activeImageButtonWidget.m_ImageRef = Assets::AssetService::GetTexture2D(entry.m_Handle);
		activeImageButtonWidget.m_ImageHandle = entry.m_Handle;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnOpenImageButtonWidgetImagePopup()
	{
		// Clear existing options
		m_ImageButtonWidgetImage.ClearOptions();
		m_ImageButtonWidgetImage.AddToOptions("Clear", "None", Assets::EmptyHandle);

		// Add all compatible textures to the select options
		for (auto& [handle, assetInfo] : Assets::AssetService::GetTexture2DRegistry())
		{
			// Get texture from handle
			Ref<Rendering::Texture2D> script = Assets::AssetService::GetTexture2D(handle);

			// Add texture to the select options
			m_ImageButtonWidgetImage.AddToOptions("All Options", assetInfo.Data.FileLocation.stem().string(), handle);
		}
	}
	void UIEditorPropertiesPanel::OnModifyImageButtonWidgetFixedAspectRatio(EditorUI::CheckboxSpec& spec)
	{
		// Ensure active widget is a valid type and get the button widget
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::ImageButtonWidget)
		{
			KG_WARN("Attempt to modify an image button widget member, however, active widget is an invalid type");
			return;
		}
		RuntimeUI::ImageButtonWidget& imageButtonWidget = *(RuntimeUI::ImageButtonWidget*)m_ActiveWidget;

		// Update the text widget text alignment based on the editorUI widget's value
		imageButtonWidget.m_FixedAspectRatio = spec.m_CurrentBoolean;
		RuntimeUI::RuntimeUIService::CalculateWindowNavigationLinks();

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnModifyImageButtonWidgetSelectable(EditorUI::CheckboxSpec& spec)
	{
		// Ensure active window is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget's text alignment field");
			return;
		}

		// Ensure active widget is a valid type
		if (!m_ActiveWidget)
		{
			KG_WARN("No valid widget active when trying to update widget's text alignment field");
			return;
		}

		// Ensure active widget is a valid type and get the button widget
		if (m_ActiveWidget->m_WidgetType != RuntimeUI::WidgetTypes::ImageButtonWidget)
		{
			KG_WARN("Attempt to modify image button widget member, however, active widget is an invalid type");
			return;
		}
		RuntimeUI::ImageButtonWidget& imageButtonWidget = *(RuntimeUI::ImageButtonWidget*)m_ActiveWidget;

		// Update the image button widget selectable value
		imageButtonWidget.m_SelectionData.m_Selectable = spec.m_CurrentBoolean;
		RuntimeUI::RuntimeUIService::CalculateWindowNavigationLinks();

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnModifyImageButtonWidgetOnPress(const EditorUI::OptionEntry& entry)
	{
		// Get the active widget as a image button widget
		KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::ImageButtonWidget);
		RuntimeUI::ImageButtonWidget& activeButton = *(RuntimeUI::ImageButtonWidget*)m_ActiveWidget;

		// Clear the on press script if the provided handle is empty
		if (entry.m_Handle == Assets::EmptyHandle)
		{
			activeButton.m_SelectionData.m_FunctionPointers.m_OnPress = nullptr;
			activeButton.m_SelectionData.m_FunctionPointers.m_OnPressHandle = Assets::EmptyHandle;

			// Set the active editor UI as edited
			s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
			return;
		}

		// Set the on press script for the image button widget
		activeButton.m_SelectionData.m_FunctionPointers.m_OnPressHandle = entry.m_Handle;
		activeButton.m_SelectionData.m_FunctionPointers.m_OnPress = Assets::AssetService::GetScript(entry.m_Handle);

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
	}
	void UIEditorPropertiesPanel::OnOpenImageButtonWidgetOnPressPopup()
	{
		// Clear existing options
		m_ImageButtonWidgetOnPress.ClearOptions();
		m_ImageButtonWidgetOnPress.AddToOptions("Clear", "None", Assets::EmptyHandle);

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
			m_ImageButtonWidgetOnPress.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
		}
	}
	void UIEditorPropertiesPanel::OnOpenTooltipForImageButtonWidgetOnPress()
	{
		// Clear existing options
		s_UIWindow->m_TreePanel->m_SelectScriptTooltip.ClearEntries();

		// Add option to opening an existing script
		EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
		{
			m_ImageButtonWidgetOnPress.m_OpenPopup = true;
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

					// Get the active widget as a button widget
					KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::ImageButtonWidget);
					RuntimeUI::ImageButtonWidget& activeButton = *(RuntimeUI::ImageButtonWidget*)m_ActiveWidget;

					// Fill the new script handle
					activeButton.m_SelectionData.m_FunctionPointers.m_OnPressHandle = scriptHandle;
					activeButton.m_SelectionData.m_FunctionPointers.m_OnPress = script;
					m_ImageButtonWidgetOnPress.m_CurrentOption = { script->m_ScriptName, scriptHandle };

					// Set the active editor UI as edited
					s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;
				}, {});
			} };
		s_UIWindow->m_TreePanel->m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

		// Open tooltip
		s_UIWindow->m_TreePanel->m_SelectScriptTooltip.m_TooltipActive = true;
	}
	void UIEditorPropertiesPanel::OnModifyImageButtonWidgetBackgroundColor(EditorUI::EditVec4Spec& spec)
	{
		// Ensure active widget is valid
		if (!m_ActiveWindow)
		{
			KG_WARN("No valid widget active when trying to update widget background color");
			return;
		}

		// Get the active widget as a button widget
		KG_ASSERT(m_ActiveWidget->m_WidgetType == RuntimeUI::WidgetTypes::ImageButtonWidget);
		RuntimeUI::ImageButtonWidget& activeButton = *(RuntimeUI::ImageButtonWidget*)m_ActiveWidget;

		// Update the widget background color based on the editorUI widget value
		activeButton.m_SelectionData.m_DefaultBackgroundColor = m_ImageButtonWidgetBackgroundColor.m_CurrentVec4;
		activeButton.m_SelectionData.m_ActiveBackgroundColor = m_ImageButtonWidgetBackgroundColor.m_CurrentVec4;

		// Set the active editor UI as edited
		s_UIWindow->m_TreePanel->m_MainHeader.m_EditColorActive = true;

	}
}

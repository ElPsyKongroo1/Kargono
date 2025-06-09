#include "kgpch.h"

#include "Modules/RuntimeUI/RuntimeUIUserInterface.h"

#include "Modules/RuntimeUI/RuntimeUIContext.h"
#include "Modules/RuntimeUI/Widgets/RuntimeUIWidget.h"
#include "Modules/RuntimeUI/Widgets/RuntimeUIHorizontalContainerWidget.h"
#include "Modules/RuntimeUI/Widgets/RuntimeUIVerticalContainerWidget.h"
#include "Modules/RuntimeUI/RuntimeUIWindow.h"

#include "Modules/Input/InputService.h"
#include "Kargono/Utility/Operations.h"
#include "Kargono/Math/Interpolation.h"
#include "Modules/Rendering/RenderingService.h"
#include "Modules/Assets/AssetService.h"

#include "Modules/Core/Engine.h"

namespace Kargono::RuntimeUI
{
	void UIWindowsState::Init()
	{
		// Revalidate UI Context
		RevalidateDisplayedWindows();
		RevalidateIDToLocationMap();

		// Set the first window as active if applicable
		if (m_Windows.size() > 0)
		{
			// Display the first window
			m_Windows.at(0).DisplayWindow();
		}

		RevalidateDefaultWidgets();
		RevalidateTextDimensions();
		RevalidateNavigationLinks();

		m_Active = true;
		KG_ASSERT(m_Active);
	}
	void UIWindowsState::Terminate()
	{
		m_Active = false;
		KG_ASSERT(!m_Active);
	}
	void UIWindowsState::OnRender(RuntimeUIContext* uiContext, ViewportData viewportData)
	{
		Rendering::RendererInputSpec& backgroundSpec{ uiContext->m_BackgroundInputSpec };
		Rendering::RendererInputSpec& imageSpec{ uiContext->m_ImageInputSpec };

		// Submit rendering data from all windows
		WindowIndex windowIndex{ 0 };
		std::vector<WindowIndex>& windowIndices = m_DisplayedWindowIndices;
		for (Window* window : m_DisplayedWindows)
		{
			// Get position data for rendering window
			Math::vec3 windowSize = window->GetSize(viewportData);
			Math::vec3 windowCornerPos = window->GetLowerCornerPosition(viewportData);
			Math::vec3 windowCenterPos = window->GetCenterPosition(windowCornerPos, windowSize);

			if (window->m_BackgroundColor.w > 0.001f)
			{
				// Create background rendering data
				backgroundSpec.m_TransformMatrix = glm::translate(Math::mat4(1.0f), windowCenterPos)
					* glm::scale(Math::mat4(1.0f), windowSize);
				Rendering::Shader::SetDataAtInputLocation<Math::vec4>(window->m_BackgroundColor,
					Utility::FileSystem::CRCFromString("a_Color"),
					backgroundSpec.m_Buffer, backgroundSpec.m_Shader);

				// Push window ID and invalid widgetID
				Rendering::Shader::SetDataAtInputLocation<int32_t>(window->m_ID,
					Utility::FileSystem::CRCFromString("a_EntityID"),
					backgroundSpec.m_Buffer, backgroundSpec.m_Shader);

				// Submit background data to GPU
				Rendering::RenderingService::SubmitDataToRenderer(backgroundSpec);
			}

			// Call rendering function for every widget
			windowCornerPos.z += 0.1f;
			WidgetIndex widgetIndex{ 0 };
			for (Ref<Widget> widgetRef : window->m_Widgets)
			{
				// Push widget ID
				Rendering::Shader::SetDataAtInputLocation<int32_t>(widgetRef->m_ID,
					Utility::FileSystem::CRCFromString("a_EntityID"),
					backgroundSpec.m_Buffer, backgroundSpec.m_Shader);
				Rendering::Shader::SetDataAtInputLocation<int32_t>(widgetRef->m_ID,
					Utility::FileSystem::CRCFromString("a_EntityID"),
					imageSpec.m_Buffer, imageSpec.m_Shader);
				RuntimeUI::FontService::GetActiveContext().SetID((uint32_t)widgetRef->m_ID);

				// Call the widget's rendering function
				widgetRef->OnRender(uiContext, windowCornerPos, windowSize, (float)viewportData.m_Width);
				widgetIndex++;
			}
			windowIndex++;
		}
	}
	void UIWindowsState::AddWindow(Window& window)
	{
		// Store the window in the active user interface
		m_Windows.push_back(window);

		RevalidateIDToLocationMap();

		// Display the window
		window.DisplayWindow();
	}
	bool UIWindowsState::DeleteWindow(WindowID windowID)
	{
		// Get the window location
		UILocation* locationDir = GetLocationFromID(windowID);

		// Ensure the location is valid and represents a window
		KG_ASSERT(locationDir);
		KG_ASSERT(locationDir->size() == 1);
		uint16_t windowLocation{ locationDir->at(0) };

		// Attempt to delete the window from the active user interface
		bool success = DeleteWindowInternal(windowLocation);

		// Ensure deletion was successful
		if (!success)
		{
			KG_WARN("Attempt to delete window from active user interface failed.");
			return false;
		}

		// Ensure correct windows are displayed
		RevalidateDisplayedWindows();
		RevalidateIDToLocationMap();
		return true;
	}

	bool UIWindowsState::DeleteWidgetInternal(WidgetID widgetID)
	{
		// Get the ID -> Widget Location map and location directions
		KG_ASSERT(m_IDToLocation.contains(widgetID));
		UILocation& locationDirections = m_IDToLocation.at(widgetID);

		// Ensure the directions map to a widget
		KG_ASSERT(locationDirections.size() > 1);

		// Get widget using location directions
		WindowIndex windowIndex = (WindowIndex)locationDirections.at(0);
		WidgetIndex firstWidgetIndex = (WidgetIndex)locationDirections.at(1);

		// Ensure window index is valid
		if (windowIndex >= m_Windows.size())
		{
			KG_WARN("Attempt to delete widget from window with out of bounds index");
			return false;
		}

		// Get the window reference
		std::vector<RuntimeUI::Window>& uiWindows = m_Windows;
		RuntimeUI::Window& indicatedWindow = uiWindows.at(windowIndex);

		// Ensure widget index is valid
		if (firstWidgetIndex >= indicatedWindow.m_Widgets.size())
		{
			KG_WARN("Attempt to delete widget with out of bounds index");
			return false;
		}

		// Clear default active widget if necessary
		if (widgetID == indicatedWindow.m_DefaultActiveWidget)
		{
			indicatedWindow.m_DefaultActiveWidget = k_InvalidWidgetID;
			indicatedWindow.m_DefaultActiveWidgetRef = nullptr;
		}

		// Handle the case where the location directions only uses one widget (Ex: Window -> Widget)
		if (locationDirections.size() == 2)
		{
			// Delete the widget
			indicatedWindow.m_Widgets.erase(indicatedWindow.m_Widgets.begin() + firstWidgetIndex);
			return true;
		}

		// Get the first widget (Ex: Window -> [Widget])
		Ref<Widget> currentWidget = indicatedWindow.m_Widgets.at(firstWidgetIndex);
		KG_ASSERT(currentWidget);

		// Handle all other cases where the location direction uses two or more widgets (Ex: Window -> Widget -> Widget..)
		for (WidgetIndex widgetIndex{ 2 }; widgetIndex < locationDirections.size(); widgetIndex++)
		{
			// Get the container data for the current widget
			ContainerData* data = currentWidget->GetContainerData();
			KG_ASSERT(data);

			// Ensure the widget directions fall within the bounds of the widget array
			WidgetIndex currentIndex = locationDirections.at(widgetIndex);
			KG_ASSERT(currentIndex < data->m_ContainedWidgets.size());

			// Exit early if we reach the final valid index of locationDirections (Ex: Window -> Widget -> (Final)[Widget])
			if (widgetIndex == locationDirections.size() - 1)
			{
				// Remove the indicated widget
				data->m_ContainedWidgets.erase(data->m_ContainedWidgets.begin() + currentIndex);
				break;
			}

			// Set the current widget to the indicated container widget (Window -> ... -> (Current)Widget -> [Widget])
			currentWidget = data->m_ContainedWidgets.at(currentIndex);
			KG_ASSERT(currentWidget);
		}

		return true;
	}

	bool UIWindowsState::DeleteWindowInternal(WindowIndex windowIndex)
	{
		// Ensure window location is valid
		if (windowIndex >= m_Windows.size())
		{
			KG_WARN("Attempt to delete window, however, the provided index is out of bounds");
			return false;
		}

		// Delete the indicated window
		m_Windows.erase(m_Windows.begin() + windowIndex);
		return true;
	}

	void UIWindowsState::SetWidgetTextInternal(Ref<Widget> currentWidget, std::string_view newText)
	{
		// Ensure the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget's text");
			return;
		}

		// Get the selection specific data from the widget
		MultiLineTextData* multiLineData = currentWidget->GetMultiLineTextData();
		if (multiLineData)
		{
			// Set the text of the widget
			multiLineData->m_Text = newText;
			currentWidget->RevalidateTextDimensions();
			return;
		}

		// Get the selection specific data from the widget
		SingleLineTextData* singleLineData = currentWidget->GetSingleLineTextData();
		if (singleLineData)
		{
			// Modify the single line text
			singleLineData->SetText(currentWidget->i_ParentUI, newText);
			return;
		}

		KG_WARN("Invalid widget type provided when modifying a widget's text");
		return;
	}

	void UIInteractionState::SetSelectedWidgetInternal(Ref<Widget> newSelectedWidget)
	{
		// Ensure the widget is valid
		if (!newSelectedWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget as selected");
			return;
		}

		// Ensure the widget is selectable
		if (!newSelectedWidget->Selectable())
		{
			return;
		}

		// If the selected widget is the same as the new selected widget, just exit
		if (newSelectedWidget.get() == m_SelectedWidget)
		{
			return;
		}

		// Set the new widget as selected and set it's color to the active color
		m_SelectedWidget = newSelectedWidget.get();

		// Call the on move function if applicable
		if (i_Config->m_FunctionPointers.m_OnMove)
		{
			Utility::CallWrapped<WrappedVoidNone>(i_Config->m_FunctionPointers.m_OnMove->m_Function);
		}
	}

	void UIInteractionState::SetHoveredWidgetInternal(Ref<Widget> newHoveredWidget)
	{
		// Ensure the widget is valid
		if (!newHoveredWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget as selected");
			return;
		}

		// Ensure the widget is selectable
		if (!newHoveredWidget->Selectable())
		{
			return;
		}

		// If the selected widget is the same as the new selected widget, just exit
		if (newHoveredWidget.get() == m_HoveredWidget)
		{
			return;
		}

		// Set the new widget as selected and set it's color to the active color
		m_HoveredWidget = newHoveredWidget.get();

		// Set the cursor to IBeam
		if (newHoveredWidget->m_WidgetType == WidgetTypes::InputTextWidget)
		{
			// Reset the cursor icon
			EngineService::GetActiveEngine().GetWindow().SetMouseCursorIcon(CursorIconType::IBeam);
		}

		// Do not handle the on move function if the widget is a slider
		if (newHoveredWidget->m_WidgetType == WidgetTypes::SliderWidget)
		{
			return;
		}

		// Call the on move function if applicable
		if (i_Config->m_FunctionPointers.m_OnHover)
		{
			Utility::CallWrapped<WrappedVoidNone>(i_Config->m_FunctionPointers.m_OnHover->m_Function);
		}
	}

	void UIWindowsState::SetWidgetTextColorInternal(Ref<Widget> currentWidget, const Math::vec4& newColor)
	{
		// Ensure the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget's text color");
			return;
		}

		// Get the selection specific data from the widget
		MultiLineTextData* multiLineData = currentWidget->GetMultiLineTextData();
		if (multiLineData)
		{
			// Set the text of the widget
			multiLineData->m_TextColor = newColor;
			return;
		}

		// Get the selection specific data from the widget
		SingleLineTextData* singleLineData = currentWidget->GetSingleLineTextData();
		if (singleLineData)
		{
			// Modify the single line text
			singleLineData->m_TextColor = newColor;
			return;
		}

		KG_WARN("Invalid widget type provided when modifying a widget's text color");
		return;
	}

	void UIWindowsState::SetWidgetSelectableInternal(Ref<Widget> currentWidget, bool selectable)
	{
		// Ensure the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget as selectable");
			return;
		}

		// Get the selection specific data from the widget
		SelectionData* selectionData = currentWidget->GetSelectionData();
		if (!selectionData)
		{
			KG_WARN("Unable to retrieve selection data. May be invalid widget type!");
			return;
		}

		// Set the widget as selectable
		selectionData->m_Selectable = selectable;

		RevalidateNavigationLinks();
	}

	void UIWindowsState::SetWidgetBackgroundColorInternal(Ref<Widget> currentWidget, const Math::vec4& newColor)
	{
		// Ensure the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget's background color");
			return;
		}

		// Get the selection specific data from the widget
		SelectionData* selectionData = currentWidget->GetSelectionData();
		if (!selectionData)
		{
			KG_WARN("Unable to retrieve selection data. May be invalid widget type!");
			return;
		}

		// Set the widget's new color
		selectionData->m_DefaultBackgroundColor = newColor;
	}

	bool UIWindowsState::DeleteWidget(WidgetID widgetID)
	{
		// Attempt to delete the widget from the active user interface
		bool success = DeleteWidgetInternal(widgetID);

		// Ensure deletion was successful
		if (!success)
		{
			KG_WARN("Attempt to delete widget from active user interface failed.");
			return false;
		}

		RevalidateIDToLocationMap();
		RevalidateNavigationLinks();

		return true;
	}

	void UIWindowsState::SetWidgetSelectableByTag(std::string_view windowTag, std::string_view widgetTag, bool selectable)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidgetFromTag(windowTag, widgetTag);

		SetWidgetSelectableInternal(currentWidget, selectable);
	}

	void UIWindowsState::SetWidgetSelectableByHandle(WidgetHandle widgetHandle, bool selectable)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidgetFromID(widgetHandle.m_WidgetID);

		SetWidgetSelectableInternal(currentWidget, selectable);
	}

	void UIWindowsState::SetDisplayWindowByTag(std::string_view windowTag, bool display)
	{
		// Search for the indicated window
		for (Window& window : m_Windows)
		{
			if (window.m_Tag == windowTag)
			{
				// Display or hide the window
				if (display)
				{
					window.DisplayWindow();
				}
				else
				{
					window.HideWindow();
				}
				return;
			}
		}
	}

	void UIWindowsState::SetDisplayWindowByHandle(WindowHandle windowHandle, bool display)
	{
		// Get the ID -> Widget Location map and location directions
		KG_ASSERT(m_IDToLocation.contains(windowHandle.m_WindowID));
		UILocation& locationDirections = m_IDToLocation.at(windowHandle.m_WindowID);

		// Get widget using location directions
		// TODO: This currently assumes the window -> widget structure
		KG_ASSERT(locationDirections.size() == 1); // TODO: Remove this later
		WindowIndex windowIndex = (WindowIndex)locationDirections.at(0);

		if (windowIndex > (m_Windows.size() - 1))
		{
			KG_WARN("Provided window ID is out of bounds for the UI's windows");
			return;
		}

		Window& window = m_Windows.at(windowIndex);

		// Display or hide the window
		if (display)
		{
			window.DisplayWindow();
		}
		else
		{
			window.HideWindow();
		}
	}

	void UIWindowsState::SetWidgetImageByHandle(WidgetHandle widgetHandle, Assets::AssetHandle textureHandle)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidgetFromID(widgetHandle.m_WidgetID);

		// Ensure the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget's image");
			return;
		}

		// Ensure the texture provided is valid
		Ref<Rendering::Texture2D> textureRef{ Assets::AssetService::GetTexture2D(textureHandle) };
		if (!textureRef)
		{
			KG_WARN("Attempt to modify the image of a widget, however, the provided image is invalid!");
			return;
		}

		// Get the selection specific data from the widget
		ImageData* imageData = currentWidget->GetImageData();
		if (!imageData)
		{
			KG_WARN("Unable to retrieve image data. May be invalid widget type!");
			return;
		}

		imageData->m_ImageHandle = textureHandle;
		imageData->m_ImageRef = textureRef;
	}

	void UIWindowsState::SetWidgetTextColorByTag(std::string_view windowTag, std::string_view widgetTag, const Math::vec4& color)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidgetFromTag(windowTag, widgetTag);

		SetWidgetTextColorInternal(currentWidget, color);
	}

	void UIWindowsState::SetWidgetTextColorByHandle(WidgetHandle widgetHandle, const Math::vec4& color)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidgetFromID(widgetHandle.m_WidgetID);

		SetWidgetTextColorInternal(currentWidget, color);
	}

	void UIWindowsState::SetWidgetBackgroundColorByTag(std::string_view windowTag, std::string_view widgetTag, const Math::vec4& color)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidgetFromTag(windowTag, widgetTag);

		SetWidgetBackgroundColorInternal(currentWidget, color);
	}

	void UIWindowsState::SetWidgetBackgroundColorByHandle(WidgetHandle widgetHandle, const Math::vec4& color)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidgetFromID(widgetHandle.m_WidgetID);

		SetWidgetBackgroundColorInternal(currentWidget, color);
	}

	void UserInterface::Init(RuntimeUIContext* parentContext)
	{
		// Add injected dependencies
		KG_ASSERT(parentContext);
		i_ParentContext = parentContext;

		// Initialize local component/structs
		m_WindowsState.Init();
		KG_ASSERT(m_WindowsState.m_Active);

		m_InteractState.Init(&m_WindowsState, &m_Config);
		KG_ASSERT(m_InteractState.m_Active);

		m_Config.Init(&m_WindowsState, i_ParentContext->m_DefaultFont);
		KG_ASSERT(m_Config.m_Active);

		m_Active = true;
		KG_ASSERT(m_Active);
	}

	void UserInterface::Terminate()
	{
		m_WindowsState.Terminate();
		m_InteractState.Terminate();
		m_Config.Terminate();

		KG_ASSERT(!m_WindowsState.m_Active);
		KG_ASSERT(!m_InteractState.m_Active);
		KG_ASSERT(!m_Config.m_Active);

		i_ParentContext = nullptr;

		m_Active = false;
		KG_ASSERT(!m_Active);
	}

	void UserInterface::OnUpdate(Timestep ts)
	{
		// Get mouse position and active viewport
		Math::vec2 mousePosition = Input::InputService::GetViewportMousePosition();
		ViewportData* viewportData = EngineService::GetActiveEngine().GetApp().GetViewportData();

		// Handle IBeam flipping for input text widgets
		if (m_InteractState.m_EditingWidget)
		{
			m_CaretState.m_CaretAccumulator += ts;
			if (m_CaretState.m_CaretAccumulator > m_CaretState.m_CaretVisibleInterval)
			{
				m_CaretState.m_CaretAccumulator -= m_CaretState.m_CaretVisibleInterval;
				Utility::Operations::ToggleBoolean(m_CaretState.m_CaretVisible);
			}
		}

		if (m_InteractState.m_PressedWidget)
		{
			// Get the widget's transform
			Bounds widgetTransform = m_WindowsState.GetWidgetBoundsFromID(
				m_InteractState.m_PressedWidget->m_ID, *viewportData);

			// Get the underlying widget type
			KG_ASSERT(m_InteractState.m_PressedWidget->m_WidgetType == WidgetTypes::SliderWidget);
			SliderWidget& activeSlider = *(SliderWidget*)m_InteractState.m_PressedWidget;

			// Get the line dimensions
			float sliderLineLowerBound{ widgetTransform.m_Translation.x };
			float sliderLineUpperBound{ widgetTransform.m_Translation.x + widgetTransform.m_Size.x };

			// Check if the mouse click falls within the bounds of the slider
			if (mousePosition.x < sliderLineLowerBound)
			{
				// Set the resultant value to its lowest!
				activeSlider.m_CurrentValue = activeSlider.m_Bounds.x;
				// Call the on move slider function if applicable
				if (activeSlider.m_OnMoveSlider)
				{
					Utility::CallWrapped<WrappedVoidFloat>(activeSlider.m_OnMoveSlider->m_Function, activeSlider.m_CurrentValue);
				}

			}
			else if (mousePosition.x > sliderLineUpperBound)
			{
				// Set the resultant value to its highest!
				activeSlider.m_CurrentValue = activeSlider.m_Bounds.y;
				// Call the on move slider function if applicable
				if (activeSlider.m_OnMoveSlider)
				{
					Utility::CallWrapped<WrappedVoidFloat>(activeSlider.m_OnMoveSlider->m_Function, activeSlider.m_CurrentValue);
				}
			}
			else
			{
				// Get the slider's current normalized location based on the bounds and currentValue
				float normalizedSliderLocation = (mousePosition.x - sliderLineLowerBound) /
					(sliderLineUpperBound - sliderLineLowerBound);

				// Use linear interpolation to set the value
				activeSlider.m_CurrentValue = Math::Interpolation::Linear
				(
					activeSlider.m_Bounds.x,
					activeSlider.m_Bounds.y,
					normalizedSliderLocation
				);

				// Call the on move slider function if applicable
				if (activeSlider.m_OnMoveSlider)
				{
					Utility::CallWrapped<WrappedVoidFloat>(activeSlider.m_OnMoveSlider->m_Function, activeSlider.m_CurrentValue);
				}
			}
		}
	}

	bool UserInterface::OnKeyTypedEvent(Events::KeyTypedEvent event)
	{
		// Pass in key typed events for the current widget to be edited
		if (m_InteractState.m_EditingWidget)
		{
			// Get the text data
			SingleLineTextData* textData = m_InteractState.m_EditingWidget->GetSingleLineTextData();
			KG_ASSERT(textData);

			// Ensure the cursor index is within the correct text bounds
			if (textData->m_CaretIndex >= 0 && textData->m_CaretIndex <= textData->m_Text.size())
			{
				std::string character { (char)event.GetKeyCode() };
				// Add the event's character to the text buffer
				textData->m_Text.insert(textData->m_CaretIndex, character);
				textData->m_CaretIndex++;

				// Revalidate text dimensions
				m_InteractState.m_EditingWidget->RevalidateTextDimensions();

				// Run on move cursor if necessary
				OnMoveCursorInternal(m_InteractState.m_EditingWidget);

				// Ensure the IBeam is visible when moving
				m_CaretState.m_CaretVisible = true;
				m_CaretState.m_CaretAccumulator = 0.0f;
			}
		}
		return false;
	}

	bool UserInterface::OnKeyPressedEvent(Events::KeyPressedEvent event)
	{
		KeyCode key = event.GetKeyCode();

		// Handle key pressed events for the currently editing widget
		if (m_InteractState.m_EditingWidget)
		{
			// Get the text data
			SingleLineTextData* textData = m_InteractState.m_EditingWidget->GetSingleLineTextData();
			KG_ASSERT(textData);

			if (key == Key::Backspace)
			{
				bool isCaretWithinBounds
				{ 
					textData->m_Text.size() > 0 &&
					textData->m_CaretIndex > 0 && 
					textData->m_CaretIndex <= textData->m_Text.size() 
				};

				if (isCaretWithinBounds)
				{
					textData->m_Text.erase(textData->m_CaretIndex - 1, 1);
					textData->m_CaretIndex--;
					// Revalidate text dimensions
					m_InteractState.m_EditingWidget->RevalidateTextDimensions();

					// Run on move cursor if necessary
					OnMoveCursorInternal(m_InteractState.m_EditingWidget);
				}
				return true;
			}

			// Handle exiting 
			if (key == Key::Enter || key == Key::Escape)
			{
				m_InteractState.ClearEditingWidget();
				return true;
			}

			if (key == Key::Left)
			{
				// Enforce lower bounds of the text
				if (textData->m_CaretIndex > 0)
				{
					// Decriment the cursor
					textData->m_CaretIndex--;

					// Ensure the IBeam is visible when moving
					m_CaretState.m_CaretVisible = true;
					m_CaretState.m_CaretAccumulator = 0.0f;

					// Run on move cursor if necessary
					OnMoveCursorInternal(m_InteractState.m_EditingWidget);

				}
				return true;
			}
			if (key == Key::Right)
			{
				// Enforce upper bounds of the text
				if (textData->m_CaretIndex < textData->m_Text.size())
				{
					textData->m_CaretIndex++;

					// Ensure the IBeam is visible when moving
					m_CaretState.m_CaretVisible = true;
					m_CaretState.m_CaretAccumulator = 0.0f;

					// Run on move cursor if necessary
					OnMoveCursorInternal(m_InteractState.m_EditingWidget);
				}
				return true;
			}
		}

		return false;
	}

	void UserInterface::OnMouseButtonPressedEvent(const Events::MouseButtonPressedEvent& event)
	{
		// Get mouse position and active viewport
		Math::vec2 mousePosition = Input::InputService::GetViewportMousePosition();
		ViewportData* viewportData = EngineService::GetActiveEngine().GetApp().GetViewportData();

		// Handle mouse click for the editing widget
		if (m_InteractState.m_EditingWidget)
		{
			SingleLineTextData* textData = m_InteractState.m_EditingWidget->GetSingleLineTextData();
			KG_ASSERT(textData);

			// Get the widget's transform
			Bounds widgetTransform = m_WindowsState.GetWidgetBoundsFromID(
				m_InteractState.m_EditingWidget->m_ID, *viewportData);

			// Get the widget's text's starting position
			Math::vec2 resolution = Utility::ScreenResolutionToAspectRatio(Projects::ProjectService::GetActiveContext().GetTargetResolution());
			float textScalingFactor{ (viewportData->m_Width * 0.15f * textData->m_TextSize) * (resolution.y / resolution.x) };
			Math::vec3 textStartingPoint = textData->GetTextStartingPosition(widgetTransform.m_Translation, widgetTransform.m_Size, textScalingFactor);

			// Find new cursor location
			size_t newCursorIndex = textData->GetCursorIndexFromMousePosition(m_Config.m_Font, textStartingPoint.x, mousePosition.x, textScalingFactor);

			// Update the cursor
			textData->m_CaretIndex = newCursorIndex;

			// Ensure the IBeam is visible when moving
			m_CaretState.m_CaretVisible = true;
			m_CaretState.m_CaretAccumulator = 0.0f;
		}
		else if (m_InteractState.m_SelectedWidget && m_InteractState.m_SelectedWidget->m_WidgetType == WidgetTypes::SliderWidget)
		{
			// Get the underlying widget type
			KG_ASSERT(m_InteractState.m_SelectedWidget->m_WidgetType == WidgetTypes::SliderWidget);
			SliderWidget& activeSlider = *(SliderWidget*)m_InteractState.m_SelectedWidget;

			// Get the widget's transform
			Bounds widgetTransform = m_WindowsState.GetWidgetBoundsFromID(
				m_InteractState.m_SelectedWidget->m_ID, *viewportData);

			// Get the slider's current normalized location based on the bounds and currentValue
			float normalizedSliderLocation = (activeSlider.m_CurrentValue - activeSlider.m_Bounds.x) /
				(activeSlider.m_Bounds.y - activeSlider.m_Bounds.x);

			// Get slider bounding box
			Math::vec3 sliderSize{ 0.04f * widgetTransform.m_Size.x , 0.35f * widgetTransform.m_Size.y  , widgetTransform.m_Size.z };
			Math::vec3 sliderLocation{ widgetTransform.m_Translation.x + widgetTransform.m_Size.x * normalizedSliderLocation - (sliderSize.x / 2.0f), widgetTransform.m_Translation.y + (widgetTransform.m_Size.y / 2.0f) - (sliderSize.y / 2.0f), widgetTransform.m_Translation.z };

			// Check if the mouse click falls within the bounds of the slider
			if (mousePosition.x > (sliderLocation.x) && mousePosition.x < (sliderLocation.x + sliderSize.x) &&
				mousePosition.y >(sliderLocation.y) && mousePosition.y < (sliderLocation.y + sliderSize.y))
			{
				m_InteractState.m_PressedWidget = m_InteractState.m_SelectedWidget;
			}
		}

		else if (m_InteractState.m_SelectedWidget && m_InteractState.m_SelectedWidget->m_WidgetType == WidgetTypes::DropDownWidget)
		{
			// Get the underlying widget type
			KG_ASSERT(m_InteractState.m_SelectedWidget->m_WidgetType == WidgetTypes::DropDownWidget);
			DropDownWidget& activeDropDown = *(DropDownWidget*)m_InteractState.m_SelectedWidget;

			// Get the widget's transform
			Bounds widgetTransform = m_WindowsState.GetWidgetBoundsFromID(
				m_InteractState.m_SelectedWidget->m_ID, *viewportData);

			// Check if the mouse position is within bounds of the current option
			if (mousePosition.x > widgetTransform.m_Translation.x && mousePosition.x < (widgetTransform.m_Translation.x + widgetTransform.m_Size.x) &&
				mousePosition.y > widgetTransform.m_Translation.y && mousePosition.y < (widgetTransform.m_Translation.y + widgetTransform.m_Size.y))
			{
				// Toggle the drop down begin open if we click the current option
				Utility::Operations::ToggleBoolean(activeDropDown.m_DropDownOpen);
				return;
			}

			// Draw drop-down options
			if (activeDropDown.m_DropDownOpen)
			{
				// This variable represents the *visible* drop-down position/index
				// Note: IT IS NOT THE INDEX INTO THE DROPDOWNOPTIONS VECTOR
				size_t visibleDropDownOffset{ 0 };
				for (size_t iteration{ 0 }; iteration < activeDropDown.m_DropDownOptions.size(); iteration++)
				{
					// Exclude the current option
					if (activeDropDown.m_CurrentOption == iteration)
					{
						continue;
					}

					// Create translation for the current option
					Math::vec2 currentOptionTranslation
					{
						widgetTransform.m_Translation.x,
						widgetTransform.m_Translation.y - widgetTransform.m_Size.y * (float)(visibleDropDownOffset + 1)
					};

					// Check if the mouse position is within bounds of the current option
					if (mousePosition.x > currentOptionTranslation.x &&
						mousePosition.x < (currentOptionTranslation.x + widgetTransform.m_Size.x) &&
						mousePosition.y > currentOptionTranslation.y &&
						mousePosition.y < (currentOptionTranslation.y + widgetTransform.m_Size.y))
					{
						// Turn off the drop-down open boolean and set the current option to this windowIteration
						activeDropDown.m_DropDownOpen = false;
						activeDropDown.m_CurrentOption = iteration;
						// Call the select option function if applicable
						if (activeDropDown.m_OnSelectOption)
						{
							const SingleLineTextData& currentTextData = activeDropDown.m_DropDownOptions.at(iteration);
							Utility::CallWrapped<WrappedVoidString>
								(
									activeDropDown.m_OnSelectOption->m_Function,
									currentTextData.m_Text
								);
						}
						return;
					}
					visibleDropDownOffset++;
				}
			}
		}
	}

	void UserInterface::OnMouseButtonReleasedEvent(const Events::MouseButtonReleasedEvent& /*mouseEvent*/)
	{
		if (m_InteractState.m_PressedWidget)
		{
			// Clear the pressed widget
			m_InteractState.m_PressedWidget = nullptr;
		}
	}

	void UserInterface::OnRenderCamera(const Math::mat4& cameraViewMatrix, ViewportData viewportData)
	{
		KG_PROFILE_FUNCTION();
		KG_ASSERT(i_ParentContext);

		// Reset the rendering context
		Rendering::RendererAPI::ClearDepthBuffer();

		// Start rendering context
		Rendering::RenderingService::BeginScene(cameraViewMatrix);

		m_WindowsState.OnRender(i_ParentContext, viewportData);

		// End rendering context and submit rendering data to GPU
		Rendering::RenderingService::EndScene();
	}

	void UserInterface::OnRenderViewport(ViewportData viewportData)
	{
		// Calculate orthographic projection matrix for user interface
		Math::mat4 orthographicProjection = glm::ortho
		(
			0.0f, (float)viewportData.m_Width,
			0.0f, (float)viewportData.m_Height,
			-1.0f, 1.0f
		);
		OnRenderCamera(orthographicProjection, viewportData);
	}

	void UserInterface::MoveRight()
	{
		// Ensure the selected widget is valid
		if (!m_InteractState.m_SelectedWidget || !m_InteractState.m_ActiveWindow)
		{
			return;
		}

		// Get the selected widget's selection data
		SelectionData* originalSelectionData = m_InteractState.m_SelectedWidget->GetSelectionData();
		KG_ASSERT(originalSelectionData);

		// Move to the right
		if (originalSelectionData->m_NavigationLinks.m_RightWidgetID != k_InvalidWidgetID)
		{
			// Set the new selected widget
			m_InteractState.m_SelectedWidget = m_WindowsState.GetWidgetFromID(originalSelectionData->m_NavigationLinks.m_RightWidgetID).get();

			// Call the on move function if applicable
			if (m_Config.m_FunctionPointers.m_OnMove)
			{
				Utility::CallWrapped<WrappedVoidNone>(m_Config.m_FunctionPointers.m_OnMove->m_Function);
			}

			// Handle modifying the editing widget
			if (m_InteractState.m_EditingWidget != m_InteractState.m_SelectedWidget)
			{
				m_InteractState.ClearEditingWidget();
			}
		}
	}

	void UserInterface::MoveLeft()
	{
		// Ensure the selected widget is valid
		if (!m_InteractState.m_SelectedWidget || !m_InteractState.m_ActiveWindow)
		{
			return;
		}

		// Get the selected widget's selection data
		SelectionData* originalSelectionData = m_InteractState.m_SelectedWidget->GetSelectionData();
		KG_ASSERT(originalSelectionData);

		// Move to the left
		if (originalSelectionData->m_NavigationLinks.m_LeftWidgetID != k_InvalidWidgetID)
		{

			// Set the new selected widget
			m_InteractState.m_SelectedWidget = m_WindowsState.GetWidgetFromID(originalSelectionData->m_NavigationLinks.m_LeftWidgetID).get();

			// Call the on move function if applicable
			if (m_Config.m_FunctionPointers.m_OnMove)
			{
				Utility::CallWrapped<WrappedVoidNone>(m_Config.m_FunctionPointers.m_OnMove->m_Function);
			}

			// Handle modifying the editing widget
			if (m_InteractState.m_EditingWidget != m_InteractState.m_SelectedWidget)
			{
				m_InteractState.ClearEditingWidget();
			}
		}
	}

	void UserInterface::MoveUp()
	{
		// Ensure the selected widget is valid
		if (!m_InteractState.m_SelectedWidget || !m_InteractState.m_ActiveWindow)
		{
			return;
		}

		// Get the selected widget's selection data
		SelectionData* originalSelectionData = m_InteractState.m_SelectedWidget->GetSelectionData();
		KG_ASSERT(originalSelectionData);

		// Move up
		if (originalSelectionData->m_NavigationLinks.m_UpWidgetID != k_InvalidWidgetID)
		{
			// Set the new selected widget
			m_InteractState.m_SelectedWidget = m_WindowsState.GetWidgetFromID(originalSelectionData->m_NavigationLinks.m_UpWidgetID).get();

			// Get the new widget's selection data
			SelectionData* newSelectionData = m_InteractState.m_SelectedWidget->GetSelectionData();
			KG_ASSERT(newSelectionData);

			// Call the on move function if applicable
			if (m_Config.m_FunctionPointers.m_OnMove)
			{
				Utility::CallWrapped<WrappedVoidNone>(m_Config.m_FunctionPointers.m_OnMove->m_Function);
			}

			// Handle modifying the editing widget
			if (m_InteractState.m_EditingWidget != m_InteractState.m_SelectedWidget)
			{
				m_InteractState.ClearEditingWidget();
			}
		}
	}

	void UserInterface::MoveDown()
	{
		// Ensure the selected widget is valid
		if (!m_InteractState.m_SelectedWidget || !m_InteractState.m_ActiveWindow)
		{
			return;
		}

		// Get the selected widget's selection data
		SelectionData* originalSelectionData = m_InteractState.m_SelectedWidget->GetSelectionData();
		KG_ASSERT(originalSelectionData);

		// Move down
		if (originalSelectionData->m_NavigationLinks.m_DownWidgetID != k_InvalidWidgetID)
		{
			// Set the new selected widget
			m_InteractState.m_SelectedWidget = m_WindowsState.GetWidgetFromID(originalSelectionData->m_NavigationLinks.m_DownWidgetID).get();

			// Call the on move function if applicable
			if (m_Config.m_FunctionPointers.m_OnMove)
			{
				Utility::CallWrapped<WrappedVoidNone>(m_Config.m_FunctionPointers.m_OnMove->m_Function);
			}

			// Handle modifying the editing widget
			if (m_InteractState.m_EditingWidget != m_InteractState.m_SelectedWidget)
			{
				m_InteractState.ClearEditingWidget();
			}
		}
	}

	void UserInterface::OnPress()
	{
		Widget* currentWidget = m_InteractState.m_SelectedWidget;

		// Ensure selected widget is valid
		if (!currentWidget)
		{
			return;
		}

		OnPressInternal(currentWidget);
	}

	void UserInterface::OnPressByIndex(WidgetHandle widgetHandle)
	{
		// Get the current widget
		Ref<Widget> currentWidget = m_WindowsState.GetWidgetFromID(widgetHandle.m_WidgetID);

		if (!currentWidget)
		{
			return;
		}

		OnPressInternal(currentWidget.get());
	}

	void UserInterface::OnPressInternal(Widget* currentWidget)
	{
		// Get the selection specific data from the widget
		SelectionData* selectionData = currentWidget->GetSelectionData();
		if (!selectionData)
		{
			KG_WARN("Unable to retrieve selection data. May be invalid widget type!");
			return;
		}

		// Ensure the indicated widget is selectable
		if (!currentWidget->Selectable())
		{
			return;
		}

		// Set the pressed widget to be selected
		m_InteractState.m_SelectedWidget = currentWidget;

		// Call the on press function if applicable
		if (currentWidget->m_WidgetType == WidgetTypes::CheckboxWidget)
		{
			// Handle case for checkbox
			CheckboxWidget& checkboxWidget = *(CheckboxWidget*)currentWidget;
			Utility::Operations::ToggleBoolean(checkboxWidget.m_Checked);

			if (selectionData->m_FunctionPointers.m_OnPress)
			{
				Utility::CallWrapped<WrappedVoidBool>(selectionData->m_FunctionPointers.m_OnPress->m_Function, checkboxWidget.m_Checked);
			}
		}
		else if (currentWidget->m_WidgetType == WidgetTypes::InputTextWidget)
		{
			// Handle the input text widget
			if (selectionData->m_FunctionPointers.m_OnPress)
			{
				Utility::CallWrapped<WrappedVoidNone>(selectionData->m_FunctionPointers.m_OnPress->m_Function);
			}

			// Set the current widget as the editing widget
			m_InteractState.m_EditingWidget = currentWidget;
		}
		else
		{
			// Handle all other cases
			if (!selectionData->m_FunctionPointers.m_OnPress)
			{
				return;
			}
			Utility::CallWrapped<WrappedVoidNone>(selectionData->m_FunctionPointers.m_OnPress->m_Function);
		}
	}

	void UserInterface::OnMoveCursorInternal(Widget* currentWidget)
	{
		// Ensure the widget is the correct type and get it
		KG_ASSERT(currentWidget->m_WidgetType == WidgetTypes::InputTextWidget);
		InputTextWidget* inputTextWidget = (InputTextWidget*)currentWidget;

		// Call on move cursor if available
		if (inputTextWidget->m_OnMoveCursor)
		{
			Utility::CallWrapped<WrappedVoidNone>(inputTextWidget->m_OnMoveCursor->m_Function);
		}
	}

	void UIConfig::SetFont(Ref<Font> newFont, Assets::AssetHandle fontHandle)
	{
		// Set the active font for the active user interface
		m_Font = newFont;
		m_FontHandle = fontHandle;

		i_WindowsState->RevalidateTextDimensions();
	}

	void UIConfig::SetOnMove(Assets::AssetHandle functionHandle, Ref<Scripting::Script> function)
	{
		m_FunctionPointers.m_OnMove = function;
		m_FunctionPointers.m_OnMoveHandle = functionHandle;
	}

	void UIConfig::SetOnHover(Assets::AssetHandle functionHandle, Ref<Scripting::Script> function)
	{
		m_FunctionPointers.m_OnHover = function;
		m_FunctionPointers.m_OnHoverHandle = functionHandle;
	}

	void UIInteractionState::ClearSelectedWidget()
	{
		// Set the new widget as selected and set it's color to the active color
		m_SelectedWidget = nullptr;
	}

	void UIInteractionState::ClearHoveredWidget()
	{
		if (!m_HoveredWidget)
		{
			return;
		}

		m_HoveredWidget = nullptr;

		// Reset the cursor icon
		EngineService::GetActiveEngine().GetWindow().SetMouseCursorIcon(CursorIconType::Standard);
	}

	void UIInteractionState::ClearEditingWidget()
	{
		m_EditingWidget = nullptr;
	}

	void UIInteractionState::SetSelectedWidgetByTag(std::string_view windowTag, std::string_view widgetTag)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = i_WindowsState->GetWidgetFromTag(windowTag, widgetTag);

		SetSelectedWidgetInternal(currentWidget);
	}

	void UIInteractionState::SetSelectedWidgetByHandle(WidgetHandle widgetHandle)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = i_WindowsState->GetWidgetFromID(widgetHandle.m_WidgetID);

		SetSelectedWidgetInternal(currentWidget);
	}

	void UIInteractionState::SetHoveredWidgetByHandle(WidgetHandle widgetHandle)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = i_WindowsState->GetWidgetFromID(widgetHandle.m_WidgetID);

		SetHoveredWidgetInternal(currentWidget);
	}

	void UIInteractionState::SetEditingWidgetByHandle(WidgetHandle widgetHandle)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = i_WindowsState->GetWidgetFromID(widgetHandle.m_WidgetID);

		// Ensure the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget as edited");
			return;
		}

		// Ensure the widget is selectable
		if (!currentWidget->Selectable())
		{
			return;
		}

		// If we do not have the correct type, just clear the editing widget
		if (currentWidget->m_WidgetType != WidgetTypes::InputTextWidget)
		{
			ClearEditingWidget();
			return;
		}

		// If the indicated widget is the same as the current editing widget, just exit
		if (currentWidget.get() == m_EditingWidget)
		{
			return;
		}

		// Set the new widget as editing
		m_EditingWidget = currentWidget.get();
	}

	void UIInteractionState::SetSelectedWidgetColor(const Math::vec4& color)
	{
		if (!m_SelectedWidget)
		{
			return;
		}

		// Get the selection specific data from the widget
		SelectionData* selectionData = m_SelectedWidget->GetSelectionData();
		if (!selectionData)
		{
			KG_WARN("Unable to retrieve selection data. May be invalid widget type!");
			return;
		}

		// Set the widget's active color
		selectionData->m_DefaultBackgroundColor = color;
	}

	void UIWindowsState::SetActiveWidgetTextByTag(std::string_view windowTag, std::string_view widgetTag, std::string_view newText)
	{
		// Search for the indicated widget
		ResultWidgetWindow result = GetWidgetAndWindow(windowTag, widgetTag);
		SetWidgetTextInternal(result.m_Widget, newText);
	}

	void UIWindowsState::SetActiveWidgetTextByHandle(WidgetHandle widgetHandle, std::string_view newText)
	{
		// Search for the indicated widget
		ResultWidgetWindow result = GetWidgetAndWindow(widgetHandle.m_WidgetID);
		SetWidgetTextInternal(result.m_Widget, newText);
	}

	void UIWindowsState::RevalidateDisplayedWindows()
	{
		// Ensure/validate that the correct window is being displayed
		m_DisplayedWindows.clear();
		m_DisplayedWindowIndices.clear();
		WindowIndex windowIndex{ 0 };
		for (Window& window : m_Windows)
		{
			// Add the window to the displayed windows if it is flagged to do so
			if (window.GetWindowDisplayed())
			{
				m_DisplayedWindows.push_back(&window);
				m_DisplayedWindowIndices.push_back(windowIndex);
			}
			windowIndex++;
		}
	}

	void UIWindowsState::RevalidateIDToLocationMap()
	{
		m_IDToLocation.clear();

		// Parse through each window
		WindowIndex windowIndex{ 0 };
		for (Window& window : m_Windows)
		{
			// Create the window's location and add it to the locationMap
			UILocation windowLocation{ (uint16_t)windowIndex };
			auto [iter, success] = m_IDToLocation.insert_or_assign(window.m_ID, std::move(windowLocation));
			KG_ASSERT(success);

			// Parse through each window's widget (Window -> ChildWidget)
			WidgetIndex widgetIndex{ 0 };
			for (Ref<Widget> currentWidget : window.m_Widgets)
			{
				// Create the widget's location and add it to the locationMap
				UILocation widgetLocation{ windowIndex, widgetIndex };
				auto [iter, success] = m_IDToLocation.insert_or_assign(currentWidget->m_ID, widgetLocation);
				KG_ASSERT(success);

				// Handle the case where the widget is a container
				ContainerData* container = currentWidget->GetContainerData();
				if (container)
				{
					// Recursively handle this container's widgets and all of its sub-containers
					RevalidateContainerInLocationMap(container, widgetLocation);
				}
				widgetIndex++;
			}
			windowIndex++;
		}
	}

	void UIWindowsState::RevalidateTextDimensions()
	{
		// Revalidate/calculate text sizes for all windows
		for (Window& window : m_Windows)
		{
			for (Ref<Widget> widget : window.m_Widgets)
			{
				widget->RevalidateTextDimensions();

				// Ensure the default cursor position is at the end of the text
				SingleLineTextData* textData = widget->GetSingleLineTextData();
				if (textData)
				{
					textData->m_CaretIndex = textData->m_Text.size();
				}
			}
		}
	}

	void UIWindowsState::RevalidateDefaultWidgets()
	{
		// Handle window specific set-up
		for (Window& window : m_Windows)
		{
			// Set the default widget for each window
			if (window.m_DefaultActiveWidget != RuntimeUI::k_InvalidWidgetID)
			{
				Ref<RuntimeUI::Widget> newDefaultWidget = GetWidgetFromID(window.m_DefaultActiveWidget);
				KG_ASSERT(newDefaultWidget);
				window.m_DefaultActiveWidgetRef = newDefaultWidget;
			}
		}
	}

	void UIWindowsState::RevalidateNavigationLinks()
	{
		NavigationLinksCalculator newCalculator
		{
			this,
			EngineService::GetActiveEngine().GetWindow().GetActiveViewport()
		};
		newCalculator.CalculateNavigationLinks();
	}

	void UIWindowsState::RevalidateContainerInLocationMap(ContainerData* container, UILocation& parentLocation)
	{
		KG_ASSERT(container);
		KG_ASSERT(parentLocation.size() > 1);

		WidgetIndex widgetIndex{ 0 };
		for (Ref<Widget> childWidget : container->m_ContainedWidgets)
		{
			// Add the current widget index to the location and insert the location
			parentLocation.push_back(widgetIndex);
			auto [iter, success] = m_IDToLocation.insert_or_assign(childWidget->m_ID, parentLocation);
			KG_ASSERT(success);

			// Handle the case where the childWidget is a container
			ContainerData* container = childWidget->GetContainerData();
			if (container)
			{
				// Recursive call to handle all containers
				RevalidateContainerInLocationMap(container, parentLocation);
			}

			// Clean up the location and continue for all other child widgets
			widgetIndex++;
			parentLocation.pop_back();
		}
	}

	Ref<Widget> UIWindowsState::GetWidgetFromTag(std::string_view windowTag, std::string_view widgetTag)
	{
		// Get widget using its parent window tag and its widget tag
		for (Window& window : m_Windows)
		{
			// Ensure window tag matches
			if (window.m_Tag == windowTag)
			{
				// Ensure widget tag matches
				for (Ref<Widget> widget : window.m_Widgets)
				{
					if (widget->m_Tag == widgetTag)
					{
						return widget;
					}
				}
			}
		}
		return nullptr;
	}

	Ref<Widget> UIWindowsState::GetWidgetFromID(WidgetID widgetHandle)
	{
		KG_ASSERT(m_IDToLocation.contains(widgetHandle));
		UILocation& locationDirections = m_IDToLocation.at(widgetHandle);

		// Ensure directions to a widget a presented
		KG_ASSERT(locationDirections.size() > 1);

		WindowIndex windowIndex = (WindowIndex)locationDirections.at(0);
		WidgetIndex firstWidgetIndex = (WidgetIndex)locationDirections.at(1);

		// Ensure window index is within bounds
		if (windowIndex > (m_Windows.size() - 1))
		{
			KG_WARN("Attempt to retrieve a widget but the window index was out of bounds");
			return nullptr;
		}
		// Get the indiciated window
		Window& currentWindow = m_Windows.at((size_t)windowIndex);

		// Ensure widget index is within bounds
		if (firstWidgetIndex > (currentWindow.m_Widgets.size() - 1))
		{
			KG_WARN("Attempt to retrieve a widget but the widget index was out of bounds");
			return nullptr;
		}

		// Get the first widget
		Ref<RuntimeUI::Widget> currentWidget = currentWindow.m_Widgets.at(firstWidgetIndex);

		// Handle the case where this only one window and one widget (Window -> Widget)
		if (locationDirections.size() == 2)
		{
			// Return the indicated widget
			return currentWidget;
		}

		// Window -> Widget -> Widget (Container Data) -> Widget (Container Data)
		// Handle the case where there is more nesting
		for (WidgetIndex iteration{ 2 }; iteration < locationDirections.size(); iteration++)
		{
			// Get the container data for this widget
			ContainerData* data = currentWidget->GetContainerData();
			KG_ASSERT(data);

			// Ensure the widget directions fall within the bounds of the widget array
			WidgetIndex currentIndex = (WidgetIndex)locationDirections.at(iteration);
			KG_ASSERT(currentIndex < data->m_ContainedWidgets.size());

			// Set the current widget to the indicated container widget (Window -> Widget -> [Widget])
			currentWidget = data->m_ContainedWidgets.at(currentIndex);
		}

		// Return the nested widget
		return currentWidget;
	}

	Ref<Widget> UIWindowsState::GetWidgetFromDirections(const UILocation& locationDirections)
	{
		// Ensure directions to a widget a presented
		KG_ASSERT(locationDirections.size() > 1);

		// Get widget using location directions
		KG_ASSERT(locationDirections.size() == 2); // TODO: Remove this later
		WindowIndex windowIndex = (WindowIndex)locationDirections.at(0);
		WidgetIndex widgetIndex = (WidgetIndex)locationDirections.at(1);

		// Ensure window index is within bounds
		if (windowIndex > (m_Windows.size() - 1))
		{
			KG_WARN("Attempt to retrieve a widget but the window index was out of bounds");
			return nullptr;
		}

		// Get the indicated window
		Window& currentWindow = m_Windows.at((size_t)windowIndex);

		// Ensure widget index is within bounds
		if (widgetIndex > (currentWindow.m_Widgets.size() - 1))
		{
			KG_WARN("Attempt to retrieve a widget but the widget index was out of bounds");
			return nullptr;
		}

		// Return the indicated widget and window
		return currentWindow.m_Widgets.at(widgetIndex);
	}

	IDType UIWindowsState::CheckIDType(WidgetOrWindowID windowOrWidgetID)
	{
		// Check for invalid ID
		if (!m_IDToLocation.contains(windowOrWidgetID) || windowOrWidgetID == k_InvalidWidgetID)
		{
			return IDType::None;
		}

		UILocation& locationDirections = m_IDToLocation.at(windowOrWidgetID);

		// Discern whether the ID is a window or widget type
		if (locationDirections.size() > 1)
		{
			return IDType::Widget;
		}
		else
		{
			return IDType::Window;
		}
	}

	bool UIInteractionState::IsWidgetSelectedInternal(Ref<Widget> currentWidget)
	{
		// Check if the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when checking the widget is selected");
			return false;
		}

		// Return if the widget is selected
		return m_SelectedWidget == currentWidget.get();
	}

	Window& UIWindowsState::GetWindowFromID(WindowID windowID)
	{
		// Get the window location
		UILocation* locationDir = GetLocationFromID(windowID);

		// Ensure the location is valid and represents a window
		KG_ASSERT(locationDir);
		KG_ASSERT(locationDir->size() == 1);
		uint16_t windowLocation{ locationDir->at(0) };

		// Ensure the window location leads to a valid window instance
		KG_ASSERT(windowLocation < m_Windows.size());

		// Return the window
		return m_Windows.at(windowLocation);
	}

	Window& UIWindowsState::GetParentWindowFromWidgetID(WidgetID widgetID)
	{
		// Get the widget location
		UILocation* locationDir = GetLocationFromID(widgetID);

		// Ensure the location is valid and represents a widget
		KG_ASSERT(locationDir);
		KG_ASSERT(locationDir->size() > 1);
		uint16_t windowLocation{ locationDir->at(0) };

		// Ensure the window location leads to a valid window instance
		KG_ASSERT(windowLocation < m_Windows.size());

		// Return the window
		return m_Windows.at(windowLocation);
	}

	Ref<Widget> UIWindowsState::GetParentWidgetFromID(WidgetID widgetID)
	{
		// Get the widget location
		UILocation* locationDir = GetLocationFromID(widgetID);

		// Ensure the location is valid and represents a widget
		KG_ASSERT(locationDir);
		KG_ASSERT(locationDir->size() > 1);

		// If the parent of the widget is a window (UI->Window(1)->Widget(2)), return null
		if (locationDir->size() == 2)
		{
			return nullptr;
		}

		// Get the location directions for the parent widget
		UILocation parentLocationDir = *locationDir;
		parentLocationDir.pop_back();

		// Return the parent widget
		return GetWidgetFromDirections(parentLocationDir);
	}

	UILocation* UIWindowsState::GetLocationFromID(WidgetOrWindowID windowOrWidgetID)
	{
		// Get the ID -> Widget Location map and location directions
		// Check for invalid ID
		if (!m_IDToLocation.contains(windowOrWidgetID) || windowOrWidgetID == k_InvalidWidgetID)
		{
			return nullptr;
		}

		return &(m_IDToLocation.at(windowOrWidgetID));
	}

	ResultWidgetWindow UIWindowsState::GetWidgetAndWindow(std::string_view windowTag, std::string_view widgetTag)
	{
		// Get widget using its parent window tag and its widget tag
		for (Window& window : m_Windows)
		{
			// Ensure window tag matches
			if (window.m_Tag == windowTag)
			{
				// Ensure widget tag matches
				for (Ref<Widget> widget : window.m_Widgets)
				{
					if (widget->m_Tag == widgetTag)
					{
						return { widget, &window };
					}
				}
			}
		}
		return { nullptr, nullptr };
	}

	ResultWidgetWindow UIWindowsState::GetWidgetAndWindow(WidgetID widgetID)
	{
		KG_ASSERT(m_IDToLocation.contains(widgetID));
		UILocation& locationDirections = m_IDToLocation.at(widgetID);

		// Check for a window ID
		if (locationDirections.size() == 1)
		{
			KG_WARN("Attempt to retrieve a window and widget, however, found only a window ID");
			return { nullptr, nullptr };
		}

		// Get widget using location directions
		// TODO: This currently assumes the window -> widget structure
		KG_ASSERT(locationDirections.size() == 2); // TODO: Remove this later
		WindowIndex windowIndex = (WindowIndex)locationDirections.at(0);
		WidgetIndex widgetIndex = (WidgetIndex)locationDirections.at(1);

		// Ensure window index is within bounds
		if (windowIndex > (m_Windows.size() - 1))
		{
			KG_WARN("Attempt to retrieve a widget but the window index was out of bounds");
			return { nullptr, nullptr };
		}
		// Get the indiciated window
		Window& currentWindow = m_Windows.at((size_t)windowIndex);

		// Ensure widget index is within bounds
		if (widgetIndex > (currentWindow.m_Widgets.size() - 1))
		{
			KG_WARN("Attempt to retrieve a widget but the widget index was out of bounds");
			return { nullptr, nullptr };
		}

		// Return the indicated widget and window
		return { currentWindow.m_Widgets.at(widgetIndex), &currentWindow };
	}


	std::string_view UIWindowsState::GetWidgetTextByHandle(WidgetHandle widgetHandle)
	{
		// Search for the indicated widget
		Ref<Widget> widget = GetWidgetFromID(widgetHandle.m_WidgetID);

		KG_ASSERT(widget);

		SingleLineTextData* singleLineTextData = widget->GetSingleLineTextData();
		if (singleLineTextData)
		{
			return singleLineTextData->m_Text;
		}

		MultiLineTextData* multiLineTextData = widget->GetMultiLineTextData();
		if (multiLineTextData)
		{
			return multiLineTextData->m_Text;
		}

		KG_WARN("Provide widget does not yield a text data struct when attempting to get text");
		return {};
	}

	bool UIInteractionState::IsWidgetSelectedByTag(std::string_view windowTag, std::string_view widgetTag)
	{
		// Get the current widget
		Ref<Widget> currentWidget = i_WindowsState->GetWidgetFromTag(windowTag, widgetTag);

		return IsWidgetSelectedInternal(currentWidget);
	}

	bool UIInteractionState::IsWidgetSelectedByHandle(WidgetHandle widgetHandle)
	{
		// Get the current widget
		Ref<Widget> currentWidget = i_WindowsState->GetWidgetFromID(widgetHandle.m_WidgetID);

		return IsWidgetSelectedInternal(currentWidget);
	}

	Bounds UIWindowsState::GetParentBoundsFromID(WidgetID widgetID, ViewportData viewportData)
	{
		KG_ASSERT(m_IDToLocation.contains(widgetID));
		UILocation& locationDirections = m_IDToLocation.at(widgetID);

		// Ensure directions to a widget a presented
		KG_ASSERT(locationDirections.size() > 1);

		// Get the first two entries (Window -> Widget)
		WindowIndex windowIndex = (WindowIndex)locationDirections.at(0);
		WidgetIndex firstWidgetIndex = (WidgetIndex)locationDirections.at(1);

		// Get the widget's parent window
		KG_ASSERT(windowIndex < m_Windows.size());
		Window& parentWindow = m_Windows.at(windowIndex);

		// Calculate the parent window's dimensions
		Bounds returnDimensions;
		returnDimensions.m_Translation = parentWindow.GetLowerCornerPosition(viewportData);
		returnDimensions.m_Size = parentWindow.GetSize(viewportData);

		// Return the parent dimensions if the location only involves a single widget (Ex: Window -> Widget)
		if (locationDirections.size() == 2)
		{
			return returnDimensions;
		}

		// Get the first widget
		KG_ASSERT(firstWidgetIndex < parentWindow.m_Widgets.size());
		Ref<Widget> currentWidget = parentWindow.m_Widgets.at(firstWidgetIndex);

		// Continue to calculate the dimensions for a longer widget chain (Ex: Window -> Widget -> ...)
		for (WidgetIndex widgetIndex{ 2 }; widgetIndex < locationDirections.size(); widgetIndex++)
		{
			// Update the dimensions based on the current widget and it's parent's dimensions
			returnDimensions.m_Translation = currentWidget->CalculateWorldPosition(returnDimensions.m_Translation, returnDimensions.m_Size);
			returnDimensions.m_Size = currentWidget->CalculateWidgetSize(returnDimensions.m_Size);

			// Handle the horizontal container case
			if (currentWidget->m_WidgetType == WidgetTypes::HorizontalContainerWidget)
			{
				HorizontalContainerWidget* horContainer = (HorizontalContainerWidget*)currentWidget.get();
				KG_ASSERT(horContainer);
				returnDimensions.m_Translation.x +=
					(returnDimensions.m_Size.x * horContainer->m_ColumnWidth) * locationDirections.at(widgetIndex) +
					(returnDimensions.m_Size.x * horContainer->m_ColumnSpacing) * locationDirections.at(widgetIndex);
				returnDimensions.m_Size.x *= horContainer->m_ColumnWidth;
			}

			// Handle the vertical container case
			else if (currentWidget->m_WidgetType == WidgetTypes::VerticalContainerWidget)
			{
				VerticalContainerWidget* vertContainer = (VerticalContainerWidget*)currentWidget.get();
				KG_ASSERT(vertContainer);
				returnDimensions.m_Translation.y += returnDimensions.m_Size.y -
					(returnDimensions.m_Size.y * vertContainer->m_RowHeight) * (locationDirections.at(widgetIndex) + 1) -
					(returnDimensions.m_Size.y * vertContainer->m_RowSpacing) * locationDirections.at(widgetIndex);
				returnDimensions.m_Size.y *= vertContainer->m_RowHeight;
			}

			// Exit early if we reach the final valid location direction
			if (widgetIndex == locationDirections.size() - 1)
			{
				break;
			}

			// Get the container data from the active widget
			ContainerData* currentData = currentWidget->GetContainerData();
			KG_ASSERT(currentData);

			// Get the current widget index and set the next parent widget
			WidgetIndex currentContainerIndex = locationDirections.at(widgetIndex);
			KG_ASSERT(currentContainerIndex < currentData->m_ContainedWidgets.size());
			currentWidget = currentData->m_ContainedWidgets.at(currentContainerIndex);
		}

		// Return the dimensions
		return returnDimensions;
	}

	Bounds UIWindowsState::GetWidgetBoundsFromID(WidgetID widgetID, ViewportData viewportData)
	{
		KG_ASSERT(m_IDToLocation.contains(widgetID));
		UILocation& locationDirections = m_IDToLocation.at(widgetID);

		// Ensure directions to a widget a presented
		KG_ASSERT(locationDirections.size() > 1);

		// Get the first two entries (Window -> Widget)
		WindowIndex windowIndex = (WindowIndex)locationDirections.at(0);
		WidgetIndex firstWidgetIndex = (WidgetIndex)locationDirections.at(1);

		// Get the widget's parent window
		KG_ASSERT(windowIndex < m_Windows.size());
		Window& parentWindow = m_Windows.at(windowIndex);

		// Calculate the parent window's dimensions
		Bounds returnDimensions;
		returnDimensions.m_Translation = parentWindow.GetLowerCornerPosition(
			viewportData);
		returnDimensions.m_Size = parentWindow.GetSize(
			viewportData);

		// Get the first widget
		KG_ASSERT(firstWidgetIndex < parentWindow.m_Widgets.size());
		Ref<Widget> currentWidget = parentWindow.m_Widgets.at(firstWidgetIndex);

		// Return the first widget's dimensions if the location only involves a single widget (Ex: Window -> Widget)
		if (locationDirections.size() == 2)
		{
			returnDimensions.m_Translation = currentWidget->CalculateWorldPosition(returnDimensions.m_Translation, returnDimensions.m_Size);
			returnDimensions.m_Size = currentWidget->CalculateWidgetSize(returnDimensions.m_Size);
			return  returnDimensions;
		}

		// Continue to calculate the dimensions for a longer widget chain (Ex: Window -> Widget -> ...)
		for (WidgetIndex widgetIndex{ 2 }; widgetIndex < locationDirections.size(); widgetIndex++)
		{
			// Update the dimensions based on the current widget and it's parent's dimensions
			returnDimensions.m_Translation = currentWidget->CalculateWorldPosition(returnDimensions.m_Translation, returnDimensions.m_Size);
			returnDimensions.m_Size = currentWidget->CalculateWidgetSize(returnDimensions.m_Size);


			// Handle the Horizontal container case
			if (currentWidget->m_WidgetType == WidgetTypes::HorizontalContainerWidget)
			{
				HorizontalContainerWidget* horizContainer = (HorizontalContainerWidget*)currentWidget.get();
				KG_ASSERT(horizContainer);
				returnDimensions.m_Translation.x +=
					returnDimensions.m_Size.x * horizContainer->m_ColumnWidth * locationDirections.at(widgetIndex) +
					returnDimensions.m_Size.x * horizContainer->m_ColumnSpacing * locationDirections.at(widgetIndex);
				returnDimensions.m_Size.x *= horizContainer->m_ColumnWidth;
			}
			// Handle the vertical container case
			else if (currentWidget->m_WidgetType == WidgetTypes::VerticalContainerWidget)
			{
				VerticalContainerWidget* vertContainer = (VerticalContainerWidget*)currentWidget.get();
				KG_ASSERT(vertContainer);
				returnDimensions.m_Translation.y += returnDimensions.m_Size.y -
					returnDimensions.m_Size.y * vertContainer->m_RowHeight * (locationDirections.at(widgetIndex) + 1) -
					returnDimensions.m_Size.y * vertContainer->m_RowSpacing * locationDirections.at(widgetIndex);
				returnDimensions.m_Size.y *= vertContainer->m_RowHeight;
			}

			// Get the container data from the active widget
			ContainerData* currentData = currentWidget->GetContainerData();
			KG_ASSERT(currentData);

			// Get the current widget index and set the next parent widget
			WidgetIndex currentContainerIndex = locationDirections.at(widgetIndex);
			KG_ASSERT(currentContainerIndex < currentData->m_ContainedWidgets.size());
			currentWidget = currentData->m_ContainedWidgets.at(currentContainerIndex);
		}

		// Do the final calculation
		returnDimensions.m_Translation = currentWidget->CalculateWorldPosition(returnDimensions.m_Translation, returnDimensions.m_Size);
		returnDimensions.m_Size = currentWidget->CalculateWidgetSize(returnDimensions.m_Size);

		// Return the dimensions
		return returnDimensions;
	}
	void UIInteractionState::Init(UIWindowsState* windowsState, UIConfig* config)
	{
		KG_ASSERT(windowsState);
		KG_ASSERT(config);
		i_WindowsState = windowsState;
		i_Config = config;

		// Initialize interaction state values
		std::vector<Window>& activeWindows{ i_WindowsState->m_Windows };
		if (activeWindows.size() > 0)
		{
			m_ActiveWindow = &activeWindows.at(0);

			// Set default widget as the selected widget if it exists
			if (m_ActiveWindow->m_DefaultActiveWidgetRef)
			{
				// Set the default widget
				m_SelectedWidget = m_ActiveWindow->m_DefaultActiveWidgetRef.get();

				// Ensure the widget is selectable
				KG_ASSERT(m_SelectedWidget->Selectable());
			}
			else
			{
				m_SelectedWidget = nullptr;
			}
		}

		m_Active = true;
	}
	void UIInteractionState::Terminate()
	{
		i_WindowsState = nullptr;
		i_Config = nullptr;

		m_Active = false;
	}
	void UIConfig::Init(UIWindowsState* windowsState, Ref<Font> defaultFont)
	{
		KG_ASSERT(windowsState);
		KG_ASSERT(defaultFont);
		i_WindowsState = windowsState;

		// Load default font if necessary
		if (!m_Font)
		{
			m_Font = defaultFont;
			m_FontHandle = Assets::EmptyHandle;
		}

		m_Active = true;
	}
	void UIConfig::Terminate()
	{
		i_WindowsState = nullptr;

		m_Active = false;
	}
}
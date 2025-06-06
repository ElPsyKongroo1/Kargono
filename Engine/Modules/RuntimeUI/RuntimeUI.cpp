#include "kgpch.h"

#include "Modules/RuntimeUI/RuntimeUI.h"

#include "Modules/Assets/AssetService.h"
#include "Modules/Core/Engine.h"
#include "Kargono/Core/Window.h"
#include "Kargono/Projects/Project.h"
#include "Modules/Rendering/RenderingService.h"
#include "Modules/Rendering/Shader.h"
#include "Modules/ECS/EngineComponents.h"
#include "Kargono/Utility/Operations.h"
#include "Kargono/Math/Interpolation.h"
#include "Modules/EditorUI/EditorUI.h"
#include "Modules/Input/InputService.h"

namespace Kargono::Utility
{
	Ref<Rendering::Texture2D> WidgetTypeToIcon(RuntimeUI::WidgetTypes widgetType)
	{
		switch (widgetType)
		{
		case RuntimeUI::WidgetTypes::TextWidget:
			return EditorUI::EditorUIService::s_IconTextWidget;
		case RuntimeUI::WidgetTypes::ButtonWidget:
			return EditorUI::EditorUIService::s_IconButtonWidget;
		case RuntimeUI::WidgetTypes::ImageWidget:
			return EditorUI::EditorUIService::s_IconTexture;
		case RuntimeUI::WidgetTypes::ImageButtonWidget:
			return EditorUI::EditorUIService::s_IconImageButtonWidget;
		case RuntimeUI::WidgetTypes::CheckboxWidget:
			return EditorUI::EditorUIService::s_IconCheckbox_Enabled;
		case RuntimeUI::WidgetTypes::ContainerWidget:
			return EditorUI::EditorUIService::s_IconBoxCollider;
		case RuntimeUI::WidgetTypes::HorizontalContainerWidget:
			return EditorUI::EditorUIService::s_IconHorizontalContainer;
		case RuntimeUI::WidgetTypes::VerticalContainerWidget:
			return EditorUI::EditorUIService::s_IconVerticalContainer;
		case RuntimeUI::WidgetTypes::InputTextWidget:
			return EditorUI::EditorUIService::s_IconInputTextWidget;
		case RuntimeUI::WidgetTypes::DropDownWidget:
			return EditorUI::EditorUIService::s_IconDropDownWidget;
		case RuntimeUI::WidgetTypes::SliderWidget:
			return EditorUI::EditorUIService::s_IconSliderWidget;
		default:
			KG_ERROR("Invalid widget type provided");
			return nullptr;
		}
	}
}

namespace Kargono::RuntimeUI
{
	void RuntimeUIContext::Init()
	{
		// Initialize Runtime UI Context
		m_ActiveUI = nullptr;
		m_ActiveUIHandle = Assets::EmptyHandle;
		m_DefaultFont = FontService::InstantiateEditorFont("Resources/Fonts/arial.ttf");

		// Initialize Window/Widget background Rendering Data
		{
			// Create shader for UI background/quad rendering
			Rendering::ShaderSpecification shaderSpec {Rendering::ColorInputType::FlatColor, Rendering::TextureInputType::None, false, true, true, Rendering::RenderingType::DrawIndex, false};
			auto [uuid, localShader] = Assets::AssetService::GetShader(shaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>({ 1.0f, 1.0f, 1.0f, 1.0f }, 
				Utility::FileSystem::CRCFromString("a_Color"),
				localBuffer, localShader);

			// Create basic shape component for UI quad rendering
			ECS::ShapeComponent* shapeComp = new ECS::ShapeComponent();
			shapeComp->CurrentShape = Rendering::ShapeTypes::Quad;
			shapeComp->Vertices = CreateRef<std::vector<Math::vec3>>(Rendering::Shape::s_Quad.GetIndexVertices());
			shapeComp->Indices = CreateRef<std::vector<uint32_t>>(Rendering::Shape::s_Quad.GetIndices());

			m_BackgroundInputSpec.m_Shader = localShader;
			m_BackgroundInputSpec.m_Buffer = localBuffer;
			m_BackgroundInputSpec.m_ShapeComponent = shapeComp;
		}

		// Initialize texture rendering data
		{
			// Create shader for UI background/quad rendering
			Rendering::ShaderSpecification shaderSpec{ Rendering::ColorInputType::None, Rendering::TextureInputType::ColorTexture, false, true, true, Rendering::RenderingType::DrawIndex, false };
			auto [uuid, localShader] = Assets::AssetService::GetShader(shaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			// Create basic shape component for UI quad rendering
			ECS::ShapeComponent* shapeComp = new ECS::ShapeComponent();
			shapeComp->CurrentShape = Rendering::ShapeTypes::Quad;
			shapeComp->Vertices = CreateRef<std::vector<Math::vec3>>(Rendering::Shape::s_Quad.GetIndexVertices());
			shapeComp->Indices = CreateRef<std::vector<uint32_t>>(Rendering::Shape::s_Quad.GetIndices());
			shapeComp->TextureCoordinates = CreateRef<std::vector<Math::vec2>>(Rendering::Shape::s_Quad.GetIndexTextureCoordinates());
			shapeComp->Shader = localShader;
			shapeComp->Texture = nullptr;
			
			float* tilingFactor = Rendering::Shader::GetInputLocation<float>(
				Utility::FileSystem::CRCFromString("a_TilingFactor"), 
				localBuffer, localShader);
			*tilingFactor = 1.0f;

			m_ImageInputSpec.m_Shader = localShader;
			m_ImageInputSpec.m_Buffer = localBuffer;
			m_ImageInputSpec.m_ShapeComponent = shapeComp;
		}

		// Verify Initialization
		KG_VERIFY(true, "Runtime UI Engine Init");
	}

	void RuntimeUIContext::Terminate()
	{
		// Clear input spec data
		m_BackgroundInputSpec.ClearData();
		m_ImageInputSpec.ClearData();

		// Verify Termination
		KG_VERIFY(true, "Runtime UI Engine Terminate");
	}

	void RuntimeUIContext::OnUpdate(Timestep ts)
	{
		// Ensure a valid user interface is active and get it
		if (!m_ActiveUI)
		{
			return;
		}

		Ref<UserInterface> activeUI = m_ActiveUI;

		// Get mouse position and active viewport
		Math::vec2 mousePosition = Input::InputService::GetViewportMousePosition();
		ViewportData* viewportData = EngineService::GetActiveEngine().GetApp().GetViewportData();

		// Handle IBeam flipping for input text widgets
		if (activeUI->m_EditingWidget)
		{
			activeUI->m_IBeamAccumulator += ts;
			if (activeUI->m_IBeamAccumulator > activeUI->m_IBeamVisiblilityInterval)
			{
				activeUI->m_IBeamAccumulator -= activeUI->m_IBeamVisiblilityInterval;
				Utility::Operations::ToggleBoolean(activeUI->m_IBeamVisible);
			}
		}

		if (activeUI->m_PressedWidget)
		{
			// Get the widget's transform
			BoundingBoxTransform widgetTransform = GetWidgetDimensionsFromID(
				activeUI->m_PressedWidget->m_ID,
				viewportData->m_Width, viewportData->m_Height);

			// Get the underlying widget type
			KG_ASSERT(activeUI->m_PressedWidget->m_WidgetType == WidgetTypes::SliderWidget);
			SliderWidget& activeSlider = *(SliderWidget*)activeUI->m_PressedWidget;

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

	bool RuntimeUIContext::OnKeyTypedEvent(Events::KeyTypedEvent event)
	{
		// Ensure a valid user interface is active
		if (!m_ActiveUI)
		{
			return false;
		}

		// Pass in key typed events for the current widget to be edited
		if (m_ActiveUI->m_EditingWidget)
		{
			// Get the text data
			SingleLineTextData* textData = GetSingleLineTextDataFromWidget(m_ActiveUI->m_EditingWidget);
			KG_ASSERT(textData);

			// Ensure the cursor index is within the correct text bounds
			if (textData->m_CursorIndex >= 0 && textData->m_CursorIndex <= textData->m_Text.size())
			{
				std::string character = { (char)event.GetKeyCode() };
				// Add the event's character to the text buffer
				textData->m_Text.insert(textData->m_CursorIndex, character);
				textData->m_CursorIndex++;

				// Revalidate text dimensions
				RecalculateTextData(m_ActiveUI->m_EditingWidget);

				// Run on move cursor if necessary
				OnMoveCursorInternal(m_ActiveUI->m_EditingWidget);

				// Ensure the IBeam is visible when moving
				m_ActiveUI->m_IBeamVisible = true;
				m_ActiveUI->m_IBeamAccumulator = 0.0f;
			}
		}
		return false;
	}

	bool RuntimeUIContext::OnKeyPressedEvent(Events::KeyPressedEvent event)
	{
		// Ensure a valid user interface is active
		if (!m_ActiveUI)
		{
			return false;
		}

		KeyCode key = event.GetKeyCode();

		// Handle key pressed events for the currently editing widget
		if (m_ActiveUI->m_EditingWidget)
		{
			// Get the text data
			SingleLineTextData* textData = GetSingleLineTextDataFromWidget(m_ActiveUI->m_EditingWidget);
			KG_ASSERT(textData);

			if (key == Key::Backspace)
			{
				if (textData->m_Text.size() > 0)
				{
					if (textData->m_CursorIndex > 0 && textData->m_CursorIndex <= textData->m_Text.size())
					{
						textData->m_Text.erase(textData->m_CursorIndex - 1, 1);
						textData->m_CursorIndex--;
						// Revalidate text dimensions
						RecalculateTextData(m_ActiveUI->m_EditingWidget);

						// Run on move cursor if necessary
						OnMoveCursorInternal(m_ActiveUI->m_EditingWidget);

					}
					
				}
				return true;
			}

			// Handle exiting 
			if (key == Key::Enter || key == Key::Escape)
			{
				ClearEditingWidget();
				return true;
			}

			if (key == Key::Left)
			{
				// Enforce lower bounds of the text
				if (textData->m_CursorIndex > 0)
				{
					// Decriment the cursor
					textData->m_CursorIndex--;

					// Ensure the IBeam is visible when moving
					m_ActiveUI->m_IBeamVisible = true;
					m_ActiveUI->m_IBeamAccumulator = 0.0f;

					// Run on move cursor if necessary
					OnMoveCursorInternal(m_ActiveUI->m_EditingWidget);

				}
				return true;
			}
			if (key == Key::Right)
			{
				// Enforce upper bounds of the text
				if (textData->m_CursorIndex < textData->m_Text.size())
				{
					textData->m_CursorIndex++;

					// Ensure the IBeam is visible when moving
					m_ActiveUI->m_IBeamVisible = true;
					m_ActiveUI->m_IBeamAccumulator = 0.0f;

					// Run on move cursor if necessary
					OnMoveCursorInternal(m_ActiveUI->m_EditingWidget);
				}
				return true;
			}
		}

		return false;
	}

	void RuntimeUIContext::OnMouseButtonPressedEvent(const Events::MouseButtonPressedEvent& event)
	{
		// Get mouse position and active viewport
		Math::vec2 mousePosition = Input::InputService::GetViewportMousePosition();
		ViewportData* viewportData = EngineService::GetActiveEngine().GetApp().GetViewportData();

		// Handle mouse click for the editing widget
		if (m_ActiveUI->m_EditingWidget)
		{
			SingleLineTextData* textData = GetSingleLineTextDataFromWidget(m_ActiveUI->m_EditingWidget);
			KG_ASSERT(textData);

			// Get the widget's transform
			BoundingBoxTransform widgetTransform = GetWidgetDimensionsFromID(
				m_ActiveUI->m_EditingWidget->m_ID, 
				viewportData->m_Width, viewportData->m_Height);

			// Get the widget's text's starting position
			Math::vec2 resolution = Utility::ScreenResolutionToAspectRatio(Projects::ProjectService::GetActiveContext().GetTargetResolution());
			float textScalingFactor{ (viewportData->m_Width * 0.15f * textData->m_TextSize) * (resolution.y / resolution.x) };
			Math::vec3 textStartingPoint = GetSingleLineTextStartingPosition(*textData, widgetTransform.m_Translation, widgetTransform.m_Size, textScalingFactor);

			// Find new cursor location
			size_t newCursorIndex = CalculateCursorIndexFromMousePosition(*textData, textStartingPoint.x, mousePosition.x, textScalingFactor);

			// Update the cursor
			textData->m_CursorIndex = newCursorIndex;

			// Ensure the IBeam is visible when moving
			m_ActiveUI->m_IBeamVisible = true;
			m_ActiveUI->m_IBeamAccumulator = 0.0f;
		}
		else if (m_ActiveUI->m_SelectedWidget && m_ActiveUI->m_SelectedWidget->m_WidgetType == WidgetTypes::SliderWidget)
		{
			// Get the underlying widget type
			KG_ASSERT(m_ActiveUI->m_SelectedWidget->m_WidgetType == WidgetTypes::SliderWidget);
			SliderWidget& activeSlider = *(SliderWidget*)m_ActiveUI->m_SelectedWidget;

			// Get the widget's transform
			BoundingBoxTransform widgetTransform = GetWidgetDimensionsFromID(
				m_ActiveUI->m_SelectedWidget->m_ID,
				viewportData->m_Width, viewportData->m_Height);

			// Get the slider's current normalized location based on the bounds and currentValue
			float normalizedSliderLocation = (activeSlider.m_CurrentValue - activeSlider.m_Bounds.x) /
				(activeSlider.m_Bounds.y - activeSlider.m_Bounds.x);

			// Get slider bounding box
			Math::vec3 sliderSize { 0.04f * widgetTransform.m_Size.x , 0.35f * widgetTransform.m_Size.y  , widgetTransform.m_Size.z };
			Math::vec3 sliderLocation{ widgetTransform.m_Translation.x + widgetTransform.m_Size.x * normalizedSliderLocation - (sliderSize.x / 2.0f), widgetTransform.m_Translation.y + (widgetTransform.m_Size.y / 2.0f) - (sliderSize.y / 2.0f), widgetTransform.m_Translation.z};

			// Check if the mouse click falls within the bounds of the slider
			if (mousePosition.x > (sliderLocation.x) && mousePosition.x < (sliderLocation.x + sliderSize.x) &&
				mousePosition.y >(sliderLocation.y) && mousePosition.y < (sliderLocation.y + sliderSize.y))
			{
				m_ActiveUI->m_PressedWidget = m_ActiveUI->m_SelectedWidget;
			}
		}

		else if (m_ActiveUI->m_SelectedWidget && m_ActiveUI->m_SelectedWidget->m_WidgetType == WidgetTypes::DropDownWidget)
		{
			// Get the underlying widget type
			KG_ASSERT(m_ActiveUI->m_SelectedWidget->m_WidgetType == WidgetTypes::DropDownWidget);
			DropDownWidget& activeDropDown = *(DropDownWidget*)m_ActiveUI->m_SelectedWidget;

			// Get the widget's transform
			BoundingBoxTransform widgetTransform = GetWidgetDimensionsFromID(
				m_ActiveUI->m_SelectedWidget->m_ID,
				viewportData->m_Width, viewportData->m_Height);

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

	void RuntimeUIContext::OnMouseButtonReleasedEvent(const Events::MouseButtonReleasedEvent& mouseEvent)
	{
		UNREFERENCED_PARAMETER(mouseEvent);

		if (m_ActiveUI->m_PressedWidget)
		{
			// Clear the pressed widget
			m_ActiveUI->m_PressedWidget = nullptr;
		}
	}

	void RuntimeUIContext::SetActiveUI(Ref<UserInterface> userInterface, Assets::AssetHandle uiHandle)
	{
		if (!userInterface || uiHandle == Assets::EmptyHandle)
		{
			KG_WARN("Attempt to make a user interface active that is null or has an empty handle");
			return;
		}

		// Reset previous active UI
		ClearActiveUI();

		// Set new active UI
		m_ActiveUI = userInterface;
		m_ActiveUIHandle = uiHandle;

		// Revalidate UI Context
		RevalidateDisplayedWindows();
		RevalidateWidgetIDToLocationMap();

		// Load default font if necessary
		if (!userInterface->m_Font)
		{
			userInterface->m_Font = m_DefaultFont;
			userInterface->m_FontHandle = Assets::EmptyHandle;
		}

		// Set the first window as active if applicable
		Ref<UserInterface> activeUI = m_ActiveUI;
		if (activeUI->m_Windows.size() > 0)
		{
			// Display the first window
			activeUI->m_Windows.at(0).DisplayWindow();
			activeUI->m_ActiveWindow = &activeUI->m_Windows.at(0);

			// Handle window specific set-up
			for (Window& window : activeUI->m_Windows)
			{
				// Set the default widget for each window
				if (window.m_DefaultActiveWidget != RuntimeUI::k_InvalidWidgetID)
				{
					Ref<RuntimeUI::Widget> newDefaultWidget = GetWidgetFromID(window.m_DefaultActiveWidget);
					KG_ASSERT(newDefaultWidget);
					window.m_DefaultActiveWidgetRef = newDefaultWidget;
				}

				// Calculate Text Sizes for all windows
				for (Ref<Widget> widget : window.m_Widgets)
				{
					RecalculateTextData(widget.get());

					// Ensure the default cursor position is at the end of the text
					SingleLineTextData* textData = GetSingleLineTextDataFromWidget(widget.get());
					if (textData)
					{
						textData->m_CursorIndex = textData->m_Text.size();
					}
				}
			}
			
			// Set default widget as the selected widget if it exists
			if (activeUI->m_ActiveWindow->m_DefaultActiveWidgetRef)
			{
				// Set the default widget
				activeUI->m_SelectedWidget = activeUI->m_ActiveWindow->m_DefaultActiveWidgetRef.get();

				// Ensure the widget is selectable
				KG_ASSERT(activeUI->m_SelectedWidget->Selectable());
			}
			else
			{
				activeUI->m_SelectedWidget = nullptr;
			}

		}

		// Create widget navigation links for all windows
		NavigationLinksCalculator newCalculator
		{
			m_ActiveUI.get(),
			EngineService::GetActiveEngine().GetWindow().GetActiveViewport()
		};
		newCalculator.CalculateNavigationLinks();
	}

	void RuntimeUIContext::SetActiveUIFromHandle(Assets::AssetHandle uiHandle)
	{
		// Get user interface from asset service system
		Ref<RuntimeUI::UserInterface> uiReference = Assets::AssetService::GetUserInterface(uiHandle);

		// Validate returned user interface
		if (!uiReference)
		{
			KG_WARN("Could not locate user interface. Provided handle did not lead to a valid user interface.");
			return;
		}

		// Set active user interface
		SetActiveUI(uiReference, uiHandle);
	}

	bool RuntimeUIContext::IsUIActiveFromHandle(Assets::AssetHandle uiHandle)
	{
		// Ensure an invalid state is not presented
		if (uiHandle == Assets::EmptyHandle || !m_ActiveUI)
		{
			return false;
		}

		return m_ActiveUIHandle == uiHandle;
	}

	bool RuntimeUIContext::DeleteActiveUIWindow(int32_t windowID)
	{
		// Get the window location
		std::vector<uint16_t>* locationDir = GetLocationFromID(windowID);

		// Ensure the location is valid and represents a window
		KG_ASSERT(locationDir);
		KG_ASSERT(locationDir->size() == 1);
		uint16_t windowLocation{ locationDir->at(0) };

		// Attempt to delete the window from the active user interface
		bool success = DeleteUIWindow(m_ActiveUI, windowLocation);

		// Ensure deletion was successful
		if (!success)
		{
			KG_WARN("Attempt to delete window from active user interface failed.");
			return false;
		}

		// Ensure correct windows are displayed
		RevalidateDisplayedWindows();
		RevalidateWidgetIDToLocationMap();
		return true;
	}

	bool RuntimeUIContext::DeleteActiveUIWidget(int32_t widgetID)
	{
		// Attempt to delete the widget from the active user interface
		bool success = DeleteUIWidget(m_ActiveUI, widgetID);

		// Ensure deletion was successful
		if (!success)
		{
			KG_WARN("Attempt to delete widget from active user interface failed.");
			return false;
		}

		RevalidateWidgetIDToLocationMap();

		// Revalidate navigation links
		NavigationLinksCalculator newCalculator
		{
			m_ActiveUI.get(),
			EngineService::GetActiveEngine().GetWindow().GetActiveViewport()
		};
		newCalculator.CalculateNavigationLinks();

		return true;
	}

	void RuntimeUIContext::AddWidgetToContainer(ContainerData* container, Ref<Widget> newWidget)
	{
		KG_ASSERT(newWidget);
		KG_ASSERT(container);

		// Add the parent window
		container->m_ContainedWidgets.push_back(newWidget);

		// Ensure ID -> Location map is valid
		RevalidateWidgetIDToLocationMap();


		// Ensure the new widget is validated
		RecalculateTextData(newWidget.get());
	}

	bool RuntimeUIContext::DeleteUIWindow(Ref<UserInterface> userInterface, std::size_t windowLocation)
	{
		// Ensure user interface is valid
		if (!userInterface)
		{
			KG_WARN("Attempt to delete window from invalid user interface reference");
			return false;
		}

		// Get all windows
		std::vector<Window>& uiWindows = userInterface->m_Windows;

		// Ensure window location is valid
		if (windowLocation >= uiWindows.size())
		{
			KG_WARN("Attempt to delete window, however, the provided index is out of bounds");
			return false;
		}

		// Delete the indicated window
		uiWindows.erase(uiWindows.begin() + windowLocation);
		return true;
	}

	bool RuntimeUIContext::DeleteUIWidget(Ref<UserInterface> userInterface, int32_t widgetID)
	{
		if (!userInterface)
		{
			KG_WARN("Attempt to delete widget from invalid user interface reference");
			return false;
		}

		// Get the ID -> Widget Location map and location directions
		IDToLocationMap& idToLocationMap = m_ActiveUI->m_IDToLocation;
		KG_ASSERT(idToLocationMap.contains(widgetID));
		std::vector<uint16_t>& locationDirections = idToLocationMap.at(widgetID);

		// Ensure the directions map to a widget
		KG_ASSERT(locationDirections.size() > 1);

		// Get widget using location directions
		// TODO: This currently assumes the window -> widget structure
		size_t windowIndex = locationDirections.at(0);
		size_t firstWidgetIndex = locationDirections.at(1);

		// Ensure window index is valid
		if (windowIndex >= m_ActiveUI->m_Windows.size())
		{
			KG_WARN("Attempt to delete widget from window with out of bounds index");
			return false;
		}

		// Get the window reference
		std::vector<RuntimeUI::Window>& uiWindows = userInterface->m_Windows;
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
		for (size_t iteration{ 2 }; iteration < locationDirections.size(); iteration++)
		{
			// Get the container data for the current widget
			ContainerData* data = GetContainerDataFromWidget(currentWidget.get());
			KG_ASSERT(data);

			// Ensure the widget directions fall within the bounds of the widget array
			size_t currentIndex = locationDirections.at(iteration);
			KG_ASSERT(currentIndex < data->m_ContainedWidgets.size());

			// Exit early if we reach the final valid index of locationDirections (Ex: Window -> Widget -> (Final)[Widget])
			if (iteration == locationDirections.size() - 1)
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


	void RuntimeUIContext::OnRender(const Math::mat4& cameraViewMatrix, uint32_t viewportWidth, uint32_t viewportHeight)
	{
		KG_PROFILE_FUNCTION()
		// Ensure active user interface is valid
		if (!m_ActiveUI)
		{
			return;
		}

		// Reset the rendering context
		Rendering::RendererAPI::ClearDepthBuffer();

		// Start rendering context
		Rendering::RenderingService::BeginScene(cameraViewMatrix);

		// Submit rendering data from all windows
		uint16_t windowIteration{ 0 };
		std::vector<size_t>& windowIndices = m_ActiveUI->m_DisplayedWindowIndices;
		for (Window* window : m_ActiveUI->m_DisplayedWindows)
		{
			// Get position data for rendering window
			Math::vec3 scale = window->CalculateSize(viewportWidth, viewportHeight);
			Math::vec3 initialTranslation = window->CalculateWorldPosition(viewportWidth, viewportHeight);
			Math::vec3 bottomLeftTranslation = Math::vec3( initialTranslation.x + (scale.x / 2),  initialTranslation.y + (scale.y / 2), initialTranslation.z);


			if (window->m_BackgroundColor.w > 0.001f)
			{
				// Create background rendering data
				m_BackgroundInputSpec.m_TransformMatrix = glm::translate(Math::mat4(1.0f), bottomLeftTranslation)
					* glm::scale(Math::mat4(1.0f), scale);
				Rendering::Shader::SetDataAtInputLocation<Math::vec4>(window->m_BackgroundColor, 
					Utility::FileSystem::CRCFromString("a_Color"),
					m_BackgroundInputSpec.m_Buffer, m_BackgroundInputSpec.m_Shader);

				// Push window ID and invalid widgetID
				Rendering::Shader::SetDataAtInputLocation<int32_t>(window->m_ID, 
					Utility::FileSystem::CRCFromString("a_EntityID"),
					m_BackgroundInputSpec.m_Buffer, m_BackgroundInputSpec.m_Shader);

				// Submit background data to GPU
				Rendering::RenderingService::SubmitDataToRenderer(m_BackgroundInputSpec);
			}
			

			// Call rendering function for every widget
			initialTranslation.z += 0.1f;
			uint16_t widgetIteration{ 0 };
			for (Ref<Widget> widgetRef : window->m_Widgets)
			{
				// TODO: Seperate these into multiple functions
				// Push widget ID
				Rendering::Shader::SetDataAtInputLocation<int32_t>(widgetRef->m_ID, 
					Utility::FileSystem::CRCFromString("a_EntityID"),
					m_BackgroundInputSpec.m_Buffer, m_BackgroundInputSpec.m_Shader);
				Rendering::Shader::SetDataAtInputLocation<int32_t>(widgetRef->m_ID,
					Utility::FileSystem::CRCFromString("a_EntityID"),
					m_ImageInputSpec.m_Buffer, m_ImageInputSpec.m_Shader);
				RuntimeUI::FontService::SetID((uint32_t)widgetRef->m_ID);
				// Call the widget's rendering function
				widgetRef->OnRender(initialTranslation, scale, (float)viewportWidth);
				widgetIteration++;
			}
			windowIteration++;
		}

		// End rendering context and submit rendering data to GPU
		Rendering::RenderingService::EndScene();

	}

	void RuntimeUIContext::OnRender(uint32_t viewportWidth, uint32_t viewportHeight)
	{
		// Calculate orthographic projection matrix for user interface
		Math::mat4 orthographicProjection = glm::ortho(0.0f, (float)viewportWidth,
			0.0f, (float)viewportHeight, -1.0f, 1.0f);
		OnRender(orthographicProjection, viewportWidth, viewportHeight);
	}

	void RuntimeUIContext::AddActiveWindow(Window& window)
	{
		// Store the window in the active user interface
		m_ActiveUI->m_Windows.push_back(window);

		RevalidateWidgetIDToLocationMap();

		// Display the window
		window.DisplayWindow();
	}

	void RuntimeUIContext::ClearHoveredWidget()
	{
		KG_ASSERT(m_ActiveUI);

		if (!m_ActiveUI->m_HoveredWidget)
		{
			return;
		}

		m_ActiveUI->m_HoveredWidget = nullptr;

		// Reset the cursor icon
		EngineService::GetActiveEngine().GetWindow().SetMouseCursorIcon(CursorIconType::Standard);
	}

	void RuntimeUIContext::ClearEditingWidget()
	{
		KG_ASSERT(m_ActiveUI);
		m_ActiveUI->m_EditingWidget = nullptr;
	}

	void RuntimeUIContext::SetActiveFont(Ref<Font> newFont, Assets::AssetHandle fontHandle)
	{
		// Set the active font for the active user interface
		m_ActiveUI->m_Font = newFont;
		m_ActiveUI->m_FontHandle = fontHandle;

		// Revalidate/calculate text sizes for all windows
		for (Window& window : m_ActiveUI->m_Windows)
		{
			for (Ref<Widget> widget : window.m_Widgets)
			{
				RecalculateTextData(widget.get());
			}
		}
	}

	std::vector<Window>& RuntimeUIContext::GetAllActiveWindows()
	{
		return m_ActiveUI->m_Windows;
	}

	BoundingBoxTransform RuntimeUIContext::GetParentDimensionsFromID(int32_t widgetID, uint32_t viewportWidth, uint32_t viewportHeight)
	{
		KG_ASSERT(m_ActiveUI);

		// Get the ID -> Widget Location map and location directions
		IDToLocationMap& idToLocationMap = 
			m_ActiveUI->m_IDToLocation;
		KG_ASSERT(idToLocationMap.contains(widgetID));
		std::vector<uint16_t>& locationDirections = idToLocationMap.at(widgetID);

		// Ensure directions to a widget a presented
		KG_ASSERT(locationDirections.size() > 1);

		// Get the first two entries (Window -> Widget)
		size_t windowIndex = locationDirections.at(0);
		size_t firstWidgetIndex = locationDirections.at(1);

		// Get the widget's parent window
		KG_ASSERT(windowIndex < m_ActiveUI->m_Windows.size());
		Window& parentWindow = m_ActiveUI->m_Windows.at(windowIndex);

		// Calculate the parent window's dimensions
		BoundingBoxTransform returnDimensions;
		returnDimensions.m_Translation = parentWindow.CalculateWorldPosition(
			viewportWidth, viewportHeight);
		returnDimensions.m_Size = parentWindow.CalculateSize(
			viewportWidth, viewportHeight);

		// Return the parent dimensions if the location only involves a single widget (Ex: Window -> Widget)
		if (locationDirections.size() == 2) 
		{
			return returnDimensions;
		}

		// Get the first widget
		KG_ASSERT(firstWidgetIndex < parentWindow.m_Widgets.size());
		Ref<Widget> currentWidget = parentWindow.m_Widgets.at(firstWidgetIndex);

		// Continue to calculate the dimensions for a longer widget chain (Ex: Window -> Widget -> ...)
		for (size_t iteration{ 2 }; iteration < locationDirections.size(); iteration++)
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
					(returnDimensions.m_Size.x * horContainer->m_ColumnWidth) * locationDirections.at(iteration) +
					(returnDimensions.m_Size.x * horContainer->m_ColumnSpacing) * locationDirections.at(iteration);
				returnDimensions.m_Size.x *= horContainer->m_ColumnWidth;
			}

			// Handle the vertical container case
			else if (currentWidget->m_WidgetType == WidgetTypes::VerticalContainerWidget)
			{
				VerticalContainerWidget* vertContainer = (VerticalContainerWidget*)currentWidget.get();
				KG_ASSERT(vertContainer);
				returnDimensions.m_Translation.y += returnDimensions.m_Size.y - 
					(returnDimensions.m_Size.y * vertContainer->m_RowHeight) * (locationDirections.at(iteration) + 1) - 
					(returnDimensions.m_Size.y * vertContainer->m_RowSpacing) * locationDirections.at(iteration);
				returnDimensions.m_Size.y *= vertContainer->m_RowHeight;
			}

			// Exit early if we reach the final valid location direction
			if (iteration == locationDirections.size() - 1)
			{
				break;
			}

			// Get the container data from the active widget
			ContainerData* currentData = GetContainerDataFromWidget(currentWidget.get());
			KG_ASSERT(currentData);

			// Get the current widget index and set the next parent widget
			uint16_t currentContainerIndex = locationDirections.at(iteration);
			KG_ASSERT(currentContainerIndex < currentData->m_ContainedWidgets.size());
			currentWidget = currentData->m_ContainedWidgets.at(currentContainerIndex);
		}

		// Return the dimensions
		return returnDimensions;
	}

	BoundingBoxTransform RuntimeUIContext::GetWidgetDimensionsFromID(int32_t widgetID, uint32_t viewportWidth, uint32_t viewportHeight)
	{
		// Get the ID -> Widget Location map and location directions
		IDToLocationMap& idToLocationMap = 
			m_ActiveUI->m_IDToLocation;
		KG_ASSERT(idToLocationMap.contains(widgetID));
		std::vector<uint16_t>& locationDirections = idToLocationMap.at(widgetID);

		// Ensure directions to a widget a presented
		KG_ASSERT(locationDirections.size() > 1);

		// Get the first two entries (Window -> Widget)
		size_t windowIndex = locationDirections.at(0);
		size_t firstWidgetIndex = locationDirections.at(1);

		// Get the widget's parent window
		KG_ASSERT(windowIndex < m_ActiveUI->m_Windows.size());
		Window& parentWindow = m_ActiveUI->m_Windows.at(windowIndex);

		// Calculate the parent window's dimensions
		BoundingBoxTransform returnDimensions;
		returnDimensions.m_Translation = parentWindow.CalculateWorldPosition(
			viewportWidth, viewportHeight);
		returnDimensions.m_Size = parentWindow.CalculateSize(
			viewportWidth, viewportHeight);

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
		for (size_t iteration{ 2 }; iteration < locationDirections.size(); iteration++)
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
					returnDimensions.m_Size.x * horizContainer->m_ColumnWidth * locationDirections.at(iteration) +
					returnDimensions.m_Size.x * horizContainer->m_ColumnSpacing * locationDirections.at(iteration);
				returnDimensions.m_Size.x *= horizContainer->m_ColumnWidth;
			}
			// Handle the vertical container case
			else if (currentWidget->m_WidgetType == WidgetTypes::VerticalContainerWidget)
			{
				VerticalContainerWidget* vertContainer = (VerticalContainerWidget*)currentWidget.get();
				KG_ASSERT(vertContainer);
				returnDimensions.m_Translation.y += returnDimensions.m_Size.y - 
					returnDimensions.m_Size.y * vertContainer->m_RowHeight * (locationDirections.at(iteration) + 1) - 
					returnDimensions.m_Size.y * vertContainer->m_RowSpacing * locationDirections.at(iteration);
				returnDimensions.m_Size.y *= vertContainer->m_RowHeight;
			}

			// Get the container data from the active widget
			ContainerData* currentData = GetContainerDataFromWidget(currentWidget.get());
			KG_ASSERT(currentData);

			// Get the current widget index and set the next parent widget
			uint16_t currentContainerIndex = locationDirections.at(iteration);
			KG_ASSERT(currentContainerIndex < currentData->m_ContainedWidgets.size());
			currentWidget = currentData->m_ContainedWidgets.at(currentContainerIndex);
		}

		// Do the final calculation
		returnDimensions.m_Translation = currentWidget->CalculateWorldPosition(returnDimensions.m_Translation, returnDimensions.m_Size);
		returnDimensions.m_Size = currentWidget->CalculateWidgetSize(returnDimensions.m_Size);

		// Return the dimensions
		return returnDimensions;
	}


	void RuntimeUIContext::RevalidateDisplayedWindows()
	{
		// Ensure/validate that the correct window is being displayed
		m_ActiveUI->m_DisplayedWindows.clear();
		m_ActiveUI->m_DisplayedWindowIndices.clear();
		size_t iteration{ 0 };
		for (Window& window : GetAllActiveWindows())
		{
			// Add the window to the displayed windows if it is flagged to do so
			if (window.GetWindowDisplayed()) 
			{ 
				m_ActiveUI->m_DisplayedWindows.push_back(&window);
				m_ActiveUI->m_DisplayedWindowIndices.push_back(iteration);
			}
			iteration++;
		}
	}

	void RuntimeUIContext::RevalidateWidgetIDToLocationMap()
	{
		IDToLocationMap& locationMap = m_ActiveUI->m_IDToLocation;
		locationMap.clear();

		// Parse through each window
		size_t windowIteration{ 0 };
		for (Window& window : m_ActiveUI->m_Windows)
		{
			// Create the window's location and add it to the locationMap
			std::vector<uint16_t> windowLocation{ (uint16_t)windowIteration };
			auto [iter, success] = locationMap.insert_or_assign(window.m_ID, std::move(windowLocation));
			KG_ASSERT(success);

			// Parse through each window's widget (Window -> ChildWidget)
			size_t widgetIteration{ 0 };
			for (Ref<Widget> currentWidget : window.m_Widgets)
			{
				// Create the widget's location and add it to the locationMap
				std::vector<uint16_t> widgetLocation{ (uint16_t)windowIteration, (uint16_t)widgetIteration };
				auto [iter, success] = locationMap.insert_or_assign(currentWidget->m_ID, widgetLocation);
				KG_ASSERT(success);

				// Handle the case where the widget is a container
				ContainerData* container = GetContainerDataFromWidget(currentWidget.get());
				if (container)
				{
					// Recursively handle this container's widgets and all of its sub-containers
					RevalidateContainerInLocationMap(locationMap, container, widgetLocation);
				}
				widgetIteration++;
			}
			windowIteration++;
		}
	}

	void RuntimeUIContext::RevalidateContainerInLocationMap(IDToLocationMap& locationMap, ContainerData* container, std::vector<uint16_t>& parentLocation)
	{
		KG_ASSERT(container);
		KG_ASSERT(parentLocation.size() > 1);
		
		size_t iteration{ 0 };
		for (Ref<Widget> childWidget : container->m_ContainedWidgets)
		{
			// Add the current widget index to the location and insert the location
			parentLocation.push_back((uint16_t)iteration);
			auto [iter, success] = locationMap.insert_or_assign(childWidget->m_ID, parentLocation);
			KG_ASSERT(success);

			// Handle the case where the childWidget is a container
			ContainerData* container = GetContainerDataFromWidget(childWidget.get());
			if (container)
			{
				// Recursive call to handle all containers
				RevalidateContainerInLocationMap(locationMap, container, parentLocation);
			}

			// Clean up the location and continue for all other child widgets
			iteration++;
			parentLocation.pop_back();
		}
	}

	void RuntimeUIContext::CalculateSingleLineText(SingleLineTextData& textData)
	{
		// Calculate the text size of the widget using the default font if the active user interface is not set
		if (!m_ActiveUI)
		{
			textData.m_CachedTextDimensions = m_DefaultFont->CalculateSingleLineTextSize(textData.m_Text);
			return;
		}

		// Calculate the text size of the widget using the active user interface font
		textData.m_CachedTextDimensions= m_ActiveUI->m_Font->CalculateSingleLineTextSize(textData.m_Text);
	}

	Math::vec2 RuntimeUIContext::CalculateSingleLineText(std::string_view text)
	{
		// Calculate the text size of the widget using the default font if the active user interface is not set
		if (!m_ActiveUI)
		{
			return m_DefaultFont->CalculateSingleLineTextSize(text);
		}

		// Calculate the text size of the widget using the active user interface font
		return m_ActiveUI->m_Font->CalculateSingleLineTextSize(text);
	}

	size_t RuntimeUIContext::CalculateCursorIndexFromMousePosition(SingleLineTextData& textData, float textStartingPosition, float mouseXPosition, float textScalingFactor)
	{
		// Calculate the text size of the widget using the default font if the active user interface is not set
		if (!m_ActiveUI)
		{
			return m_DefaultFont->CalculateIndexFromMousePosition(
				textData.m_Text, textStartingPosition, mouseXPosition, textScalingFactor);
		}

		// Calculate the text size of the widget using the active user interface font
		return m_ActiveUI->m_Font->CalculateIndexFromMousePosition(
			textData.m_Text, textStartingPosition, mouseXPosition, textScalingFactor);
	}

	void RuntimeUIContext::CalculateMultiLineText(MultiLineTextData& textData, const Math::vec3& widgetSize, float textSize)
	{
		// Calculate the text size of the widget using the default font if the active user interface is not set
		if (!m_ActiveUI)
		{
			if (textData.m_TextWrapped)
			{
				m_DefaultFont->CalculateMultiLineTextMetadata(textData.m_Text, textData.m_CachedTextDimensions, textSize, (int)widgetSize.x);
			}
			else
			{
				m_DefaultFont->CalculateMultiLineTextMetadata(textData.m_Text, textData.m_CachedTextDimensions, textSize);
			}

			return;
		}

		// Calculate the text size of the widget using the active user interface font
		if (textData.m_TextWrapped)
		{
			m_ActiveUI->m_Font->CalculateMultiLineTextMetadata(textData.m_Text, textData.m_CachedTextDimensions, textSize, (int)widgetSize.x);
		}
		else
		{
			m_ActiveUI->m_Font->CalculateMultiLineTextMetadata(textData.m_Text, textData.m_CachedTextDimensions, textSize);
		}
	}

	void RuntimeUIContext::SetWidgetTextInternal(Ref<Widget> currentWidget, const std::string& newText)
	{
		// Ensure the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget's text");
			return;
		}

		// Get the selection specific data from the widget
		MultiLineTextData* multiLineData = GetMultiLineTextDataFromWidget(currentWidget.get());
		if (multiLineData)
		{
			// Set the text of the widget
			multiLineData->m_Text = newText;
			RecalculateTextData(currentWidget.get());
			return;
		}

		// Get the selection specific data from the widget
		SingleLineTextData* singleLineData = GetSingleLineTextDataFromWidget(currentWidget.get());
		if (singleLineData)
		{
			// Modify the single line text
			singleLineData->m_Text = newText;
			CalculateSingleLineText(*singleLineData);
			return;
		}

		KG_WARN("Invalid widget type provided when modifying a widget's text");
		return;
		
	}

	void RuntimeUIContext::SetSelectedWidgetInternal(Ref<Widget> newSelectedWidget)
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
		if (newSelectedWidget.get() == m_ActiveUI->m_SelectedWidget)
		{
			return;
		}

		// Set the new widget as selected and set it's color to the active color
		m_ActiveUI->m_SelectedWidget = newSelectedWidget.get();

		// Call the on move function if applicable
		if (m_ActiveUI->m_FunctionPointers.m_OnMove)
		{
			Utility::CallWrapped<WrappedVoidNone>(m_ActiveUI->m_FunctionPointers.m_OnMove->m_Function);
		}
	}

	void RuntimeUIContext::SetHoveredWidgetInternal(Ref<Widget> newHoveredWidget)
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
		if (newHoveredWidget.get() == m_ActiveUI->m_HoveredWidget)
		{
			return;
		}

		// Set the new widget as selected and set it's color to the active color
		m_ActiveUI->m_HoveredWidget = newHoveredWidget.get();

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
		if (m_ActiveUI->m_FunctionPointers.m_OnHover)
		{
			Utility::CallWrapped<WrappedVoidNone>(m_ActiveUI->m_FunctionPointers.m_OnHover->m_Function);
		}
	}

	void RuntimeUIContext::SetWidgetTextColorInternal(Ref<Widget> currentWidget, const Math::vec4& newColor)
	{
		// Ensure the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget's text color");
			return;
		}

		// Get the selection specific data from the widget
		MultiLineTextData* multiLineData = GetMultiLineTextDataFromWidget(currentWidget.get());
		if (multiLineData)
		{
			// Set the text of the widget
			multiLineData->m_TextColor = newColor;
			return;
		}

		// Get the selection specific data from the widget
		SingleLineTextData* singleLineData = GetSingleLineTextDataFromWidget(currentWidget.get());
		if (singleLineData)
		{
			// Modify the single line text
			singleLineData->m_TextColor = newColor;
			return;
		}

		KG_WARN("Invalid widget type provided when modifying a widget's text color");
		return;
	}

	void RuntimeUIContext::SetWidgetBackgroundColorInternal(Ref<Widget> currentWidget, const Math::vec4& newColor)
	{
		// Ensure the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget's background color");
			return;
		}

		// Get the selection specific data from the widget
		SelectionData* selectionData = GetSelectionDataFromWidget(currentWidget.get());
		if (!selectionData)
		{
			KG_WARN("Unable to retrieve selection data. May be invalid widget type!");
			return;
		}

		// Set the widget's new color
		selectionData->m_DefaultBackgroundColor = newColor;
	}


	void RuntimeUIContext::OnPressInternal(Widget* currentWidget)
	{
		// Get the selection specific data from the widget
		SelectionData* selectionData = GetSelectionDataFromWidget(currentWidget);
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
		m_ActiveUI->m_SelectedWidget = currentWidget;

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
			m_ActiveUI->m_EditingWidget = currentWidget;
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

	void RuntimeUIContext::OnMoveCursorInternal(Widget* currentWidget)
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

	void RuntimeUIContext::SetWidgetSelectableInternal(Ref<Widget> currentWidget, bool selectable)
	{
		// Ensure the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget as selectable");
			return;
		}

		// Get the selection specific data from the widget
		SelectionData* selectionData = GetSelectionDataFromWidget(currentWidget.get());
		if (!selectionData)
		{
			KG_WARN("Unable to retrieve selection data. May be invalid widget type!");
			return;
		}

		// Set the widget as selectable
		selectionData->m_Selectable = selectable;

		// Calculate navigation links
		NavigationLinksCalculator newCalculator
		{
			m_ActiveUI.get(),
			EngineService::GetActiveEngine().GetWindow().GetActiveViewport()
		};
		newCalculator.CalculateNavigationLinks();
	}

	bool RuntimeUIContext::IsWidgetSelectedInternal(Ref<Widget> currentWidget)
	{
		// Check if the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when checking the widget is selected");
			return false;
		}

		// Return if the widget is selected
		return m_ActiveUI->m_SelectedWidget == currentWidget.get();
	}

	void RuntimeUIContext::RenderBackground(const Math::vec4& color, const Math::vec3& translation, const Math::vec3 size)
	{
		Rendering::RendererInputSpec& renderSpec = m_BackgroundInputSpec;

		if (color.w > 0.001f)
		{
			// Create the widget's background rendering data
			renderSpec.m_TransformMatrix = glm::translate(Math::mat4(1.0f), 
				Math::vec3(translation.x + (size.x / 2), translation.y + (size.y / 2), translation.z))
				* glm::scale(Math::mat4(1.0f), size);
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(color, 
				Utility::FileSystem::CRCFromString("a_Color"),
				renderSpec.m_Buffer, renderSpec.m_Shader);

			// Submit background data to GPU
			Rendering::RenderingService::SubmitDataToRenderer(renderSpec);
		}
	}

	void RuntimeUIContext::RenderImage(const ImageData& imageData, const Math::vec3& translation, const Math::vec3 size)
	{
		Rendering::RendererInputSpec& renderSpec = m_ImageInputSpec;

		if (imageData.m_ImageRef)
		{
			// Create the widget's background rendering data
			renderSpec.m_TransformMatrix = glm::translate(Math::mat4(1.0f), Math::vec3(translation.x + (size.x / 2), translation.y + (size.y / 2), translation.z))
				* glm::scale(Math::mat4(1.0f), size);


			renderSpec.m_Texture = imageData.m_ImageRef;
			renderSpec.m_ShapeComponent->Texture = imageData.m_ImageRef;

			// Submit background data to GPU
			Rendering::RenderingService::SubmitDataToRenderer(renderSpec);
		}
	}

	void RuntimeUIContext::RenderSingleLineText(const SingleLineTextData& textData, const Math::vec3& textStartingPoint, float textScalingFactor)
	{
		// Call the text's rendering function
		m_ActiveUI->m_Font->OnRenderSingleLineText(textData.m_Text, textStartingPoint, textData.m_TextColor, textScalingFactor);
	}

	void RuntimeUIContext::RenderTextCursor(const SingleLineTextData& textData, const Math::vec3& textStartingPoint, float textScalingFactor)
	{
		Rendering::RendererInputSpec& renderSpec = m_BackgroundInputSpec;

		// Start the cursor's translation at the text's origin
		Math::vec3 cursorTranslation{ textStartingPoint };

		// Get the cursor offset relative to the text's starting point
		Math::vec2 cursorOffset;
		float ascender = m_ActiveUI->m_Font->m_Ascender;
		if (textData.m_Text.size() == 0 || textData.m_CachedTextDimensions.y < 0.001f)
		{
			cursorOffset = textData.m_CachedTextDimensions;
		}
		else
		{
			cursorOffset = CalculateSingleLineText(std::string_view(textData.m_Text.data(), textData.m_CursorIndex));
			// Move the cursor back down by a half-extent
			cursorTranslation.y += textScalingFactor * 0.5f * ascender;
		}

		// Move the x-cursor to the end of the text
		cursorTranslation.x += cursorOffset.x * textScalingFactor;

		// Create the widget's background rendering data
		renderSpec.m_TransformMatrix = glm::translate(Math::mat4(1.0f),
			cursorTranslation)
			* glm::scale(Math::mat4(1.0f), Math::vec3(textScalingFactor * 0.05f, ascender * textScalingFactor, 1.0f));
		Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Math::vec4(1.0f),
			Utility::FileSystem::CRCFromString("a_Color"),
			renderSpec.m_Buffer, renderSpec.m_Shader);

		// Submit background data to GPU
		Rendering::RenderingService::SubmitDataToRenderer(renderSpec);
	}

	void RuntimeUIContext::RenderSliderLine(const Math::vec4& color, const Math::vec3& translation, const Math::vec3& size)
	{
		Rendering::RendererInputSpec& renderSpec = m_BackgroundInputSpec;

		Math::vec3 sliderSize = { size.x , 0.1f * size.y , size.z};

		if (color.w > 0.001f)
		{
			// Create the transform of the quad
			renderSpec.m_TransformMatrix = glm::translate(Math::mat4(1.0f),
				Math::vec3(translation.x + (sliderSize.x / 2.0f), translation.y + (size.y / 2.0f), translation.z))
				* glm::scale(Math::mat4(1.0f), sliderSize);
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(color,
				Utility::FileSystem::CRCFromString("a_Color"),
				renderSpec.m_Buffer, renderSpec.m_Shader);

			// Submit background data to GPU
			Rendering::RenderingService::SubmitDataToRenderer(renderSpec);
		}
	}

	void RuntimeUIContext::RenderSlider(const Math::vec4& color, const Math::vec3& translation, const Math::vec3& size)
	{
		Rendering::RendererInputSpec& renderSpec = m_BackgroundInputSpec;

		Math::vec3 sliderSize = { 0.04f * size.x , 0.35f * size.y , size.z };

		if (color.w > 0.001f)
		{
			// Create the widget's background rendering data
			renderSpec.m_TransformMatrix = glm::translate(Math::mat4(1.0f),
				Math::vec3(translation.x, translation.y + (size.y / 2.0f), translation.z))
				* glm::scale(Math::mat4(1.0f), sliderSize);
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(color,
				Utility::FileSystem::CRCFromString("a_Color"),
				renderSpec.m_Buffer, renderSpec.m_Shader);

			// Submit background data to GPU
			Rendering::RenderingService::SubmitDataToRenderer(renderSpec);
		}
	}

	Math::vec3 RuntimeUIContext::GetSingleLineTextStartingPosition(const SingleLineTextData& textData, const Math::vec3& translation, const Math::vec3 size, float textScalingFactor)
	{
		Math::vec3 translationOutput = translation;
		constexpr float k_CenterAdjustmentSize{ 2.6f }; // Magic number for adjusting the height of a line TODO: Find better solution

		// Place the starting x-location of the text widget based on the provided alignment option
		switch (textData.m_TextAlignment)
		{
		case Constraint::Left:
			break;
		case Constraint::Right:
			translationOutput.x = translation.x + (size.x) - ((textData.m_CachedTextDimensions.x) * textScalingFactor);
			break;
		case Constraint::Center:
			// Adjust current line translation to be centered
			translationOutput.x = translation.x + (size.x * 0.5f) - ((textData.m_CachedTextDimensions.x * 0.5f) * textScalingFactor);
			break;
		case Constraint::Bottom:
		case Constraint::Top:
		case Constraint::None:
			KG_ERROR("Invalid constraint type for aligning text {}", Utility::ConstraintToString(textData.m_TextAlignment));
			break;
		}

		// Set the starting y/z locations
		translationOutput.y = translation.y + (size.y * 0.5f) - ((textData.m_CachedTextDimensions.y * 0.5f) * textScalingFactor) + k_CenterAdjustmentSize;

		// Return starting point
		return translationOutput;
	}

	SelectionData* RuntimeUIContext::GetSelectionDataFromWidget(Widget* currentWidget)
	{
		KG_ASSERT(currentWidget);

		switch (currentWidget->m_WidgetType)
		{
		case WidgetTypes::ButtonWidget:
			return &((ButtonWidget*)currentWidget)->m_SelectionData;
		case WidgetTypes::ImageButtonWidget:
			return &((ImageButtonWidget*)currentWidget)->m_SelectionData;
		case WidgetTypes::CheckboxWidget:
			return &((CheckboxWidget*)currentWidget)->m_SelectionData;
		case WidgetTypes::InputTextWidget:
			return &((InputTextWidget*)currentWidget)->m_SelectionData;
		case WidgetTypes::SliderWidget:
			return &((SliderWidget*)currentWidget)->m_SelectionData;
		case WidgetTypes::DropDownWidget:
			return &((DropDownWidget*)currentWidget)->m_SelectionData;
		default:
			return nullptr;
		}
	}

	ContainerData* RuntimeUIContext::GetContainerDataFromWidget(Widget* currentWidget)
	{
		KG_ASSERT(currentWidget);

		switch (currentWidget->m_WidgetType)
		{
		case WidgetTypes::ContainerWidget:
			return &((ContainerWidget*)currentWidget)->m_ContainerData;
		case WidgetTypes::HorizontalContainerWidget:
			return &((HorizontalContainerWidget*)currentWidget)->m_ContainerData;
		case WidgetTypes::VerticalContainerWidget:
			return &((VerticalContainerWidget*)currentWidget)->m_ContainerData;
		default:
			return nullptr;
		}
	}

	ImageData* RuntimeUIContext::GetImageDataFromWidget(Widget* currentWidget)
	{
		KG_ASSERT(currentWidget);

		switch (currentWidget->m_WidgetType)
		{
		case WidgetTypes::ImageWidget:
			return &((ImageWidget*)currentWidget)->m_ImageData;
		case WidgetTypes::ImageButtonWidget:
			return &((ImageButtonWidget*)currentWidget)->m_ImageData;
		case WidgetTypes::CheckboxWidget:
			// Return the currently appropriate image data
			if (((CheckboxWidget*)currentWidget)->m_Checked)
			{
				return &((CheckboxWidget*)currentWidget)->m_ImageChecked;
			}
			else
			{
				return &((CheckboxWidget*)currentWidget)->m_ImageUnChecked;
			}
		default:
			return nullptr;
		}
	}

	SingleLineTextData* RuntimeUIContext::GetSingleLineTextDataFromWidget(Widget* currentWidget)
	{
		KG_ASSERT(currentWidget);

		switch (currentWidget->m_WidgetType)
		{
		case WidgetTypes::ButtonWidget:
			return &((ButtonWidget*)currentWidget)->m_TextData;
		case WidgetTypes::InputTextWidget:
			return &((InputTextWidget*)currentWidget)->m_TextData;
		default:
			return nullptr;
		}
	}

	MultiLineTextData* RuntimeUIContext::GetMultiLineTextDataFromWidget(Widget* currentWidget)
	{
		KG_ASSERT(currentWidget);

		if (currentWidget->m_WidgetType == WidgetTypes::TextWidget)
		{
			return &((TextWidget*)currentWidget)->m_TextData;
		}
		return nullptr;
	}

	void RuntimeUIContext::ClearActiveUI()
	{
		m_ActiveUI = nullptr;
		m_ActiveUIHandle = Assets::EmptyHandle;
	}

	Ref<UserInterface> RuntimeUIContext::GetActiveUI()
	{
		return m_ActiveUI;
	}

	Assets::AssetHandle RuntimeUIContext::GetActiveUIHandle()
	{
		return m_ActiveUIHandle;
	}

	void RuntimeUIContext::SetSelectedWidgetColor(const Math::vec4& color)
	{
		if (!m_ActiveUI->m_SelectedWidget)
		{
			return;
		}

		// Get the selection specific data from the widget
		SelectionData* selectionData = GetSelectionDataFromWidget(m_ActiveUI->m_SelectedWidget);
		if (!selectionData)
		{
			KG_WARN("Unable to retrieve selection data. May be invalid widget type!");
			return;
		}

		// Set the widget's active color
		selectionData->m_DefaultBackgroundColor = color;
	}

	const std::string& RuntimeUIContext::GetWidgetTextByIndex(WidgetID widgetID)
	{
		static std::string nullString{ "" };
		// Ensure the correct user interface is active
		if (widgetID.m_UserInterfaceID != m_ActiveUIHandle)
		{
			KG_WARN("Incorrect user interface provided when attempting to modify the active runtime user interface");
			return nullString;
		}

		// Search for the indicated widget
		Ref<Widget> widget = GetWidgetFromID(widgetID.m_WidgetID);

		KG_ASSERT(widget);

		SingleLineTextData* singleLineTextData = GetSingleLineTextDataFromWidget(widget.get());
		if (singleLineTextData)
		{
			return singleLineTextData->m_Text;
		}

		MultiLineTextData* multiLineTextData = GetMultiLineTextDataFromWidget(widget.get());
		if (multiLineTextData)
		{
			return multiLineTextData->m_Text;
		}

		KG_WARN("Provide widget does not yield a text data struct when attempting to get text");
		return nullString;
	}

	bool RuntimeUIContext::IsWidgetSelectedByTag(const std::string& windowTag, const std::string& widgetTag)
	{
		// Get the current widget
		Ref<Widget> currentWidget = GetWidgetFromTag(windowTag, widgetTag);

		return IsWidgetSelectedInternal(currentWidget);
	}

	bool RuntimeUIContext::IsWidgetSelectedByIndex(WidgetID widgetID)
	{
		// Ensure the correct user interface is active
		if (widgetID.m_UserInterfaceID != m_ActiveUIHandle)
		{
			KG_WARN("Incorrect user interface provided when attempting to modify the active runtime user interface");
			return false;
		}

		// Get the current widget
		Ref<Widget> currentWidget = GetWidgetFromID(widgetID.m_WidgetID);

		return IsWidgetSelectedInternal(currentWidget);
	}

	void RuntimeUIContext::SetActiveWidgetTextByTag(const std::string& windowTag, const std::string& widgetTag, const std::string& newText)
	{
		// Search for the indicated widget
		auto [currentWidget, currentWindow] = GetWidgetAndWindow(windowTag, widgetTag);
		SetWidgetTextInternal(currentWidget, newText);
		
	}

	void RuntimeUIContext::SetActiveWidgetTextByIndex(WidgetID widgetID, const std::string& newText)
	{
		// Ensure the correct user interface is active
		if (widgetID.m_UserInterfaceID != m_ActiveUIHandle)
		{
			KG_WARN("Incorrect user interface provided when attempting to modify the active runtime user interface");
			return;
		}

		// Search for the indicated widget
		auto [currentWidget, currentWindow] = GetWidgetAndWindow(widgetID.m_WidgetID);
		SetWidgetTextInternal(currentWidget, newText);
	}

	void RuntimeUIContext::SetWidgetImageByIndex(WidgetID widgetID, Assets::AssetHandle textureHandle)
	{
		// Ensure the correct user interface is active
		if (widgetID.m_UserInterfaceID != m_ActiveUIHandle)
		{
			KG_WARN("Incorrect user interface provided when attempting to modify the active runtime user interface");
			return;
		}

		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidgetFromID(widgetID.m_WidgetID);

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
		ImageData* imageData = GetImageDataFromWidget(m_ActiveUI->m_SelectedWidget);
		if (!imageData)
		{
			KG_WARN("Unable to retrieve image data. May be invalid widget type!");
			return;
		}

		imageData->m_ImageHandle = textureHandle;
		imageData->m_ImageRef = textureRef;
	}

	void RuntimeUIContext::SetSelectedWidgetByTag(const std::string& windowTag, const std::string& widgetTag)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidgetFromTag(windowTag, widgetTag);

		SetSelectedWidgetInternal(currentWidget);
	}

	void RuntimeUIContext::SetSelectedWidgetByIndex(WidgetID widgetID)
	{
		// Ensure the correct user interface is active
		if (widgetID.m_UserInterfaceID != m_ActiveUIHandle)
		{
			KG_WARN("Incorrect user interface provided when attempting to modify the active runtime user interface");
			return;
		}

		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidgetFromID(widgetID.m_WidgetID);

		SetSelectedWidgetInternal(currentWidget);
	}

	void RuntimeUIContext::ClearSelectedWidget()
	{
		// Set the new widget as selected and set it's color to the active color
		m_ActiveUI->m_SelectedWidget = nullptr;
	}

	void RuntimeUIContext::SetEditingWidgetByIndex(WidgetID widgetID)
	{
		// Ensure the correct user interface is active
		if (widgetID.m_UserInterfaceID != m_ActiveUIHandle)
		{
			KG_WARN("Incorrect user interface provided when attempting to modify the active runtime user interface");
			return;
		}

		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidgetFromID(widgetID.m_WidgetID);

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

		Ref<UserInterface> activeUI = m_ActiveUI;

		// If the indicated widget is the same as the current editing widget, just exit
		if (currentWidget.get() == activeUI->m_EditingWidget)
		{
			return;
		}

		// Set the new widget as editing
		activeUI->m_EditingWidget = currentWidget.get();
	}

	void RuntimeUIContext::SetHoveredWidgetByIndex(WidgetID widgetID)
	{
		// Ensure the correct user interface is active
		if (widgetID.m_UserInterfaceID != m_ActiveUIHandle)
		{
			KG_WARN("Incorrect user interface provided when attempting to modify the active runtime user interface");
			return;
		}

		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidgetFromID(widgetID.m_WidgetID);

		SetHoveredWidgetInternal(currentWidget);
	}

	void RuntimeUIContext::SetWidgetTextColorByTag(const std::string& windowTag, const std::string& widgetTag, const Math::vec4& color)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidgetFromTag(windowTag, widgetTag);

		SetWidgetTextColorInternal(currentWidget, color);
	}

	void RuntimeUIContext::SetWidgetTextColorByIndex(WidgetID widgetID, const Math::vec4& color)
	{
		// Ensure the correct user interface is active
		if (widgetID.m_UserInterfaceID != m_ActiveUIHandle)
		{
			KG_WARN("Incorrect user interface provided when attempting to modify the active runtime user interface");
			return;
		}

		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidgetFromID(widgetID.m_WidgetID);

		SetWidgetTextColorInternal(currentWidget, color);
	}

	void RuntimeUIContext::SetWidgetBackgroundColorByTag(const std::string& windowTag, const std::string& widgetTag, const Math::vec4& color)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidgetFromTag(windowTag, widgetTag);

		SetWidgetBackgroundColorInternal(currentWidget, color);
	}

	void RuntimeUIContext::SetWidgetBackgroundColorByIndex(WidgetID widgetID, const Math::vec4& color)
	{
		// Ensure the correct user interface is active
		if (widgetID.m_UserInterfaceID != m_ActiveUIHandle)
		{
			KG_WARN("Incorrect user interface provided when attempting to modify the active runtime user interface");
			return;
		}

		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidgetFromID(widgetID.m_WidgetID);

		SetWidgetBackgroundColorInternal(currentWidget, color);
	}

	void RuntimeUIContext::SetWidgetSelectableByTag(const std::string& windowTag, const std::string& widgetTag, bool selectable)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidgetFromTag(windowTag, widgetTag);

		SetWidgetSelectableInternal(currentWidget, selectable);
	}

	void RuntimeUIContext::SetWidgetSelectableByIndex(WidgetID widgetID, bool selectable)
	{
		// Ensure the correct user interface is active
		if (widgetID.m_UserInterfaceID != m_ActiveUIHandle)
		{
			KG_WARN("Incorrect user interface provided when attempting to modify the active runtime user interface");
			return;
		}

		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidgetFromID(widgetID.m_WidgetID);

		SetWidgetSelectableInternal(currentWidget, selectable);
	}

	void RuntimeUIContext::SetActiveOnMove(Assets::AssetHandle functionHandle, Ref<Scripting::Script> function)
	{
		m_ActiveUI->m_FunctionPointers.m_OnMove = function;
		m_ActiveUI->m_FunctionPointers.m_OnMoveHandle = functionHandle;
	}

	void RuntimeUIContext::SetActiveOnHover(Assets::AssetHandle functionHandle, Ref<Scripting::Script> function)
	{
		m_ActiveUI->m_FunctionPointers.m_OnHover = function;
		m_ActiveUI->m_FunctionPointers.m_OnHoverHandle = functionHandle;
	}

	Ref<Scripting::Script> RuntimeUIContext::GetActiveOnMove()
	{
		return m_ActiveUI->m_FunctionPointers.m_OnMove;
	}

	Assets::AssetHandle RuntimeUIContext::GetActiveOnMoveHandle()
	{
		return m_ActiveUI->m_FunctionPointers.m_OnMoveHandle;
	}

	void RuntimeUIContext::SetDisplayWindowByTag(const std::string& windowTag, bool display)
	{
		// Search for the indicated window
		for (Window& window : m_ActiveUI->m_Windows)
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

	void RuntimeUIContext::SetDisplayWindowByIndex(WindowID windowID, bool display)
	{
		// Ensure the correct user interface is active
		if (windowID.m_UserInterfaceID != m_ActiveUIHandle)
		{
			KG_WARN("Incorrect user interface provided when attempting to modify the active runtime user interface");
			return;
		}

		// Get the ID -> Widget Location map and location directions
		IDToLocationMap& idToLocationMap = m_ActiveUI->m_IDToLocation;
		KG_ASSERT(idToLocationMap.contains(windowID.m_WindowID));
		std::vector<uint16_t>& locationDirections = idToLocationMap.at(windowID.m_WindowID);

		// Get widget using location directions
		// TODO: This currently assumes the window -> widget structure
		KG_ASSERT(locationDirections.size() == 1); // TODO: Remove this later
		size_t windowIndex = locationDirections.at(0);

		std::vector<Window>& activeWindows = m_ActiveUI->m_Windows;

		if (windowIndex > (activeWindows.size() - 1))
		{
			KG_WARN("Provided window ID is out of bounds for the UI's windows");
			return;
		}

		Window& window = activeWindows.at(windowIndex);
		
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

	void RuntimeUIContext::MoveRight()
	{
		// Ensure the selected widget is valid
		if (!m_ActiveUI || !m_ActiveUI->m_SelectedWidget || !m_ActiveUI->m_ActiveWindow)
		{
			return;
		}

		// Get the selected widget's selection data
		SelectionData* originalSelectionData = GetSelectionDataFromWidget(m_ActiveUI->m_SelectedWidget);
		KG_ASSERT(originalSelectionData);

		// Move to the right
		if (originalSelectionData->m_NavigationLinks.m_RightWidgetID != k_InvalidWidgetID)
		{
			// Set the new selected widget
			m_ActiveUI->m_SelectedWidget = GetWidgetFromID(originalSelectionData->m_NavigationLinks.m_RightWidgetID).get();

			// Call the on move function if applicable
			if (m_ActiveUI->m_FunctionPointers.m_OnMove)
			{
				Utility::CallWrapped<WrappedVoidNone>(m_ActiveUI->m_FunctionPointers.m_OnMove->m_Function);
			}

			// Handle modifying the editing widget
			if (m_ActiveUI->m_EditingWidget != m_ActiveUI->m_SelectedWidget)
			{
				ClearEditingWidget();
			}
		}
	}

	void RuntimeUIContext::MoveLeft()
	{
		// Ensure the selected widget is valid
		if (!m_ActiveUI || !m_ActiveUI->m_SelectedWidget || !m_ActiveUI->m_ActiveWindow)
		{
			return;
		}

		// Get the selected widget's selection data
		SelectionData* originalSelectionData = GetSelectionDataFromWidget(m_ActiveUI->m_SelectedWidget);
		KG_ASSERT(originalSelectionData);

		// Move to the left
		if (originalSelectionData->m_NavigationLinks.m_LeftWidgetID != k_InvalidWidgetID)
		{

			// Set the new selected widget
			m_ActiveUI->m_SelectedWidget = GetWidgetFromID(originalSelectionData->m_NavigationLinks.m_LeftWidgetID).get();

			// Call the on move function if applicable
			if (m_ActiveUI->m_FunctionPointers.m_OnMove)
			{
				Utility::CallWrapped<WrappedVoidNone>(m_ActiveUI->m_FunctionPointers.m_OnMove->m_Function);
			}

			// Handle modifying the editing widget
			if (m_ActiveUI->m_EditingWidget != m_ActiveUI->m_SelectedWidget)
			{
				ClearEditingWidget();
			}
		}
	}

	void RuntimeUIContext::MoveUp()
	{
		// Ensure the selected widget is valid
		if (!m_ActiveUI || !m_ActiveUI->m_SelectedWidget || !m_ActiveUI->m_ActiveWindow)
		{
			return;
		}

		// Get the selected widget's selection data
		SelectionData* originalSelectionData = GetSelectionDataFromWidget(m_ActiveUI->m_SelectedWidget);
		KG_ASSERT(originalSelectionData);

		// Move up
		if (originalSelectionData->m_NavigationLinks.m_UpWidgetID != k_InvalidWidgetID)
		{
			// Set the new selected widget
			m_ActiveUI->m_SelectedWidget = GetWidgetFromID(originalSelectionData->m_NavigationLinks.m_UpWidgetID).get();

			// Get the new widget's selection data
			SelectionData* newSelectionData = GetSelectionDataFromWidget(m_ActiveUI->m_SelectedWidget);
			KG_ASSERT(newSelectionData);

			// Call the on move function if applicable
			if (m_ActiveUI->m_FunctionPointers.m_OnMove)
			{
				Utility::CallWrapped<WrappedVoidNone>(m_ActiveUI->m_FunctionPointers.m_OnMove->m_Function);
			}

			// Handle modifying the editing widget
			if (m_ActiveUI->m_EditingWidget != m_ActiveUI->m_SelectedWidget)
			{
				ClearEditingWidget();
			}
		}
	}

	void RuntimeUIContext::MoveDown()
	{
		// Ensure the selected widget is valid
		if (!m_ActiveUI || !m_ActiveUI->m_SelectedWidget || !m_ActiveUI->m_ActiveWindow)
		{
			return;
		}

		// Get the selected widget's selection data
		SelectionData* originalSelectionData = GetSelectionDataFromWidget(m_ActiveUI->m_SelectedWidget);
		KG_ASSERT(originalSelectionData);

		// Move down
		if (originalSelectionData->m_NavigationLinks.m_DownWidgetID != k_InvalidWidgetID)
		{
			// Set the new selected widget
			
			m_ActiveUI->m_SelectedWidget = GetWidgetFromID(originalSelectionData->m_NavigationLinks.m_DownWidgetID).get();

			// Call the on move function if applicable
			if (m_ActiveUI->m_FunctionPointers.m_OnMove)
			{
				Utility::CallWrapped<WrappedVoidNone>(m_ActiveUI->m_FunctionPointers.m_OnMove->m_Function);
			}

			// Handle modifying the editing widget
			if (m_ActiveUI->m_EditingWidget != m_ActiveUI->m_SelectedWidget)
			{
				ClearEditingWidget();
			}
		}
	}

	void RuntimeUIContext::OnPress()
	{
		Widget* currentWidget = m_ActiveUI->m_SelectedWidget;

		// Ensure selected widget is valid
		if (!currentWidget) 
		{
			return;
		}

		OnPressInternal(currentWidget);
	}

	void RuntimeUIContext::OnPressByIndex(WidgetID widgetID)
	{
		// Ensure the correct user interface is active
		if (widgetID.m_UserInterfaceID != m_ActiveUIHandle)
		{
			KG_WARN("Incorrect user interface provided when attempting to modify the active runtime user interface");
			return;
		}

		// Get the current widget
		Ref<Widget> currentWidget = GetWidgetFromID(widgetID.m_WidgetID);

		if (!currentWidget)
		{
			return;
		}

		OnPressInternal(currentWidget.get());
	}

	

	void RuntimeUIContext::RecalculateTextData(Widget* widget)
	{
		KG_ASSERT(widget);

		// Revalidate text dimensions for widget
		switch (widget->m_WidgetType)
		{
		case WidgetTypes::TextWidget:
			(*(TextWidget*)widget).CalculateTextSize();
			break;
		case WidgetTypes::ButtonWidget:
			(*(ButtonWidget*)widget).CalculateTextSize();
			break;
		case WidgetTypes::InputTextWidget:
			(*(InputTextWidget*)widget).CalculateTextSize();
			break;
		case WidgetTypes::DropDownWidget:
			(*(DropDownWidget*)widget).CalculateTextSize();
			break;
		case WidgetTypes::ContainerWidget:
		case WidgetTypes::HorizontalContainerWidget:
		case WidgetTypes::VerticalContainerWidget:
		{
			ContainerData* containerData = GetContainerDataFromWidget(widget);
			KG_ASSERT(containerData);
			for (Ref<Widget> widget : containerData->m_ContainedWidgets)
			{
				RecalculateTextData(widget.get());
			}
			break;
		}
		default:
			break;
		}
	}

	void RuntimeUIContext::CalculateFixedAspectRatioSize(Widget* widget, uint32_t viewportWidth, uint32_t viewportHeight, bool useXValueAsBase)
	{
		KG_ASSERT(widget);

		BoundingBoxTransform parentTransform = GetParentDimensionsFromID(widget->m_ID, viewportWidth, viewportHeight);
		
		// Get the image data from the provided widget
		ImageData* currentImageData = GetImageDataFromWidget(widget);
		if (!currentImageData)
		{
			KG_WARN("Attempt to recalculate fixed aspect ratio of a widget that does not support images.");
			return;
		}

		// Ensure there is an image to base the aspect ratio off of
		if (!currentImageData->m_ImageRef)
		{
			return;
		}

		// Get the aspect ratio from the image value as a vec2
		Assets::AssetInfo textureInfo = Assets::AssetService::GetTexture2DInfo(currentImageData->m_ImageHandle);
		Assets::TextureMetaData* textureMetadata = textureInfo.Data.GetSpecificMetaData<Assets::TextureMetaData>();
		Math::vec2 textureAspectRatio = Math::vec2((float)textureMetadata->Width, (float)textureMetadata->Height);

		// Ensure we avoid division by 0
		if (textureMetadata->Width == 0 || textureMetadata->Height == 0)
		{
			KG_WARN("Unable to enforce fixed aspect ratio because the indicated texture's dimensions are invalid");
			return;
		}


		if (useXValueAsBase)
		{
			// Normalize the aspect ratio based on the x-value
			textureAspectRatio.y = textureAspectRatio.y / textureAspectRatio.x;
			textureAspectRatio.x = 1.0f;

			// Use the normalized y-value ratio to calculate the new y-value...

			// For the pixel dimensions
			widget->m_PixelSize.y = (int)((float)widget->m_PixelSize.x * textureAspectRatio.y);

			// And for the percentage dimensions
			widget->m_PercentSize.y = ((parentTransform.m_Size.x * widget->m_PercentSize.x) * textureAspectRatio.y) / parentTransform.m_Size.y;
		}
		else
		{
			// Normalize the aspect ratio based on the y-value
			textureAspectRatio.x = textureAspectRatio.x / textureAspectRatio.y;
			textureAspectRatio.y = 1.0f;

			// Use the normalized y-value ratio to calculate the new y-value...

			// For the pixel dimensions
			widget->m_PixelSize.x = (int)((float)widget->m_PixelSize.y * textureAspectRatio.x);

			// And for the percentage dimensions
			widget->m_PercentSize.x = ((parentTransform.m_Size.y * widget->m_PercentSize.y) * textureAspectRatio.x) / parentTransform.m_Size.x;
		}
		
	}

	Ref<Widget> RuntimeUIContext::GetWidgetFromTag(const std::string& windowTag, const std::string& widgetTag)
	{
		if (!m_ActiveUI)
		{
			KG_WARN("Attempt to get a widget from the active user interface, however, no UI is active!");
			return nullptr;
		}

		// Get widget using its parent window tag and its widget tag
		for (Window& window : m_ActiveUI->m_Windows)
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

	Ref<Widget> RuntimeUIContext::GetWidgetFromID(int32_t widgetID)
	{
		KG_ASSERT(m_ActiveUI);

		// Get the ID -> Widget Location map and location directions
		IDToLocationMap& idToLocationMap = 
			m_ActiveUI->m_IDToLocation;
		KG_ASSERT(idToLocationMap.contains(widgetID));
		std::vector<uint16_t>& locationDirections = idToLocationMap.at(widgetID);

		// Ensure directions to a widget a presented
		KG_ASSERT(locationDirections.size() > 1);

		size_t windowIndex = locationDirections.at(0);
		size_t firstWidgetIndex = locationDirections.at(1);

		// Ensure window index is within bounds
		if (windowIndex > (m_ActiveUI->m_Windows.size() - 1))
		{
			KG_WARN("Attempt to retrieve a widget but the window index was out of bounds");
			return nullptr;
		}
		// Get the indiciated window
		Window& currentWindow = m_ActiveUI->m_Windows.at((size_t)windowIndex);
		
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
		for (size_t iteration{ 2 }; iteration < locationDirections.size(); iteration++)
		{
			// Get the container data for this widget
			ContainerData* data = GetContainerDataFromWidget(currentWidget.get());
			KG_ASSERT(data);

			// Ensure the widget directions fall within the bounds of the widget array
			size_t currentIndex = locationDirections.at(iteration);
			KG_ASSERT(currentIndex < data->m_ContainedWidgets.size());

			// Set the current widget to the indicated container widget (Window -> Widget -> [Widget])
			currentWidget = data->m_ContainedWidgets.at(currentIndex);
		}

		// Return the nested widget
		return currentWidget;
	}

	Ref<Widget> RuntimeUIContext::GetWidgetFromDirections(const std::vector<uint16_t>& locationDirections)
	{
		KG_ASSERT(m_ActiveUI);

		// Ensure directions to a widget a presented
		KG_ASSERT(locationDirections.size() > 1);

		// Get widget using location directions
		KG_ASSERT(locationDirections.size() == 2); // TODO: Remove this later
		size_t windowIndex = locationDirections.at(0);
		size_t widgetIndex = locationDirections.at(1);

		// Ensure window index is within bounds
		if (windowIndex > (m_ActiveUI->m_Windows.size() - 1))
		{
			KG_WARN("Attempt to retrieve a widget but the window index was out of bounds");
			return nullptr;
		}

		// Get the indicated window
		Window& currentWindow = m_ActiveUI->m_Windows.at((size_t)windowIndex);

		// Ensure widget index is within bounds
		if (widgetIndex > (currentWindow.m_Widgets.size() - 1))
		{
			KG_WARN("Attempt to retrieve a widget but the widget index was out of bounds");
			return nullptr;
		}

		// Return the indicated widget and window
		return currentWindow.m_Widgets.at(widgetIndex);
	}

	IDType RuntimeUIContext::CheckIDType(int32_t windowOrWidgetID)
	{
		KG_ASSERT(m_ActiveUI);

		// Get the ID -> Widget Location map and location directions
		IDToLocationMap& idToLocationMap = 
			m_ActiveUI->m_IDToLocation;

		// Check for invalid ID
		if (!idToLocationMap.contains(windowOrWidgetID) || windowOrWidgetID == k_InvalidWidgetID)
		{
			return IDType::None;
		}

		std::vector<uint16_t>& locationDirections = idToLocationMap.at(windowOrWidgetID);

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

	Window& RuntimeUIContext::GetWindowFromID(int32_t windowID)
	{
		KG_ASSERT(m_ActiveUI);

		// Get the window location
		std::vector<uint16_t>* locationDir = GetLocationFromID(windowID);

		// Ensure the location is valid and represents a window
		KG_ASSERT(locationDir);
		KG_ASSERT(locationDir->size() == 1);
		uint16_t windowLocation{ locationDir->at(0) };

		// Ensure the window location leads to a valid window instance
		KG_ASSERT(windowLocation < m_ActiveUI->m_Windows.size());

		// Return the window
		return m_ActiveUI->m_Windows.at(windowLocation);
	}

	Window& RuntimeUIContext::GetParentWindowFromWidgetID(int32_t widgetID)
	{
		KG_ASSERT(m_ActiveUI);

		// Get the widget location
		std::vector<uint16_t>* locationDir = GetLocationFromID(widgetID);

		// Ensure the location is valid and represents a widget
		KG_ASSERT(locationDir);
		KG_ASSERT(locationDir->size() > 1);
		uint16_t windowLocation{ locationDir->at(0) };

		// Ensure the window location leads to a valid window instance
		KG_ASSERT(windowLocation < m_ActiveUI->m_Windows.size());

		// Return the window
		return m_ActiveUI->m_Windows.at(windowLocation);
	}

	Ref<Widget> RuntimeUIContext::GetParentWidgetFromID(int32_t widgetID)
	{
		// Get the widget location
		std::vector<uint16_t>* locationDir = GetLocationFromID(widgetID);

		// Ensure the location is valid and represents a widget
		KG_ASSERT(locationDir);
		KG_ASSERT(locationDir->size() > 1);

		// If the parent of the widget is a window (UI->Window(1)->Widget(2)), return null
		if (locationDir->size() == 2)
		{
			return nullptr;
		}

		// Get the location directions for the parent widget
		std::vector<uint16_t> parentLocationDir = *locationDir;
		parentLocationDir.pop_back();

		// Return the parent widget
		return GetWidgetFromDirections(parentLocationDir);
	}

	std::vector<uint16_t>* RuntimeUIContext::GetLocationFromID(int32_t windowOrWidgetID)
	{
		// Get the ID -> Widget Location map and location directions
		IDToLocationMap& idToLocationMap = 
			m_ActiveUI->m_IDToLocation;

		// Check for invalid ID
		if (!idToLocationMap.contains(windowOrWidgetID) || windowOrWidgetID == k_InvalidWidgetID)
		{
			return nullptr;
		}

		return &(idToLocationMap.at(windowOrWidgetID));
	}

	std::tuple<Ref<Widget>, Window*> RuntimeUIContext::GetWidgetAndWindow(const std::string& windowTag, const std::string& widgetTag)
	{
		// Get widget using its parent window tag and its widget tag
		for (Window& window : m_ActiveUI->m_Windows)
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

	std::tuple<Ref<Widget>, Window*> RuntimeUIContext::GetWidgetAndWindow(int32_t widgetID)
	{
		// Ensure a user interface is active
		if (!m_ActiveUI)
		{
			KG_WARN("Attempt to get a widget from the active user interface, however, no UI is active!");
			return {nullptr, nullptr};
		}

		// Get the ID -> Widget Location map and location directions
		IDToLocationMap& idToLocationMap = 
			m_ActiveUI->m_IDToLocation;
		KG_ASSERT(idToLocationMap.contains(widgetID));
		std::vector<uint16_t>& locationDirections = idToLocationMap.at(widgetID);

		// Check for a window ID
		if (locationDirections.size() == 1)
		{
			KG_WARN("Attempt to retrieve a window and widget, however, found only a window ID");
			return { nullptr, nullptr };
		}

		// Get widget using location directions
		// TODO: This currently assumes the window -> widget structure
		KG_ASSERT(locationDirections.size() == 2); // TODO: Remove this later
		size_t windowIndex = locationDirections.at(0);
		size_t widgetIndex = locationDirections.at(1);

		// Ensure window index is within bounds
		if (windowIndex > (m_ActiveUI->m_Windows.size() - 1))
		{
			KG_WARN("Attempt to retrieve a widget but the window index was out of bounds");
			return { nullptr, nullptr };
		}
		// Get the indiciated window
		Window& currentWindow = m_ActiveUI->m_Windows.at((size_t)windowIndex);

		// Ensure widget index is within bounds
		if (widgetIndex > (currentWindow.m_Widgets.size() - 1))
		{
			KG_WARN("Attempt to retrieve a widget but the widget index was out of bounds");
			return { nullptr, nullptr };
		}

		// Return the indicated widget and window
		return { currentWindow.m_Widgets.at(widgetIndex), &currentWindow };
	}

	void NavigationLinksCalculator::CalculateNavigationLinks()
	{
		// Iterate through all windows 
		for (Window& currentWindow : i_UserInterface->m_Windows)
		{
			// Store the current window
			m_CurrentWindow = &currentWindow;

			// Calculate window scale and position
			m_CurrentWindowTransform.m_Translation = currentWindow.CalculateWorldPosition(i_ViewportData.m_Width, i_ViewportData.m_Height);
			m_CurrentWindowTransform.m_Size = currentWindow.CalculateSize(i_ViewportData.m_Width, i_ViewportData.m_Height);

			// Iterate through all widgets in the window
			m_CurrentWidgetParentTransform = m_CurrentWindowTransform;
			for (Ref<Widget> currentWidget : currentWindow.m_Widgets)
			{
				CalculateWidgetNavigationLinks(currentWidget.get());
			}
		}
	}

	void NavigationLinksCalculator::CalculateWidgetNavigationLinks(Widget* currentWidget)
	{
		// Handle container widgets
		ContainerData* containerData = RuntimeUIService::GetActiveContext().GetContainerDataFromWidget(currentWidget);
		if (containerData)
		{
			// Calculate the current widget's transform information
			BoundingBoxTransform cachedTransform;
			cachedTransform.m_Translation = currentWidget->CalculateWorldPosition(
				m_CurrentWidgetParentTransform.m_Translation, m_CurrentWidgetParentTransform.m_Size);
			cachedTransform.m_Size = currentWidget->CalculateWidgetSize(m_CurrentWidgetParentTransform.m_Size);
			m_CurrentWidgetParentTransform = cachedTransform;

			if (currentWidget->m_WidgetType == WidgetTypes::HorizontalContainerWidget)
			{
				// Handle the verical container case
				HorizontalContainerWidget* horizContainer = (HorizontalContainerWidget*)currentWidget;
				KG_ASSERT(horizContainer);
				size_t iteration{ 0 };
				for (Ref<Widget> containedWidget : containerData->m_ContainedWidgets)
				{
					m_CurrentWidgetParentTransform.m_Translation.x = cachedTransform.m_Translation.x +
						cachedTransform.m_Size.x * horizContainer->m_ColumnWidth * iteration +
						cachedTransform.m_Size.x * horizContainer->m_ColumnSpacing * iteration;
					m_CurrentWidgetParentTransform.m_Size.x = cachedTransform.m_Size.x * horizContainer->m_ColumnWidth;
					CalculateWidgetNavigationLinks(containedWidget.get());
					iteration++;
				}
			}
			else if (currentWidget->m_WidgetType == WidgetTypes::VerticalContainerWidget)
			{
				// Handle the verical container case
				VerticalContainerWidget* vertContainer = (VerticalContainerWidget*)currentWidget;
				KG_ASSERT(vertContainer);
				size_t iteration{ 0 };
				for (Ref<Widget> containedWidget : containerData->m_ContainedWidgets)
				{
					m_CurrentWidgetParentTransform.m_Translation.y = cachedTransform.m_Translation.y + cachedTransform.m_Size.y -
						cachedTransform.m_Size.y * vertContainer->m_RowHeight * (iteration + 1) - 
						cachedTransform.m_Size.y * vertContainer->m_RowSpacing * iteration;
					m_CurrentWidgetParentTransform.m_Size.y = cachedTransform.m_Size.y * vertContainer->m_RowHeight;
					CalculateWidgetNavigationLinks(containedWidget.get());
					iteration++;
				}
			}
			else
			{
				for (Ref<Widget> containedWidget : containerData->m_ContainedWidgets)
				{
					// Calculate the navigation links for each contained widget
					CalculateWidgetNavigationLinks(containedWidget.get());
				}
			}
		}

		// Ensure the widget is selectable
		if (!currentWidget->Selectable())
		{
			return;
		}

		// Get the selection specific data from the widget
		SelectionData* selectionData = RuntimeUIService::GetActiveContext().GetSelectionDataFromWidget(currentWidget);
		if (!selectionData)
		{
			KG_WARN("Unable to retrieve selection data. May be invalid widget type!");
			return;
		}

		// Calculate the position and size of the current widget
		m_CurrentWidget = currentWidget;
		m_CurrentWidgetPosition = m_CurrentWidget->CalculateWorldPosition
		(
			m_CurrentWidgetParentTransform.m_Translation, 
			m_CurrentWidgetParentTransform.m_Size
		);
		m_CurrentWidgetSize = m_CurrentWidget->CalculateWidgetSize(m_CurrentWidgetParentTransform.m_Size);
		m_CurrentWidgetCenterPosition = 
		{ 
			m_CurrentWidgetPosition.x + (m_CurrentWidgetSize.x * 0.5f), 
			m_CurrentWidgetPosition.y + (m_CurrentWidgetSize.y * 0.5f) 
		};

		// Calculate navigation links for the current widget
		selectionData->m_NavigationLinks.m_RightWidgetID = CalculateNavigationLink(Direction::Right);
		selectionData->m_NavigationLinks.m_LeftWidgetID = CalculateNavigationLink(Direction::Left);
		selectionData->m_NavigationLinks.m_UpWidgetID = CalculateNavigationLink(Direction::Up);
		selectionData->m_NavigationLinks.m_DownWidgetID = CalculateNavigationLink(Direction::Down);
	}

	int32_t NavigationLinksCalculator::CalculateNavigationLink(Direction direction)
	{
		// Initialize calculation variables
		m_CurrentBestChoiceID = k_InvalidWidgetID;
		m_CurrentBestDistance = std::numeric_limits<float>::max();
		m_CurrentDirection = direction;

		// Iterate through each potential widget and decide which widget makes sense to navigate to
		for (Ref<Widget> potentialChoice : m_CurrentWindow->m_Widgets)
		{
			m_PotentialWidgetParentTransform = m_CurrentWindowTransform;
			CompareCurrentAndPotentialWidget(potentialChoice.get());
		}

		// Return the index of the best widget choice if it exists
		return m_CurrentBestChoiceID;
	}

	void NavigationLinksCalculator::CompareCurrentAndPotentialWidget(Widget* potentialWidget)
	{
		// Calculate the position and size of the potential widget
		Math::vec3 potentialWidgetPosition = potentialWidget->CalculateWorldPosition
		(
			m_PotentialWidgetParentTransform.m_Translation,
			m_PotentialWidgetParentTransform.m_Size
		);
		Math::vec3 potentialWidgetSize = potentialWidget->CalculateWidgetSize(m_PotentialWidgetParentTransform.m_Size);
		Math::vec2 potentialWidgetCenterPosition = { potentialWidgetPosition.x + (potentialWidgetSize.x * 0.5f), potentialWidgetPosition.y + (potentialWidgetSize.y * 0.5f) };

		// Check if the widget has nested widgets
		ContainerData* containerData = RuntimeUIService::GetActiveContext().GetContainerDataFromWidget(potentialWidget);
		m_PotentialWidgetParentTransform.m_Translation = potentialWidgetPosition;
		m_PotentialWidgetParentTransform.m_Size = potentialWidgetSize;

		if (containerData)
		{
			if (potentialWidget->m_WidgetType == WidgetTypes::HorizontalContainerWidget)
			{
				// Handle the verical container case
				size_t iteration{ 0 };
				for (Ref<Widget> containedWidget : containerData->m_ContainedWidgets)
				{
					HorizontalContainerWidget* horizContainer = (HorizontalContainerWidget*)potentialWidget;
					KG_ASSERT(horizContainer);
					m_PotentialWidgetParentTransform.m_Translation.x = potentialWidgetPosition.x + 
						potentialWidgetSize.x * horizContainer->m_ColumnWidth * iteration +
						potentialWidgetSize.x * horizContainer->m_ColumnSpacing * iteration;
					m_PotentialWidgetParentTransform.m_Size.x = potentialWidgetSize.x * horizContainer->m_ColumnWidth;
					CompareCurrentAndPotentialWidget(containedWidget.get());
					iteration++;
				}
			}
			else if (potentialWidget->m_WidgetType == WidgetTypes::VerticalContainerWidget)
			{
				// Handle the verical container case
				size_t iteration{ 0 };
				for (Ref<Widget> containedWidget : containerData->m_ContainedWidgets)
				{
					VerticalContainerWidget* vertContainer = (VerticalContainerWidget*)potentialWidget;
					KG_ASSERT(vertContainer);
					m_PotentialWidgetParentTransform.m_Translation.y = potentialWidgetPosition.y + potentialWidgetSize.y - potentialWidgetSize.y * vertContainer->m_RowHeight * (iteration + 1) - 
						potentialWidgetSize.y * vertContainer->m_RowSpacing * iteration;
					m_PotentialWidgetParentTransform.m_Size.y = potentialWidgetSize.y * vertContainer->m_RowHeight;
					CompareCurrentAndPotentialWidget(containedWidget.get());
					iteration++;
				}
			}
			else
			{
				// Handle the regular frame container case
				for (Ref<Widget> containedWidget : containerData->m_ContainedWidgets)
				{
					CompareCurrentAndPotentialWidget(containedWidget.get());
				}
			}
		}

		// Skip the current widget and any non-selectable widgets
		if (potentialWidget == m_CurrentWidget || !potentialWidget->Selectable())
		{
			return;
		}


		// Check if the potential widget is within the constraints of the current widget
		float extentDistance{ 0.0f };
		float singleDimensionDistance{ 0.0f };
		float currentWidgetExtent{ 0.0f };
		float potentialWidgetExtent{ 0.0f };

		switch (m_CurrentDirection)
		{
		case Direction::Right:
			// Ensure the current widget's left extent does not overlap with the potential widget's right extent
			currentWidgetExtent = m_CurrentWidgetPosition.x + m_CurrentWidgetSize.x;
			potentialWidgetExtent = potentialWidgetPosition.x;
			if (currentWidgetExtent >= potentialWidgetExtent)
			{
				return;
			}
			// Calculate the distance between the extents
			extentDistance = glm::distance(Math::vec2(currentWidgetExtent, m_CurrentWidgetCenterPosition.y), Math::vec2(potentialWidgetExtent, potentialWidgetCenterPosition.y));
			singleDimensionDistance = glm::distance(m_CurrentWidgetCenterPosition.y, potentialWidgetCenterPosition.y);
			break;
		case Direction::Left:
			// Ensure the current widget's right extent does not overlap with the potential widget's left extent
			currentWidgetExtent = m_CurrentWidgetPosition.x;
			potentialWidgetExtent = potentialWidgetPosition.x + potentialWidgetSize.x;
			if (currentWidgetExtent <= potentialWidgetExtent)
			{
				return;
			}
			// Calculate the distance between the extents
			extentDistance = glm::distance(Math::vec2(currentWidgetExtent, m_CurrentWidgetCenterPosition.y), Math::vec2(potentialWidgetExtent, potentialWidgetCenterPosition.y));
			singleDimensionDistance = glm::distance(m_CurrentWidgetCenterPosition.y, potentialWidgetCenterPosition.y);
			break;
		case Direction::Up:
			// Ensure the current widget's top extent does not overlap with the potential widget's bottom extent
			currentWidgetExtent = m_CurrentWidgetPosition.y + m_CurrentWidgetSize.y;
			potentialWidgetExtent = potentialWidgetPosition.y;
			if (currentWidgetExtent >= potentialWidgetExtent)
			{
				return;
			}
			// Calculate the distance between the extents
			extentDistance = glm::distance(Math::vec2(m_CurrentWidgetCenterPosition.x, currentWidgetExtent), Math::vec2(potentialWidgetCenterPosition.x, potentialWidgetExtent));
			singleDimensionDistance = glm::distance(m_CurrentWidgetCenterPosition.x, potentialWidgetCenterPosition.x);
			break;
		case Direction::Down:
			// Ensure the current widget's bottom extent does not overlap with the potential widget's top extent
			currentWidgetExtent = m_CurrentWidgetPosition.y;
			potentialWidgetExtent = potentialWidgetPosition.y + potentialWidgetSize.y;
			if (currentWidgetExtent <= potentialWidgetExtent)
			{
				return;
			}
			// Calculate the distance between the extents
			extentDistance = glm::distance(Math::vec2(m_CurrentWidgetCenterPosition.x, currentWidgetExtent), Math::vec2(potentialWidgetCenterPosition.x, potentialWidgetExtent));
			singleDimensionDistance = glm::distance(m_CurrentWidgetCenterPosition.x, potentialWidgetCenterPosition.x);
			break;
		default:
			KG_ERROR("Invalid direction provided when calculating navigation links for active user interface");
			break;
		}

		// Magic number found by tinkering with settings until it felt right...
		constexpr float singleDimensionAdjustment{ 0.65f };

		// Calculate final distance factor
		float finalDistanceFactor = extentDistance + singleDimensionAdjustment * singleDimensionDistance;

		// Save current best choice if it is the first choice
		if (m_CurrentBestChoiceID == k_InvalidWidgetID)
		{
			m_CurrentBestChoiceID = potentialWidget->m_ID;
			m_CurrentBestDistance = finalDistanceFactor;
			return;
		}


		// Replace current best choice with the potential choice if it is closer
		if (finalDistanceFactor < m_CurrentBestDistance)
		{
			m_CurrentBestChoiceID = potentialWidget->m_ID;
			m_CurrentBestDistance = finalDistanceFactor;
			return;
		}
	}
}
#include "kgpch.h"

#include "Kargono/RuntimeUI/RuntimeUI.h"

#include "Kargono/Assets/AssetService.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Core/Window.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Rendering/RenderingService.h"
#include "Kargono/Rendering/Shader.h"
#include "Kargono/ECS/EngineComponents.h"

namespace Kargono::RuntimeUI
{
	struct RuntimeUIContext
	{
		Ref<UserInterface> m_ActiveUI{ nullptr };
		Assets::AssetHandle m_ActiveUIHandle{ Assets::EmptyHandle };
		Ref<Font> m_DefaultFont{ nullptr };
		Rendering::RendererInputSpec m_BackgroundInputSpec{};
	};

	void RuntimeUIService::Init()
	{
		// Initialize Runtime UI Context
		s_RuntimeUIContext = CreateRef<RuntimeUIContext>();
		s_RuntimeUIContext->m_ActiveUI = nullptr;
		s_RuntimeUIContext->m_ActiveUIHandle = Assets::EmptyHandle;
		s_RuntimeUIContext->m_DefaultFont = FontService::InstantiateEditorFont("Resources/Fonts/arial.ttf");

		// Initialize Window/Widget Rendering Data
		{
			// Create shader for UI background/quad rendering
			Rendering::ShaderSpecification shaderSpec {Rendering::ColorInputType::FlatColor, Rendering::TextureInputType::None, false, true, true, Rendering::RenderingType::DrawIndex, false};
			auto [uuid, localShader] = Assets::AssetService::GetShader(shaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>({ 1.0f, 1.0f, 1.0f, 1.0f }, "a_Color", localBuffer, localShader);

			// Create basic shape component for UI quad rendering
			ECS::ShapeComponent* shapeComp = new ECS::ShapeComponent();
			shapeComp->CurrentShape = Rendering::ShapeTypes::Quad;
			shapeComp->Vertices = CreateRef<std::vector<Math::vec3>>(Rendering::Shape::s_Quad.GetIndexVertices());
			shapeComp->Indices = CreateRef<std::vector<uint32_t>>(Rendering::Shape::s_Quad.GetIndices());

			s_RuntimeUIContext->m_BackgroundInputSpec.m_Shader = localShader;
			s_RuntimeUIContext->m_BackgroundInputSpec.m_Buffer = localBuffer;
			s_RuntimeUIContext->m_BackgroundInputSpec.m_ShapeComponent = shapeComp;
		}

		// Verify Initialization
		KG_VERIFY(true, "Runtime UI Engine Init");
	}

	void RuntimeUIService::Terminate()
	{
		// Clear input spec data
		s_RuntimeUIContext->m_BackgroundInputSpec.ClearData();

		// Terminate Static Variables
		s_RuntimeUIContext = nullptr;

		// Verify Termination
		KG_VERIFY(true, "Runtime UI Engine Terminate");
	}

	void RuntimeUIService::SetActiveUI(Ref<UserInterface> userInterface, Assets::AssetHandle uiHandle)
	{
		if (!userInterface || uiHandle == Assets::EmptyHandle)
		{
			KG_WARN("Attempt to make a user interface active that is null or has an empty handle");
			return;
		}

		// Reset previous active UI
		ClearActiveUI();

		// Set new active UI
		s_RuntimeUIContext->m_ActiveUI = userInterface;
		s_RuntimeUIContext->m_ActiveUIHandle = uiHandle;

		// Revalidate UI Context
		RevalidateDisplayedWindows();

		// Load default font if necessary
		if (!userInterface->m_Font)
		{
			userInterface->m_Font = s_RuntimeUIContext->m_DefaultFont;
			userInterface->m_FontHandle = Assets::EmptyHandle;
		}

		// Set the first window as active if applicable
		Ref<UserInterface> activeUI = s_RuntimeUIContext->m_ActiveUI;
		if (activeUI->m_Windows.size() > 0)
		{
			// Display the first window
			activeUI->m_Windows.at(0).DisplayWindow();
			activeUI->m_ActiveWindow = &activeUI->m_Windows.at(0);

			// Set default widget as the selected widget if it exists
			if (activeUI->m_ActiveWindow->m_DefaultActiveWidgetRef)
			{
				// Set the default widget
				activeUI->m_SelectedWidget = activeUI->m_ActiveWindow->m_DefaultActiveWidgetRef.get();

				// Ensure the widget is selectable
				KG_ASSERT(activeUI->m_SelectedWidget->Selectable());

				// Set the button background color as active
				ButtonWidget& selectedButton = *(ButtonWidget*)activeUI->m_SelectedWidget;
				selectedButton.m_ActiveBackgroundColor = activeUI->m_SelectColor;
			}
			else
			{
				activeUI->m_SelectedWidget = nullptr;
			}


			// Calculate Text Sizes for all windows
			for (Window& window : activeUI->m_Windows)
			{
				for (Ref<Widget> widget : window.m_Widgets)
				{
					if (widget->m_WidgetType == WidgetTypes::TextWidget)
					{
						TextWidget* textWidget = (TextWidget*)widget.get();
						textWidget->CalculateTextMetadata(&window);
					}
					else if (widget->m_WidgetType == WidgetTypes::ButtonWidget)
					{
						ButtonWidget* buttonWidget = (ButtonWidget*)widget.get();
						buttonWidget->CalculateTextSize();
					}
				}
			}
		}

		// Create widget navigation links for all windows
		CalculateWindowNavigationLinks();
	}

	void RuntimeUIService::SetActiveUIFromHandle(Assets::AssetHandle uiHandle)
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

	 //TODO: Might use this function in the user interface editor
	//bool RuntimeUIService::RevalidateUIWidgets()
	//{
	//	// Ensure active user interface is valid
	//	if (!(bool)s_RuntimeUIContext->m_ActiveUI || s_RuntimeUIContext->m_ActiveUIHandle == Assets::EmptyHandle)
	//	{
	//		KG_WARN("Attempt to validate user interface with invalid reference or handle.");
	//		return false;
	//	}

	//	// Validate 
	//	for (Window& window : s_RuntimeUIContext->m_ActiveUI->m_Windows)
	//	{
	//		if (window.m_DefaultActiveWidgetRef)
	//		{
	//			std::vector<Ref<Widget>> iterator = std::find(window.m_Widgets.begin(), window.m_Widgets.end(), window.m_DefaultActiveWidgetRef);
	//			window.m_DefaultActiveWidget = static_cast<int32_t>(iterator - window.m_Widgets.begin());
	//		}
	//		else
	//		{
	//			window.m_DefaultActiveWidget = k_InvalidWidgetIndex;
	//		}
	//	}

	//	// Create widget navigation links for all windows
	//	CalculateWindowNavigationLinks();
	//	return true;
	//}

	bool RuntimeUIService::DeleteActiveUIWindow(std::size_t windowLocation)
	{
		// Attempt to delete the window from the active user interface
		bool success = DeleteUIWindow(s_RuntimeUIContext->m_ActiveUI, windowLocation);

		// Ensure deletion was successful
		if (!success)
		{
			KG_WARN("Attempt to delete window from active user interface failed.");
			return false;
		}

		// Ensure correct windows are displayed
		RevalidateDisplayedWindows();
		return true;
	}

	bool RuntimeUIService::DeleteActiveUIWidget(std::size_t windowIndex, std::size_t widgetIndex)
	{
		// Attempt to delete the widget from the active user interface
		bool success = DeleteUIWidget(s_RuntimeUIContext->m_ActiveUI, windowIndex, widgetIndex);

		// Ensure deletion was successful
		if (!success)
		{
			KG_WARN("Attempt to delete widget from active user interface failed.");
			return false;
		}

		// Revalidate navigation links
		CalculateWindowNavigationLinks();
		return true;
	}

	bool RuntimeUIService::DeleteUIWindow(Ref<UserInterface> userInterface, std::size_t windowLocation)
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

	bool RuntimeUIService::DeleteUIWidget(Ref<UserInterface> userInterface, std::size_t windowIndex, std::size_t widgetIndex)
	{
		if (!userInterface)
		{
			KG_WARN("Attempt to delete widget from invalid user interface reference");
			return false;
		}

		// Ensure window index is valid
		if (windowIndex >= s_RuntimeUIContext->m_ActiveUI->m_Windows.size())
		{
			KG_WARN("Attempt to delete widget from window with out of bounds index");
			return false;
		}

		// Get the window reference
		std::vector<RuntimeUI::Window>& uiWindows = userInterface->m_Windows;
		RuntimeUI::Window& indicatedWindow = uiWindows.at(windowIndex);

		// Ensure widget index is valid
		if (widgetIndex >= indicatedWindow.m_Widgets.size())
		{
			KG_WARN("Attempt to delete widget with out of bounds index");
			return false;
		}

		// Delete the widget
		indicatedWindow.m_Widgets.erase(indicatedWindow.m_Widgets.begin() + widgetIndex);
		return true;
	}


	void RuntimeUIService::OnRender(const Math::mat4& cameraViewMatrix, uint32_t viewportWidth, uint32_t viewportHeight)
	{
		// Ensure active user interface is valid
		if (!s_RuntimeUIContext->m_ActiveUI)
		{
			return;
		}

		// Reset the rendering context
		Rendering::RendererAPI::ClearDepthBuffer();

		// Start rendering context
		Rendering::RenderingService::BeginScene(cameraViewMatrix);

		// Submit rendering data from all windows
		uint16_t windowIteration{ 0 };
		std::vector<size_t>& windowIndices = s_RuntimeUIContext->m_ActiveUI->m_DisplayedWindowIndices;
		for (Window* window : s_RuntimeUIContext->m_ActiveUI->m_DisplayedWindows)
		{
			// Get position data for rendering window
			Math::vec3 scale = window->CalculateSize(viewportWidth, viewportHeight);
			Math::vec3 initialTranslation = window->CalculateWorldPosition(viewportWidth, viewportHeight);
			Math::vec3 bottomLeftTranslation = Math::vec3( initialTranslation.x + (scale.x / 2),  initialTranslation.y + (scale.y / 2), initialTranslation.z);

			// Create background rendering data
			s_RuntimeUIContext->m_BackgroundInputSpec.m_TransformMatrix = glm::translate(Math::mat4(1.0f), bottomLeftTranslation)
				* glm::scale(Math::mat4(1.0f), scale);
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(window->m_BackgroundColor, "a_Color", s_RuntimeUIContext->m_BackgroundInputSpec.m_Buffer, s_RuntimeUIContext->m_BackgroundInputSpec.m_Shader);

			// Push window ID and invalid widgetID
			Rendering::Shader::SetDataAtInputLocation<uint32_t>(((uint32_t)windowIndices[windowIteration] << 16) | (uint32_t)0xFFFF, "a_EntityID", s_RuntimeUIContext->m_BackgroundInputSpec.m_Buffer, s_RuntimeUIContext->m_BackgroundInputSpec.m_Shader);

			// Submit background data to GPU
			Rendering::RenderingService::SubmitDataToRenderer(s_RuntimeUIContext->m_BackgroundInputSpec);

			// Call rendering function for every widget
			initialTranslation.z += 0.1f;
			uint16_t widgetIteration{ 0 };
			for (Ref<Widget> widgetRef : window->m_Widgets)
			{
				// Push window ID and invalid widget ID
				Rendering::Shader::SetDataAtInputLocation<uint32_t>(((uint32_t)windowIndices[windowIteration] << 16) | (uint32_t)widgetIteration, "a_EntityID", s_RuntimeUIContext->m_BackgroundInputSpec.m_Buffer, s_RuntimeUIContext->m_BackgroundInputSpec.m_Shader);
				RuntimeUI::FontService::SetID(((uint32_t)windowIndices[windowIteration] << 16) | (uint32_t)widgetIteration);
				// Call the widget's rendering function
				widgetRef->OnRender(initialTranslation, scale, (float)viewportWidth);
				widgetIteration++;
			}
			windowIteration++;
		}

		// End rendering context and submit rendering data to GPU
		Rendering::RenderingService::EndScene();

	}

	void RuntimeUIService::OnRender(uint32_t viewportWidth, uint32_t viewportHeight)
	{
		// Calculate orthographic projection matrix for user interface
		Math::mat4 orthographicProjection = glm::ortho(0.0f, (float)viewportWidth,
			0.0f, (float)viewportHeight, -1.0f, 1.0f);
		OnRender(orthographicProjection, viewportWidth, viewportHeight);
	}

	void RuntimeUIService::AddActiveWindow(Window& window)
	{
		// Store the window in the active user interface
		s_RuntimeUIContext->m_ActiveUI->m_Windows.push_back(window);

		// Display the window
		window.DisplayWindow();
	}

	void RuntimeUIService::SetActiveFont(Ref<Font> newFont, Assets::AssetHandle fontHandle)
	{
		// Set the active font for the active user interface
		s_RuntimeUIContext->m_ActiveUI->m_Font = newFont;
		s_RuntimeUIContext->m_ActiveUI->m_FontHandle = fontHandle;

		// Revalidate/calculate text sizes for all windows
		for (Window& window : s_RuntimeUIContext->m_ActiveUI->m_Windows)
		{
			for (Ref<Widget> widget : window.m_Widgets)
			{
				if (widget->m_WidgetType == WidgetTypes::TextWidget)
				{
					TextWidget* textWidget = (TextWidget*)widget.get();
					textWidget->CalculateTextMetadata(&window);
				}
				else if (widget->m_WidgetType == WidgetTypes::ButtonWidget)
				{
					ButtonWidget* buttonWidget = (ButtonWidget*)widget.get();
					buttonWidget->CalculateTextSize();
				}
			}
		}
	}

	std::vector<Window>& RuntimeUIService::GetAllActiveWindows()
	{
		return s_RuntimeUIContext->m_ActiveUI->m_Windows;
	}

	void RuntimeUIService::RevalidateDisplayedWindows()
	{
		// Ensure/validate that the correct window is being displayed
		s_RuntimeUIContext->m_ActiveUI->m_DisplayedWindows.clear();
		s_RuntimeUIContext->m_ActiveUI->m_DisplayedWindowIndices.clear();
		size_t iteration{ 0 };
		for (Window& window : GetAllActiveWindows())
		{
			// Add the window to the displayed windows if it is flagged to do so
			if (window.GetWindowDisplayed()) 
			{ 
				s_RuntimeUIContext->m_ActiveUI->m_DisplayedWindows.push_back(&window);
				s_RuntimeUIContext->m_ActiveUI->m_DisplayedWindowIndices.push_back(iteration);
			}
			iteration++;
		}
	}

	void RuntimeUIService::SetWidgetTextInternal(Window* currentWindow, Ref<Widget> currentWidget, const std::string& newText)
	{
		KG_ASSERT(currentWindow);

		// Ensure the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget's text");
			return;
		}

		// Ensure the widget is a text widget
		if (currentWidget->m_WidgetType != WidgetTypes::TextWidget &&
			currentWidget->m_WidgetType != WidgetTypes::ButtonWidget)
		{
			KG_WARN("Attempt to change the text of a widget that is not a text widget nor a button widget");
			return;
		}

		if (currentWidget->m_WidgetType == WidgetTypes::TextWidget)
		{
			// Set the text of the widget
			TextWidget* textWidget = (TextWidget*)currentWidget.get();
			textWidget->m_Text = newText;
			textWidget->CalculateTextMetadata(currentWindow);
		}
		else if (currentWidget->m_WidgetType == WidgetTypes::ButtonWidget)
		{
			// Set the text of the widget
			ButtonWidget* buttonWidget = (ButtonWidget*)currentWidget.get();
			buttonWidget->m_Text = newText;
			buttonWidget->CalculateTextSize();
		}
	}

	void RuntimeUIService::SetSelectedWidgetInternal(Ref<Widget> currentWidget)
	{
		// Ensure the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget as selected");
			return;
		}

		// Ensure the widget is selectable
		if (!currentWidget->Selectable())
		{
			KG_WARN("Attempt to set a widget as selected that is not selectable");
			return;
		}

		// Set the previous selected widget's color to the default color
		Ref<UserInterface> activeUI = s_RuntimeUIContext->m_ActiveUI;
		if (activeUI->m_SelectedWidget)
		{
			ButtonWidget& previousSelectedWidget = *(ButtonWidget*)activeUI->m_SelectedWidget;
			previousSelectedWidget.m_ActiveBackgroundColor = previousSelectedWidget.m_DefaultBackgroundColor;
		}

		// Set the new widget as selected and set it's color to the active color
		activeUI->m_SelectedWidget = currentWidget.get();
		ButtonWidget& newSelectedWidget = *(ButtonWidget*)currentWidget.get();
		newSelectedWidget.m_ActiveBackgroundColor = activeUI->m_SelectColor;

		// Call the on move function if applicable
		if (activeUI->m_FunctionPointers.m_OnMove)
		{
			Utility::CallWrappedVoidNone(activeUI->m_FunctionPointers.m_OnMove->m_Function);
		}
	}

	void RuntimeUIService::SetWidgetTextColorInternal(Ref<Widget> currentWidget, const Math::vec4& newColor)
	{
		// Ensure the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget's text color");
			return;
		}

		// Ensure the widget is a text widget
		if (currentWidget->m_WidgetType != WidgetTypes::TextWidget &&
			currentWidget->m_WidgetType != WidgetTypes::ButtonWidget)
		{
			KG_WARN("Attempt to set text color on widget that is not a TextWidget");
			return;
		}

		if (currentWidget->m_WidgetType == WidgetTypes::TextWidget)
		{
			// Set the text color of the widget
			TextWidget* textWidget = (TextWidget*)currentWidget.get();
			textWidget->m_TextColor = newColor;
		}
		else if (currentWidget->m_WidgetType == WidgetTypes::ButtonWidget)
		{
			// Set the text of the widget
			ButtonWidget* buttonWidget = (ButtonWidget*)currentWidget.get();
			// Set the text color of the widget
			buttonWidget->m_TextColor = newColor;
		}
	}

	void RuntimeUIService::SetWidgetBackgroundColorInternal(Ref<Widget> currentWidget, const Math::vec4& newColor)
	{
		// Ensure the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget's background color");
			return;
		}

		// Ensure the widget is valid
		if (currentWidget->m_WidgetType != WidgetTypes::ButtonWidget)
		{
			KG_WARN("Invalid widget type attempting to change its background color");
			return;
		}

		// Set the background color of the button widget
		ButtonWidget& buttonWidget = *(ButtonWidget*)currentWidget.get();
		buttonWidget.m_DefaultBackgroundColor = newColor;
		buttonWidget.m_ActiveBackgroundColor = newColor;
	}

	void RuntimeUIService::SetWidgetSelectableInternal(Ref<Widget> currentWidget, bool selectable)
	{
		// Ensure the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget as selectable");
			return;
		}

		// Ensure the widget is valid
		if (currentWidget->m_WidgetType != WidgetTypes::ButtonWidget)
		{
			KG_WARN("Invalid widget type attempting to set a widget as selectable");
			return;
		}

		// Set the widget as selectable
		ButtonWidget& buttonWidget = *(ButtonWidget*)currentWidget.get();
		buttonWidget.m_Selectable = selectable;
		CalculateWindowNavigationLinks();
	}

	bool RuntimeUIService::IsWidgetSelectedInternal(Ref<Widget> currentWidget)
	{
		// Check if the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when checking the widget is selected");
			return false;
		}

		// Return if the widget is selected
		return s_RuntimeUIContext->m_ActiveUI->m_SelectedWidget == currentWidget.get();
	}

	void RuntimeUIService::ClearActiveUI()
	{
		s_RuntimeUIContext->m_ActiveUI = nullptr;
		s_RuntimeUIContext->m_ActiveUIHandle = Assets::EmptyHandle;
	}

	Ref<UserInterface> RuntimeUIService::GetActiveUI()
	{
		return s_RuntimeUIContext->m_ActiveUI;
	}

	Assets::AssetHandle RuntimeUIService::GetActiveUIHandle()
	{
		return s_RuntimeUIContext->m_ActiveUIHandle;
	}

	void RuntimeUIService::SetSelectedWidgetColor(const Math::vec4& color)
	{
		if (!s_RuntimeUIContext->m_ActiveUI->m_SelectedWidget)
		{
			return;
		}

		// Ensure the widget is valid
		if (s_RuntimeUIContext->m_ActiveUI->m_SelectedWidget->m_WidgetType != WidgetTypes::ButtonWidget)
		{
			KG_WARN("Invalid widget type attempting to change it's active color");
			return;
		}

		// Set the widget's active color
		ButtonWidget& buttonWidget = *(ButtonWidget*)s_RuntimeUIContext->m_ActiveUI->m_SelectedWidget;
		buttonWidget.m_ActiveBackgroundColor = color;
	}

	bool RuntimeUIService::IsWidgetSelectedByTag(const std::string& windowTag, const std::string& widgetTag)
	{
		// Get the current widget
		Ref<Widget> currentWidget = GetWidget(windowTag, widgetTag);

		return IsWidgetSelectedInternal(currentWidget);
	}

	bool RuntimeUIService::IsWidgetSelectedByIndex(WidgetID widgetID)
	{
		// Ensure the correct user interface is active
		if (widgetID.m_UserInterfaceID != s_RuntimeUIContext->m_ActiveUIHandle)
		{
			KG_WARN("Incorrect user interface provided when attempting to modify the active runtime user interface");
			return false;
		}

		// Get the current widget
		Ref<Widget> currentWidget = GetWidget(widgetID.m_WindowIndex, widgetID.m_WidgetIndex);

		return IsWidgetSelectedInternal(currentWidget);
	}

	void RuntimeUIService::SetActiveWidgetText(const std::string& windowTag, const std::string& widgetTag, const std::string& newText)
	{
		// Search for the indicated widget
		auto [currentWidget, currentWindow] = GetWidgetAndWindow(windowTag, widgetTag);
		SetWidgetTextInternal(currentWindow, currentWidget, newText);
		
	}

	void RuntimeUIService::SetActiveWidgetTextByIndex(WidgetID widgetID, const std::string& newText)
	{
		// Ensure the correct user interface is active
		if (widgetID.m_UserInterfaceID != s_RuntimeUIContext->m_ActiveUIHandle)
		{
			KG_WARN("Incorrect user interface provided when attempting to modify the active runtime user interface");
			return;
		}

		// Search for the indicated widget
		auto [currentWidget, currentWindow] = GetWidgetAndWindow(widgetID.m_WindowIndex, widgetID.m_WidgetIndex);
		SetWidgetTextInternal(currentWindow, currentWidget, newText);
	}

	void RuntimeUIService::SetSelectedWidgetByTag(const std::string& windowTag, const std::string& widgetTag)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidget(windowTag, widgetTag);

		SetSelectedWidgetInternal(currentWidget);
	}

	void RuntimeUIService::SetSelectedWidgetByIndex(WidgetID widgetID)
	{
		// Ensure the correct user interface is active
		if (widgetID.m_UserInterfaceID != s_RuntimeUIContext->m_ActiveUIHandle)
		{
			KG_WARN("Incorrect user interface provided when attempting to modify the active runtime user interface");
			return;
		}

		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidget(widgetID.m_WindowIndex, widgetID.m_WidgetIndex);

		SetSelectedWidgetInternal(currentWidget);
	}

	void RuntimeUIService::SetWidgetTextColorByTag(const std::string& windowTag, const std::string& widgetTag, const Math::vec4& color)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidget(windowTag, widgetTag);

		SetWidgetTextColorInternal(currentWidget, color);
	}

	void RuntimeUIService::SetWidgetTextColorByIndex(WidgetID widgetID, const Math::vec4& color)
	{
		// Ensure the correct user interface is active
		if (widgetID.m_UserInterfaceID != s_RuntimeUIContext->m_ActiveUIHandle)
		{
			KG_WARN("Incorrect user interface provided when attempting to modify the active runtime user interface");
			return;
		}

		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidget(widgetID.m_WindowIndex, widgetID.m_WidgetIndex);

		SetWidgetTextColorInternal(currentWidget, color);
	}

	void RuntimeUIService::SetWidgetBackgroundColorByTag(const std::string& windowTag, const std::string& widgetTag, const Math::vec4& color)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidget(windowTag, widgetTag);

		SetWidgetBackgroundColorInternal(currentWidget, color);
	}

	void RuntimeUIService::SetWidgetBackgroundColorByIndex(WidgetID widgetID, const Math::vec4& color)
	{
		// Ensure the correct user interface is active
		if (widgetID.m_UserInterfaceID != s_RuntimeUIContext->m_ActiveUIHandle)
		{
			KG_WARN("Incorrect user interface provided when attempting to modify the active runtime user interface");
			return;
		}

		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidget(widgetID.m_WindowIndex, widgetID.m_WidgetIndex);

		SetWidgetBackgroundColorInternal(currentWidget, color);
	}

	void RuntimeUIService::SetWidgetSelectableByTag(const std::string& windowTag, const std::string& widgetTag, bool selectable)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidget(windowTag, widgetTag);

		SetWidgetSelectableInternal(currentWidget, selectable);
	}

	void RuntimeUIService::SetWidgetSelectableByIndex(WidgetID widgetID, bool selectable)
	{
		// Ensure the correct user interface is active
		if (widgetID.m_UserInterfaceID != s_RuntimeUIContext->m_ActiveUIHandle)
		{
			KG_WARN("Incorrect user interface provided when attempting to modify the active runtime user interface");
			return;
		}

		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidget(widgetID.m_WindowIndex, widgetID.m_WidgetIndex);

		SetWidgetSelectableInternal(currentWidget, selectable);
	}

	void RuntimeUIService::SetActiveOnMove(Assets::AssetHandle functionHandle, Ref<Scripting::Script> function)
	{
		s_RuntimeUIContext->m_ActiveUI->m_FunctionPointers.m_OnMove = function;
		s_RuntimeUIContext->m_ActiveUI->m_FunctionPointers.m_OnMoveHandle = functionHandle;
	}

	Ref<Scripting::Script> RuntimeUIService::GetActiveOnMove()
	{
		return s_RuntimeUIContext->m_ActiveUI->m_FunctionPointers.m_OnMove;
	}

	Assets::AssetHandle RuntimeUIService::GetActiveOnMoveHandle()
	{
		return s_RuntimeUIContext->m_ActiveUI->m_FunctionPointers.m_OnMoveHandle;
	}

	void RuntimeUIService::SetDisplayWindowByTag(const std::string& windowTag, bool display)
	{
		// Search for the indicated window
		for (Window& window : s_RuntimeUIContext->m_ActiveUI->m_Windows)
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

	void RuntimeUIService::SetDisplayWindowByIndex(WindowID windowID, bool display)
	{
		// Ensure the correct user interface is active
		if (windowID.m_UserInterfaceID != s_RuntimeUIContext->m_ActiveUIHandle)
		{
			KG_WARN("Incorrect user interface provided when attempting to modify the active runtime user interface");
			return;
		}

		std::vector<Window>& activeWindows{ s_RuntimeUIContext->m_ActiveUI->m_Windows };

		if (windowID.m_WindowIndex > (activeWindows.size() - 1))
		{
			KG_WARN("Provided window ID is out of bounds for the UI's windows");
			return;
		}

		Window& window = activeWindows.at((size_t)windowID.m_WindowIndex);
		
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

	void RuntimeUIService::MoveRight()
	{
		// Ensure the user interface context is valid and the navigation link is valid
		Ref<UserInterface> activeUI = s_RuntimeUIContext->m_ActiveUI;

		// Ensure the selected widget is valid
		if (!activeUI || !activeUI->m_SelectedWidget || !activeUI->m_ActiveWindow)
		{
			return;
		}


		// Get the selected widget as a button
		KG_ASSERT(activeUI->m_SelectedWidget->m_WidgetType == WidgetTypes::ButtonWidget);
		ButtonWidget& selectedButton = *(ButtonWidget*)activeUI->m_SelectedWidget;

		// Move to the right
		if (selectedButton.m_NavigationLinks.m_RightWidgetIndex != k_InvalidWidgetIndex)
		{
			// Set the active background color of the original widget to indicate it is no longer selected
			selectedButton.m_ActiveBackgroundColor = selectedButton.m_DefaultBackgroundColor;

			// Set the new selected widget
			activeUI->m_SelectedWidget = activeUI->m_ActiveWindow->m_Widgets.at(selectedButton.m_NavigationLinks.m_RightWidgetIndex).get();

			// Get the new selected widget as a button
			KG_ASSERT(activeUI->m_SelectedWidget->m_WidgetType == WidgetTypes::ButtonWidget);
			ButtonWidget& newSelectedButton = *(ButtonWidget*)activeUI->m_SelectedWidget;

			// Set the active background color of the new widget to indicate it is selected
			newSelectedButton.m_ActiveBackgroundColor = activeUI->m_SelectColor;

			// Call the on move function if applicable
			if (activeUI->m_FunctionPointers.m_OnMove)
			{
				Utility::CallWrappedVoidNone(activeUI->m_FunctionPointers.m_OnMove->m_Function);
			}
		}
	}

	void RuntimeUIService::MoveLeft()
	{
		// Ensure the user interface context is valid and the navigation link is valid
		Ref<UserInterface> activeUI = s_RuntimeUIContext->m_ActiveUI;

		// Ensure the selected widget is valid
		if (!activeUI || !activeUI->m_SelectedWidget || !activeUI->m_ActiveWindow)
		{
			return;
		}
		// Get the selected widget as a button
		KG_ASSERT(activeUI->m_SelectedWidget->m_WidgetType == WidgetTypes::ButtonWidget);
		ButtonWidget& selectedButton = *(ButtonWidget*)activeUI->m_SelectedWidget;

		// Move to the left
		if (selectedButton.m_NavigationLinks.m_LeftWidgetIndex != k_InvalidWidgetIndex)
		{
			// Set the active background color of the original widget to indicate it is no longer selected
			selectedButton.m_ActiveBackgroundColor = selectedButton.m_DefaultBackgroundColor;

			// Set the new selected widget
			activeUI->m_SelectedWidget = activeUI->m_ActiveWindow->m_Widgets.at(selectedButton.m_NavigationLinks.m_LeftWidgetIndex).get();

			// Get the new selected widget as a button
			KG_ASSERT(activeUI->m_SelectedWidget->m_WidgetType == WidgetTypes::ButtonWidget);
			ButtonWidget& newSelectedButton = *(ButtonWidget*)activeUI->m_SelectedWidget;

			// Set the active background color of the new widget to indicate it is selected
			newSelectedButton.m_ActiveBackgroundColor = activeUI->m_SelectColor;

			// Call the on move function if applicable
			if (activeUI->m_FunctionPointers.m_OnMove)
			{
				Utility::CallWrappedVoidNone(activeUI->m_FunctionPointers.m_OnMove->m_Function);
			}
		}
	}

	void RuntimeUIService::MoveUp()
	{
		// Ensure the user interface context is valid and the navigation link is valid
		Ref<UserInterface> activeUI = s_RuntimeUIContext->m_ActiveUI;

		// Ensure the selected widget is valid
		if (!activeUI || !activeUI->m_SelectedWidget || !activeUI->m_ActiveWindow)
		{
			return;
		}
		// Get the current selected widget as a button
		KG_ASSERT(activeUI->m_SelectedWidget->m_WidgetType == WidgetTypes::ButtonWidget);
		ButtonWidget& selectedButton = *(ButtonWidget*)activeUI->m_SelectedWidget;

		// Move up
		if (selectedButton.m_NavigationLinks.m_UpWidgetIndex != k_InvalidWidgetIndex)
		{
			// Set the active background color of the original widget to indicate it is no longer selected
			selectedButton.m_ActiveBackgroundColor = selectedButton.m_DefaultBackgroundColor;

			// Set the new selected widget
			activeUI->m_SelectedWidget = activeUI->m_ActiveWindow->m_Widgets.at(selectedButton.m_NavigationLinks.m_UpWidgetIndex).get();

			// Get the new selected widget as a button
			KG_ASSERT(activeUI->m_SelectedWidget->m_WidgetType == WidgetTypes::ButtonWidget);
			ButtonWidget& newSelectedButton = *(ButtonWidget*)activeUI->m_SelectedWidget;

			// Set the active background color of the new widget to indicate it is selected
			newSelectedButton.m_ActiveBackgroundColor = activeUI->m_SelectColor;

			// Call the on move function if applicable
			if (activeUI->m_FunctionPointers.m_OnMove)
			{
				Utility::CallWrappedVoidNone(activeUI->m_FunctionPointers.m_OnMove->m_Function);
			}
		}
	}

	void RuntimeUIService::MoveDown()
	{
		// Ensure the user interface context is valid and the navigation link is valid
		Ref<UserInterface> activeUI = s_RuntimeUIContext->m_ActiveUI;

		// Ensure the selected widget is valid
		if (!activeUI || !activeUI->m_SelectedWidget || !activeUI->m_ActiveWindow)
		{
			return;
		}
		// Get the selected widget as a button
		KG_ASSERT(activeUI->m_SelectedWidget->m_WidgetType == WidgetTypes::ButtonWidget);
		ButtonWidget& selectedButton = *(ButtonWidget*)activeUI->m_SelectedWidget;

		// Move down
		if (selectedButton.m_NavigationLinks.m_DownWidgetIndex != k_InvalidWidgetIndex)
		{
			// Set the active background color of the original widget to indicate it is no longer selected
			selectedButton.m_ActiveBackgroundColor = selectedButton.m_DefaultBackgroundColor;

			// Set the new selected widget
			activeUI->m_SelectedWidget = activeUI->m_ActiveWindow->m_Widgets.at(selectedButton.m_NavigationLinks.m_DownWidgetIndex).get();

			// Get the new selected widget as a button
			KG_ASSERT(activeUI->m_SelectedWidget->m_WidgetType == WidgetTypes::ButtonWidget);
			ButtonWidget& newSelectedButton = *(ButtonWidget*)activeUI->m_SelectedWidget;

			// Set the active background color of the new widget to indicate it is selected
			newSelectedButton.m_ActiveBackgroundColor = activeUI->m_SelectColor;

			// Call the on move function if applicable
			if (activeUI->m_FunctionPointers.m_OnMove)
			{
				Utility::CallWrappedVoidNone(activeUI->m_FunctionPointers.m_OnMove->m_Function);
			}
		}
	}

	void RuntimeUIService::OnPress()
	{
		// Ensure selected widget is valid
		if (!s_RuntimeUIContext->m_ActiveUI->m_SelectedWidget) 
		{ 
			return; 
		}

		// Call the on press function if applicable
		KG_ASSERT(s_RuntimeUIContext->m_ActiveUI->m_SelectedWidget->m_WidgetType == WidgetTypes::ButtonWidget);
		ButtonWidget& selectedButton = *(ButtonWidget*)s_RuntimeUIContext->m_ActiveUI->m_SelectedWidget;
		if (selectedButton.m_FunctionPointers.m_OnPress)
		{
			Utility::CallWrappedVoidNone(selectedButton.m_FunctionPointers.m_OnPress->m_Function);
		}
	}

	void RuntimeUIService::CalculateWindowNavigationLinks()
	{
		ViewportData& viewportData = EngineService::GetActiveWindow().GetActiveViewport();

		// Iterate through all windows 
		for (Window& currentWindow : s_RuntimeUIContext->m_ActiveUI->m_Windows)
		{
			// Calculate window scale and position
			Math::vec3 windowScale = currentWindow.CalculateSize(viewportData.m_Width, viewportData.m_Height);
			Math::vec3 windowPosition = currentWindow.CalculateWorldPosition(viewportData.m_Width, viewportData.m_Height);

			// Iterate through all widgets in the window
			for (Ref<Widget> currentWidget : currentWindow.m_Widgets)
			{
				if (!currentWidget->Selectable())
				{
					continue;
				}

				// Get the widget as a button widget
				KG_ASSERT(currentWidget->m_WidgetType == WidgetTypes::ButtonWidget);
				ButtonWidget& selectedButton = *(ButtonWidget*)currentWidget.get();

				// Calculate navigation links for the current widget
				selectedButton.m_NavigationLinks.m_RightWidgetIndex = CalculateNavigationLink(currentWindow, currentWidget, 
					Direction::Right, windowPosition, windowScale);
				selectedButton.m_NavigationLinks.m_LeftWidgetIndex = CalculateNavigationLink(currentWindow, currentWidget, 
					Direction::Left, windowPosition, windowScale);
				selectedButton.m_NavigationLinks.m_UpWidgetIndex = CalculateNavigationLink(currentWindow, currentWidget,
					Direction::Up, windowPosition, windowScale);
				selectedButton.m_NavigationLinks.m_DownWidgetIndex = CalculateNavigationLink(currentWindow, currentWidget,
					Direction::Down, windowPosition, windowScale);
			}
		}
	}

	std::size_t RuntimeUIService::CalculateNavigationLink(Window& currentWindow, Ref<Widget> currentWidget, Direction direction, const Math::vec3& windowPosition, const Math::vec3& windowSize)
	{
		// Initialize variables for navigation link calculation
		Ref<Widget> currentBestChoice{ nullptr };
		std::size_t currentChoiceLocation{ 0 };
		float currentBestDistance{ std::numeric_limits<float>::max() };
		std::size_t iteration{ 0 };

		// Calculate the position and size of the current widget
		Math::vec2 currentWidgetPosition = currentWidget->CalculateWorldPosition(windowPosition, windowSize);
		Math::vec2 currentWidgetSize = currentWidget->CalculateWidgetSize(windowSize);

		// Iterate through each potential widget and decide which widget makes sense to navigate to
		for (Ref<Widget> potentialChoice : currentWindow.m_Widgets)
		{
			// Skip the current widget and any non-selectable widgets
			if (potentialChoice == currentWidget || !potentialChoice->Selectable())
			{
				iteration++;
				continue;
			}
			// Calculate the position and size of the potential widget
			Math::vec2 potentialChoicePosition = potentialChoice->CalculateWorldPosition(windowPosition, windowSize);
			Math::vec2 potentialChoiceSize = potentialChoice->CalculateWidgetSize(windowSize);

			// Calculate the distance between the current widget and the potential widget
			float currentDistance = glm::abs(glm::distance(currentWidgetPosition, potentialChoicePosition));

			// Check if the potential widget is within the constraints of the current widget
			float currentWidgetExtent;
			float potentialWidgetExtent;

			switch (direction)
			{
			case Direction::Right:
				currentWidgetExtent = currentWidgetPosition.x + (currentWidgetSize.x / 2);
				potentialWidgetExtent = potentialChoicePosition.x - (potentialChoiceSize.x / 2);
				if (currentWidgetExtent >= potentialWidgetExtent)
				{
					iteration++;
					continue;
				}
				break;
			case Direction::Left:
				currentWidgetExtent = currentWidgetPosition.x - (currentWidgetSize.x / 2);
				potentialWidgetExtent = potentialChoicePosition.x + (potentialChoiceSize.x / 2);
				if (currentWidgetExtent <= potentialWidgetExtent)
				{
					iteration++;
					continue;
				}
				break;
			case Direction::Up:
				currentWidgetExtent = currentWidgetPosition.y + (currentWidgetSize.y / 2);
				potentialWidgetExtent = potentialChoicePosition.y - (potentialChoiceSize.y / 2);
				if (currentWidgetExtent >= potentialWidgetExtent)
				{
					iteration++;
					continue;
				}
				break;
			case Direction::Down:
				currentWidgetExtent = currentWidgetPosition.y - (currentWidgetSize.y / 2);
				potentialWidgetExtent = potentialChoicePosition.y + (potentialChoiceSize.y / 2);
				if (currentWidgetExtent <= potentialWidgetExtent)
				{
					iteration++;
					continue;
				}
				break;
			default:
				KG_ERROR("Invalid direction provided when calculating navigation links for active user interface");
				break;
			}
			// Save current best choice if it is the first choice
			if (currentBestChoice == nullptr)
			{
				currentBestChoice = potentialChoice;
				currentChoiceLocation = iteration;
				currentBestDistance = currentDistance;
				iteration++;
				continue;
			}

			// Replace current best choice with the potential choice if it is closer
			if (currentDistance < currentBestDistance)
			{
				currentBestChoice = potentialChoice;
				currentChoiceLocation = iteration;
				currentBestDistance = currentDistance;
				iteration++;
				continue;
			}

			iteration++;
		}

		// Return the index of the best widget choice if it exists
		if (currentBestChoice)
		{
			return currentChoiceLocation;
		}
		else
		{
			return k_InvalidWidgetIndex;
		}
	}

	Ref<Widget> RuntimeUIService::GetWidget(const std::string& windowTag, const std::string& widgetTag)
	{
		if (!s_RuntimeUIContext->m_ActiveUI)
		{
			KG_WARN("Attempt to get a widget from the active user interface, however, no UI is active!");
			return nullptr;
		}

		// Get widget using its parent window tag and its widget tag
		for (Window& window : s_RuntimeUIContext->m_ActiveUI->m_Windows)
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

	Ref<Widget> RuntimeUIService::GetWidget(uint16_t windowIndex, uint16_t widgetIndex)
	{
		// Ensure a user interface is active
		if (!s_RuntimeUIContext->m_ActiveUI)
		{
			KG_WARN("Attempt to get a widget from the active user interface, however, no UI is active!");
			return nullptr;
		}

		// Ensure window index is within bounds
		if (windowIndex > (s_RuntimeUIContext->m_ActiveUI->m_Windows.size() - 1))
		{
			KG_WARN("Attempt to retrieve a widget but the window index was out of bounds");
			return nullptr;
		}
		// Get the indiciated window
		Window& currentWindow = s_RuntimeUIContext->m_ActiveUI->m_Windows.at((size_t)windowIndex);

		// Ensure widget index is within bounds
		if (widgetIndex > (currentWindow.m_Widgets.size() - 1))
		{
			KG_WARN("Attempt to retrieve a widget but the widget index was out of bounds");
			return nullptr;
		}

		// Return the indicated widget and window
		return currentWindow.m_Widgets.at(widgetIndex);
	}

	std::tuple<Ref<Widget>, Window*> RuntimeUIService::GetWidgetAndWindow(const std::string& windowTag, const std::string& widgetTag)
	{
		// Get widget using its parent window tag and its widget tag
		for (Window& window : s_RuntimeUIContext->m_ActiveUI->m_Windows)
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

	std::tuple<Ref<Widget>, Window*> RuntimeUIService::GetWidgetAndWindow(uint16_t windowIndex, uint16_t widgetIndex)
	{
		// Ensure window index is within bounds
		if (windowIndex > (s_RuntimeUIContext->m_ActiveUI->m_Windows.size() - 1))
		{
			KG_WARN("Attempt to retrieve a window/widget but the window index was out of bounds");
			return { nullptr, nullptr };
		}
		// Get the indiciated window
		Window& currentWindow = s_RuntimeUIContext->m_ActiveUI->m_Windows.at((size_t)windowIndex);

		// Ensure widget index is within bounds
		if (widgetIndex > (currentWindow.m_Widgets.size() - 1))
		{
			KG_WARN("Attempt to retrieve a window/widget but the widget index was out of bounds");
			return { nullptr, nullptr };
		}

		// Return the indicated widget and window
		return { currentWindow.m_Widgets.at(widgetIndex), &currentWindow };
	}

	void Window::DisplayWindow()
	{
		// Return if the window is already displayed
		if (m_WindowDisplayed) 
		{ 
			return; 
		}

		// Set window as displayed and revalidate displayed windows for current user interface
		m_WindowDisplayed = true;
		RuntimeUIService::RevalidateDisplayedWindows();

	}

	void Window::HideWindow()
	{
		// Return if the window is not displayed
		if (!m_WindowDisplayed) 
		{ 
			return; 
		}

		// Set window as hidden and revalidate displayed windows for current user interface
		m_WindowDisplayed = false;
		RuntimeUIService::RevalidateDisplayedWindows();
	}

	bool Window::GetWindowDisplayed()
	{
		return m_WindowDisplayed;
	}

	Math::vec3 Window::CalculateSize(uint32_t viewportWidth, uint32_t viewportHeight)
	{
		return Math::vec3(viewportWidth * m_Size.x, viewportHeight * m_Size.y, 1.0f);
	}

	Math::vec3 Window::CalculateWorldPosition(uint32_t viewportWidth, uint32_t viewportHeight)
	{
		return Math::vec3((viewportWidth * m_ScreenPosition.x), (viewportHeight * m_ScreenPosition.y), m_ScreenPosition.z);
	}

	Math::vec3 Window::CalculateScreenPosition(Math::vec2 worldPosition, uint32_t viewportWidth, uint32_t viewportHeight)
	{
		return Math::vec3(worldPosition.x / viewportWidth, worldPosition.y / viewportHeight, m_ScreenPosition.z);
	}

	void Window::AddWidget(Ref<Widget> newWidget)
	{
		KG_ASSERT(newWidget);

		// Add new widget to buffer
		m_Widgets.push_back(newWidget);

		// Revalidate text dimensions for widget
		switch (newWidget->m_WidgetType)
		{
		case WidgetTypes::TextWidget:
			(*(TextWidget*)newWidget.get()).CalculateTextMetadata(this);
			break;
		case WidgetTypes::ButtonWidget:
			(*(ButtonWidget*)newWidget.get()).CalculateTextSize();
			break;
		default:
			KG_ERROR("Invalid widget type provided when revalidating widget text size");
			break;
		}
	}

	void Window::DeleteWidget(std::size_t widgetLocation)
	{
		// Return if the widget location is out of bounds
		if (widgetLocation >= m_Widgets.size())
		{
			KG_WARN("Attempt to delete a widget, however, the provided index is out of bounds");
			return;
		}

		// Delete the widget
		m_Widgets.erase(m_Widgets.begin() + widgetLocation);
	}

	void TextWidget::OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth)
	{
		Rendering::RendererInputSpec& inputSpec = RuntimeUIService::s_RuntimeUIContext->m_BackgroundInputSpec;

		// Calculate the widget's rendering data
		Math::vec3 widgetSize = CalculateWidgetSize(windowSize);

		// Get widget translation
		Math::vec3 widgetTranslation = CalculateWorldPosition(windowTranslation, windowSize);

		// Create the widget's text rendering data
		widgetTranslation.z += 0.001f;
		Math::vec2 resolution = Utility::ScreenResolutionToAspectRatio(Projects::ProjectService::GetActiveTargetResolution());
		float textSize{ (viewportWidth * 0.15f * m_TextSize) * (resolution.y / resolution.x) };
		float lineAdvance = RuntimeUIService::s_RuntimeUIContext->m_ActiveUI->m_Font->m_LineHeight;
		float allLineAdvanceHeight{ 0.0f };
		// Calculate entire text's height
		if (m_TextMetadata.m_LineSize.size() > 0)
		{
			//// Add the first line's dimensions
			//allLineAdvanceHeight += m_TextMetadata.m_LineSize[0].y;

			// Add the line advance for the remaining lines
			allLineAdvanceHeight += lineAdvance * (float)(m_TextMetadata.m_LineSize.size() - 1);
		}

		// Call the text's rendering function
		for (size_t iteration{ 0 }; iteration < m_TextMetadata.m_LineSize.size(); iteration++)
		{

			Math::vec3 finalTranslation;
			Math::vec2 lineDimensions{ m_TextMetadata.m_LineSize[iteration] };
			Math::ivec2 currentBreaks{ m_TextMetadata.m_LineBreaks[iteration] };

			
			constexpr float k_CenterAdjustmentSize{ 2.6f }; // Magic number for adjusting the height of a line TODO: Find better solution

			// Place the starting x-location of the text widget based on the provided alignment option
			switch (m_TextAlignment)
			{
			case Constraint::Left:
				finalTranslation.x = widgetTranslation.x;
				break;
			case Constraint::Right:
				finalTranslation.x = widgetTranslation.x + (widgetSize.x) - ((lineDimensions.x) * textSize);
				break;
			case Constraint::Center:
				// Adjust current line translation to be centered
				finalTranslation.x = widgetTranslation.x + (widgetSize.x * 0.5f) - ((lineDimensions.x * 0.5f) * textSize);
				break;
			case Constraint::Bottom:
			case Constraint::Top:
			case Constraint::None:
				KG_ERROR("Invalid constraint type for aligning text {}", Utility::ConstraintToString(m_TextAlignment));
				break;
			}

			// Set the starting y/z locations
			finalTranslation.y = widgetTranslation.y + (widgetSize.y * 0.5f) - ((m_TextMetadata.m_LineSize[0].y * 0.5f - (allLineAdvanceHeight * 0.5f)) * textSize) + k_CenterAdjustmentSize;
			finalTranslation.z = widgetTranslation.z;

			// Move the line down in the y-axis to it's correct location
			finalTranslation.y -= iteration * textSize * lineAdvance;

			// Render the single line of text
			std::string_view outputText{ m_Text.data() + currentBreaks.x, (size_t)(currentBreaks.y - currentBreaks.x) };
			RuntimeUIService::s_RuntimeUIContext->m_ActiveUI->m_Font->OnRenderSingleLineText(
				outputText,
				finalTranslation, m_TextColor, textSize);
		}

	}

	void TextWidget::CalculateTextMetadata(Window* parentWindow)
	{
		KG_ASSERT(parentWindow);

		// Get the resolution of the screen and the viewport
		Math::vec2 resolution = Utility::ScreenResolutionToAspectRatio(Projects::ProjectService::GetActiveTargetResolution());
		ViewportData viewportData = EngineService::GetActiveWindow().GetActiveViewport();
		float textSize{ (viewportData.m_Width * 0.15f * m_TextSize) * (resolution.y / resolution.x) };

		// Get widget width
		Math::vec3 widgetSize = CalculateWidgetSize(parentWindow->CalculateSize(viewportData.m_Width, viewportData.m_Height));

		// Calculate the text size of the widget using the default font if the active user interface is not set
		if (!RuntimeUIService::s_RuntimeUIContext->m_ActiveUI)
		{
			if (m_TextWrapped)
			{
				RuntimeUIService::s_RuntimeUIContext->m_DefaultFont->CalculateTextMetadata(m_Text, m_TextMetadata, textSize, (int)widgetSize.x);
			}
			else
			{
				RuntimeUIService::s_RuntimeUIContext->m_DefaultFont->CalculateTextMetadata(m_Text, m_TextMetadata, textSize);
			}
			
			return;
		}

		// Calculate the text size of the widget using the active user interface font
		if (m_TextWrapped)
		{
			RuntimeUIService::s_RuntimeUIContext->m_ActiveUI->m_Font->CalculateTextMetadata(m_Text, m_TextMetadata, textSize, (int)widgetSize.x);
		}
		else
		{
			RuntimeUIService::s_RuntimeUIContext->m_ActiveUI->m_Font->CalculateTextMetadata(m_Text, m_TextMetadata, textSize);
		}
		
	}

	void TextWidget::SetText(const std::string& newText, Window* parentWindow)
	{
		// Set the text of the widget
		m_Text = newText;

		// Calculate the new text size
		CalculateTextMetadata(parentWindow);
	}

	void ButtonWidget::SetText(const std::string& newText)
	{
		// Set the text of the widget
		m_Text = newText;

		// Calculate the new text size
		CalculateTextSize();
	}

	void ButtonWidget::CalculateTextSize()
	{
		// Calculate the text size of the widget using the default font if the active user interface is not set
		if (!RuntimeUIService::s_RuntimeUIContext->m_ActiveUI)
		{
			m_TextDimensions = RuntimeUIService::s_RuntimeUIContext->m_DefaultFont->CalculateTextSize(m_Text);
			return;
		}

		// Calculate the text size of the widget using the active user interface font
		m_TextDimensions = RuntimeUIService::s_RuntimeUIContext->m_ActiveUI->m_Font->CalculateTextSize(m_Text);
	}

	Math::vec3 Widget::CalculateWidgetSize(const Math::vec3& windowSize)
	{
		return Math::vec3(windowSize.x * m_Size.x, windowSize.y * m_Size.y, 1.0f);
	}

	Math::vec3 Widget::CalculateWorldPosition(const Math::vec3& windowTranslation, const Math::vec3& windowSize)
	{
		float widgetXPos{ m_XPositionType == PixelOrPercent::Percent ? windowSize.x * m_PercentPosition.x : (float)m_PixelPosition.x };
		float widgetYPos{ m_YPositionType == PixelOrPercent::Percent ? windowSize.y * m_PercentPosition.y : (float)m_PixelPosition.y };
		if (m_XRelativeOrAbsolute == RelativeOrAbsolute::Relative && m_XConstraint != Constraint::None)
		{
			// Handle relative code
			switch (m_XConstraint)
			{
			case Constraint::Center:
				widgetXPos = (windowSize.x * 0.5f) + widgetXPos - (m_Size.x * windowSize.x / 2.0f);
				break;
			case Constraint::Right:
				widgetXPos = windowSize.x + widgetXPos - (m_Size.x * windowSize.x);
				break;
			case Constraint::Left:
				break;
			default:
				KG_ERROR("Invalid constraint {} provided while calculating widget's position", (uint16_t)m_XConstraint);
				break;
			}
		}

		if (m_YRelativeOrAbsolute == RelativeOrAbsolute::Relative && m_YConstraint != Constraint::None)
		{
			// Handle relative code
			switch (m_YConstraint)
			{
			case Constraint::Center:
				widgetYPos = (windowSize.y * 0.5f) + widgetYPos - (m_Size.y * windowSize.y / 2.0f);;
				break;
			case Constraint::Top:
				widgetYPos = windowSize.y + widgetYPos - (m_Size.y * windowSize.y);
				break;
			case Constraint::Bottom:
				break;
			default:
				KG_ERROR("Invalid constraint {} provided while calculating widget's position", (uint16_t)m_YConstraint);
				break;
			}
		}


		// Calculate final widget position on screen
		return Math::vec3(windowTranslation.x + widgetXPos, windowTranslation.y + widgetYPos, windowTranslation.z);
	}

	Math::vec3 Widget::CalculateWindowPosition(Math::vec2 worldPosition, const Math::vec3& windowTranslation, const Math::vec3& windowSize)
	{
		float widgetXPos{ worldPosition.x - windowTranslation.x };
		float widgetYPos{ worldPosition.y - windowTranslation.y };

		if (m_XRelativeOrAbsolute == RelativeOrAbsolute::Relative && m_XConstraint != Constraint::None)
		{
			// Handle relative code
			switch (m_XConstraint)
			{
			case Constraint::Center:
				widgetXPos = widgetXPos - (windowSize.x * 0.5f) + (m_Size.x * windowSize.x / 2.0f);
				break;
			case Constraint::Right:
				widgetXPos = widgetXPos - windowSize.x + (m_Size.x * windowSize.x);
				break;
			case Constraint::Left:
				break;
			default:
				KG_ERROR("Invalid constraint {} provided while calculating widget's position", (uint16_t)m_XConstraint);
				break;
			}
		}

		if (m_YRelativeOrAbsolute == RelativeOrAbsolute::Relative && m_YConstraint != Constraint::None)
		{
			// Handle relative code
			switch (m_YConstraint)
			{
			case Constraint::Center:
				widgetYPos = widgetYPos - (windowSize.y * 0.5f) + (m_Size.y * windowSize.y / 2.0f);
				break;
			case Constraint::Top:
				widgetYPos = widgetYPos - windowSize.y + (m_Size.y * windowSize.y);
				break;
			case Constraint::Bottom:
				break;
			default:
				KG_ERROR("Invalid constraint {} provided while calculating widget's position", (uint16_t)m_YConstraint);
				break;
			}
		}
		// Calculate final widget position on screen
		widgetXPos = m_XPositionType == PixelOrPercent::Percent ? widgetXPos / windowSize.x : widgetXPos;
		widgetYPos = m_YPositionType == PixelOrPercent::Percent ? widgetYPos / windowSize.y : widgetYPos;
		return Math::vec3(widgetXPos, widgetYPos, 0.0f);
	}

	void ButtonWidget::OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth)
	{
		Rendering::RendererInputSpec& inputSpec = RuntimeUIService::s_RuntimeUIContext->m_BackgroundInputSpec;

		// Calculate the widget's rendering data
		Math::vec3 widgetSize = CalculateWidgetSize(windowSize);

		// Get widget translation
		Math::vec3 widgetTranslation = CalculateWorldPosition(windowTranslation, windowSize);

		// Create the widget's background rendering data
		inputSpec.m_TransformMatrix = glm::translate(Math::mat4(1.0f), Math::vec3(widgetTranslation.x + (widgetSize.x / 2), widgetTranslation.y + (widgetSize.y / 2), widgetTranslation.z))
			* glm::scale(Math::mat4(1.0f), widgetSize);
		Rendering::Shader::SetDataAtInputLocation<Math::vec4>(m_ActiveBackgroundColor, "a_Color", inputSpec.m_Buffer, inputSpec.m_Shader);

		// Submit background data to GPU
		Rendering::RenderingService::SubmitDataToRenderer(RuntimeUIService::s_RuntimeUIContext->m_BackgroundInputSpec);

		// Create the widget's text rendering data
		widgetTranslation.z += 0.001f;
		Math::vec2 resolution = Utility::ScreenResolutionToAspectRatio(Projects::ProjectService::GetActiveTargetResolution());
		float textSize{ (viewportWidth * 0.15f * m_TextSize) * (resolution.y / resolution.x) };

		constexpr float k_CenterAdjustmentSize{ 2.6f }; // Magic number for adjusting the height of a line TODO: Find better solution

		// Place the starting x-location of the text widget based on the provided alignment option
		switch (m_TextAlignment)
		{
		case Constraint::Left:
			break;
		case Constraint::Right:
			widgetTranslation.x = widgetTranslation.x + (widgetSize.x) - ((m_TextDimensions.x) * textSize);
			break;
		case Constraint::Center:
			// Adjust current line translation to be centered
			widgetTranslation.x = widgetTranslation.x + (widgetSize.x * 0.5f) - ((m_TextDimensions.x * 0.5f) * textSize);
			break;
		case Constraint::Bottom:
		case Constraint::Top:
		case Constraint::None:
			KG_ERROR("Invalid constraint type for aligning text {}", Utility::ConstraintToString(m_TextAlignment));
			break;
		}

		// Set the starting y/z locations
		widgetTranslation.y = widgetTranslation.y + (widgetSize.y * 0.5f) - ((m_TextDimensions.y * 0.5f) * textSize) + k_CenterAdjustmentSize;

		// Call the text's rendering function
		RuntimeUIService::s_RuntimeUIContext->m_ActiveUI->m_Font->OnRenderSingleLineText(m_Text, widgetTranslation, m_TextColor, textSize);
	}

}

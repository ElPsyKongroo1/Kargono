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
		// Terminate Static Variables
		s_RuntimeUIContext = nullptr;

		// Terminate Window/Widget Rendering Data
		{
			delete s_RuntimeUIContext->m_BackgroundInputSpec.m_ShapeComponent;
		}

		// Verify Termination
		KG_VERIFY(true, "Runtime UI Engine Terminate");
	}

	void RuntimeUIService::SetActiveUI(Ref<UserInterface> userInterface, Assets::AssetHandle uiHandle)
	{
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
				activeUI->m_SelectedWidget = activeUI->m_ActiveWindow->m_DefaultActiveWidgetRef.get();
				activeUI->m_SelectedWidget->m_ActiveBackgroundColor = activeUI->m_SelectColor;
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
						textWidget->CalculateTextSize();
					}
				}
			}
		}

		// Create widget navigation links for all windows
		CalculateWindowNavigationLinks();
	}

	void RuntimeUIService::SetActiveUIFromName(const std::string& uiName)
	{
		// Get user interface from asset service system
		auto [uiHandle, uiReference] = Assets::AssetService::GetUserInterface(uiName);

		// Validate returned user interface
		if (!uiReference)
		{
			KG_WARN("Could not locate user interface. Provided name did not lead to a valid user interface.");
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


	void RuntimeUIService::PushRenderData(const Math::mat4& cameraViewMatrix, uint32_t viewportWidth, uint32_t viewportHeight)
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
			Math::vec3 initialTranslation = window->CalculatePosition(viewportWidth, viewportHeight);
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
				widgetRef->PushRenderData(initialTranslation, scale, (float)viewportWidth);
				widgetIteration++;
			}
			windowIteration++;
		}

		// End rendering context and submit rendering data to GPU
		Rendering::RenderingService::EndScene();

	}

	void RuntimeUIService::PushRenderData(uint32_t viewportWidth, uint32_t viewportHeight)
	{
		// Calculate orthographic projection matrix for user interface
		Math::mat4 orthographicProjection = glm::ortho(0.0f, (float)viewportWidth,
			0.0f, (float)viewportHeight, -1.0f, 1.0f);
		PushRenderData(orthographicProjection, viewportWidth, viewportHeight);
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
					textWidget->CalculateTextSize();
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
		s_RuntimeUIContext->m_ActiveUI->m_SelectedWidget->m_ActiveBackgroundColor = color;
	}

	bool RuntimeUIService::IsWidgetSelected(const std::string& windowTag, const std::string& widgetTag)
	{
		// Get the current widget
		Ref<Widget> currentWidget = GetWidget(windowTag, widgetTag);

		// Check if the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when checking the widget is selected");
			return false;
		}

		// Return if the widget is selected
		return s_RuntimeUIContext->m_ActiveUI->m_SelectedWidget == currentWidget.get();
	}

	void RuntimeUIService::SetActiveWidgetText(const std::string& windowTag, const std::string& widgetTag, const std::string& newText)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidget(windowTag, widgetTag);

		// Ensure the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget's text");
			return;
		}

		// Ensure the widget is a text widget
		if (currentWidget->m_WidgetType != WidgetTypes::TextWidget)
		{
			KG_WARN("Attempt to change the text of a widget that is not a text widget");
			return;
		}

		// Set the text of the widget
		TextWidget* textWidget = (TextWidget*)currentWidget.get();
		textWidget->m_Text = newText;
	}

	void RuntimeUIService::SetSelectedWidget(const std::string& windowTag, const std::string& widgetTag)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidget(windowTag, widgetTag);

		// Ensure the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget as selected");
			return;
		}

		// Ensure the widget is selectable
		if (!currentWidget->m_Selectable)
		{
			KG_WARN("Attempt to set a widget as selected that is not selectable");
			return;
		}

		// Set the widget as selected
		Ref<UserInterface> activeUI = s_RuntimeUIContext->m_ActiveUI;
		if (activeUI->m_SelectedWidget)
		{
			activeUI->m_SelectedWidget->m_ActiveBackgroundColor = activeUI->m_SelectedWidget->m_DefaultBackgroundColor;
		}
		activeUI->m_SelectedWidget = currentWidget.get();
		activeUI->m_SelectedWidget->m_ActiveBackgroundColor = activeUI->m_SelectColor;

		// Call the on move function if applicable
		if (activeUI->m_FunctionPointers.m_OnMove)
		{
			Utility::CallWrappedVoidNone(activeUI->m_FunctionPointers.m_OnMove->m_Function);
		}
	}

	void RuntimeUIService::SetWidgetTextColor(const std::string& windowTag, const std::string& widgetTag, const Math::vec4& color)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidget(windowTag, widgetTag);

		// Ensure the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget's text color");
			return;
		}

		// Ensure the widget is a text widget
		if (currentWidget->m_WidgetType != WidgetTypes::TextWidget)
		{
			KG_WARN("Attempt to set text color on widget that is not a TextWidget");
			return;
		}

		// Set the text color of the widget
		TextWidget* textWidget = (TextWidget*)currentWidget.get();
		textWidget->m_TextColor = color;
	}

	void RuntimeUIService::SetWidgetBackgroundColor(const std::string& windowTag, const std::string& widgetTag, const Math::vec4& color)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidget(windowTag, widgetTag);

		// Ensure the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget's background color");
			return;
		}

		// Set the background color of the widget
		currentWidget->m_DefaultBackgroundColor = color;
		currentWidget->m_ActiveBackgroundColor = color;
	}

	void RuntimeUIService::SetWidgetSelectable(const std::string& windowTag, const std::string& widgetTag, bool selectable)
	{
		// Search for the indicated widget
		Ref<Widget> currentWidget = GetWidget(windowTag, widgetTag);

		// Ensure the widget is valid
		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget as selectable");
			return;
		}

		// Set the widget as selectable
		currentWidget->m_Selectable = selectable;
		CalculateWindowNavigationLinks();
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

	void RuntimeUIService::SetDisplayWindow(const std::string& windowTag, bool display)
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

	void RuntimeUIService::MoveRight()
	{
		// Ensure the user interface context is valid and the navigation link is valid
		Ref<UserInterface> activeUI = s_RuntimeUIContext->m_ActiveUI;
		if (activeUI && activeUI->m_SelectedWidget &&
			activeUI->m_ActiveWindow && 
			activeUI->m_SelectedWidget->m_NavigationLinks.m_RightWidgetIndex != k_InvalidWidgetIndex)
		{
			// Set the active background color of the original widget to indicate it is no longer selected
			activeUI->m_SelectedWidget->m_ActiveBackgroundColor = activeUI->m_SelectedWidget->m_DefaultBackgroundColor;

			// Set the new selected widget
			activeUI->m_SelectedWidget = activeUI->m_ActiveWindow->m_Widgets.at(activeUI->m_SelectedWidget->m_NavigationLinks.m_RightWidgetIndex).get();

			// Set the active background color of the new widget to indicate it is selected
			activeUI->m_SelectedWidget->m_ActiveBackgroundColor = activeUI->m_SelectColor;

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
		if (activeUI && activeUI->m_SelectedWidget &&
			activeUI->m_ActiveWindow && activeUI->m_SelectedWidget->m_NavigationLinks.m_LeftWidgetIndex != k_InvalidWidgetIndex)
		{
			// Set the active background color of the original widget to indicate it is no longer selected
			activeUI->m_SelectedWidget->m_ActiveBackgroundColor = activeUI->m_SelectedWidget->m_DefaultBackgroundColor;

			// Set the new selected widget
			activeUI->m_SelectedWidget = activeUI->m_ActiveWindow->m_Widgets.at(activeUI->m_SelectedWidget->m_NavigationLinks.m_LeftWidgetIndex).get();

			// Set the active background color of the new widget to indicate it is selected
			activeUI->m_SelectedWidget->m_ActiveBackgroundColor = activeUI->m_SelectColor;

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
		if (activeUI && activeUI->m_SelectedWidget &&
			activeUI->m_ActiveWindow && activeUI->m_SelectedWidget->m_NavigationLinks.m_UpWidgetIndex != k_InvalidWidgetIndex)
		{
			// Set the active background color of the original widget to indicate it is no longer selected
			activeUI->m_SelectedWidget->m_ActiveBackgroundColor = activeUI->m_SelectedWidget->m_DefaultBackgroundColor;

			// Set the new selected widget
			activeUI->m_SelectedWidget = activeUI->m_ActiveWindow->m_Widgets.at(activeUI->m_SelectedWidget->m_NavigationLinks.m_UpWidgetIndex).get();

			// Set the active background color of the new widget to indicate it is selected
			activeUI->m_SelectedWidget->m_ActiveBackgroundColor = activeUI->m_SelectColor;

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
		if (activeUI && activeUI->m_SelectedWidget &&
			activeUI->m_ActiveWindow && 
			activeUI->m_SelectedWidget->m_NavigationLinks.m_DownWidgetIndex != k_InvalidWidgetIndex)
		{
			// Set the active background color of the original widget to indicate it is no longer selected
			activeUI->m_SelectedWidget->m_ActiveBackgroundColor = activeUI->m_SelectedWidget->m_DefaultBackgroundColor;

			// Set the new selected widget
			activeUI->m_SelectedWidget = activeUI->m_ActiveWindow->m_Widgets.at(activeUI->m_SelectedWidget->m_NavigationLinks.m_DownWidgetIndex).get();

			// Set the active background color of the new widget to indicate it is selected
			activeUI->m_SelectedWidget->m_ActiveBackgroundColor = activeUI->m_SelectColor;

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
		if (s_RuntimeUIContext->m_ActiveUI->m_SelectedWidget->m_FunctionPointers.m_OnPress)
		{
			Utility::CallWrappedVoidNone(s_RuntimeUIContext->m_ActiveUI->m_SelectedWidget->m_FunctionPointers.m_OnPress->m_Function);
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
			Math::vec3 windowPosition = currentWindow.CalculatePosition(viewportData.m_Width, viewportData.m_Height);

			// Iterate through all widgets in the window
			for (Ref<Widget> currentWidget : currentWindow.m_Widgets)
			{
				// Calculate navigation links for the current widget
				currentWidget->m_NavigationLinks.m_RightWidgetIndex = CalculateNavigationLink(currentWindow, currentWidget, 
					Direction::Right, windowPosition, windowScale);
				currentWidget->m_NavigationLinks.m_LeftWidgetIndex = CalculateNavigationLink(currentWindow, currentWidget, 
					Direction::Left, windowPosition, windowScale);
				currentWidget->m_NavigationLinks.m_UpWidgetIndex = CalculateNavigationLink(currentWindow, currentWidget,
					Direction::Up, windowPosition, windowScale);
				currentWidget->m_NavigationLinks.m_DownWidgetIndex = CalculateNavigationLink(currentWindow, currentWidget,
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
		Math::vec2 currentWidgetPosition = currentWidget->CalculatePosition(windowPosition, windowSize);
		Math::vec2 currentWidgetSize = currentWidget->CalculateSize(windowSize);

		// Iterate through each potential widget and decide which widget makes sense to navigate to
		for (Ref<Widget> potentialChoice : currentWindow.m_Widgets)
		{
			// Skip the current widget and any non-selectable widgets
			if (potentialChoice == currentWidget || !potentialChoice->m_Selectable)
			{
				iteration++;
				continue;
			}
			// Calculate the position and size of the potential widget
			Math::vec2 potentialChoicePosition = potentialChoice->CalculatePosition(windowPosition, windowSize);
			Math::vec2 potentialChoiceSize = potentialChoice->CalculateSize(windowSize);

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

	Math::vec3 Window::CalculatePosition(uint32_t viewportWidth, uint32_t viewportHeight)
	{
		return Math::vec3((viewportWidth * m_ScreenPosition.x), (viewportHeight * m_ScreenPosition.y), m_ScreenPosition.z);
	}

	void Window::AddWidget(Ref<Widget> newWidget)
	{
		m_Widgets.push_back(newWidget);
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

	void TextWidget::PushRenderData(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth)
	{
		Rendering::RendererInputSpec& inputSpec = RuntimeUIService::s_RuntimeUIContext->m_BackgroundInputSpec;

		// Calculate the widget's rendering data
		Math::vec3 widgetSize = CalculateSize(windowSize);

		// Get widget translation
		Math::vec3 widgetTranslation = CalculatePosition(windowTranslation, windowSize);

		// Create the widget's background rendering data
		inputSpec.m_TransformMatrix = glm::translate(Math::mat4(1.0f), Math::vec3(widgetTranslation.x + (widgetSize.x / 2), widgetTranslation.y + (widgetSize.y / 2), widgetTranslation.z))
			* glm::scale(Math::mat4(1.0f), widgetSize);
		Rendering::Shader::SetDataAtInputLocation<Math::vec4>(m_ActiveBackgroundColor, "a_Color", inputSpec.m_Buffer, inputSpec.m_Shader);

		// Submit background data to GPU
		Rendering::RenderingService::SubmitDataToRenderer(RuntimeUIService::s_RuntimeUIContext->m_BackgroundInputSpec);

		// Create the widget's text rendering data
		widgetTranslation.z += 0.001f;
		Math::vec2 resolution = Utility::ScreenResolutionToAspectRatio(Projects::ProjectService::GetActiveTargetResolution());
		float textSize = (viewportWidth * 0.15f * m_TextSize) * (resolution.y / resolution.x);
		if (m_TextCentered)
		{
			widgetTranslation = Math::vec3(widgetTranslation.x + (widgetSize.x * 0.5f) - ((m_TextAbsoluteDimensions.x * 0.5f) * textSize), widgetTranslation.y + (widgetSize.y * 0.5f) - ((m_TextAbsoluteDimensions.y * 0.5f) * textSize), widgetTranslation.z);
		}

		// Call the text's rendering function
		RuntimeUIService::s_RuntimeUIContext->m_ActiveUI->m_Font->PushTextData(m_Text, widgetTranslation, m_TextColor, textSize);
	}

	void TextWidget::CalculateTextSize()
	{
		// Calculate the text size of the widget using the default font if the active user interface is not set
		if (!RuntimeUIService::s_RuntimeUIContext->m_ActiveUI)
		{
			m_TextAbsoluteDimensions = RuntimeUIService::s_RuntimeUIContext->m_DefaultFont->CalculateTextSize(m_Text);
			return;
		}

		// Calculate the text size of the widget using the active user interface font
		m_TextAbsoluteDimensions = RuntimeUIService::s_RuntimeUIContext->m_ActiveUI->m_Font->CalculateTextSize(m_Text);
	}

	void TextWidget::SetText(const std::string& newText)
	{
		// Set the text of the widget
		m_Text = newText;

		// Calculate the new text size
		CalculateTextSize();
	}

	Math::vec3 Widget::CalculateSize(const Math::vec3& windowSize)
	{
		return Math::vec3(windowSize.x * m_Size.x, windowSize.y * m_Size.y, 1.0f);
	}

	Math::vec3 Widget::CalculatePosition(const Math::vec3& windowTranslation, const Math::vec3& windowSize)
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

}

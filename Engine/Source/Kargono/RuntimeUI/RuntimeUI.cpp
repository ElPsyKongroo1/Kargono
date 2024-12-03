#include "kgpch.h"

#include "Kargono/RuntimeUI/RuntimeUI.h"

#include "Kargono/Assets/AssetService.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Rendering/RenderingService.h"
#include "Kargono/Rendering/Shader.h"
#include "Kargono/ECS/EngineComponents.h"

namespace Kargono::RuntimeUI
{
	static Ref<Font> s_DefaultFont = nullptr;

	static Rendering::RendererInputSpec s_BackgroundInputSpec{};

	void RuntimeUIService::Init()
	{
		s_ActiveUI = nullptr;
		s_ActiveUIHandle = Assets::EmptyHandle;
		s_DefaultFont = FontService::InstantiateEditorFont("Resources/fonts/arial.ttf");
		// Initialize Window Spec Data
		{
			Rendering::ShaderSpecification shaderSpec {Rendering::ColorInputType::FlatColor, Rendering::TextureInputType::None, false, true, false, Rendering::RenderingType::DrawIndex, false};
			auto [uuid, localShader] = Assets::AssetService::GetShader(shaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			Rendering::Shader::SetDataAtInputLocation<Math::vec4>({ 1.0f, 1.0f, 1.0f, 1.0f }, "a_Color", localBuffer, localShader);

			ECS::ShapeComponent* shapeComp = new ECS::ShapeComponent();
			shapeComp->CurrentShape = Rendering::ShapeTypes::Quad;
			shapeComp->Vertices = CreateRef<std::vector<Math::vec3>>(Rendering::Shape::s_Quad.GetIndexVertices());
			shapeComp->Indices = CreateRef<std::vector<uint32_t>>(Rendering::Shape::s_Quad.GetIndices());

			s_BackgroundInputSpec.Shader = localShader;
			s_BackgroundInputSpec.Buffer = localBuffer;
			s_BackgroundInputSpec.ShapeComponent = shapeComp;
		}
		KG_VERIFY(true, "Runtime UI Engine Init");
	}

	void RuntimeUIService::Terminate()
	{
	}

	void RuntimeUIService::SetActiveUI(Ref<UserInterface> userInterface, Assets::AssetHandle uiHandle)
	{
		ClearActiveUI();

		s_ActiveUI = userInterface;
		s_ActiveUIHandle = uiHandle;
		RevalidateDisplayedWindow();
		if (!userInterface->m_Font)
		{
			userInterface->m_Font = s_DefaultFont;
			userInterface->m_FontHandle = Assets::EmptyHandle;
		}

		if (s_ActiveUI->m_Windows.size() > 0)
		{
			s_ActiveUI->m_Windows.at(0).DisplayWindow();
			s_ActiveUI->m_ActiveWindow = &s_ActiveUI->m_Windows.at(0);
			if (s_ActiveUI->m_ActiveWindow->m_DefaultActiveWidgetRef)
			{
				s_ActiveUI->m_SelectedWidget = s_ActiveUI->m_ActiveWindow->m_DefaultActiveWidgetRef.get();
				s_ActiveUI->m_SelectedWidget->m_ActiveBackgroundColor = s_ActiveUI->m_SelectColor;
			}
			else
			{
				s_ActiveUI->m_SelectedWidget = nullptr;
			}

			for (auto& window : s_ActiveUI->m_Windows)
			{
				for (auto& widget : window.m_Widgets)
				{
					if (widget->m_WidgetType == WidgetTypes::TextWidget)
					{
						TextWidget* textWidget = (TextWidget*)widget.get();
						textWidget->CalculateTextSize();
					}
				}
			}
		}

		CalculateWidgetDirections();
	}

	void RuntimeUIService::SetActiveUIFromName(const std::string& uiName)
	{
		auto [handle, uiReference] = Assets::AssetService::GetUserInterface(uiName);

		if (!uiReference)
		{
			KG_WARN("Could not locate user interface by name");
			return;
		}

		SetActiveUI(uiReference, handle);
	}


	bool RuntimeUIService::RevalidateUIWidgets()
	{
		// Ensure active user interface is valid
		if (!static_cast<bool>(s_ActiveUI) || s_ActiveUIHandle == Assets::EmptyHandle)
		{
			KG_ERROR("Attempt to save user interface with invalid UserInterface or Assets::AssetHandle in s_Engine");
			return false;
		}

		// Set default widget for every window
		for (Window& window : s_ActiveUI->m_Windows)
		{
			if (window.m_DefaultActiveWidgetRef)
			{
				auto iterator = std::find(window.m_Widgets.begin(), window.m_Widgets.end(), window.m_DefaultActiveWidgetRef);
				window.m_DefaultActiveWidget = static_cast<int32_t>(iterator - window.m_Widgets.begin());
			}
			else
			{
				window.m_DefaultActiveWidget = -1;
			}
		}

		CalculateWidgetDirections();
		return true;
	}

	void RuntimeUIService::DeleteActiveWindow(uint32_t windowLocation)
	{
		auto& windows = GetActiveWindows();
		std::vector<Window>::iterator windowPointer = windows.begin() + static_cast<uint32_t>(windowLocation);
		windows.erase(windowPointer);

		RuntimeUIService::RevalidateDisplayedWindow();
	}


	void RuntimeUIService::PushRenderData(const Math::mat4& cameraViewMatrix, uint32_t viewportWidth, uint32_t viewportHeight)
	{
		if (!s_ActiveUI)
		{
			return;
		}

		Rendering::RendererAPI::ClearDepthBuffer();
		// Iterate through all characters
		Math::mat4 orthographicProjection = glm::ortho((float)0, static_cast<float>(viewportWidth),
			(float)0, static_cast<float>(viewportHeight), (float)-1, (float)1);
		Math::mat4 outputMatrix = orthographicProjection;

		Rendering::RenderingService::BeginScene(outputMatrix);

		// Submit all windows
		for (auto window : s_ActiveUI->m_DisplayedWindows)
		{
			Math::vec3 scale = Math::vec3(viewportWidth * window->m_Size.x, viewportHeight * window->m_Size.y, 1.0f);
			Math::vec3 initialTranslation = Math::vec3((viewportWidth * window->m_ScreenPosition.x), (viewportHeight * window->m_ScreenPosition.y), window->m_ScreenPosition.z);
			Math::vec3 translation = Math::vec3( initialTranslation.x + (scale.x / 2),  initialTranslation.y + (scale.y / 2), initialTranslation.z);

			s_BackgroundInputSpec.TransformMatrix = glm::translate(Math::mat4(1.0f), translation)
				* glm::scale(Math::mat4(1.0f), scale);
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(window->m_BackgroundColor, "a_Color", s_BackgroundInputSpec.Buffer, s_BackgroundInputSpec.Shader);

			Rendering::RenderingService::SubmitDataToRenderer(s_BackgroundInputSpec);

			initialTranslation.z += 0.001f;
			for (auto& widget : window->m_Widgets)
			{
				widget->PushRenderData(initialTranslation, scale, static_cast<float>(viewportWidth));
			}
		}

		Rendering::RenderingService::EndScene();

	}

	void RuntimeUIService::AddActiveWindow(Window& window)
	{
		window.DisplayWindow();
		s_ActiveUI->m_Windows.push_back(window);
		RuntimeUIService::RevalidateDisplayedWindow();
	}

	void RuntimeUIService::SetActiveFont(Ref<Font> newFont, Assets::AssetHandle fontHandle)
	{
		s_ActiveUI->m_Font = newFont;
		s_ActiveUI->m_FontHandle = fontHandle;

		for (auto& window : s_ActiveUI->m_Windows)
		{
			for (auto& widget : window.m_Widgets)
			{
				if (widget->m_WidgetType == WidgetTypes::TextWidget)
				{
					TextWidget* textWidget = (TextWidget*)widget.get();
					textWidget->CalculateTextSize();
				}
			}
		}
	}

	std::vector<Window>& RuntimeUIService::GetActiveWindows()
	{
		return s_ActiveUI->m_Windows;
	}

	void RuntimeUIService::RevalidateDisplayedWindow()
	{
		// Ensure/validate that the correct window is being displayed
		s_ActiveUI->m_DisplayedWindows.clear();
		for (Window& window : GetActiveWindows())
		{
			if (window.GetWindowDisplayed()) 
			{ 
				s_ActiveUI->m_DisplayedWindows.push_back(&window); 
			}
		}
	}

	void RuntimeUIService::ClearActiveUI()
	{
		s_ActiveUI = nullptr;
		s_ActiveUIHandle = Assets::EmptyHandle;
	}

	Ref<UserInterface> RuntimeUIService::GetActiveUI()
	{
		return s_ActiveUI;
	}

	Assets::AssetHandle RuntimeUIService::GetActiveUIHandle()
	{
		return s_ActiveUIHandle;
	}

	void RuntimeUIService::SetSelectedWidgetColor(const Math::vec4& color)
	{
		s_ActiveUI->m_SelectedWidget->m_ActiveBackgroundColor = color;
	}

	bool RuntimeUIService::IsWidgetSelected(const std::string& windowTag, const std::string& widgetTag)
	{
		Ref<Widget> currentWidget = GetWidget(windowTag, widgetTag);

		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when checking the widget is selected");
			return false;
		}

		if (s_ActiveUI->m_SelectedWidget == currentWidget.get())
		{
			return true;
		}
		return false;
	}

	void RuntimeUIService::SetActiveWidgetText(const std::string& windowTag, const std::string& widgetTag, const std::string& newText)
	{
		Ref<Widget> currentWidget = GetWidget(windowTag, widgetTag);

		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget's text");
			return;
		}

		if (currentWidget->m_WidgetType != WidgetTypes::TextWidget)
		{
			KG_WARN("Attempt to change the text of a widget that is not a text widget");
			return;
		}

		TextWidget* textWidget = (TextWidget*)currentWidget.get();
		textWidget->m_Text = newText;
	}

	void RuntimeUIService::SetSelectedWidget(const std::string& windowTag, const std::string& widgetTag)
	{
		Ref<Widget> currentWidget = GetWidget(windowTag, widgetTag);

		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget as selected");
			return;
		}

		s_ActiveUI->m_SelectedWidget->m_ActiveBackgroundColor = s_ActiveUI->m_SelectedWidget->m_DefaultBackgroundColor;
		s_ActiveUI->m_SelectedWidget = currentWidget.get();
		s_ActiveUI->m_SelectedWidget->m_ActiveBackgroundColor = s_ActiveUI->m_SelectColor;
		if (s_ActiveUI->m_FunctionPointers.m_OnMove)
		{
			Utility::CallWrappedVoidNone(s_ActiveUI->m_FunctionPointers.m_OnMove->m_Function);
		}
	}

	void RuntimeUIService::SetWidgetTextColor(const std::string& windowTag, const std::string& widgetTag, const Math::vec4& color)
	{
		Ref<Widget> currentWidget = GetWidget(windowTag, widgetTag);

		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget's text color");
			return;
		}

		if (currentWidget->m_WidgetType != WidgetTypes::TextWidget)
		{
			KG_WARN("Attempt to set text color on widget that is not a TextWidget");
			return;
		}

		TextWidget* textWidget = (TextWidget*)currentWidget.get();
		textWidget->m_TextColor = color;
	}

	void RuntimeUIService::SetWidgetBackgroundColor(const std::string& windowTag, const std::string& widgetTag, const Math::vec4& color)
	{
		Ref<Widget> currentWidget = GetWidget(windowTag, widgetTag);

		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget's background color");
			return;
		}

		currentWidget->m_DefaultBackgroundColor = color;
		currentWidget->m_ActiveBackgroundColor = color;
	}

	void RuntimeUIService::SetWidgetSelectable(const std::string& windowTag, const std::string& widgetTag, bool selectable)
	{
		Ref<Widget> currentWidget = GetWidget(windowTag, widgetTag);

		if (!currentWidget)
		{
			KG_WARN("Could not locate widget when attempting to set a widget as selectable");
			return;
		}

		currentWidget->m_Selectable = selectable;
		CalculateWidgetDirections();
	}

	void RuntimeUIService::SetActiveOnMove(Assets::AssetHandle functionHandle, Ref<Scripting::Script> function)
	{
		s_ActiveUI->m_FunctionPointers.m_OnMove = function;
		s_ActiveUI->m_FunctionPointers.m_OnMoveHandle = functionHandle;
	}

	Ref<Scripting::Script> RuntimeUIService::GetActiveOnMove()
	{
		return s_ActiveUI->m_FunctionPointers.m_OnMove;
	}

	Assets::AssetHandle RuntimeUIService::GetActiveOnMoveHandle()
	{
		return s_ActiveUI->m_FunctionPointers.m_OnMoveHandle;
	}

	void RuntimeUIService::SetDisplayWindow(const std::string& windowTag, bool display)
	{
		for (auto& window : s_ActiveUI->m_Windows)
		{
			if (window.m_Tag == windowTag)
			{
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
		if (s_ActiveUI && s_ActiveUI->m_SelectedWidget &&
			s_ActiveUI->m_ActiveWindow && s_ActiveUI->m_SelectedWidget->m_DirectionPointer.m_RightWidgetIndex != -1)
		{
			s_ActiveUI->m_SelectedWidget->m_ActiveBackgroundColor = s_ActiveUI->m_SelectedWidget->m_DefaultBackgroundColor;
			s_ActiveUI->m_SelectedWidget = s_ActiveUI->m_ActiveWindow->m_Widgets.at(s_ActiveUI->m_SelectedWidget->m_DirectionPointer.m_RightWidgetIndex).get();
			s_ActiveUI->m_SelectedWidget->m_ActiveBackgroundColor = s_ActiveUI->m_SelectColor;
			if (s_ActiveUI->m_FunctionPointers.m_OnMove)
			{
				Utility::CallWrappedVoidNone(s_ActiveUI->m_FunctionPointers.m_OnMove->m_Function);
			}
		}
	}

	void RuntimeUIService::MoveLeft()
	{
		if (s_ActiveUI && s_ActiveUI->m_SelectedWidget &&
			s_ActiveUI->m_ActiveWindow && s_ActiveUI->m_SelectedWidget->m_DirectionPointer.m_LeftWidgetIndex != -1)
		{
			s_ActiveUI->m_SelectedWidget->m_ActiveBackgroundColor = s_ActiveUI->m_SelectedWidget->m_DefaultBackgroundColor;
			s_ActiveUI->m_SelectedWidget = s_ActiveUI->m_ActiveWindow->m_Widgets.at(s_ActiveUI->m_SelectedWidget->m_DirectionPointer.m_LeftWidgetIndex).get();
			s_ActiveUI->m_SelectedWidget->m_ActiveBackgroundColor = s_ActiveUI->m_SelectColor;
			if (s_ActiveUI->m_FunctionPointers.m_OnMove)
			{
				Utility::CallWrappedVoidNone(s_ActiveUI->m_FunctionPointers.m_OnMove->m_Function);
			}
		}
	}

	void RuntimeUIService::MoveUp()
	{
		if (s_ActiveUI && s_ActiveUI->m_SelectedWidget &&
			s_ActiveUI->m_ActiveWindow && s_ActiveUI->m_SelectedWidget->m_DirectionPointer.m_UpWidgetIndex != -1)
		{
			s_ActiveUI->m_SelectedWidget->m_ActiveBackgroundColor = s_ActiveUI->m_SelectedWidget->m_DefaultBackgroundColor;
			s_ActiveUI->m_SelectedWidget = s_ActiveUI->m_ActiveWindow->m_Widgets.at(s_ActiveUI->m_SelectedWidget->m_DirectionPointer.m_UpWidgetIndex).get();
			s_ActiveUI->m_SelectedWidget->m_ActiveBackgroundColor = s_ActiveUI->m_SelectColor;
			if (s_ActiveUI->m_FunctionPointers.m_OnMove)
			{
				Utility::CallWrappedVoidNone(s_ActiveUI->m_FunctionPointers.m_OnMove->m_Function);
			}
		}
	}

	void RuntimeUIService::MoveDown()
	{
		if (s_ActiveUI && s_ActiveUI->m_SelectedWidget &&
			s_ActiveUI->m_ActiveWindow && s_ActiveUI->m_SelectedWidget->m_DirectionPointer.m_DownWidgetIndex != -1)
		{
			s_ActiveUI->m_SelectedWidget->m_ActiveBackgroundColor = s_ActiveUI->m_SelectedWidget->m_DefaultBackgroundColor;
			s_ActiveUI->m_SelectedWidget = s_ActiveUI->m_ActiveWindow->m_Widgets.at(s_ActiveUI->m_SelectedWidget->m_DirectionPointer.m_DownWidgetIndex).get();
			s_ActiveUI->m_SelectedWidget->m_ActiveBackgroundColor = s_ActiveUI->m_SelectColor;
			if (s_ActiveUI->m_FunctionPointers.m_OnMove)
			{
				Utility::CallWrappedVoidNone(s_ActiveUI->m_FunctionPointers.m_OnMove->m_Function);
			}
			
		}
	}

	void RuntimeUIService::OnPress()
	{
		if (!s_ActiveUI->m_SelectedWidget) { return; }
		if (s_ActiveUI->m_SelectedWidget->m_FunctionPointers.m_OnPress)
		{
			Utility::CallWrappedVoidNone(s_ActiveUI->m_SelectedWidget->m_FunctionPointers.m_OnPress->m_Function);
		}
	}

	void RuntimeUIService::CalculateWidgetDirections()
	{
		for (Window& window : s_ActiveUI->m_Windows)
		{
			for (Ref<Widget> currentWidget : window.m_Widgets)
			{
				Ref<Widget> currentBestChoice;
				uint32_t currentChoiceLocation;
				float currentBestDistance;
				uint32_t iteration;


				// Calculate Right
				currentBestChoice = nullptr;
				currentChoiceLocation = 0;
				currentBestDistance = std::numeric_limits<float>::max();
				iteration = 0;

				for (Ref<Widget> potentialChoice : window.m_Widgets)
				{
					if (potentialChoice == currentWidget || !potentialChoice->m_Selectable)
					{
						iteration++;
						continue;
					}

					float currentDistance = glm::abs(glm::distance(potentialChoice->m_WindowPosition, currentWidget->m_WindowPosition));

					// Constraints ----------------
					float currentWidgetRightExtent = currentWidget->m_WindowPosition.x + (currentWidget->m_Size.x / 2);
					float potentialWidgetLeftExtent = potentialChoice->m_WindowPosition.x - (potentialChoice->m_Size.x / 2);
					if (currentWidgetRightExtent >= potentialWidgetLeftExtent)
					{
						iteration++;
						continue;
					}

					// Preferences ----------------
					if (currentBestChoice == nullptr)
					{
						currentBestChoice = potentialChoice;
						currentChoiceLocation = iteration;
						currentBestDistance = currentDistance;
						iteration++;
						continue;
					}

					
					// Prefer Lowest Distance Widget
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
				if (currentBestChoice)
				{
					currentWidget->m_DirectionPointer.m_RightWidgetIndex = currentChoiceLocation;
				}
				else
				{
					currentWidget->m_DirectionPointer.m_RightWidgetIndex = -1;
				}

				// Calculate Left
				currentBestChoice = nullptr;
				currentChoiceLocation = 0;
				currentBestDistance = std::numeric_limits<float>::max();
				iteration = 0;

				for (Ref<Widget> potentialChoice : window.m_Widgets)
				{
					if (potentialChoice == currentWidget || !potentialChoice->m_Selectable)
					{
						iteration++;
						continue;
					}

					float currentDistance = glm::abs(glm::distance(potentialChoice->m_WindowPosition, currentWidget->m_WindowPosition));

					// Constraints ----------------
					float currentWidgetLeftExtent = currentWidget->m_WindowPosition.x - (currentWidget->m_Size.x / 2);
					float potentialWidgetRightExtent = potentialChoice->m_WindowPosition.x + (potentialChoice->m_Size.x / 2);
					if (currentWidgetLeftExtent <= potentialWidgetRightExtent)
					{
						iteration++;
						continue;
					}

					// Preferences ----------------
					if (currentBestChoice == nullptr)
					{
						currentBestChoice = potentialChoice;
						currentChoiceLocation = iteration;
						currentBestDistance = currentDistance;
						iteration++;
						continue;
					}


					// Prefer Lowest Distance Widget
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
				if (currentBestChoice)
				{
					currentWidget->m_DirectionPointer.m_LeftWidgetIndex = currentChoiceLocation;
				}
				else
				{
					currentWidget->m_DirectionPointer.m_LeftWidgetIndex = -1;
				}

				// Calculate Up
				currentBestChoice = nullptr;
				currentChoiceLocation = 0;
				currentBestDistance = std::numeric_limits<float>::max();
				iteration = 0;

				for (Ref<Widget> potentialChoice : window.m_Widgets)
				{
					if (potentialChoice == currentWidget || !potentialChoice->m_Selectable)
					{
						iteration++;
						continue;
					}

					float currentDistance = glm::abs(glm::distance(potentialChoice->m_WindowPosition, currentWidget->m_WindowPosition));

					// Constraints ----------------
					float currentWidgetUpExtent = currentWidget->m_WindowPosition.y + (currentWidget->m_Size.y / 2);
					float potentialWidgetDownExtent = potentialChoice->m_WindowPosition.y - (potentialChoice->m_Size.y / 2);
					if (currentWidgetUpExtent >= potentialWidgetDownExtent)
					{
						iteration++;
						continue;
					}

					// Preferences ----------------
					if (currentBestChoice == nullptr)
					{
						currentBestChoice = potentialChoice;
						currentChoiceLocation = iteration;
						currentBestDistance = currentDistance;
						iteration++;
						continue;
					}


					// Prefer Lowest Distance Widget
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
				if (currentBestChoice)
				{
					currentWidget->m_DirectionPointer.m_UpWidgetIndex = currentChoiceLocation;
				}
				else
				{
					currentWidget->m_DirectionPointer.m_UpWidgetIndex = -1;
				}

				// Calculate Down
				currentBestChoice = nullptr;
				currentChoiceLocation = 0;
				currentBestDistance = std::numeric_limits<float>::max();
				iteration = 0;

				for (Ref<Widget> potentialChoice : window.m_Widgets)
				{
					if (potentialChoice == currentWidget || !potentialChoice->m_Selectable)
					{
						iteration++;
						continue;
					}

					float currentDistance = glm::abs(glm::distance(potentialChoice->m_WindowPosition, currentWidget->m_WindowPosition));

					// Constraints ----------------
					float currentWidgetDownExtent = currentWidget->m_WindowPosition.y - (currentWidget->m_Size.y / 2);
					float potentialWidgetUpExtent = potentialChoice->m_WindowPosition.y + (potentialChoice->m_Size.y / 2);
					if (currentWidgetDownExtent <= potentialWidgetUpExtent)
					{
						iteration++;
						continue;
					}

					// Preferences ----------------
					if (currentBestChoice == nullptr)
					{
						currentBestChoice = potentialChoice;
						currentChoiceLocation = iteration;
						currentBestDistance = currentDistance;
						iteration++;
						continue;
					}


					// Prefer Lowest Distance Widget
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
				if (currentBestChoice)
				{
					currentWidget->m_DirectionPointer.m_DownWidgetIndex = currentChoiceLocation;
				}
				else
				{
					currentWidget->m_DirectionPointer.m_DownWidgetIndex = -1;
				}

			}
		}
	}

	Ref<Widget> RuntimeUIService::GetWidget(const std::string& windowTag, const std::string& widgetTag)
	{
		// Get widget using its parent window tag and its widget tag
		for (Window& window : s_ActiveUI->m_Windows)
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
		if (m_WindowDisplayed) { return; }

		m_WindowDisplayed = true;

		RuntimeUIService::RevalidateDisplayedWindow();

	}

	void Window::HideWindow()
	{
		if (!m_WindowDisplayed) { return; }

		m_WindowDisplayed = false;

		RuntimeUIService::RevalidateDisplayedWindow();
	}

	bool Window::GetWindowDisplayed()
	{
		return m_WindowDisplayed;
	}

	void Window::AddWidget(Ref<Widget> newWidget)
	{
		m_Widgets.push_back(newWidget);
	}

	void Window::DeleteWidget(int32_t widgetLocation)
	{
		KG_ASSERT(widgetLocation >= 0, "Invalid Location provided to DeleteWidget!");

		m_Widgets.erase(m_Widgets.begin() + widgetLocation);
	}

	void TextWidget::PushRenderData(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth)
	{
		Math::vec3 widgetSize = Math::vec3(windowSize.x * m_Size.x, windowSize.y * m_Size.y, 1.0f);
		Math::vec3 widgetTranslation = Math::vec3(windowTranslation.x + (windowSize.x * m_WindowPosition.x),
							windowTranslation.y + (windowSize.y * m_WindowPosition.y),
							windowTranslation.z);
		// Draw Widget Background
		s_BackgroundInputSpec.TransformMatrix = glm::translate(Math::mat4(1.0f), Math::vec3(widgetTranslation.x + (widgetSize.x / 2), widgetTranslation.y + (widgetSize.y / 2), widgetTranslation.z))
			* glm::scale(Math::mat4(1.0f), widgetSize);

		Rendering::Shader::SetDataAtInputLocation<Math::vec4>(m_ActiveBackgroundColor, "a_Color", s_BackgroundInputSpec.Buffer, s_BackgroundInputSpec.Shader);
		Rendering::RenderingService::SubmitDataToRenderer(s_BackgroundInputSpec);

		widgetTranslation.z += 0.001f;

		// Render Text
		Math::vec2 resolution = Utility::ScreenResolutionToAspectRatio(Projects::ProjectService::GetActiveTargetResolution());
		float textSize = (viewportWidth * 0.15f * m_TextSize) * (resolution.y / resolution.x);

		if (m_TextCentered)
		{
			widgetTranslation = Math::vec3(widgetTranslation.x + (widgetSize.x * 0.5f) - ((m_TextAbsoluteDimensions.x * 0.5f) * textSize), widgetTranslation.y + (widgetSize.y * 0.5f) - ((m_TextAbsoluteDimensions.y * 0.5f) * textSize), widgetTranslation.z);
		}

		RuntimeUIService::s_ActiveUI->m_Font->PushTextData(m_Text, widgetTranslation, m_TextColor, textSize);
	}

	void TextWidget::CalculateTextSize()
	{
		if (!RuntimeUIService::s_ActiveUI)
		{
			m_TextAbsoluteDimensions = s_DefaultFont->CalculateTextSize(m_Text);
			return;
		}
		m_TextAbsoluteDimensions = RuntimeUIService::s_ActiveUI->m_Font->CalculateTextSize(m_Text);
	}

	void TextWidget::SetText(const std::string& newText)
	{
		m_Text = newText;
		CalculateTextSize();
	}

}

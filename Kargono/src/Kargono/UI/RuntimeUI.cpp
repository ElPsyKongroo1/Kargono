#include "kgpch.h"

#include "Kargono/UI/RuntimeUI.h"

#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Core/Application.h"
#include "Kargono/Project/Project.h"
#include "Kargono/Renderer/RenderCommand.h"
#include "Kargono/Renderer/Renderer.h"
#include "Kargono/Renderer/Shader.h"
#include "Kargono/Scene/Components.h"

namespace Kargono
{
	UIEngine UIEngine::s_Engine{};
	Ref<Font> s_DefaultFont = nullptr;

	static Shader::RendererInputSpec s_BackgroundInputSpec{};

	void UIEngine::Init()
	{
		s_Engine.m_CurrentUI = nullptr;
		s_Engine.m_CurrentUIHandle = 0;
		s_DefaultFont = TextEngine::InstantiateFontEditor("resources/fonts/arial.ttf");
		s_Engine.m_CurrentFont = s_DefaultFont;
		s_Engine.m_FontHandle = 0;
		// Initialize Window Spec Data
		{
			Shader::ShaderSpecification shaderSpec {Shader::ColorInputType::FlatColor, Shader::TextureInputType::None, false, true, false, Shape::RenderingType::DrawIndex, false};
			auto [uuid, localShader] = AssetManager::GetShader(shaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			Shader::SetDataAtInputLocation<glm::vec4>({ 1.0f, 1.0f, 1.0f, 1.0f }, "a_Color", localBuffer, localShader);

			ShapeComponent* shapeComp = new ShapeComponent();
			shapeComp->CurrentShape = Shape::ShapeTypes::Quad;
			shapeComp->Vertices = CreateRef<std::vector<glm::vec3>>(Shape::s_Quad.GetIndexVertices());
			shapeComp->Indices = CreateRef<std::vector<uint32_t>>(Shape::s_Quad.GetIndices());

			s_BackgroundInputSpec.Shader = localShader;
			s_BackgroundInputSpec.Buffer = localBuffer;
			s_BackgroundInputSpec.ShapeComponent = shapeComp;
		}
	}


	void UIEngine::LoadUIObject(Ref<UIObject> uiObject, AssetHandle uiHandle)
	{
		ClearUIEngine();

		s_Engine.m_CurrentUI = uiObject;
		s_Engine.m_CurrentUIHandle = uiHandle;
		s_Engine.m_UICache = uiObject->Windows;
		RefreshDisplayedWindows();
		s_Engine.m_CurrentFont = uiObject->m_Font;
		s_Engine.m_FontHandle = uiObject->m_FontHandle;
		if (!s_Engine.m_CurrentFont)
		{
			s_Engine.m_CurrentFont = s_DefaultFont;
			s_Engine.m_FontHandle = 0;
		}
	}

	void UIEngine::Terminate()
	{
	}

	bool UIEngine::SaveCurrentUI()
	{
		if (!static_cast<bool>(s_Engine.m_CurrentUI) || s_Engine.m_CurrentUIHandle == 0)
		{
			KG_CORE_ERROR("Attempt to save user interface with invalid UIObject or AssetHandle in s_Engine");
			return false;
		}
		auto currentUI = s_Engine.m_CurrentUI;
		currentUI->m_Font = s_Engine.m_CurrentFont;
		currentUI->m_FontHandle = s_Engine.m_FontHandle;
		currentUI->Windows = s_Engine.m_UICache;
		return true;
	}

	void UIEngine::DeleteWindow(uint32_t windowLocation)
	{
		auto& windows = s_Engine.GetAllWindows();
		std::vector<UIEngine::Window>::iterator windowPointer = windows.begin() + static_cast<uint32_t>(windowLocation);
		windows.erase(windowPointer);

		UIEngine::RefreshDisplayedWindows();
	}

	void UIEngine::PushRenderData(const EditorCamera& camera, uint32_t viewportWidth, uint32_t viewportHeight)
	{
		RenderCommand::ClearDepthBuffer();
		// Iterate through all characters
		glm::mat4 cameraViewMatrix = glm::inverse(camera.GetViewMatrix());
		glm::mat4 orthographicProjection = glm::ortho((float)0, static_cast<float>(viewportWidth),
			(float)0, static_cast<float>(viewportHeight), (float)-1, (float)1);
		glm::mat4 outputMatrix = orthographicProjection;

		/*glm::vec2 windowLocation = glm::vec2(0.3f, 0.2f);
		glm::vec2 windowSize = glm::vec2(0.2f, 0.2f);*/

		Renderer::BeginScene(outputMatrix);

		// Submit all windows
		for (auto window : s_Engine.m_DisplayedWindows)
		{
			glm::vec3 scale = glm::vec3(viewportWidth * window->Size.x, viewportHeight * window->Size.y, 1.0f);
			glm::vec3 initialTranslation = glm::vec3((viewportWidth * window->ScreenPosition.x), (viewportHeight * window->ScreenPosition.y), window->ScreenPosition.z);
			glm::vec3 translation = glm::vec3( initialTranslation.x + (scale.x / 2),  initialTranslation.y + (scale.y / 2), initialTranslation.z);

			s_BackgroundInputSpec.TransformMatrix = glm::translate(glm::mat4(1.0f), translation)
				* glm::scale(glm::mat4(1.0f), scale);
			Shader::SetDataAtInputLocation<glm::vec4>(window->BackgroundColor, "a_Color", s_BackgroundInputSpec.Buffer, s_BackgroundInputSpec.Shader);

			Renderer::SubmitDataToRenderer(s_BackgroundInputSpec);

			initialTranslation.z += 0.001f;
			for (auto& widget : window->Widgets)
			{
				widget->PushRenderData(initialTranslation, scale, static_cast<float>(viewportWidth));
			}
		}

		Renderer::EndScene();

	}

	void UIEngine::AddWindow(UIEngine::Window& window)
	{
		window.DisplayWindow();
		s_Engine.m_UICache.push_back(window);
		UIEngine::RefreshDisplayedWindows();
	}

	void UIEngine::SetFont(Ref<Font> newFont, AssetHandle fontHandle)
	{
		s_Engine.m_CurrentFont = newFont;
		s_Engine.m_FontHandle = fontHandle;
	}

	std::vector<UIEngine::Window>& UIEngine::GetAllWindows()
	{
		return s_Engine.m_UICache;
	}

	void UIEngine::Window::IncrementIterators(uint16_t iterator)
	{
		// Note that comparisons with itself does not cause increment
		//		Ex: TextWidgetLocation is not greater than TextWidgetLocation
		if (WidgetCounts.TextWidgetLocation > iterator) { WidgetCounts.TextWidgetLocation++; }
		if (WidgetCounts.ButtonWidgetLocation > iterator) { WidgetCounts.ButtonWidgetLocation++; }
		if (WidgetCounts.CheckboxWidgetLocation > iterator) { WidgetCounts.CheckboxWidgetLocation++; }
		if (WidgetCounts.ComboWidgetLocation > iterator) { WidgetCounts.ComboWidgetLocation++; }
		if (WidgetCounts.PopupWidgetLocation > iterator) { WidgetCounts.PopupWidgetLocation++; }
	}

	void UIEngine::Window::DecrementIterators(uint16_t iterator)
	{
		// Note that comparisons with itself does not cause increment
		//		Ex: TextWidgetLocation is not greater than TextWidgetLocation
		if (WidgetCounts.TextWidgetLocation > iterator) { WidgetCounts.TextWidgetLocation--; }
		if (WidgetCounts.ButtonWidgetLocation > iterator) { WidgetCounts.ButtonWidgetLocation--; }
		if (WidgetCounts.CheckboxWidgetLocation > iterator) { WidgetCounts.CheckboxWidgetLocation--; }
		if (WidgetCounts.ComboWidgetLocation > iterator) { WidgetCounts.ComboWidgetLocation--; }
		if (WidgetCounts.PopupWidgetLocation > iterator) { WidgetCounts.PopupWidgetLocation--; }
	}

	void UIEngine::RefreshDisplayedWindows()
	{
		s_Engine.m_DisplayedWindows.clear();
		for (auto& window : s_Engine.GetAllWindows())
		{
			if (window.GetWindowDisplayed()) { s_Engine.m_DisplayedWindows.push_back(&window); }
		}
	}

	void UIEngine::ClearUIEngine()
	{
		s_Engine.m_CurrentUI = nullptr;
		s_Engine.m_CurrentUIHandle = 0;
		s_Engine.m_UICache.clear();
		s_Engine.m_DisplayedWindows.clear();
		s_Engine.m_SelectedWidget = nullptr;
		s_Engine.m_HoveredWidget = nullptr;
		s_Engine.m_ActiveWindow = nullptr;
		s_Engine.m_CurrentFont = s_DefaultFont;
		s_Engine.m_FontHandle = 0;
	}

	Ref<UIEngine::UIObject> UIEngine::GetCurrentUIObject()
	{
		return s_Engine.m_CurrentUI;
	}

	AssetHandle UIEngine::GetCurrentUIHandle()
	{
		return s_Engine.m_CurrentUIHandle;
	}

	void UIEngine::SetCurrentUIObject(Ref<UIEngine::UIObject> newUI)
	{
		s_Engine.m_CurrentUI = newUI;
	}

	void UIEngine::SetCurrentUIHandle(AssetHandle newHandle)
	{
		s_Engine.m_CurrentUIHandle = newHandle;
	}

	void UIEngine::Window::DisplayWindow()
	{
		if (WindowDisplayed) { return; }

		WindowDisplayed = true;

		UIEngine::RefreshDisplayedWindows();

	}

	void UIEngine::Window::HideWindow()
	{
		if (!WindowDisplayed) { return; }

		WindowDisplayed = false;

		UIEngine::RefreshDisplayedWindows();
	}

	bool UIEngine::Window::GetWindowDisplayed()
	{
		return WindowDisplayed;
	}

	void UIEngine::Window::AddTextWidget(Ref<UIEngine::TextWidget> newWidget)
	{
		if (WidgetCounts.TextWidgetCount == 0)
		{
			WidgetCounts.TextWidgetLocation = static_cast<uint16_t>(Widgets.size());
			Widgets.push_back(newWidget);
			WidgetCounts.TextWidgetCount++;
			return;
		}

		Widgets.insert(Widgets.begin() + WidgetCounts.TextWidgetLocation + WidgetCounts.TextWidgetCount, newWidget);
		Window::IncrementIterators(WidgetCounts.TextWidgetLocation);
		WidgetCounts.TextWidgetCount++;

	}
	void UIEngine::Window::AddButtonWidget(Ref<UIEngine::ButtonWidget> newWidget)
	{
		if (WidgetCounts.ButtonWidgetCount == 0)
		{
			WidgetCounts.ButtonWidgetLocation = static_cast<uint16_t>(Widgets.size());
			Widgets.push_back(newWidget);
			WidgetCounts.ButtonWidgetCount++;
			return;
		}

		Widgets.insert(Widgets.begin() + WidgetCounts.ButtonWidgetLocation + WidgetCounts.ButtonWidgetCount, newWidget);
		Window::IncrementIterators(WidgetCounts.ButtonWidgetLocation);
		WidgetCounts.ButtonWidgetCount++;
	}
	void UIEngine::Window::AddCheckboxWidget(Ref<UIEngine::CheckboxWidget> newWidget)
	{
		if (WidgetCounts.CheckboxWidgetCount == 0)
		{
			WidgetCounts.CheckboxWidgetLocation = static_cast<uint16_t>(Widgets.size());
			Widgets.push_back(newWidget);
			WidgetCounts.CheckboxWidgetCount++;
			return;
		}

		Widgets.insert(Widgets.begin() + WidgetCounts.CheckboxWidgetLocation + WidgetCounts.CheckboxWidgetCount, newWidget);
		Window::IncrementIterators(WidgetCounts.CheckboxWidgetLocation);
		WidgetCounts.CheckboxWidgetCount++;
	}
	void UIEngine::Window::AddComboWidget(Ref<UIEngine::ComboWidget> newWidget)
	{
		if (WidgetCounts.ComboWidgetCount == 0)
		{
			WidgetCounts.ComboWidgetLocation = static_cast<uint16_t>(Widgets.size());
			Widgets.push_back(newWidget);
			WidgetCounts.ComboWidgetCount++;
			return;
		}

		Widgets.insert(Widgets.begin() + WidgetCounts.ComboWidgetLocation + WidgetCounts.ComboWidgetCount, newWidget);
		Window::IncrementIterators(WidgetCounts.ComboWidgetLocation);
		WidgetCounts.ComboWidgetCount++;
	}
	void UIEngine::Window::AddPopupWidget(Ref<UIEngine::PopupWidget> newWidget)
	{
		if (WidgetCounts.PopupWidgetCount == 0)
		{
			WidgetCounts.PopupWidgetLocation = static_cast<uint16_t>(Widgets.size());
			Widgets.push_back(newWidget);
			WidgetCounts.PopupWidgetCount++;
			return;
		}

		Widgets.insert(Widgets.begin() + WidgetCounts.PopupWidgetLocation + WidgetCounts.PopupWidgetCount, newWidget);
		Window::IncrementIterators(WidgetCounts.PopupWidgetLocation);
		WidgetCounts.PopupWidgetCount++;
	}

	void UIEngine::Window::DeleteWidget(int32_t widgetLocation)
	{
		KG_CORE_ASSERT(widgetLocation >= 0, "Invalid Location provided to DeleteWidget!");
		switch (Widgets.at(widgetLocation)->WidgetType)
		{
		case WidgetTypes::TextWidget: {WidgetCounts.TextWidgetCount--;  break; }
		case WidgetTypes::ButtonWidget: {WidgetCounts.ButtonWidgetCount--;  break; }
		case WidgetTypes::CheckboxWidget: {WidgetCounts.CheckboxWidgetCount--;  break; }
		case WidgetTypes::ComboWidget: {WidgetCounts.ComboWidgetCount--;  break; }
		case WidgetTypes::PopupWidget: {WidgetCounts.PopupWidgetCount--;  break; }
		}

		DecrementIterators(static_cast<int16_t>(widgetLocation));

		Widgets.erase(Widgets.begin() + widgetLocation);
	}

	void UIEngine::TextWidget::PushRenderData(glm::vec3 windowTranslation, const glm::vec3& windowSize, float viewportWidth)
	{
		glm::vec3 widgetSize = glm::vec3(windowSize.x * Size.x, windowSize.y * Size.y, 1.0f);
		glm::vec3 widgetTranslation = glm::vec3(windowTranslation.x + (windowSize.x * WindowPosition.x),
							windowTranslation.y + (windowSize.y * WindowPosition.y),
							windowTranslation.z);
		// Draw Widget Background
		s_BackgroundInputSpec.TransformMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(widgetTranslation.x + (widgetSize.x / 2), widgetTranslation.y + (widgetSize.y / 2), widgetTranslation.z))
			* glm::scale(glm::mat4(1.0f), widgetSize);

		Shader::SetDataAtInputLocation<glm::vec4>(BackgroundColor, "a_Color", s_BackgroundInputSpec.Buffer, s_BackgroundInputSpec.Shader);
		Renderer::SubmitDataToRenderer(s_BackgroundInputSpec);

		widgetTranslation.z += 0.001f;

		// Render Text
		glm::vec2 resolution = Utility::ScreenResolutionToAspectRatio(Project::GetTargetResolution());
		float textSize = (viewportWidth * 0.15f * TextSize) * (resolution.y / resolution.x);
		textSize = textSize / s_Engine.m_CurrentFont->m_AverageWidth;
		if (TextCentered)
		{
			widgetTranslation = glm::vec3(widgetTranslation.x + (widgetSize.x * 0.5f) - ((TextAbsoluteDimensions.x * 0.5f) * textSize), widgetTranslation.y + (widgetSize.y * 0.5f) - ((TextAbsoluteDimensions.y * 0.5f) * textSize), widgetTranslation.z);
		}
		//KG_CORE_ERROR("The Pre Text Value is {}", )
		s_Engine.m_CurrentFont->PushTextData(Text, widgetTranslation, textSize, {0.8f, 0.3f, 0.2f});
	}

	void UIEngine::TextWidget::CalculateTextSize()
	{
		TextAbsoluteDimensions = s_Engine.m_CurrentFont->CalculateTextSize(Text);
	}

	void UIEngine::TextWidget::SetText(const std::string& newText)
	{
		Text = newText;
		CalculateTextSize();
	}

}

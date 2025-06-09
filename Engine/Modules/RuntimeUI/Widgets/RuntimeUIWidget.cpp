#include "kgpch.h"

#include "Modules/RuntimeUI/Widgets/RuntimeUIWidget.h"
#include "Modules/RuntimeUI/RuntimeUIContext.h"
#include "Modules/Rendering/RenderingService.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Modules/ECS/EngineComponents.h"
#include "Modules/Assets/AssetService.h"
#include "Modules/EditorUI/EditorUI.h"

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
	void Widget::RenderBackground(RuntimeUIContext* uiContext, const Math::vec4& color, const Math::vec3& translation, const Math::vec3 size)
	{
		Rendering::RendererInputSpec& renderSpec = uiContext->m_BackgroundInputSpec;

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
	Math::vec3 Widget::CalculateWidgetSize(const Math::vec3& windowSize)
	{
		return Math::vec3
		(
			m_SizeType == PixelOrPercent::Percent ? windowSize.x * m_PercentSize.x : m_PixelSize.x,
			m_SizeType == PixelOrPercent::Percent ? windowSize.y * m_PercentSize.y : m_PixelSize.y,
			1.0f
		);
	}

	Math::vec3 Widget::CalculateWorldPosition(const Math::vec3& windowTranslation, const Math::vec3& windowSize)
	{
		float widgetXPos{ m_XPositionType == PixelOrPercent::Percent ? windowSize.x * m_PercentPosition.x : (float)m_PixelPosition.x };
		float widgetYPos{ m_YPositionType == PixelOrPercent::Percent ? windowSize.y * m_PercentPosition.y : (float)m_PixelPosition.y };
		Math::vec3 widgetSize = CalculateWidgetSize(windowSize);
		if (m_XRelativeOrAbsolute == RelativeOrAbsolute::Relative && m_XConstraint != Constraint::None)
		{
			// Handle relative code
			switch (m_XConstraint)
			{
			case Constraint::Center:
				widgetXPos = (windowSize.x * 0.5f) + widgetXPos - (widgetSize.x / 2.0f);
				break;
			case Constraint::Right:
				widgetXPos = windowSize.x + widgetXPos - (widgetSize.x);
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
				widgetYPos = (windowSize.y * 0.5f) + widgetYPos - (widgetSize.y / 2.0f);;
				break;
			case Constraint::Top:
				widgetYPos = windowSize.y + widgetYPos - widgetSize.y;
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
		Math::vec3 widgetSize = CalculateWidgetSize(windowSize);

		if (m_XRelativeOrAbsolute == RelativeOrAbsolute::Relative && m_XConstraint != Constraint::None)
		{
			// Handle relative code
			switch (m_XConstraint)
			{
			case Constraint::Center:
				widgetXPos = widgetXPos - (windowSize.x * 0.5f) + (widgetSize.x / 2.0f);
				break;
			case Constraint::Right:
				widgetXPos = widgetXPos - windowSize.x + (widgetSize.x);
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
				widgetYPos = widgetYPos - (windowSize.y * 0.5f) + (widgetSize.y / 2.0f);
				break;
			case Constraint::Top:
				widgetYPos = widgetYPos - windowSize.y + (widgetSize.y);
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
	void Widget::RevalidateImageSize(ViewportData viewportData, bool useXValueAsBase)
	{
		Bounds parentTransform = i_ParentUI->m_WindowsState.GetParentBoundsFromID(m_ID, viewportData);

		// Get the image data from the provided widget
		ImageData* currentImageData = GetImageData();
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
			m_PixelSize.y = (int)((float)m_PixelSize.x * textureAspectRatio.y);

			// And for the percentage dimensions
			m_PercentSize.y = ((parentTransform.m_Size.x * m_PercentSize.x) * textureAspectRatio.y) / parentTransform.m_Size.y;
		}
		else
		{
			// Normalize the aspect ratio based on the y-value
			textureAspectRatio.x = textureAspectRatio.x / textureAspectRatio.y;
			textureAspectRatio.y = 1.0f;

			// Use the normalized y-value ratio to calculate the new y-value...

			// For the pixel dimensions
			m_PixelSize.x = (int)((float)m_PixelSize.y * textureAspectRatio.x);

			// And for the percentage dimensions
			m_PercentSize.x = ((parentTransform.m_Size.y * m_PercentSize.y) * textureAspectRatio.x) / parentTransform.m_Size.x;
		}
	}
	void MultiLineTextData::RevalidateTextDimensions(UserInterface* parentUI, const Math::vec3& widgetSize, float textSize)
	{
		// Calculate the text size of the widget using the active user interface font
		if (m_TextWrapped)
		{
			parentUI->m_Config.m_Font->GetMultiLineTextMetadata(m_Text, m_CachedTextDimensions, textSize, (int)widgetSize.x);
		}
		else
		{
			parentUI->m_Config.m_Font->GetMultiLineTextMetadata(m_Text, m_CachedTextDimensions, textSize);
		}
	}
	void SingleLineTextData::OnRender(RuntimeUIContext* uiContext, const Math::vec3& textStartingPoint, float textScalingFactor)
	{
		Ref<UserInterface> activeUI = uiContext->m_ActiveUI;

		// Call the text's rendering function
		activeUI->m_Config.m_Font->OnRenderSingleLineText(m_Text, textStartingPoint, m_TextColor, textScalingFactor);
	}
	void SingleLineTextData::RenderTextCursor(RuntimeUIContext* uiContext, const Math::vec3& textStartingPoint, float textScalingFactor)
	{
		Ref<UserInterface> activeUI = uiContext->m_ActiveUI;

		Rendering::RendererInputSpec& renderSpec = uiContext->m_BackgroundInputSpec;

		// Start the cursor's translation at the text's origin
		Math::vec3 cursorTranslation{ textStartingPoint };

		// Get the cursor offset relative to the text's starting point
		Math::vec2 cursorOffset;
		float ascender = activeUI->m_Config.m_Font->m_Ascender;
		if (m_Text.size() == 0 || m_CachedTextDimensions.y < 0.001f)
		{
			cursorOffset = m_CachedTextDimensions;
		}
		else
		{
			cursorOffset = GetTextDimensions(uiContext->m_ActiveUI.get(), std::string_view(m_Text.data(), m_CaretIndex));
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
	void SingleLineTextData::SetText(UserInterface* parentUI, std::string_view text)
	{
		m_Text = text;
		RevalidateTextDimensions(parentUI);
	}
	void SingleLineTextData::RevalidateTextDimensions(UserInterface* parentUI)
	{
		// Calculate the text size of the widget using the active user interface font
		m_CachedTextDimensions = parentUI->m_Config.m_Font->GetSingleLineTextSize(m_Text);
	}
	Math::vec2 SingleLineTextData::GetTextDimensions(UserInterface* parentUI, std::string_view text) const
	{
		// Calculate the text size of the widget using the active user interface font
		return parentUI->m_Config.m_Font->GetSingleLineTextSize(text);
	}
	size_t SingleLineTextData::GetCursorIndexFromMousePosition(Ref<Font> font, float textStartingPosition, float mouseXPosition, float textScalingFactor)
	{
		return font->GetIndexFromMousePosition(m_Text, textStartingPosition, mouseXPosition, textScalingFactor);
	}
	Math::vec3 SingleLineTextData::GetTextStartingPosition(const Math::vec3& translation, const Math::vec3 size, float textScalingFactor)
	{
		Math::vec3 translationOutput = translation;
		constexpr float k_CenterAdjustmentSize{ 2.6f }; // Magic number for adjusting the height of a line TODO: Find better solution

		// Place the starting x-location of the text widget based on the provided alignment option
		switch (m_TextAlignment)
		{
		case Constraint::Left:
			break;
		case Constraint::Right:
			translationOutput.x = translation.x + (size.x) - ((m_CachedTextDimensions.x) * textScalingFactor);
			break;
		case Constraint::Center:
			// Adjust current line translation to be centered
			translationOutput.x = translation.x + (size.x * 0.5f) - ((m_CachedTextDimensions.x * 0.5f) * textScalingFactor);
			break;
		case Constraint::Bottom:
		case Constraint::Top:
		case Constraint::None:
			KG_ERROR("Invalid constraint type for aligning text {}", Utility::ConstraintToString(m_TextAlignment));
			break;
		}

		// Set the starting y/z locations
		translationOutput.y = translation.y + (size.y * 0.5f) - ((m_CachedTextDimensions.y * 0.5f) * textScalingFactor) + k_CenterAdjustmentSize;

		// Return starting point
		return translationOutput;
	}
	void ImageData::RenderImage(RuntimeUIContext* uiContext, const Math::vec3& translation, const Math::vec3 size)
	{
		Rendering::RendererInputSpec& renderSpec = uiContext->m_ImageInputSpec;

		if (m_ImageRef)
		{
			// Create the widget's background rendering data
			renderSpec.m_TransformMatrix = glm::translate(Math::mat4(1.0f), Math::vec3(translation.x + (size.x / 2), translation.y + (size.y / 2), translation.z))
				* glm::scale(Math::mat4(1.0f), size);


			renderSpec.m_Texture = m_ImageRef;
			renderSpec.m_ShapeComponent->Texture = m_ImageRef;

			// Submit background data to GPU
			Rendering::RenderingService::SubmitDataToRenderer(renderSpec);
		}
	}
	void ContainerData::AddWidget(Ref<Widget> newWidget)
	{
		// Add the parent window
		m_ContainedWidgets.push_back(newWidget);

		// Ensure ID -> Location map is valid
		newWidget->i_ParentUI->m_WindowsState.RevalidateIDToLocationMap();

		// Ensure the new widget is validated
		newWidget->RevalidateTextDimensions();
	}
	void ContainerData::RevalidateTextDimensions()
	{
		for (Ref<Widget> widget : m_ContainedWidgets)
		{
			widget->RevalidateTextDimensions();
		}
	}

	
}
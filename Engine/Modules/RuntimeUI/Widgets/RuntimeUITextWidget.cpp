#include "kgpch.h"

#include "Modules/RuntimeUI/Widgets/RuntimeUITextWidget.h"
#include "Modules/RuntimeUI/RuntimeUI.h"

#include "Kargono/Core/Resolution.h"
#include "Kargono/Projects/Project.h"
#include "Modules/Core/Engine.h"

namespace Kargono::RuntimeUI
{
	void TextWidget::OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth)
	{
		KG_PROFILE_FUNCTION();

		RuntimeUIContext& uiContext{ RuntimeUIService::GetActiveContext() };

		// Calculate the widget's rendering data
		Math::vec3 widgetSize = CalculateWidgetSize(windowSize);

		// Get widget translation
		Math::vec3 widgetTranslation = CalculateWorldPosition(windowTranslation, windowSize);

		// Create the widget's text rendering data
		widgetTranslation.z += 0.001f;
		Math::vec2 resolution = Utility::ScreenResolutionToAspectRatio(Projects::ProjectService::GetActiveContext().GetTargetResolution());
		float textSize{ (viewportWidth * 0.15f * m_TextData.m_TextSize) * (resolution.y / resolution.x) };
		float yAdvance = uiContext.m_ActiveUI->m_Font->m_LineHeight;
		float allLineAdvanceHeight{ 0.0f };
		// Calculate entire text's height
		if (m_TextData.m_CachedTextDimensions.m_LineSize.size() > 0)
		{
			//// Add the first line's dimensions
			//allLineAdvanceHeight += m_TextMetadata.m_LineSize[0].y;

			// Add the line advance for the remaining lines
			allLineAdvanceHeight += yAdvance * (float)(m_TextData.m_CachedTextDimensions.m_LineSize.size() - 1);
		}

		// Call the text's rendering function
		for (size_t iteration{ 0 }; iteration < m_TextData.m_CachedTextDimensions.m_LineSize.size(); iteration++)
		{

			Math::vec3 finalTranslation;
			Math::vec2 lineDimensions{ m_TextData.m_CachedTextDimensions.m_LineSize[iteration] };
			Math::ivec2 currentBreaks{ m_TextData.m_CachedTextDimensions.m_LineBreaks[iteration] };

			constexpr float k_CenterAdjustmentSize{ 2.6f }; // Magic number for adjusting the height of a line TODO: Find better solution

			// Place the starting x-location of the text widget based on the provided alignment option
			switch (m_TextData.m_TextAlignment)
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
				KG_ERROR("Invalid constraint type for aligning text {}", Utility::ConstraintToString(m_TextData.m_TextAlignment));
				break;
			}

			// Set the starting y/z locations
			finalTranslation.y = widgetTranslation.y + (widgetSize.y * 0.5f) - ((m_TextData.m_CachedTextDimensions.m_LineSize[0].y * 0.5f - (allLineAdvanceHeight * 0.5f)) * textSize) + k_CenterAdjustmentSize;
			finalTranslation.z = widgetTranslation.z;

			// Move the line down in the y-axis to it's correct location
			finalTranslation.y -= iteration * textSize * yAdvance;

			// Render the single line of text
			std::string_view outputText{ m_TextData.m_Text.data() + currentBreaks.x, (size_t)(currentBreaks.y - currentBreaks.x) };
			uiContext.m_ActiveUI->m_Font->OnRenderSingleLineText(
				outputText,
				finalTranslation, m_TextData.m_TextColor, textSize);
		}

	}

	void TextWidget::CalculateTextSize()
	{
		RuntimeUIContext& uiContext{ RuntimeUIService::GetActiveContext() };

		// Get the resolution of the screen and the viewport
		Math::vec2 resolution = Utility::ScreenResolutionToAspectRatio(Projects::ProjectService::GetActiveContext().GetTargetResolution());
		ViewportData& viewportData = EngineService::GetActiveEngine().GetWindow().GetActiveViewport();

		// Calculate the text size used by the rendering calls
		float textSize{ (viewportData.m_Width * 0.15f * m_TextData.m_TextSize) * (resolution.y / resolution.x) };

		// Get parent transform
		BoundingBoxTransform parentTransform = uiContext.GetParentDimensionsFromID(m_ID, viewportData.m_Width, viewportData.m_Height);

		// Get widget width
		Math::vec3 widgetSize = CalculateWidgetSize(parentTransform.m_Size);
		uiContext.CalculateMultiLineText(m_TextData, widgetSize, textSize);
	}

	void TextWidget::SetText(const std::string& newText)
	{
		// Set the text of the widget
		m_TextData.m_Text = newText;

		// Calculate the new text size
		CalculateTextSize();
	}
}
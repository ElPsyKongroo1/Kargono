#include "kgpch.h"

#include "Modules/RuntimeUI/Widgets/RuntimeUIImageButtonWidget.h"
#include "Modules/RuntimeUI/RuntimeUI.h"

#include "Kargono/Core/Resolution.h"
#include "Kargono/Projects/Project.h"

namespace Kargono::RuntimeUI
{
	void InputTextWidget::SetText(const std::string& newText)
	{
		// Set the text of the widget
		m_TextData.m_Text = newText;

		// Calculate the new text size
		CalculateTextSize();
	}

	void InputTextWidget::CalculateTextSize()
	{
		RuntimeUIService::GetActiveContext().CalculateSingleLineText(m_TextData);
	}

	void InputTextWidget::OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth)
	{
		KG_PROFILE_FUNCTION();

		RuntimeUIContext& uiContext{ RuntimeUIService::GetActiveContext() };
		Ref<UserInterface> activeUI = uiContext.m_ActiveUI;

		// Calculate the widget's rendering data
		Math::vec3 widgetSize = CalculateWidgetSize(windowSize);

		// Get widget translation
		Math::vec3 widgetTranslation = CalculateWorldPosition(windowTranslation, windowSize);

		// Draw background
		if (activeUI->m_EditingWidget == this)
		{
			uiContext.RenderBackground(activeUI->m_EditingColor, widgetTranslation, widgetSize);
		}
		else if (activeUI->m_HoveredWidget == this)
		{
			uiContext.RenderBackground(activeUI->m_HoveredColor, widgetTranslation, widgetSize);
		}
		else if (activeUI->m_SelectedWidget == this)
		{
			uiContext.RenderBackground(activeUI->m_SelectColor, widgetTranslation, widgetSize);
		}
		else
		{
			uiContext.RenderBackground(m_SelectionData.m_DefaultBackgroundColor, widgetTranslation, widgetSize);
		}

		// Calculate text starting point
		Math::vec2 resolution = Utility::ScreenResolutionToAspectRatio(Projects::ProjectService::GetActiveContext().GetTargetResolution());
		float textScalingFactor{ (viewportWidth * 0.15f * m_TextData.m_TextSize) * (resolution.y / resolution.x) };
		Math::vec3 textStartingPoint = uiContext.GetSingleLineTextStartingPosition(m_TextData, widgetTranslation, widgetSize, textScalingFactor);

		// Render the widget's text
		textStartingPoint.z += 0.001f;
		uiContext.RenderSingleLineText(m_TextData, textStartingPoint, textScalingFactor);

		// Render the IBeam icon/cursor if necessary
		textStartingPoint.z += 0.001f;
		if (uiContext.m_ActiveUI->m_EditingWidget == this &&
			uiContext.m_ActiveUI->m_IBeamVisible)
		{
			uiContext.RenderTextCursor(m_TextData, textStartingPoint, textScalingFactor);
		}

	}
}
#include "kgpch.h"

#include "Modules/RuntimeUI/Widgets/RuntimeUIImageButtonWidget.h"
#include "Modules/RuntimeUI/RuntimeUIContext.h"

#include "Kargono/Core/Resolution.h"
#include "Kargono/Projects/Project.h"

namespace Kargono::RuntimeUI
{
	void InputTextWidget::SetText(const std::string& newText)
	{
		// Set the text of the widget
		m_TextData.m_Text = newText;

		// Calculate the new text size
		RevalidateTextDimensions();
	}

	void InputTextWidget::RevalidateTextDimensions()
	{
		m_TextData.RevalidateTextDimensions(i_ParentUI);
	}

	void InputTextWidget::OnRender(RuntimeUIContext* uiContext, Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth)
	{
		KG_PROFILE_FUNCTION();

		Ref<UserInterface> activeUI = uiContext->m_ActiveUI;

		// Calculate the widget's rendering data
		Math::vec3 widgetSize = CalculateWidgetSize(windowSize);

		// Get widget translation
		Math::vec3 widgetTranslation = CalculateWorldPosition(windowTranslation, windowSize);

		// Draw background
		if (activeUI->m_InteractState.m_EditingWidget == this)
		{
			RenderBackground(uiContext, activeUI->m_Config.m_EditingColor, widgetTranslation, widgetSize);
		}
		else if (activeUI->m_InteractState.m_HoveredWidget == this)
		{
			RenderBackground(uiContext, activeUI->m_Config.m_HoveredColor, widgetTranslation, widgetSize);
		}
		else if (activeUI->m_InteractState.m_SelectedWidget == this)
		{
			RenderBackground(uiContext, activeUI->m_Config.m_SelectColor, widgetTranslation, widgetSize);
		}
		else
		{
			RenderBackground(uiContext, m_SelectionData.m_DefaultBackgroundColor, widgetTranslation, widgetSize);
		}

		// Calculate text starting point
		Math::vec2 resolution = Utility::ScreenResolutionToAspectRatio(Projects::ProjectService::GetActiveContext().GetTargetResolution());
		float textScalingFactor{ (viewportWidth * 0.15f * m_TextData.m_TextSize) * (resolution.y / resolution.x) };
		Math::vec3 textStartingPoint = m_TextData.GetTextStartingPosition(widgetTranslation, widgetSize, textScalingFactor);

		// Render the widget's text
		textStartingPoint.z += 0.001f;
		m_TextData.OnRender(uiContext, textStartingPoint, textScalingFactor);

		// Render the IBeam icon/cursor if necessary
		textStartingPoint.z += 0.001f;
		if (uiContext->m_ActiveUI->m_InteractState.m_EditingWidget == this &&
			uiContext->m_ActiveUI->m_CaretState.m_CaretVisible)
		{
			m_TextData.RenderTextCursor(uiContext, textStartingPoint, textScalingFactor);
		}
	}
}
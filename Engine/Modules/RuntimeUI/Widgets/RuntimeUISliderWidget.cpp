#include "kgpch.h"

#include "Modules/RuntimeUI/Widgets/RuntimeUISliderWidget.h"
#include "Modules/RuntimeUI/RuntimeUI.h"

namespace Kargono::RuntimeUI
{
	void SliderWidget::OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth)
	{
		UNREFERENCED_PARAMETER(viewportWidth);
		KG_PROFILE_FUNCTION();

		RuntimeUIContext& uiContext{ RuntimeUIService::GetActiveContext() };
		Ref<UserInterface> activeUI = uiContext.m_ActiveUI;

		// Calculate the widget's rendering data
		Math::vec3 widgetSize = CalculateWidgetSize(windowSize);

		// Get widget translation
		Math::vec3 widgetTranslation = CalculateWorldPosition(windowTranslation, windowSize);

		// Render the slider
		uiContext.RenderSliderLine(m_LineColor, widgetTranslation, widgetSize);

		widgetTranslation.z += 0.001f;

		// Get the slider's current normalized location based on the bounds and currentValue
		float normalizedSliderLocation = (m_CurrentValue - m_Bounds.x) / (m_Bounds.y - m_Bounds.x);

		if (activeUI->m_SelectedWidget == this)
		{
			uiContext.RenderSlider(activeUI->m_SelectColor,
				{ widgetTranslation.x + widgetSize.x * normalizedSliderLocation, widgetTranslation.y, widgetTranslation.z },
				widgetSize);
		}
		else
		{
			uiContext.RenderSlider(m_SliderColor,
				{ widgetTranslation.x + widgetSize.x * normalizedSliderLocation, widgetTranslation.y, widgetTranslation.z },
				widgetSize);
		}
	}
}
#include "kgpch.h"

#include "Modules/RuntimeUI/Widgets/RuntimeUIImageButtonWidget.h"
#include "Modules/RuntimeUI/RuntimeUIContext.h"

namespace Kargono::RuntimeUI
{
	void ImageButtonWidget::OnRender(RuntimeUIContext* uiContext, Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth)
	{
		UNREFERENCED_PARAMETER(viewportWidth);
		KG_PROFILE_FUNCTION();

		Ref<UserInterface> activeUI = uiContext->m_ActiveUI;

		// Calculate the widget's rendering data
		Math::vec3 widgetSize = CalculateWidgetSize(windowSize);

		// Get widget translation
		Math::vec3 widgetTranslation = CalculateWorldPosition(windowTranslation, windowSize);

		// Draw background
		if (activeUI->m_InteractState.m_HoveredWidget == this)
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

		widgetTranslation.z += 0.001f;

		// Draw image
		m_ImageData.RenderImage(uiContext, widgetTranslation, widgetSize);
	}
}
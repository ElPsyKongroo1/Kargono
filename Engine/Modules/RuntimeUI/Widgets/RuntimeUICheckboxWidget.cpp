#include "kgpch.h"

#include "Modules/RuntimeUI/Widgets/RuntimeUICheckboxWidget.h"
#include "Modules/RuntimeUI/RuntimeUIContext.h"

namespace Kargono::RuntimeUI
{
	void CheckboxWidget::OnRender(RuntimeUIContext* uiContext, Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth)
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

		if (m_Checked)
		{
			m_ImageChecked.RenderImage(uiContext, widgetTranslation, widgetSize);
		}
		else
		{
			m_ImageUnChecked.RenderImage(uiContext, widgetTranslation, widgetSize);
		}
	}
	ImageData* CheckboxWidget::GetImageData()
	{
		// Return the currently appropriate image data
		if (m_Checked)
		{
			return &m_ImageChecked;
		}
		else
		{
			return &m_ImageUnChecked;
		}
	}
}
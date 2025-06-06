#include "kgpch.h"

#include "Modules/RuntimeUI/Widgets/RuntimeUICheckboxWidget.h"
#include "Modules/RuntimeUI/RuntimeUI.h"

namespace Kargono::RuntimeUI
{
	void CheckboxWidget::OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth)
	{
		UNREFERENCED_PARAMETER(viewportWidth);
		KG_PROFILE_FUNCTION();

		RuntimeUIContext& uiContext{ RuntimeUIService::GetActiveContext()};
		Ref<UserInterface> activeUI = uiContext.m_ActiveUI;

		// Calculate the widget's rendering data
		Math::vec3 widgetSize = CalculateWidgetSize(windowSize);

		// Get widget translation
		Math::vec3 widgetTranslation = CalculateWorldPosition(windowTranslation, windowSize);

		// Draw background
		if (activeUI->m_HoveredWidget == this)
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

		widgetTranslation.z += 0.001f;

		if (m_Checked)
		{
			uiContext.RenderImage(m_ImageChecked, widgetTranslation, widgetSize);
		}
		else
		{
			uiContext.RenderImage(m_ImageUnChecked, widgetTranslation, widgetSize);
		}
	}
}
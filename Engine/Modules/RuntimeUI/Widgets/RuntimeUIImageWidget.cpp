#include "kgpch.h"

#include "Modules/RuntimeUI/Widgets/RuntimeUIImageWidget.h"
#include "Modules/RuntimeUI/RuntimeUIContext.h"

namespace Kargono::RuntimeUI
{
	void ImageWidget::OnRender(RuntimeUIContext* uiContext, Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth)
	{
		UNREFERENCED_PARAMETER(viewportWidth);
		KG_PROFILE_FUNCTION();

		// Calculate the widget's rendering data
		Math::vec3 widgetSize = CalculateWidgetSize(windowSize);

		// Get widget translation
		Math::vec3 widgetTranslation = CalculateWorldPosition(windowTranslation, windowSize);

		// Draw image
		m_ImageData.RenderImage(uiContext, widgetTranslation, widgetSize);
	}
}
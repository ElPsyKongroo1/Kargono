#include "kgpch.h"

#include "Modules/RuntimeUI/Widgets/RuntimeUIContainerWidget.h"
#include "Modules/RuntimeUI/RuntimeUIContext.h"

namespace Kargono::RuntimeUI
{
	void ContainerWidget::OnRender(RuntimeUIContext* uiContext, Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth)
	{
		Rendering::RendererInputSpec& backgroundSpec = uiContext->m_BackgroundInputSpec;
		Rendering::RendererInputSpec& imageSpec = uiContext->m_ImageInputSpec;

		// Calculate the widget's rendering data
		Math::vec3 widgetSize = CalculateWidgetSize(windowSize);
		// Get widget translation
		Math::vec3 widgetTranslation = CalculateWorldPosition(windowTranslation, windowSize);
		// Draw the background
		RenderBackground(uiContext, m_ContainerData.m_BackgroundColor, widgetTranslation, widgetSize);

		widgetTranslation.z += 0.001f;

		// NOTE: This code needs to be at the end of this function!
		// Updating the render input locations causes further render calls to
		// associate its mouse picking with an incorrect widget
		// Render the child widgets
		for (Ref<Widget> containedWidget : m_ContainerData.m_ContainedWidgets)
		{
			// Push widget ID
			Rendering::Shader::SetDataAtInputLocation<int32_t>(containedWidget->m_ID,
				Utility::FileSystem::CRCFromString("a_EntityID"),
				backgroundSpec.m_Buffer, backgroundSpec.m_Shader);
			Rendering::Shader::SetDataAtInputLocation<int32_t>(containedWidget->m_ID,
				Utility::FileSystem::CRCFromString("a_EntityID"),
				imageSpec.m_Buffer, imageSpec.m_Shader);
			RuntimeUI::FontService::GetActiveContext().SetID((uint32_t)containedWidget->m_ID);

			// Render the indicated widget
			containedWidget->OnRender(uiContext, widgetTranslation, widgetSize, viewportWidth);
		}
	}
}
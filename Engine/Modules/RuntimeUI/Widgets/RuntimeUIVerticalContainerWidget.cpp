#include "kgpch.h"

#include "Modules/RuntimeUI/Widgets/RuntimeUIVerticalContainerWidget.h"
#include "Modules/RuntimeUI/RuntimeUI.h"

namespace Kargono::RuntimeUI
{
	void VerticalContainerWidget::OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth)
	{
		RuntimeUIContext& uiContext{ RuntimeUIService::GetActiveContext() };

		Rendering::RendererInputSpec& backgroundSpec = uiContext.m_BackgroundInputSpec;

		// Calculate the widget's rendering data
		Math::vec3 widgetSize = CalculateWidgetSize(windowSize);
		// Get widget translation
		Math::vec3 widgetTranslation = CalculateWorldPosition(windowTranslation, windowSize);
		// Draw the background
		uiContext.RenderBackground(m_ContainerData.m_BackgroundColor, widgetTranslation, widgetSize);

		widgetTranslation.z += 0.001f;

		// NOTE: This code needs to be at the end of this function!
		// Updating the render input locations causes further render calls to
		// associate its mouse picking with an incorrect widget
		// Render the child widgets
		size_t iteration{ 0 };
		for (Ref<Widget> containedWidget : m_ContainerData.m_ContainedWidgets)
		{
			// Push widget ID
			Rendering::Shader::SetDataAtInputLocation<int32_t>(containedWidget->m_ID,
				Utility::FileSystem::CRCFromString("a_EntityID"),
				backgroundSpec.m_Buffer, backgroundSpec.m_Shader);
			RuntimeUI::FontService::SetID((uint32_t)containedWidget->m_ID);

			Math::vec3 outputSize{ widgetSize.x, widgetSize.y * m_RowHeight, widgetSize.z };
			Math::vec3 outputTranslation
			{
				widgetTranslation.x,
				widgetTranslation.y + widgetSize.y - outputSize.y * (iteration + 1) - widgetSize.y * m_RowSpacing * iteration,
				widgetTranslation.z };

			// Render the indicated widget
			//containedWidget->OnRender(widgetTranslation, widgetSize, viewportWidth);
			containedWidget->OnRender(outputTranslation, outputSize, viewportWidth);
			iteration++;
		}
	}
}
#include "kgpch.h"

#include "Modules/RuntimeUI/Widgets/RuntimeUIHorizontalContainerWidget.h"
#include "Modules/RuntimeUI/RuntimeUI.h"

namespace Kargono::RuntimeUI
{
	void HorizontalContainerWidget::OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth)
	{
		RuntimeUIContext& uiContext{ RuntimeUIService::GetActiveContext() };

		Rendering::RendererInputSpec& backgroundSpec = uiContext.m_BackgroundInputSpec;
		Rendering::RendererInputSpec& imageSpec = uiContext.m_ImageInputSpec;

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
			Rendering::Shader::SetDataAtInputLocation<int32_t>(containedWidget->m_ID,
				Utility::FileSystem::CRCFromString("a_EntityID"),
				imageSpec.m_Buffer, imageSpec.m_Shader);

			Math::vec3 outputSize{ widgetSize.x * m_ColumnWidth, widgetSize.y, widgetSize.z };
			Math::vec3 outputTranslation
			{
				widgetTranslation.x + outputSize.x * iteration + widgetSize.x * m_ColumnSpacing * iteration,
				widgetTranslation.y,
				widgetTranslation.z };

			// Render the indicated widget
			//containedWidget->OnRender(widgetTranslation, widgetSize, viewportWidth);
			containedWidget->OnRender(outputTranslation, outputSize, viewportWidth);
			iteration++;
		}
	}
}
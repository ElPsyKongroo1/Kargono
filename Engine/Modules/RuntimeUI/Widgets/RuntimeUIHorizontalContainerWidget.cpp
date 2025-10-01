#include "kgpch.h"

#include "Modules/RuntimeUI/Widgets/RuntimeUIHorizontalContainerWidget.h"
#include "Modules/RuntimeUI/RuntimeUIContext.h"

namespace Kargono::RuntimeUI
{
	void HorizontalContainerWidget::OnRender(RuntimeUIContext* uiContext, Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth)
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
		size_t iteration{ 0 };
		for (Ref<Widget> containedWidget : m_ContainerData.m_ContainedWidgets)
		{
			// Push widget ID
			Rendering::Shader::SetDataAtInputLocation<int32_t>(containedWidget->m_ID,
				Utility::FileSystem::CRCFromString("a_EntityID"),
				backgroundSpec.m_Buffer, backgroundSpec.m_Shader);
			RuntimeUI::FontService::GetActiveContext().SetID((uint32_t)containedWidget->m_ID);
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
			containedWidget->OnRender(uiContext, outputTranslation, outputSize, viewportWidth);
			iteration++;
		}
	}

	void HorizontalContainerWidget::Serialize(YAML::Emitter& emitter, UserInterface* parentUI)
	{
		emitter << YAML::BeginMap; // Begin Widget Map

		// Call base serialization
		Widget::Serialize(emitter, parentUI);

		emitter << YAML::Key << "HorizontalContainerWidget" << YAML::Value;
		// Container fields
		emitter << YAML::BeginMap; // Begin Container Map

		// Save unique fields
		emitter << YAML::Key << "ColumnWidth" << YAML::Value << m_ColumnWidth;
		emitter << YAML::Key << "ColumnSpacing" << YAML::Value << m_ColumnSpacing;

		// Save container data
		m_ContainerData.Serialize(emitter);
		emitter << YAML::EndMap; // End Container Map

		emitter << YAML::EndMap; // End Widget Map
	}
	void HorizontalContainerWidget::Deserialize(const YAML::Node& node, UserInterface* parentUI)
	{
		// Call base deserialization
		Widget::Deserialize(node, parentUI);

		YAML::Node specificWidget = node["HorizontalContainerWidget"];
		m_WidgetType = RuntimeUI::WidgetTypes::HorizontalContainerWidget;
		// Get unique data
		m_ColumnWidth = specificWidget["ColumnWidth"].as<float>();
		m_ColumnSpacing = specificWidget["ColumnSpacing"].as<float>();
		// Get container data
		m_ContainerData.Deserialize(specificWidget, parentUI);
	}
}
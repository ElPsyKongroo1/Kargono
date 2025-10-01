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

	void ImageWidget::Serialize(YAML::Emitter& emitter, UserInterface* parentUI)
	{
		emitter << YAML::BeginMap; // Begin Widget Map

		// Call base serialization
		Widget::Serialize(emitter, parentUI);

		emitter << YAML::Key << "ImageWidget" << YAML::Value;
		// Image field
		emitter << YAML::BeginMap; // Begin ImageWidget Map
		// Save image data
		m_ImageData.Serialize(emitter, "");
		emitter << YAML::EndMap; // End ImageWidget Map

		emitter << YAML::EndMap; // End Widget Map
	}
	void ImageWidget::Deserialize(const YAML::Node& node, UserInterface* parentUI)
	{
		// Call base deserialization
		Widget::Deserialize(node, parentUI);

		YAML::Node specificWidget = node["ImageWidget"];
		m_WidgetType = RuntimeUI::WidgetTypes::ImageWidget;
		// Get image data
		m_ImageData.Deserialize(specificWidget, "");
	}
}
#include "kgpch.h"

#include "Modules/RuntimeUI/Widgets/RuntimeUIImageButtonWidget.h"
#include "Modules/RuntimeUI/RuntimeUIContext.h"

namespace Kargono::RuntimeUI
{
	void ImageButtonWidget::OnRender(RuntimeUIContext* uiContext, Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth)
	{
		UNREFERENCED_PARAMETER(viewportWidth);
		KG_PROFILE_FUNCTION();

		Assets::AssetRef<UserInterface> activeUI = uiContext->m_ActiveUI.GetAssetRef();

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

	void ImageButtonWidget::Serialize(YAML::Emitter& emitter, UserInterface* parentUI)
	{
		emitter << YAML::BeginMap; // Begin Widget Map

		// Call base serialization
		Widget::Serialize(emitter, parentUI);

		emitter << YAML::Key << "ImageButtonWidget" << YAML::Value;
		// Image field
		emitter << YAML::BeginMap; // Begin ImageWidget Map
		// Save image data
		m_ImageData.Serialize(emitter, "");
		// Save selection fields
		m_SelectionData.Serialize(emitter);
		emitter << YAML::EndMap; // End ImageWidget Map

		emitter << YAML::EndMap; // End Widget Map
	}
	void ImageButtonWidget::Deserialize(const YAML::Node& node, UserInterface* parentUI)
	{
		// Call base deserialization
		Widget::Deserialize(node, parentUI);

		YAML::Node specificWidget = node["ImageButtonWidget"];
		m_WidgetType = RuntimeUI::WidgetTypes::ImageButtonWidget;
		// Get selection data
		m_SelectionData.Deserialize(specificWidget);
		// Get image data
		m_ImageData.Deserialize(specificWidget, "");
	}
}
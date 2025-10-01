#include "kgpch.h"

#include "Modules/RuntimeUI/Widgets/RuntimeUICheckboxWidget.h"
#include "Modules/RuntimeUI/RuntimeUIContext.h"

namespace Kargono::RuntimeUI
{
	void CheckboxWidget::Serialize(YAML::Emitter& emitter, UserInterface* parentUI)
	{
		// Call base serialization
		Widget::Serialize(emitter, parentUI);

		emitter << YAML::Key << "CheckboxWidget" << YAML::Value;
		// Image fields
		emitter << YAML::BeginMap; // Begin Checkbox Map
		// Save checked
		emitter << YAML::Key << "Checked" << YAML::Value << m_Checked;
		// Save image data
		m_ImageChecked.Serialize(emitter, "Checked");
		// Save image data
		m_ImageUnChecked.Serialize(emitter, "UnChecked");
		// Save selection fields
		m_SelectionData.Serialize(emitter);
		emitter << YAML::EndMap; // End Checkbox Map
	}

	void CheckboxWidget::Deserialize(YAML::Node& node, UserInterface* parentUI)
	{
		// Call base deserialization
		Widget::Deserialize(node, parentUI);

		
	}

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
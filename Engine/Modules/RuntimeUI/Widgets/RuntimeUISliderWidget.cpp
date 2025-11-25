#include "kgpch.h"

#include "Modules/RuntimeUI/Widgets/RuntimeUISliderWidget.h"
#include "Modules/RuntimeUI/RuntimeUIContext.h"
#include "Modules/Assets/Managers/ScriptManager.h"

#include "Modules/Rendering/RenderingService.h"

namespace Kargono::RuntimeUI
{
	void SliderWidget::OnRender(RuntimeUIContext* uiContext, Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth)
	{
		UNREFERENCED_PARAMETER(viewportWidth);
		KG_PROFILE_FUNCTION();

		Assets::AssetRef<UserInterface> activeUI = uiContext->m_ActiveUI.GetAssetRef();

		// Calculate the widget's rendering data
		Math::vec3 widgetSize = CalculateWidgetSize(windowSize);

		// Get widget translation
		Math::vec3 widgetTranslation = CalculateWorldPosition(windowTranslation, windowSize);

		// Render the slider
		RenderSliderLine(uiContext, m_LineColor, widgetTranslation, widgetSize);

		widgetTranslation.z += 0.001f;

		// Get the slider's current normalized location based on the bounds and currentValue
		float normalizedSliderLocation = (m_CurrentValue - m_Bounds.x) / (m_Bounds.y - m_Bounds.x);

		if (activeUI->m_InteractState.m_SelectedWidget == this)
		{
			RenderSlider(uiContext, activeUI->m_Config.m_SelectColor,
				{ widgetTranslation.x + widgetSize.x * normalizedSliderLocation, widgetTranslation.y, widgetTranslation.z },
				widgetSize);
		}
		else
		{
			RenderSlider(uiContext, m_SliderColor,
				{ widgetTranslation.x + widgetSize.x * normalizedSliderLocation, widgetTranslation.y, widgetTranslation.z },
				widgetSize
			);
		}
	}
	void SliderWidget::RenderSliderLine(RuntimeUIContext* uiContext, const Math::vec4& color, const Math::vec3& translation, const Math::vec3& size)
	{
		Rendering::RendererInputSpec& renderSpec = uiContext->m_BackgroundInputSpec;

		Math::vec3 sliderSize = { size.x , 0.1f * size.y , size.z };

		if (color.w > 0.001f)
		{
			// Create the transform of the quad
			renderSpec.m_TransformMatrix = glm::translate(Math::mat4(1.0f),
				Math::vec3(translation.x + (sliderSize.x / 2.0f), translation.y + (size.y / 2.0f), translation.z))
				* glm::scale(Math::mat4(1.0f), sliderSize);
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(color,
				Utility::FileSystem::CRCFromString("a_Color"),
				renderSpec.m_Buffer, renderSpec.m_Shader.GetAssetRef());

			// Submit background data to GPU
			Rendering::RenderingService::SubmitDataToRenderer(renderSpec);
		}
	}
	void SliderWidget::RenderSlider(RuntimeUIContext* uiContext, const Math::vec4& color, const Math::vec3& translation, const Math::vec3& size)
	{
		Rendering::RendererInputSpec& renderSpec = uiContext->m_BackgroundInputSpec;

		Math::vec3 sliderSize = { 0.04f * size.x , 0.35f * size.y , size.z };

		if (color.w > 0.001f)
		{
			// Create the widget's background rendering data
			renderSpec.m_TransformMatrix = glm::translate(Math::mat4(1.0f),
				Math::vec3(translation.x, translation.y + (size.y / 2.0f), translation.z))
				* glm::scale(Math::mat4(1.0f), sliderSize);
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(color,
				Utility::FileSystem::CRCFromString("a_Color"),
				renderSpec.m_Buffer, renderSpec.m_Shader.GetAssetRef());

			// Submit background data to GPU
			Rendering::RenderingService::SubmitDataToRenderer(renderSpec);
		}
	}

	void SliderWidget::Serialize(YAML::Emitter& emitter, UserInterface* parentUI)
	{
		emitter << YAML::BeginMap; // Begin Widget Map

		// Call base serialization
		Widget::Serialize(emitter, parentUI);

		emitter << YAML::Key << "SliderWidget" << YAML::Value;
		emitter << YAML::BeginMap; // Begin SliderWidget Map
		// Save selection fields
		m_SelectionData.Serialize(emitter);
		// Save slider unique function pointers
		emitter << YAML::Key << "OnMoveSlider" << YAML::Value << (uint64_t)m_OnMoveSlider.GetAssetHandle();
		// Save other slider options
		emitter << YAML::Key << "Bounds" << YAML::Value << m_Bounds;
		emitter << YAML::Key << "SliderColor" << YAML::Value << m_SliderColor;
		emitter << YAML::Key << "LineColor" << YAML::Value << m_LineColor;

		emitter << YAML::EndMap; // End SliderWidget Map

		emitter << YAML::EndMap; // End Widget Map
	}
	void SliderWidget::Deserialize(const YAML::Node& node, UserInterface* parentUI)
	{
		// Call base deserialization
		Widget::Deserialize(node, parentUI);

		YAML::Node specificWidget = node["SliderWidget"];
		m_WidgetType = RuntimeUI::WidgetTypes::SliderWidget;
		// Get selection data
		m_SelectionData.Deserialize(specificWidget);

		// Get slider specific fields
		m_Bounds = specificWidget["Bounds"].as<Math::vec2>();
		m_SliderColor = specificWidget["SliderColor"].as<Math::vec4>();
		m_LineColor = specificWidget["LineColor"].as<Math::vec4>();

		// Get slider widget specific function pointers
		Assets::AssetHandle onMoveSliderHandle = specificWidget["OnMoveSlider"].as<uint64_t>();
		if (!onMoveSliderHandle.IsValid())
		{
			m_OnMoveSlider.Reset();
		}
		else
		{
		    Assets::AssetRef<Scripting::Script> onPressScript = Assets::s_ScriptManager.GetAssetByHandle(onMoveSliderHandle);
			if (!onPressScript)
			{
				KG_WARN("Unable to locate on move slider Script!");
				return;
			}
			m_OnMoveSlider = onPressScript;
		}
	}
}
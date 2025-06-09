#include "kgpch.h"

#include "Modules/RuntimeUI/Widgets/RuntimeUISliderWidget.h"
#include "Modules/RuntimeUI/RuntimeUIContext.h"

#include "Modules/Rendering/RenderingService.h"
#include "Modules/ECS/EngineComponents.h"

namespace Kargono::RuntimeUI
{
	void SliderWidget::OnRender(RuntimeUIContext* uiContext, Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth)
	{
		UNREFERENCED_PARAMETER(viewportWidth);
		KG_PROFILE_FUNCTION();

		Ref<UserInterface> activeUI = uiContext->m_ActiveUI;

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
				renderSpec.m_Buffer, renderSpec.m_Shader);

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
				renderSpec.m_Buffer, renderSpec.m_Shader);

			// Submit background data to GPU
			Rendering::RenderingService::SubmitDataToRenderer(renderSpec);
		}
	}
}
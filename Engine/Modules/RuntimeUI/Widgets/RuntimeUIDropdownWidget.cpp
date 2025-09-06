#include "kgpch.h"

#include "Modules/RuntimeUI/Widgets/RuntimeUIDropdownWidget.h"
#include "Modules/RuntimeUI/RuntimeUIContext.h"

#include "Kargono/Core/Resolution.h"
#include "Kargono/Projects/Project.h"
#include "Modules/Input/InputService.h"
#include "Modules/Core/Engine.h"

namespace Kargono::RuntimeUI
{
	void DropDownWidget::OnRender(RuntimeUIContext* uiContext, Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth)
	{
		KG_PROFILE_FUNCTION();

		Ref<UserInterface> activeUI = uiContext->m_ActiveUI;

		// Get mouse position and active viewport
		Math::vec2 mousePosition = Input::InputService::GetViewportMousePosition();
		ViewportData* viewportData = EngineService::GetActiveEngine().GetApp().GetViewportData();

		// Calculate the widget's rendering data
		Math::vec3 widgetSize = CalculateWidgetSize(windowSize);
		// Get widget translation
		Math::vec3 widgetTranslation = CalculateWorldPosition(windowTranslation, windowSize);

		// Check if the mouse position is within bounds of the current option
		if (activeUI->m_InteractState.m_HoveredWidget &&
			mousePosition.x > widgetTranslation.x && mousePosition.x < (widgetTranslation.x + widgetSize.x) &&
			mousePosition.y > widgetTranslation.y && mousePosition.y < (widgetTranslation.y + widgetSize.y))
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

		// Ensure that current option is within bounds
		if (m_CurrentOption < m_DropDownOptions.size())
		{
			// Get the active option data
			SingleLineTextData& textData = m_DropDownOptions.at(m_CurrentOption);

			// Calculate text starting point
			Math::vec2 resolution = Utility::ScreenResolutionToAspectRatio(Projects::ProjectService::GetActiveContext().GetTargetResolution());
			float textScalingFactor{ (viewportWidth * 0.15f * textData.m_TextSize) * (resolution.y / resolution.x) };
			Math::vec3 textStartingPoint = textData.GetTextStartingPosition(widgetTranslation, widgetSize, textScalingFactor);

			// Create the widget's text rendering data
			textStartingPoint.z += 0.001f;

			// Draw current option's text
			textData.OnRender(uiContext, textStartingPoint, textScalingFactor);
		}

		// Draw drop-down options
		if (m_DropDownOpen)
		{
			// This variable represents the *visible* drop-down position/index
			// Note: IT IS NOT THE INDEX INTO THE DROPDOWNOPTIONS VECTOR
			size_t visibleDropDownOffset{ 0 };
			for (size_t iteration{ 0 }; iteration < m_DropDownOptions.size(); iteration++)
			{
				// Exclude the current option
				if (m_CurrentOption == iteration)
				{
					continue;
				}

				// Increment z-location
				widgetTranslation.z += 0.001f;

				// Create transform for the current drop-down option
				Math::vec3 currentOptionTranslation
				{
					widgetTranslation.x,
					widgetTranslation.y - widgetSize.y * (float)(visibleDropDownOffset + 1),
					widgetTranslation.z
				};

				// Check if the mouse position is within bounds of the current option
				if (activeUI->m_InteractState.m_HoveredWidget &&
					mousePosition.x > currentOptionTranslation.x &&
					mousePosition.x < (currentOptionTranslation.x + widgetSize.x) &&
					mousePosition.y > currentOptionTranslation.y &&
					mousePosition.y < (currentOptionTranslation.y + widgetSize.y))
				{
					// Draw the background for the current option
					RenderBackground(uiContext, activeUI->m_Config.m_HoveredColor, currentOptionTranslation, widgetSize);
				}
				else
				{
					// Draw the background for the current option
					RenderBackground(uiContext, m_DropDownBackground, currentOptionTranslation, widgetSize);
				}


				// Draw the current option label
				// Get the active option data
				SingleLineTextData& textData = m_DropDownOptions.at(iteration);

				// Calculate text starting point
				Math::vec2 resolution = Utility::ScreenResolutionToAspectRatio(Projects::ProjectService::GetActiveContext().GetTargetResolution());
				float textScalingFactor{ (viewportWidth * 0.15f * textData.m_TextSize) * (resolution.y / resolution.x) };
				Math::vec3 textStartingPoint = textData.GetTextStartingPosition(currentOptionTranslation, widgetSize, textScalingFactor);

				// Create the widget's text rendering data
				textStartingPoint.z += 0.001f;

				// Draw current option's text
				textData.OnRender(uiContext, textStartingPoint, textScalingFactor);

				visibleDropDownOffset++;
			}
		}


	}

	void DropDownWidget::RevalidateTextDimensions()
	{
		// Calculate text size for all the current options
		for (SingleLineTextData& textData : m_DropDownOptions)
		{
			textData.RevalidateTextDimensions(i_ParentUI);
		}
	}
}
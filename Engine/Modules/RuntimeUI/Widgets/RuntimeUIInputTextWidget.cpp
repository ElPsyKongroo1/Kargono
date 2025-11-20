#include "kgpch.h"

#include "Modules/RuntimeUI/Widgets/RuntimeUIImageButtonWidget.h"
#include "Modules/RuntimeUI/RuntimeUIContext.h"

#include "Kargono/Core/Resolution.h"
#include "Kargono/Projects/Project.h"

namespace Kargono::RuntimeUI
{
	void InputTextWidget::SetText(const std::string& newText)
	{
		// Set the text of the widget
		m_TextData.m_Text = newText;

		// Calculate the new text size
		RevalidateTextDimensions();
	}

	void InputTextWidget::RevalidateTextDimensions()
	{
		m_TextData.RevalidateTextDimensions(i_ParentUI);
	}

	void InputTextWidget::OnRender(RuntimeUIContext* uiContext, Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth)
	{
		KG_PROFILE_FUNCTION();

		Ref<UserInterface> activeUI = uiContext->m_ActiveUI;

		// Calculate the widget's rendering data
		Math::vec3 widgetSize = CalculateWidgetSize(windowSize);

		// Get widget translation
		Math::vec3 widgetTranslation = CalculateWorldPosition(windowTranslation, windowSize);

		// Draw background
		if (activeUI->m_InteractState.m_EditingWidget == this)
		{
			RenderBackground(uiContext, activeUI->m_Config.m_EditingColor, widgetTranslation, widgetSize);
		}
		else if (activeUI->m_InteractState.m_HoveredWidget == this)
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

		// Calculate text starting point
		Math::vec2 resolution = Utility::ScreenResolutionToAspectRatio(Projects::ProjectService::GetActiveContext().GetTargetResolution());
		float textScalingFactor{ (viewportWidth * 0.15f * m_TextData.m_TextSize) * (resolution.y / resolution.x) };
		Math::vec3 textStartingPoint = m_TextData.GetTextStartingPosition(widgetTranslation, widgetSize, textScalingFactor);

		// Render the widget's text
		textStartingPoint.z += 0.001f;
		m_TextData.OnRender(uiContext, textStartingPoint, textScalingFactor);

		// Render the IBeam icon/cursor if necessary
		textStartingPoint.z += 0.001f;
		if (uiContext->m_ActiveUI->m_InteractState.m_EditingWidget == this &&
			uiContext->m_ActiveUI->m_CaretState.m_CaretVisible)
		{
			m_TextData.RenderTextCursor(uiContext, textStartingPoint, textScalingFactor);
		}
	}

	void InputTextWidget::Serialize(YAML::Emitter& emitter, UserInterface* parentUI)
	{
		emitter << YAML::BeginMap; // Begin Widget Map

		// Call base serialization
		Widget::Serialize(emitter, parentUI);

		emitter << YAML::Key << "InputTextWidget" << YAML::Value;
		emitter << YAML::BeginMap; // Begin InputTextWidget Map
		// Save text data
		m_TextData.Serialize(emitter);
		// Save selection fields
		m_SelectionData.Serialize(emitter);
		// Save input text unique function pointers
		emitter << YAML::Key << "OnMoveCursor" << YAML::Value << (uint64_t)m_OnMoveCursorHandle;
		emitter << YAML::EndMap; // End InputTextWidget Map

		emitter << YAML::EndMap; // End Widget Map
	}
	void InputTextWidget::Deserialize(const YAML::Node& node, UserInterface* parentUI)
	{
		// Call base deserialization
		Widget::Deserialize(node, parentUI);

		YAML::Node specificWidget = node["InputTextWidget"];
		m_WidgetType = RuntimeUI::WidgetTypes::InputTextWidget;
		// Get single line data
		m_TextData.Deserialize(specificWidget);
		// Get selection data
		m_SelectionData.Deserialize(specificWidget);
		// Get input map specific function pointers
		m_OnMoveCursorHandle = specificWidget["OnMoveCursor"].as<uint64_t>();
		if (m_OnMoveCursorHandle == Assets::k_EmptyHandle)
		{
			m_OnMoveCursor = nullptr;
		}
		else
		{
		    Assets::AssetRef<Scripting::Script> onPressScript = Assets::AssetService::m_ScriptManager.GetAssetByHandle(m_OnMoveCursorHandle);
			if (!onPressScript)
			{
				KG_WARN("Unable to locate On Move Cursor Script!");
				return;
			}
			m_OnMoveCursor = onPressScript;
		}
	}
}
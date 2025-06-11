#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUICheckbox.h"
#include "Modules/EditorUI/EditorUIContext.h"

#include "Kargono/Utility/Operations.h"

namespace Kargono::EditorUI
{
	void CheckboxWidget::RenderCheckbox()
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(m_WidgetID);
		uint32_t widgetCount{ 0 };

		// Draw background
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();
		draw_list->AddRectFilled(ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne + 21.0f, screenPosition.y + EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUIService::s_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

		if (m_Flags & Checkbox_Indented)
		{
			ImGui::SetCursorPosX(EditorUIService::s_TextLeftIndentOffset);
		}
		// Display Primary Label
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::m_ConfigColors.s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & Checkbox_Indented ? EditorUIService::s_PrimaryTextIndentedWidth : EditorUIService::s_PrimaryTextWidth);
		EditorUIService::TruncateText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		if (m_Flags & Checkbox_LeftLean)
		{
			ImGui::SameLine(EditorUIService::s_SecondaryTextPosOne - 2.5f);
		}
		else
		{
			ImGui::SameLine(360.0f);
		}

		if (m_Editing)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, k_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, k_PureEmpty);
			EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
				{

					if (m_CurrentBoolean)
					{
						m_CurrentBoolean = false;
						if (!m_ConfirmAction)
						{
							return;
						}
						m_ConfirmAction(*this);
					}
					else
					{
						m_CurrentBoolean = true;
						if (!m_ConfirmAction)
						{
							return;
						}
						m_ConfirmAction(*this);
					}
				}, EditorUIService::s_SmallCheckboxButton, m_CurrentBoolean, EditorUIService::m_ConfigColors.s_HighlightColor1);
			ImGui::PopStyleColor(2);
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, k_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, k_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, k_PureEmpty);
			EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), nullptr,
				EditorUIService::s_SmallCheckboxDisabledButton,
				m_CurrentBoolean, EditorUIService::m_ConfigColors.s_SecondaryTextColor);
			ImGui::PopStyleColor(3);
		}

		ImGui::SameLine();
		EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
			{
				Utility::Operations::ToggleBoolean(m_Editing);
			},
			EditorUIService::s_SmallEditButton,
			m_Editing, m_Editing ? EditorUIService::m_ConfigColors.s_PrimaryTextColor : EditorUIService::m_ConfigColors.s_DisabledColor);
	}
}
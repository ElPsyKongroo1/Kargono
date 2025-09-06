#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUICheckbox.h"
#include "Modules/EditorUI/EditorUIContext.h"

#include "Kargono/Utility/Operations.h"

namespace Kargono::EditorUI
{
	void CheckboxWidget::RenderCheckbox()
	{
		ResetChildID();

		// Draw background
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne + 21.0f, screenPosition.y + EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

		if (m_Flags & Checkbox_Indented)
		{
			ImGui::SetCursorPosX(EditorUIContext::m_ConfigSpacing.m_PrimaryTextIndent);
		}
		// Display Primary Label
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & Checkbox_Indented ? EditorUIContext::m_ActiveWindowData.m_PrimaryTextIndentedWidth : EditorUIContext::m_ActiveWindowData.m_PrimaryTextWidth);
		EditorUIContext::RenderTruncatedText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		if (m_Flags & Checkbox_LeftLean)
		{
			ImGui::SameLine(EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne - 2.5f);
		}
		else
		{
			ImGui::SameLine(360.0f);
		}

		if (m_Editing)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, k_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, k_PureEmpty);
			EditorUIContext::RenderInlineButton(GetNextChildID(), [&]()
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
				}, EditorUIContext::m_UIPresets.m_SmallCheckboxButton, m_CurrentBoolean, EditorUIContext::m_ConfigColors.m_HighlightColor1);
			ImGui::PopStyleColor(2);
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, k_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, k_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, k_PureEmpty);
			EditorUIContext::RenderInlineButton(GetNextChildID(), nullptr,
				EditorUIContext::m_UIPresets.m_SmallCheckboxDisabledButton,
				m_CurrentBoolean, EditorUIContext::m_ConfigColors.m_SecondaryTextColor);
			ImGui::PopStyleColor(3);
		}

		ImGui::SameLine();
		EditorUIContext::RenderInlineButton(GetNextChildID(), [&]()
			{
				Utility::Operations::ToggleBoolean(m_Editing);
			},
			EditorUIContext::m_UIPresets.m_SmallEditButton,
			m_Editing, m_Editing ? EditorUIContext::m_ConfigColors.m_PrimaryTextColor : EditorUIContext::m_ConfigColors.m_DisabledColor);
	}
}
#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUIRadioSelectWidget.h"
#include "Modules/EditorUI/EditorUIContext.h"

#include "Kargono/Utility/Operations.h"

namespace Kargono::EditorUI
{
	void RadioSelectWidget::RenderRadio()
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(m_WidgetID);
		uint32_t widgetCount{ 0 };

		// Draw backgrounds
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne + EditorUIContext::m_ActiveWindowData.m_SecondaryTextMediumWidth + 19.0f, screenPosition.y + EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosMiddle - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosMiddle + EditorUIContext::m_ActiveWindowData.m_SecondaryTextMediumWidth + 19.0f, screenPosition.y + EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

		if (m_Flags & RadioSelect_Indented)
		{
			ImGui::SetCursorPosX(EditorUIContext::m_ConfigSpacing.m_PrimaryTextIndent);
		}

		// Display Item
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & RadioSelect_Indented ? EditorUIContext::m_ActiveWindowData.m_PrimaryTextIndentedWidth : EditorUIContext::m_ActiveWindowData.m_PrimaryTextWidth);
		EditorUIContext::RenderTruncatedText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);

		ImGui::PopStyleColor();
		ImGui::SameLine(EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne - 2.5f);

		if (m_Editing)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, k_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, k_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_SecondaryTextColor);
			EditorUIContext::RenderInlineButton(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount), [&]()
				{
					if (m_SelectedOption == 0)
					{
						m_SelectedOption = 1;
					}
					else
					{
						m_SelectedOption = 0;
					}
					m_SelectAction();
				}, EditorUIContext::m_UIPresets.m_SmallCheckboxButton, m_SelectedOption == 0, EditorUIContext::m_ConfigColors.m_HighlightColor1);
			ImGui::SameLine();

			int32_t position = ImGui::FindPositionAfterLength(m_FirstOptionLabel.c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextMediumWidth - 18.0f);
			EditorUIContext::RenderTruncatedText(m_FirstOptionLabel, position == -1 ? std::numeric_limits<int32_t>::max() : position);

			ImGui::SameLine(EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosMiddle - 2.5f);
			EditorUIContext::RenderInlineButton(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount), [&]()
				{
					if (m_SelectedOption == 1)
					{
						m_SelectedOption = 0;
					}
					else
					{
						m_SelectedOption = 1;
					}
					m_SelectAction();
				}, EditorUIContext::m_UIPresets.m_SmallCheckboxButton, m_SelectedOption == 1, EditorUIContext::m_ConfigColors.m_HighlightColor2);
			ImGui::SameLine();
			position = ImGui::FindPositionAfterLength(m_SecondOptionLabel.c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextMediumWidth - 18.0f);
			EditorUIContext::RenderTruncatedText(m_SecondOptionLabel, position == -1 ? std::numeric_limits<int32_t>::max() : position);

			ImGui::PopStyleColor(3);
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, k_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, k_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, k_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_SecondaryTextColor);
			EditorUIContext::RenderInlineButton(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount), nullptr,
				EditorUIContext::m_UIPresets.m_SmallCheckboxDisabledButton, m_SelectedOption == 0, EditorUIContext::m_ConfigColors.m_SecondaryTextColor);
			ImGui::SameLine();

			int32_t position = ImGui::FindPositionAfterLength(m_FirstOptionLabel.c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextMediumWidth - 18.0f);
			EditorUIContext::RenderTruncatedText(m_FirstOptionLabel, position == -1 ? std::numeric_limits<int32_t>::max() : position);

			ImGui::SameLine(EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosMiddle - 2.5f);
			EditorUIContext::RenderInlineButton(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount), nullptr,
				EditorUIContext::m_UIPresets.m_SmallCheckboxDisabledButton, m_SelectedOption == 1, EditorUIContext::m_ConfigColors.m_SecondaryTextColor);
			ImGui::SameLine();
			position = ImGui::FindPositionAfterLength(m_SecondOptionLabel.c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextMediumWidth - 18.0f);
			EditorUIContext::RenderTruncatedText(m_SecondOptionLabel, position == -1 ? std::numeric_limits<int32_t>::max() : position);
			ImGui::PopStyleColor(4);
		}

		ImGui::SameLine();
		EditorUIContext::RenderInlineButton(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount), [&]()
			{
				Utility::Operations::ToggleBoolean(m_Editing);
			},
			EditorUIContext::m_UIPresets.m_SmallEditButton,
			m_Editing, m_Editing ? EditorUIContext::m_ConfigColors.m_PrimaryTextColor : EditorUIContext::m_ConfigColors.m_DisabledColor);
	}
}
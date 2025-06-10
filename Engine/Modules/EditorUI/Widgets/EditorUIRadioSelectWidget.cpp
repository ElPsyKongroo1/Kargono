#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUIRadioSelectWidget.h"
#include "Modules/EditorUI/EditorUI.h"

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
		draw_list->AddRectFilled(ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne + EditorUIService::s_SecondaryTextMediumWidth + 19.0f, screenPosition.y + EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUIService::s_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosMiddle - 5.0f, screenPosition.y),
			ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosMiddle + EditorUIService::s_SecondaryTextMediumWidth + 19.0f, screenPosition.y + EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUIService::s_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

		if (m_Flags & RadioSelect_Indented)
		{
			ImGui::SetCursorPosX(EditorUIService::s_TextLeftIndentOffset);
		}

		// Display Item
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & RadioSelect_Indented ? EditorUIService::s_PrimaryTextIndentedWidth : EditorUIService::s_PrimaryTextWidth);
		EditorUIService::TruncateText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);

		ImGui::PopStyleColor();
		ImGui::SameLine(EditorUIService::s_SecondaryTextPosOne - 2.5f);

		if (m_Editing)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorUIService::s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, EditorUIService::s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::s_SecondaryTextColor);
			EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
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
				}, EditorUIService::s_SmallCheckboxButton, m_SelectedOption == 0, EditorUIService::s_HighlightColor1);
			ImGui::SameLine();

			int32_t position = ImGui::FindPositionAfterLength(m_FirstOptionLabel.c_str(), EditorUIService::s_SecondaryTextMediumWidth - 18.0f);
			EditorUIService::TruncateText(m_FirstOptionLabel, position == -1 ? std::numeric_limits<int32_t>::max() : position);

			ImGui::SameLine(EditorUIService::s_SecondaryTextPosMiddle - 2.5f);
			EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
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
				}, EditorUIService::s_SmallCheckboxButton, m_SelectedOption == 1, EditorUIService::s_HighlightColor2);
			ImGui::SameLine();
			position = ImGui::FindPositionAfterLength(m_SecondOptionLabel.c_str(), EditorUIService::s_SecondaryTextMediumWidth - 18.0f);
			EditorUIService::TruncateText(m_SecondOptionLabel, position == -1 ? std::numeric_limits<int32_t>::max() : position);

			ImGui::PopStyleColor(3);
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorUIService::s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, EditorUIService::s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, EditorUIService::s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::s_SecondaryTextColor);
			EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), nullptr,
				EditorUIService::s_SmallCheckboxDisabledButton, m_SelectedOption == 0, EditorUIService::s_SecondaryTextColor);
			ImGui::SameLine();

			int32_t position = ImGui::FindPositionAfterLength(m_FirstOptionLabel.c_str(), EditorUIService::s_SecondaryTextMediumWidth - 18.0f);
			EditorUIService::TruncateText(m_FirstOptionLabel, position == -1 ? std::numeric_limits<int32_t>::max() : position);

			ImGui::SameLine(EditorUIService::s_SecondaryTextPosMiddle - 2.5f);
			EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), nullptr,
				EditorUIService::s_SmallCheckboxDisabledButton, m_SelectedOption == 1, EditorUIService::s_SecondaryTextColor);
			ImGui::SameLine();
			position = ImGui::FindPositionAfterLength(m_SecondOptionLabel.c_str(), EditorUIService::s_SecondaryTextMediumWidth - 18.0f);
			EditorUIService::TruncateText(m_SecondOptionLabel, position == -1 ? std::numeric_limits<int32_t>::max() : position);
			ImGui::PopStyleColor(4);
		}

		ImGui::SameLine();
		EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
			{
				Utility::Operations::ToggleBoolean(m_Editing);
			},
			EditorUIService::s_SmallEditButton,
			m_Editing, m_Editing ? EditorUIService::s_PrimaryTextColor : EditorUIService::s_DisabledColor);
	}
}
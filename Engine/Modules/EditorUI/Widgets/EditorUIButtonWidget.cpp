#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUIButtonWidget.h"
#include "Modules/EditorUI/EditorUI.h"

#include "Kargono/Utility/Operations.h"

namespace Kargono::EditorUI
{
	bool ButtonSpec::RenderButton()
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(m_WidgetID);
		uint32_t widgetCount{ 0 };
		bool returnValue{ false };

		if (m_Flags & Button_Indented)
		{
			ImGui::SetCursorPosX(EditorUIService::s_TextLeftIndentOffset);
		}
		// Display Primary Label
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & Button_Indented ? EditorUIService::s_PrimaryTextIndentedWidth : EditorUIService::s_PrimaryTextWidth);
		EditorUIService::TruncateText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		// Setup background drawlist
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();

		// Shift button to secondary text position one
		ImGui::SameLine(EditorUIService::s_SecondaryTextPosOne - 2.5f);
		if (ImGui::InvisibleButton(
			("##" + std::to_string(m_WidgetID + EditorUIService::WidgetIterator(widgetCount))).c_str(),
			ImVec2(EditorUIService::s_SecondaryTextSmallWidth, EditorUIService::s_TextBackgroundHeight)))
		{
			if (m_Button.m_OnPress)
			{
				m_Button.m_OnPress(m_Button);
			}
			returnValue = true;
		}

		ImVec4 buttonColor;

		if (ImGui::IsItemActive())
		{
			buttonColor = EditorUIService::s_ActiveColor;
		}
		else if (ImGui::IsItemHovered())
		{
			buttonColor = EditorUIService::s_HoveredColor;
		}
		else
		{
			buttonColor = EditorUIService::s_HighlightColor1_UltraThin;
		}

		// Draw the relevant background
		draw_list->AddRectFilled(ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne - 5.0f, screenPosition.y - EditorUIService::s_TextBackgroundHeight),
			ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne + EditorUIService::s_SecondaryTextSmallWidth, screenPosition.y), ImColor(buttonColor),
			4.0f, ImDrawFlags_RoundCornersAll);

		// Display entry text
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::s_PrimaryTextColor);
		ImGui::SameLine(EditorUIService::s_SecondaryTextPosOne);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
		int floatPosition = ImGui::FindPositionAfterLength(m_Button.m_Label.CString(), EditorUIService::s_SecondaryTextSmallWidth);
		EditorUIService::TruncateText(m_Button.m_Label.CString(),
			floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
		ImGui::PopStyleColor();

		// Indicate if button is pressed
		return returnValue;
	}
}
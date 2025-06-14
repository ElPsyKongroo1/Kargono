#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUIButtonWidget.h"
#include "Modules/EditorUI/EditorUIContext.h"

#include "Kargono/Utility/Operations.h"

namespace Kargono::EditorUI
{
	bool ButtonSpec::RenderButton()
	{
		// Local Variables
		bool returnValue{ false };

		if (m_Flags & Button_Indented)
		{
			ImGui::SetCursorPosX(EditorUIContext::m_ConfigSpacing.m_PrimaryTextIndent);
		}
		// Display Primary Label
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & Button_Indented ? EditorUIContext::m_ActiveWindowData.m_PrimaryTextIndentedWidth : EditorUIContext::m_ActiveWindowData.m_PrimaryTextWidth);
		EditorUIContext::RenderTruncatedText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		// Setup background drawlist
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();

		// Shift button to secondary text position one
		ImGui::SameLine(EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne - 2.5f);
		if (ImGui::InvisibleButton(
			("##" + std::to_string(GetNextChildID())).c_str(),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth, EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight)))
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
			buttonColor = EditorUIContext::m_ConfigColors.m_ActiveColor;
		}
		else if (ImGui::IsItemHovered())
		{
			buttonColor = EditorUIContext::m_ConfigColors.m_HoveredColor;
		}
		else
		{
			buttonColor = EditorUIContext::m_ConfigColors.m_HighlightColor1_UltraThin;
		}

		// Draw the relevant background
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne - 5.0f, screenPosition.y - EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth, screenPosition.y), ImColor(buttonColor),
			4.0f, ImDrawFlags_RoundCornersAll);

		// Display entry text
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);
		ImGui::SameLine(EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
		int floatPosition = ImGui::FindPositionAfterLength(m_Button.m_Label.CString(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
		EditorUIContext::RenderTruncatedText(m_Button.m_Label.CString(),
			floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
		ImGui::PopStyleColor();

		// Indicate if button is pressed
		return returnValue;
	}
}
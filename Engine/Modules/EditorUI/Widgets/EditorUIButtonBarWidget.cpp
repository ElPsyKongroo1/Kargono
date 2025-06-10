#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUIButtonBarWidget.h"
#include "Modules/EditorUI/EditorUI.h"

#include "Kargono/Utility/Operations.h"

namespace Kargono::EditorUI
{
	void ButtonBarWidget::RenderBar()
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(m_WidgetID);
		uint32_t widgetCount{ 0 };

		std::array<float, 4> buttonPositions
		{
			EditorUIService::s_SecondaryTextPosOne,
			EditorUIService::s_SecondaryTextPosTwo,
			EditorUIService::s_SecondaryTextPosThree,
			EditorUIService::s_SecondaryTextPosFour
		};

		std::array<ImVec4, 4> buttonColors
		{
			EditorUIService::s_HighlightColor1_UltraThin,
			EditorUIService::s_HighlightColor2_UltraThin,
			EditorUIService::s_HighlightColor3_UltraThin,
			EditorUIService::s_HighlightColor4_UltraThin,
		};

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
		for (size_t i{ 0 }; i < m_ButtonCount; i++)
		{
			// Shift button to secondary text position one
			ImGui::SameLine(buttonPositions[i] - 2.5f);
			EditorUI::Button& currentButton = m_Buttons[i];
			if (ImGui::InvisibleButton(
				("##" + std::to_string(m_WidgetID + EditorUIService::WidgetIterator(widgetCount))).c_str(),
				ImVec2(EditorUIService::s_SecondaryTextSmallWidth, EditorUIService::s_TextBackgroundHeight)))
			{
				if (currentButton.m_OnPress)
				{
					currentButton.m_OnPress(currentButton);
				}
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
				buttonColor = buttonColors[i];
			}

			// Draw the relevant background
			draw_list->AddRectFilled(ImVec2(EditorUIService::s_WindowPosition.x + buttonPositions[i] - 5.0f, screenPosition.y - EditorUIService::s_TextBackgroundHeight),
				ImVec2(EditorUIService::s_WindowPosition.x + buttonPositions[i] + EditorUIService::s_SecondaryTextSmallWidth, screenPosition.y), ImColor(buttonColor),
				4.0f, ImDrawFlags_RoundCornersAll);

			// Display entry text
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::s_PrimaryTextColor);
			ImGui::SameLine(buttonPositions[i]);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
			int floatPosition = ImGui::FindPositionAfterLength(currentButton.m_Label.CString(), EditorUIService::s_SecondaryTextSmallWidth);
			EditorUIService::TruncateText(currentButton.m_Label.CString(),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::PopStyleColor();
		}
	}

	bool ButtonBarWidget::AddButton(Button& button)
	{
		// Ensure the maximum button bar count is not reached
		if (m_ButtonCount >= k_MaxButtonBarSize)
		{
			KG_WARN("Failed to add button to button bar. Button capacity reached.");
			return false;
		}

		// Add the button to the button bar
		m_Buttons[m_ButtonCount] = button;
		m_ButtonCount++;

		return true;
	}
	bool ButtonBarWidget::AddButton(std::string_view label, std::function<void(Button&)> onClick, Ref<void> providedData)
	{
		// Ensure the maximum button bar count is not reached
		if (m_ButtonCount >= k_MaxButtonBarSize)
		{
			KG_WARN("Failed to add button to button bar. Button capacity reached.");
			return false;
		}

		// Add the button to the button bar
		m_Buttons[m_ButtonCount].m_Label = label;
		m_Buttons[m_ButtonCount].m_OnPress = onClick;
		m_Buttons[m_ButtonCount].m_ProvidedData = providedData;
		m_ButtonCount++;

		return true;
	}
	void ButtonBarWidget::ClearButtons()
	{
		// Reset button count
		m_ButtonCount = 0;

		// Reset all button fields (I mainly do this because of the shared pointers)
		for (Button& button : m_Buttons)
		{
			button = Button();
		}
	}
	Button* ButtonBarWidget::GetButton(size_t index)
	{
		if (index >= m_ButtonCount)
		{
			KG_WARN("Failed to retrieve button. Provided index is out of bounds.")
				return nullptr;
		}

		return &m_Buttons[index];
	}
}
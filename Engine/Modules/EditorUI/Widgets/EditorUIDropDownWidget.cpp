#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUIDropDownWidget.h"
#include "Modules/EditorUI/EditorUI.h"

#include "Modules/EditorUI/ExternalAPI/ImGuiBackendAPI.h"

namespace Kargono::EditorUI
{
	void DropDownWidget::RenderDropDown()
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(m_WidgetID);
		uint32_t widgetCount{ 0 };

		if (m_Flags & DropDown_Indented)
		{
			ImGui::SetCursorPosX(EditorUIService::s_TextLeftIndentOffset);
		}
		// Display Primary Label
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & DropDown_Indented ? EditorUIService::s_PrimaryTextIndentedWidth : EditorUIService::s_PrimaryTextWidth);
		EditorUIService::TruncateText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();

		ImGui::PushStyleColor(ImGuiCol_FrameBg, EditorUIService::s_ActiveBackgroundColor);
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, EditorUIService::s_HoveredColor);
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, EditorUIService::s_ActiveColor);

		ImGui::SetNextItemWidth(EditorUIService::s_SecondaryTextLargeWidth + 5.0f);
		ImGui::SameLine(EditorUIService::s_SecondaryTextPosOne - 5.0f);

		// Shift button to secondary text position one
		ImGui::SameLine(EditorUIService::s_SecondaryTextPosOne - 2.5f);
		if (ImGui::InvisibleButton(
			("##" + std::to_string(m_WidgetID + EditorUIService::WidgetIterator(widgetCount))).c_str(),
			ImVec2(EditorUIService::s_SecondaryTextLargeWidth, EditorUIService::s_TextBackgroundHeight)))
		{
			ImGui::OpenPopupEx(m_WidgetID, ImGuiPopupFlags_None);
		}

		ImVec4 dropdownColor;

		if (ImGui::IsItemActive())
		{
			dropdownColor = EditorUIService::s_ActiveColor;
		}
		else if (ImGui::IsItemHovered())
		{
			dropdownColor = EditorUIService::s_HoveredColor;
		}
		else
		{
			dropdownColor = EditorUIService::s_ActiveBackgroundColor;
		}

		// Draw the relevant background
		draw_list->AddRectFilled(ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne - 5.0f, screenPosition.y - EditorUIService::s_TextBackgroundHeight),
			ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne + EditorUIService::s_SecondaryTextLargeWidth, screenPosition.y), ImColor(dropdownColor),
			4.0f, ImDrawFlags_RoundCornersAll);

		// Get the selected entry
		OptionEntry* selectedEntry{ m_OptionsList.GetOption(m_CurrentOption) };
		const char* selectedText = selectedEntry ? selectedEntry->m_Label.CString() : "";

		// Display selected text
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::s_PrimaryTextColor);
		ImGui::SameLine(EditorUIService::s_SecondaryTextPosOne);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
		int floatPosition = ImGui::FindPositionAfterLength(selectedText, EditorUIService::s_SecondaryTextLargeWidth);
		EditorUIService::TruncateText(selectedText,
			floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
		ImGui::PopStyleColor();

		if (ImGui::IsPopupOpen(m_WidgetID, ImGuiPopupFlags_None))
		{
			const ImRect popupBoundingBox
			(
				ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne, screenPosition.y),
				ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne + EditorUIService::s_SecondaryTextLargeWidth, screenPosition.y)
			);
			ImGui::BeginComboPopup(m_WidgetID, popupBoundingBox, 0);

			OptionIndex entryIndex{ 0 };
			for (OptionEntry& entry : m_OptionsList)
			{
				if (entry.m_Label.IsEmpty())
				{
					entryIndex++;
					continue;
				}
				if (ImGui::Selectable(entry.m_Label.CString()))
				{
					// Set entry as selected
					m_CurrentOption = entryIndex;

					// Handle the newly selected entry
					if (m_ConfirmAction)
					{
						m_ConfirmAction(entry);
					}

					break;
				}
				entryIndex++;
			}
			ImGui::EndCombo();
		}

#if 0
		if (ImGui::BeginCombo(id, selectedEntry ? selectedEntry->m_Label.CString() : "",
			ImGuiComboFlags_NoArrowButton))
		{
			OptionIndex entryIndex{ 0 };
			for (OptionEntry& entry : m_OptionsList)
			{
				if (ImGui::Selectable(entry.m_Label.CString()))
				{
					// Set entry as selected
					m_CurrentOption = entryIndex;

					// Handle the newly selected entry
					if (m_ConfirmAction)
					{
						m_ConfirmAction(entry);
					}

					break;
				}
				entryIndex++;
			}
			ImGui::EndCombo();
		}
#endif

		ImGui::PopStyleColor(3);
	}
	OptionEntry* DropDownWidget::CreateOption()
	{
		return m_OptionsList.CreateOption();
	}
	bool DropDownWidget::SetCurrentOption(UUID handle)
	{
		OptionIndex index{ 0 };
		for (OptionEntry& entry : m_OptionsList)
		{
			// Use the first-found option entry
			if (entry.m_Handle == handle)
			{
				m_CurrentOption = index;
				return true;
			}
			index++;
		}

		return false;
	}
}
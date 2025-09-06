#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUIDropDownWidget.h"
#include "Modules/EditorUI/EditorUIContext.h"

#include "Modules/EditorUI/ExternalAPI/ImGuiBackendAPI.h"

namespace Kargono::EditorUI
{
	void DropDownWidget::RenderDropDown()
	{
		ResetChildID();

		if (m_Flags & DropDown_Indented)
		{
			ImGui::SetCursorPosX(EditorUIContext::m_ConfigSpacing.m_PrimaryTextIndent);
		}
		// Display Primary Label
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & DropDown_Indented ? EditorUIContext::m_ActiveWindowData.m_PrimaryTextIndentedWidth : EditorUIContext::m_ActiveWindowData.m_PrimaryTextWidth);
		EditorUIContext::RenderTruncatedText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();

		ImGui::PushStyleColor(ImGuiCol_FrameBg, EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor);
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, EditorUIContext::m_ConfigColors.m_HoveredColor);
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, EditorUIContext::m_ConfigColors.m_ActiveColor);

		ImGui::SetNextItemWidth(EditorUIContext::m_ActiveWindowData.m_SecondaryTextLargeWidth + 5.0f);
		ImGui::SameLine(EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne - 5.0f);

		// Shift button to secondary text position one
		ImGui::SameLine(EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne - 2.5f);
		if (ImGui::InvisibleButton(
			("##" + std::to_string(GetNextChildID())).c_str(),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_SecondaryTextLargeWidth, EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight)))
		{
			ImGui::OpenPopupEx(m_WidgetID, ImGuiPopupFlags_None);
		}

		ImVec4 dropdownColor;

		if (ImGui::IsItemActive())
		{
			dropdownColor = EditorUIContext::m_ConfigColors.m_ActiveColor;
		}
		else if (ImGui::IsItemHovered())
		{
			dropdownColor = EditorUIContext::m_ConfigColors.m_HoveredColor;
		}
		else
		{
			dropdownColor = EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor;
		}

		// Draw the relevant background
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne - 5.0f, screenPosition.y - EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne + EditorUIContext::m_ActiveWindowData.m_SecondaryTextLargeWidth, screenPosition.y), ImColor(dropdownColor),
			4.0f, ImDrawFlags_RoundCornersAll);

		// Get the selected entry
		OptionEntry* selectedEntry{ m_OptionsList.GetOption(m_CurrentOption) };
		const char* selectedText = selectedEntry ? selectedEntry->m_Label.CString() : "";

		// Display selected text
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);
		ImGui::SameLine(EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
		int floatPosition = ImGui::FindPositionAfterLength(selectedText, EditorUIContext::m_ActiveWindowData.m_SecondaryTextLargeWidth);
		EditorUIContext::RenderTruncatedText(selectedText,
			floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
		ImGui::PopStyleColor();

		if (ImGui::IsPopupOpen(m_WidgetID, ImGuiPopupFlags_None))
		{
			const ImRect popupBoundingBox
			(
				ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne, screenPosition.y),
				ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne + EditorUIContext::m_ActiveWindowData.m_SecondaryTextLargeWidth, screenPosition.y)
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
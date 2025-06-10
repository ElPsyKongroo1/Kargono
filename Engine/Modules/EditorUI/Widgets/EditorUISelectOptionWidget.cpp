#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUISelectOptionWidget.h"
#include "Modules/EditorUI/EditorUI.h"

#include "Modules/EditorUI/ExternalAPI/ImGuiBackendAPI.h"

namespace Kargono::EditorUI
{
	void SelectOptionWidget::ClearOptions()
	{
		m_ActiveOptions.clear();
	}
	void SelectOptionWidget::RenderOptions()
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(m_WidgetID);
		uint32_t widgetCount{ 0 };

		if (m_Flags & (SelectOption_PopupOnly | SelectOption_HandleEditButtonExternally))
		{
			if (m_OpenPopup)
			{
				ImGui::OpenPopup(id);
				m_OpenPopup = false;
				if (m_PopupAction)
				{
					m_PopupAction(*this);
				}
				m_CachedSelection = m_CurrentOption;
			}
		}

		if ((m_Flags & SelectOption_PopupOnly) == 0)
		{
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			ImVec2 screenPosition = ImGui::GetCursorScreenPos();
			draw_list->AddRectFilled(ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne - 5.0f, screenPosition.y),
				ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne + EditorUIService::s_SecondaryTextLargeWidth, screenPosition.y + EditorUIService::s_TextBackgroundHeight),
				ImColor(EditorUIService::s_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

			// Display Menu Item
			if (m_Flags & SelectOption_Indented)
			{
				ImGui::SetCursorPosX(EditorUIService::s_TextLeftIndentOffset);
			}
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::s_PrimaryTextColor);
			int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
				m_Flags & SelectOption_Indented ? EditorUIService::s_PrimaryTextIndentedWidth : EditorUIService::s_PrimaryTextWidth);
			EditorUIService::TruncateText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::s_SecondaryTextColor);
			EditorUIService::WriteMultilineText(m_CurrentOption.m_Label.CString(), EditorUIService::s_SecondaryTextLargeWidth, EditorUIService::s_SecondaryTextPosOne);
			ImGui::PopStyleColor();

			ImGui::SameLine();
			EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
				{
					// Handle custom edit functionality
					if (m_Flags & SelectOption_HandleEditButtonExternally)
					{
						if (m_OnEdit)
						{
							m_OnEdit(*this);
						}
					}
					// Open the button normally
					else
					{
						ImGui::OpenPopup(id);
						if (m_PopupAction)
						{
							m_PopupAction(*this);
						}
						m_CachedSelection = m_CurrentOption;
					}
				},
				EditorUIService::s_SmallEditButton, false, EditorUIService::s_DisabledColor);
		}

		// Display Popup
		ImGui::SetNextWindowSize(ImVec2(700.0f, 500.0f));
		if (ImGui::BeginPopupModal(id, NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar))
		{
			static char searchBuffer[256];

			// Set up the header for the popup
			EditorUIService::TitleText(m_Label.CString());
			ImGui::PushFont(EditorUIService::s_FontAntaRegular);
			if (m_Searching)
			{
				ImGui::SameLine(ImGui::GetWindowWidth() - 124.0f - 200.0f);
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.6f);
				ImGui::SetNextItemWidth(200.0f);

				ImGuiInputTextCallback callback = [](ImGuiInputTextCallbackData* data)
					{
						std::string currentData = std::string(data->Buf);
						SelectOptionWidget* providedSpec = (SelectOptionWidget*)data->UserData;
						providedSpec->m_CachedSearchResults = EditorUIService::GenerateSearchCache(providedSpec->GetAllOptions(), currentData);
						return 0;
					};

				ImGui::PushStyleColor(ImGuiCol_FrameBg, EditorUIService::s_ActiveColor);
				ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, EditorUIService::s_ActiveColor);
				ImGui::PushStyleColor(ImGuiCol_FrameBgActive, EditorUIService::s_ActiveColor);

				ImGui::InputText((id + "InputText").c_str(), searchBuffer, sizeof(searchBuffer), ImGuiInputTextFlags_CallbackEdit, callback, (void*)this);
				ImGui::PopStyleColor(3);
			}

			// Search Tool Bar Button
			ImGui::SameLine();
			EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
				{
					if (m_Searching)
					{
						m_Searching = false;
					}
					else
					{
						m_Searching = true;
						m_CachedSearchResults = EditorUIService::GenerateSearchCache(m_ActiveOptions, searchBuffer);
					}
				}, EditorUIService::s_LargeSearchButton, m_Searching, EditorUIService::s_PrimaryTextColor);

			// Cancel Tool Bar Button
			ImGui::SameLine();
			EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
				{
					m_Searching = false;
					memset(searchBuffer, 0, sizeof(searchBuffer));
					ImGui::CloseCurrentPopup();
				}, EditorUIService::s_LargeCancelButton, false, EditorUIService::s_PrimaryTextColor);

			// Confirm Tool Bar Button
			ImGui::SameLine();
			EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
				{
					m_CurrentOption = m_CachedSelection;
					if (m_ConfirmAction)
					{
						m_ConfirmAction(m_CurrentOption);
					}

					m_Searching = false;
					memset(searchBuffer, 0, sizeof(searchBuffer));
					ImGui::CloseCurrentPopup();
				}, EditorUIService::s_LargeConfirmButton, false, EditorUIService::s_PrimaryTextColor);

			ImGui::Separator();

			EditorUIService::Spacing(SpacingAmount::Small);

			ImGui::BeginChildEx("##", m_WidgetID + EditorUIService::WidgetIterator(widgetCount),
				{ 0.0f, 0.0f }, false, 0);
			// Start the window body
			for (auto& [title, options] :
				m_Searching ? m_CachedSearchResults : GetAllOptions())
			{
				ImGui::TextUnformatted(title.c_str());
				uint32_t iteration{ 1 };
				bool selectedButton = false;
				for (OptionEntry& option : options)
				{
					if (m_CachedSelection == option)
					{
						selectedButton = true;
					}

					if (selectedButton)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, EditorUIService::s_SelectedColor);
					}

					if (ImGui::Button((option.m_Label.CString() + id + std::string(option.m_Handle)).c_str()))
					{
						m_CachedSelection = option;
					}
					if (iteration % m_LineCount != 0 && iteration != 0 && iteration != options.size())
					{
						ImGui::SameLine();
					}

					if (selectedButton)
					{
						ImGui::PopStyleColor();
						selectedButton = false;
					}
					iteration++;
				}
				EditorUIService::Spacing(SpacingAmount::Medium);
			}

			ImGui::EndChild();

			ImGui::PopFont();
			ImGui::EndPopup();
		}
	}
	void SelectOptionWidget::AddToOptions(const std::string& group, const std::string& optionLabel, UUID optionIdentifier)
	{
		const OptionEntry newEntry{ optionLabel.c_str(), optionIdentifier };
		if (!m_ActiveOptions.contains(group))
		{
			std::vector<OptionEntry> newVector{};
			newVector.push_back(newEntry);
			m_ActiveOptions.insert_or_assign(group, newVector);
			return;
		}

		m_ActiveOptions.at(group).push_back(newEntry);
	}
	OptionEntry* OptionList::CreateOption()
	{
		return &m_Options.emplace_back();
	}
	void OptionList::Clear()
	{
		m_Options.clear();
	}
	OptionEntry* OptionList::GetOption(OptionIndex index)
	{
		if (index >= m_Options.size())
		{
			return nullptr;
		}

		return &m_Options[index];
	}
}
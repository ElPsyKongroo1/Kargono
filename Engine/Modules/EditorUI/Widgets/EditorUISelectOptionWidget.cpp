#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUISelectOptionWidget.h"
#include "Modules/EditorUI/EditorUIContext.h"
#include "Kargono/Utility/Regex.h"

#include "Modules/EditorUI/ExternalAPI/ImGuiBackendAPI.h"

namespace Kargono::EditorUI
{
	void SelectOptionWidget::ClearOptions()
	{
		m_ActiveOptions.clear();
	}
	void SelectOptionWidget::RenderOptions()
	{
		ResetChildID();

		if (m_Flags & (SelectOption_PopupOnly | SelectOption_HandleEditButtonExternally))
		{
			if (m_OpenPopup)
			{
				ImGui::OpenPopup(m_WidgetIDString);
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
			draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne - 5.0f, screenPosition.y),
				ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne + EditorUIContext::m_ActiveWindowData.m_SecondaryTextLargeWidth, screenPosition.y + EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
				ImColor(EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

			// Display Menu Item
			if (m_Flags & SelectOption_Indented)
			{
				ImGui::SetCursorPosX(EditorUIContext::m_ConfigSpacing.m_PrimaryTextIndent);
			}
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);
			int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
				m_Flags & SelectOption_Indented ? EditorUIContext::m_ActiveWindowData.m_PrimaryTextIndentedWidth : EditorUIContext::m_ActiveWindowData.m_PrimaryTextWidth);
			EditorUIContext::RenderTruncatedText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_SecondaryTextColor);
			EditorUIContext::RenderMultiLineText(m_CurrentOption.m_Label.CString(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextLargeWidth, EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne);
			ImGui::PopStyleColor();

			ImGui::SameLine();
			EditorUIContext::RenderInlineButton(GetNextChildID(), [&]()
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
						ImGui::OpenPopup(m_WidgetIDString);
						if (m_PopupAction)
						{
							m_PopupAction(*this);
						}
						m_CachedSelection = m_CurrentOption;
					}
				},
				EditorUIContext::m_UIPresets.m_SmallEditButton, false, EditorUIContext::m_ConfigColors.m_DisabledColor);
		}

		// Display Popup
		ImGui::SetNextWindowSize(ImVec2(700.0f, 500.0f));
		if (ImGui::BeginPopupModal(m_WidgetIDString, NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar))
		{
			static char searchBuffer[256];

			// Set up the header for the popup
			EditorUIContext::TitleText(m_Label.CString());
			ImGui::PushFont(EditorUIContext::m_ConfigFonts.m_HeaderRegular);
			if (m_Searching)
			{
				ImGui::SameLine(ImGui::GetWindowWidth() - 124.0f - 200.0f);
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.6f);
				ImGui::SetNextItemWidth(200.0f);

				ImGuiInputTextCallback callback = [](ImGuiInputTextCallbackData* data)
				{
					std::string currentData = std::string(data->Buf);
					SelectOptionWidget* providedSpec = (SelectOptionWidget*)data->UserData;
					providedSpec->m_CachedSearchResults = providedSpec->GenerateSearchCache(providedSpec->GetAllOptions(), currentData.c_str());
					return 0;
				};

				ImGui::PushStyleColor(ImGuiCol_FrameBg, EditorUIContext::m_ConfigColors.m_ActiveColor);
				ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, EditorUIContext::m_ConfigColors.m_ActiveColor);
				ImGui::PushStyleColor(ImGuiCol_FrameBgActive, EditorUIContext::m_ConfigColors.m_ActiveColor);

				ImGui::InputText((m_WidgetIDString + "InputText").c_str(), searchBuffer, sizeof(searchBuffer), ImGuiInputTextFlags_CallbackEdit, callback, (void*)this);
				ImGui::PopStyleColor(3);
			}

			// Search Tool Bar Button
			ImGui::SameLine();
			EditorUIContext::RenderInlineButton(GetNextChildID(), [&]()
				{
					if (m_Searching)
					{
						m_Searching = false;
					}
					else
					{
						m_Searching = true;
						m_CachedSearchResults = GenerateSearchCache(m_ActiveOptions, searchBuffer);
					}
				}, EditorUIContext::m_UIPresets.m_LargeSearchButton, m_Searching, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);

			// Cancel Tool Bar Button
			ImGui::SameLine();
			EditorUIContext::RenderInlineButton(GetNextChildID(), [&]()
				{
					m_Searching = false;
					memset(searchBuffer, 0, sizeof(searchBuffer));
					ImGui::CloseCurrentPopup();
				}, EditorUIContext::m_UIPresets.m_LargeCancelButton, false, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);

			// Confirm Tool Bar Button
			ImGui::SameLine();
			EditorUIContext::RenderInlineButton(GetNextChildID(), [&]()
				{
					m_CurrentOption = m_CachedSelection;
					if (m_ConfirmAction)
					{
						m_ConfirmAction(m_CurrentOption);
					}

					m_Searching = false;
					memset(searchBuffer, 0, sizeof(searchBuffer));
					ImGui::CloseCurrentPopup();
				}, EditorUIContext::m_UIPresets.m_LargeConfirmButton, false, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);

			ImGui::Separator();

			EditorUIContext::Spacing(SpacingAmount::Small);

			ImGui::BeginChildEx("##", GetNextChildID(),
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
						ImGui::PushStyleColor(ImGuiCol_Button, EditorUIContext::m_ConfigColors.m_SelectedColor);
					}

					if (ImGui::Button((option.m_Label.CString() + m_WidgetIDString + std::string(option.m_Handle)).c_str()))
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
				EditorUIContext::Spacing(SpacingAmount::Medium);
			}

			ImGui::EndChild();

			ImGui::PopFont();
			ImGui::EndPopup();
		}
	}
	OptionMap SelectOptionWidget::GenerateSearchCache(OptionMap& originalList, const char* searchQuery)
	{
		OptionMap returnList{};
		for (auto& [title, options] : originalList)
		{
			std::vector<OptionEntry> returnOptions{};
			for (auto& option : options)
			{
				if (!Utility::Regex::GetMatchSuccess(option.m_Label.CString(), searchQuery, false))
				{
					continue;
				}
				returnOptions.push_back(option);
			}
			if (!returnOptions.empty())
			{
				returnList.insert_or_assign(title, returnOptions);
			}
		}
		return returnList;
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
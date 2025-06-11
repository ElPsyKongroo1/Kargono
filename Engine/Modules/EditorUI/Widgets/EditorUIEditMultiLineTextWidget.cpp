#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUIEditMultiLineTextWidget.h"
#include "Modules/EditorUI/EditorUIContext.h"

namespace Kargono::EditorUI
{
	void EditMultiLineTextWidget::RenderText()
	{
		// Local Variables
		uint32_t widgetCount{ 0 };
		FixedString<16> id{ "##" };
		id.AppendInteger(m_WidgetID);
		std::string popUpLabel = m_Label.CString();

		if (m_Flags & EditMultiLineText_PopupOnly)
		{
			if (m_StartPopup)
			{
				ImGui::OpenPopup(id);
				m_StartPopup = false;
				memset(m_Buffer.Data(), 0, m_Buffer.BufferSize());
				memcpy(m_Buffer.Data(), m_CurrentOption.data(), m_CurrentOption.size());
			}
		}
		else
		{
			if (m_Flags & EditMultiLineText_Indented)
			{
				ImGui::SetCursorPosX(EditorUIService::s_TextLeftIndentOffset);
			}
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::m_ConfigColors.s_PrimaryTextColor);
			int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
				m_Flags & EditMultiLineText_Indented ? EditorUIService::s_PrimaryTextIndentedWidth : EditorUIService::s_PrimaryTextWidth);
			EditorUIService::TruncateText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::m_ConfigColors.s_SecondaryTextColor);
			EditorUIService::WriteMultilineText(m_CurrentOption, EditorUIService::s_SecondaryTextLargeWidth, EditorUIService::s_SecondaryTextPosOne);
			ImGui::PopStyleColor();

			ImGui::SameLine();
			EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
			{
				ImGui::OpenPopup(id);
				memset(m_Buffer.Data(), 0, m_Buffer.BufferSize());
				memcpy(m_Buffer.Data(), m_CurrentOption.data(), m_CurrentOption.size());
			},
			EditorUIService::s_SmallEditButton, false, EditorUIService::m_ConfigColors.s_DisabledColor);
		}

		ImGuiWindowFlags popupFlags
		{
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollWithMouse
		};

		ImGui::SetNextWindowSize(ImVec2(700.0f, 500.0f));
		if (ImGui::BeginPopupModal(id, NULL, popupFlags))
		{
			EditorUIService::TitleText(popUpLabel);

			ImGui::PushFont(EditorUI::EditorUIService::m_ConfigFonts.m_HeaderRegular);

			// Cancel Tool Bar Button
			ImGui::SameLine();
			EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
				{
					memset(m_Buffer.Data(), 0, m_Buffer.BufferSize());
					ImGui::CloseCurrentPopup();
				}, EditorUIService::s_LargeCancelButton);

			// Confirm Tool Bar Button
			ImGui::SameLine();
			EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
				{
					m_CurrentOption = m_Buffer;
					if (m_ConfirmAction)
					{
						m_ConfirmAction(*this);
					}
					memset(m_Buffer.Data(), 0, m_Buffer.BufferSize());
					ImGui::CloseCurrentPopup();
				}, EditorUIService::s_LargeConfirmButton);

			ImGui::Separator();

			//ImGui::SetNextItemWidth(0.0f);

			ImGui::InputTextMultiline((id + "InputText").c_str(), (char*)m_Buffer.Data(), m_Buffer.BufferSize(), ImVec2(683.0f, 450.0f));
			ImGui::PopFont();
			ImGui::EndPopup();
		}
	}
}
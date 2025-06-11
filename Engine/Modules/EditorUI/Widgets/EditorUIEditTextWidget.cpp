#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUICheckbox.h"
#include "Modules/EditorUI/EditorUIContext.h"

#include "Kargono/Utility/Operations.h"
#include "EditorUIEditTextWidget.h"

namespace Kargono::EditorUI
{
	void EditTextSpec::RenderText()
	{
		// Local Variables
		uint32_t widgetCount{ 0 };
		FixedString<16> id{ "##" };
		id.AppendInteger(m_WidgetID);
		std::string popUpLabel = m_Label.CString();

		// Allow opening the edit text popup externally
		if (m_StartPopup)
		{
			ImGui::OpenPopup(id);
			m_StartPopup = false;
			memset(m_Buffer.Data(), 0, m_Buffer.BufferSize());
			memcpy(m_Buffer.Data(), m_CurrentOption.data(), m_CurrentOption.size());
		}

		if (!(m_Flags & EditText_PopupOnly))
		{
			if (m_Flags & EditText_Indented)
			{
				ImGui::SetCursorPosX(EditorUIService::s_TextLeftIndentOffset);
			}
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::m_ConfigColors.s_PrimaryTextColor);
			int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
				m_Flags & EditText_Indented ? EditorUIService::s_PrimaryTextIndentedWidth : EditorUIService::s_PrimaryTextWidth);
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

		ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f));
		if (ImGui::BeginPopupModal(id, NULL, ImGuiWindowFlags_NoTitleBar))
		{
			EditorUI::EditorUIService::TitleText(popUpLabel);

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

			ImGui::SetNextItemWidth(583.0f);
			ImGui::InputText((id + "InputText").c_str(), (char*)m_Buffer.Data(), m_Buffer.BufferSize());
			ImGui::PopFont();
			ImGui::EndPopup();
		}
	}
}
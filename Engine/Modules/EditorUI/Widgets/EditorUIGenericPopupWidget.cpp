#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUIGenericPopupWidget.h"
#include "Modules/EditorUI/EditorUIContext.h"

namespace Kargono::EditorUI
{
	void GenericPopupWidget::RenderPopup()
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(m_WidgetID);
		uint32_t widgetCount{ 0 };

		if (m_OpenPopup)
		{
			ImGui::OpenPopup(id);
			m_OpenPopup = false;
			m_CloseActivePopup = false;

			if (m_PopupAction)
			{
				m_PopupAction();
			}
		}

		// Display Popup
		ImGui::SetNextWindowSize(ImVec2(m_PopupWidth, 0.0f));
		if (ImGui::BeginPopupModal(id, NULL, ImGuiWindowFlags_NoTitleBar))
		{
			// Close popup externally
			if (m_CloseActivePopup)
			{
				ImGui::CloseCurrentPopup();
			}

			EditorUIService::RecalculateWindowDimensions();
			EditorUIService::TitleText(m_Label.CString());

			ImGui::PushFont(EditorUIService::m_ConfigFonts.m_HeaderRegular);

			// Optional Delete Tool Bar Button
			if (m_DeleteAction)
			{
				ImGui::SameLine();
				EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
					{
						if (m_DeleteAction)
						{
							m_DeleteAction();
						}
						ImGui::CloseCurrentPopup();
					}, EditorUIService::s_LargeDeleteButton, false, EditorUIService::m_ConfigColors.s_PrimaryTextColor);
			}

			// Cancel Tool Bar Button
			ImGui::SameLine();
			EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
				{
					if (m_CancelAction)
					{
						m_CancelAction();
					}
					ImGui::CloseCurrentPopup();
				}, EditorUIService::s_LargeCancelButton, false, EditorUIService::m_ConfigColors.s_PrimaryTextColor);

			// Confirm Tool Bar Button
			ImGui::SameLine();
			EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
				{
					if (m_ConfirmAction)
					{
						m_ConfirmAction();
					}
					ImGui::CloseCurrentPopup();
				}, EditorUIService::s_LargeConfirmButton, false, EditorUIService::m_ConfigColors.s_PrimaryTextColor);
			ImGui::PopFont();

			ImGui::Separator();

			EditorUIService::Spacing(SpacingAmount::Small);

			ImVec4 cachedBackgroundColor{ EditorUIService::s_ActiveBackgroundColor };
			EditorUIService::s_ActiveBackgroundColor = EditorUIService::m_ConfigColors.s_BackgroundColor;

			if (m_PopupContents)
			{
				m_PopupContents();
			}

			EditorUIService::Spacing(SpacingAmount::Small);

			EditorUIService::s_ActiveBackgroundColor = cachedBackgroundColor;
			ImGui::EndPopup();
			EditorUIService::RecalculateWindowDimensions();
		}
	}
	void GenericPopupWidget::CloseActivePopup()
	{
		m_CloseActivePopup = true;
	}
}
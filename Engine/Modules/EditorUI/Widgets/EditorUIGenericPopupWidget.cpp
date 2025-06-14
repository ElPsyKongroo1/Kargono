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

			EditorUIContext::m_ActiveWindowData.RecalculateDimensions();
			EditorUIContext::TitleText(m_Label.CString());

			ImGui::PushFont(EditorUIContext::m_ConfigFonts.m_HeaderRegular);

			// Optional Delete Tool Bar Button
			if (m_DeleteAction)
			{
				ImGui::SameLine();
				EditorUIContext::RenderInlineButton(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount), [&]()
					{
						if (m_DeleteAction)
						{
							m_DeleteAction();
						}
						ImGui::CloseCurrentPopup();
					}, EditorUIContext::m_UIPresets.m_LargeDeleteButton, false, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);
			}

			// Cancel Tool Bar Button
			ImGui::SameLine();
			EditorUIContext::RenderInlineButton(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount), [&]()
				{
					if (m_CancelAction)
					{
						m_CancelAction();
					}
					ImGui::CloseCurrentPopup();
				}, EditorUIContext::m_UIPresets.m_LargeCancelButton, false, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);

			// Confirm Tool Bar Button
			ImGui::SameLine();
			EditorUIContext::RenderInlineButton(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount), [&]()
				{
					if (m_ConfirmAction)
					{
						m_ConfirmAction();
					}
					ImGui::CloseCurrentPopup();
				}, EditorUIContext::m_UIPresets.m_LargeConfirmButton, false, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);
			ImGui::PopFont();

			ImGui::Separator();

			EditorUIContext::Spacing(SpacingAmount::Small);

			ImVec4 cachedBackgroundColor{ EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor };
			EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor = EditorUIContext::m_ConfigColors.m_BackgroundColor;

			if (m_PopupContents)
			{
				m_PopupContents();
			}

			EditorUIContext::Spacing(SpacingAmount::Small);

			EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor = cachedBackgroundColor;
			ImGui::EndPopup();
			EditorUIContext::m_ActiveWindowData.RecalculateDimensions();
		}
	}
	void GenericPopupWidget::CloseActivePopup()
	{
		m_CloseActivePopup = true;
	}
}
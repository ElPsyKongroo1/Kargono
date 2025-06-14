#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUIWarningPopupWidget.h"
#include "Modules/EditorUI/EditorUIContext.h"

namespace Kargono::EditorUI
{
	void WarningPopupWidget::RenderPopup()
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(m_WidgetID);
		uint32_t widgetCount{ 0 };

		if (m_OpenPopup)
		{
			ImGui::OpenPopup(id);
			m_OpenPopup = false;
		}

		// Display Popup
		ImGui::SetNextWindowSize(ImVec2(m_PopupWidth, 0.0f));
		if (ImGui::BeginPopupModal(id, NULL, ImGuiWindowFlags_NoTitleBar))
		{
			EditorUIContext::m_ActiveWindowData.RecalculateDimensions();
			EditorUIContext::TitleText(m_Label.CString());

			ImGui::PushFont(EditorUIContext::m_ConfigFonts.m_HeaderRegular);

			// Confirm Tool Bar Button
			ImGui::SameLine();
			EditorUIContext::RenderInlineButton(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount), [&]()
				{
					ImGui::CloseCurrentPopup();
				}, EditorUIContext::m_UIPresets.m_LargeConfirmButton, false, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);

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

			ImGui::PopFont();
			ImGui::EndPopup();
			EditorUIContext::m_ActiveWindowData.RecalculateDimensions();
		}
	}
}
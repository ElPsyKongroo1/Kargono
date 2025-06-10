#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUIWarningPopupWidget.h"
#include "Modules/EditorUI/EditorUI.h"

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
			EditorUIService::RecalculateWindowDimensions();
			EditorUIService::TitleText(m_Label.CString());

			ImGui::PushFont(EditorUIService::s_FontAntaRegular);

			// Confirm Tool Bar Button
			ImGui::SameLine();
			EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
				{
					ImGui::CloseCurrentPopup();
				}, EditorUIService::s_LargeConfirmButton, false, EditorUIService::s_PrimaryTextColor);

			ImGui::Separator();

			EditorUIService::Spacing(SpacingAmount::Small);

			ImVec4 cachedBackgroundColor{ EditorUIService::s_ActiveBackgroundColor };
			EditorUIService::s_ActiveBackgroundColor = EditorUIService::s_BackgroundColor;

			if (m_PopupContents)
			{
				m_PopupContents();
			}

			EditorUIService::Spacing(SpacingAmount::Small);

			EditorUIService::s_ActiveBackgroundColor = cachedBackgroundColor;

			ImGui::PopFont();
			ImGui::EndPopup();
			EditorUIService::RecalculateWindowDimensions();
		}
	}
}
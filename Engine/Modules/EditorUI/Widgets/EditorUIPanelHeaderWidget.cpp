#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUIPanelHeaderWidget.h"
#include "Modules/EditorUI/EditorUIContext.h"

#include "Kargono/Utility/Operations.h"

namespace Kargono::EditorUI
{
	void PanelHeaderWidget::RenderHeader()
	{
		FixedString<16> id{ "##" };
		id.AppendInteger(m_WidgetID);
		ImGui::PushFont(EditorUIService::m_ConfigFonts.m_HeaderLarge);
		ImGui::PushStyleColor(ImGuiCol_Text, m_EditColorActive ? EditorUIService::m_ConfigColors.s_HighlightColor2 : EditorUIService::m_ConfigColors.s_PrimaryTextColor);
		ImGui::TextUnformatted(m_Label.CString());
		ImGui::PopStyleColor();
		ImGui::PopFont();

		if (m_SelectionsList.size() > 0)
		{
			ImGui::SameLine();
			EditorUIService::CreateButton(m_WidgetID, [&]()
				{
					ImGui::OpenPopup(id);
				}, EditorUIService::s_MediumOptionsButton, false, EditorUIService::m_ConfigColors.s_DisabledColor);

			if (ImGui::BeginPopup(id))
			{
				for (auto& [label, func] : GetSelectionList())
				{
					if (ImGui::Selectable((label.c_str() + id).c_str()))
					{
						func();
					}
				}
				ImGui::EndPopup();
			}
		}
		ImGui::Separator(1.0f, EditorUIService::m_ConfigColors.s_HighlightColor1_Thin);
		EditorUIService::Spacing(0.2f);
	}
	void PanelHeaderWidget::ClearSelectionList()
	{
		m_SelectionsList.clear();
	}
	void PanelHeaderWidget::AddToSelectionList(const std::string& label, std::function<void()> function)
	{
		if (!m_SelectionsList.contains(label))
		{
			m_SelectionsList.insert_or_assign(label, function);
			return;
		}
	}
}
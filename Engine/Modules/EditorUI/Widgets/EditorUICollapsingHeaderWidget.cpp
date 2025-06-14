#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUICollapsingHeaderWidget.h"
#include "Modules/EditorUI/EditorUIContext.h"

#include "Modules/EditorUI/ExternalAPI/ImGuiBackendAPI.h"
#include "Kargono/Utility/Operations.h"

namespace Kargono::EditorUI
{
	void Kargono::EditorUI::CollapsingHeaderWidget::RenderHeader()
	{
		ResetChildID();

		ImGui::PushFont(EditorUIContext::m_ConfigFonts.m_HeaderLarge);
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);
		ImGui::TextUnformatted(m_Label.CString());
		ImGui::PopStyleColor();
		ImGui::PopFont();
		ImGui::SameLine();
		EditorUIContext::RenderInlineButton(GetNextChildID(), [&]()
			{
				Utility::Operations::ToggleBoolean(m_Expanded);
			},
			EditorUIContext::m_UIPresets.m_SmallExpandButton, m_Expanded, m_Expanded ? EditorUIContext::m_ConfigColors.m_HighlightColor1 : EditorUIContext::m_ConfigColors.m_DisabledColor);

		if (m_Expanded && !m_SelectionList.empty())
		{
			ImGui::SameLine();
			EditorUIContext::RenderInlineButton(GetNextChildID(), [&]()
				{
					ImGui::OpenPopup(m_WidgetID - 1);
				}, EditorUIContext::m_UIPresets.m_MediumOptionsButton, false, EditorUIContext::m_ConfigColors.m_DisabledColor);

			if (ImGui::BeginPopupEx(m_WidgetID - 1, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
			{
				for (auto& [label, func] : m_SelectionList)
				{
					if (ImGui::Selectable((label.c_str() + m_WidgetIDString).c_str()))
					{
						func(*this);
					}
				}
				ImGui::EndPopup();
			}
		}

		if ((m_Flags & CollapsingHeader_UnderlineTitle) && m_Expanded)
		{
			ImGui::Separator();
		}

		if (m_Expanded && m_OnExpand)
		{
			m_OnExpand();
		}
	}
	void CollapsingHeaderWidget::AddToSelectionList(const std::string& label, std::function<void(CollapsingHeaderWidget&)> function)
	{
		if (!m_SelectionList.contains(label))
		{
			m_SelectionList.insert_or_assign(label, function);
			return;
		}
	}
	void CollapsingHeaderWidget::ClearSelectionList()
	{
		m_SelectionList.clear();
	}
}
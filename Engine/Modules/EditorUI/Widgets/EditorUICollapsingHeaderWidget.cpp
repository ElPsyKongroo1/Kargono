#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUICollapsingHeaderWidget.h"
#include "Modules/EditorUI/EditorUI.h"

#include "Modules/EditorUI/ExternalAPI/ImGuiBackendAPI.h"
#include "Kargono/Utility/Operations.h"

namespace Kargono::EditorUI
{
	void Kargono::EditorUI::CollapsingHeaderWidget::RenderHeader()
	{
		uint32_t widgetCount{ 0 };
		FixedString<16> id{ "##" };
		id.AppendInteger(m_WidgetID);
		ImGui::PushFont(EditorUIService::s_FontAntaLarge);
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::s_PrimaryTextColor);
		ImGui::TextUnformatted(m_Label.CString());
		ImGui::PopStyleColor();
		ImGui::PopFont();
		ImGui::SameLine();
		EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
			{
				Utility::Operations::ToggleBoolean(m_Expanded);
			},
			EditorUIService::s_SmallExpandButton, m_Expanded, m_Expanded ? EditorUIService::s_HighlightColor1 : EditorUIService::s_DisabledColor);

		if (m_Expanded && !m_SelectionList.empty())
		{
			ImGui::SameLine();
			EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
				{
					ImGui::OpenPopup(m_WidgetID - 1);
				}, EditorUIService::s_MediumOptionsButton, false, EditorUIService::s_DisabledColor);

			if (ImGui::BeginPopupEx(m_WidgetID - 1, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
			{
				for (auto& [label, func] : m_SelectionList)
				{
					if (ImGui::Selectable((label.c_str() + id).c_str()))
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
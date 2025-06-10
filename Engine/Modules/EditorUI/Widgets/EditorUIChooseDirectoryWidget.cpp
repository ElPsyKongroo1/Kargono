#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUIChooseDirectoryWidget.h"
#include "Modules/EditorUI/EditorUI.h"

#include "Kargono/Utility/FileDialogs.h"

namespace Kargono::EditorUI
{
	void ChooseDirectoryWidget::RenderChooseDir()
	{
		// Local Variables
		uint32_t widgetCount{ 0 };
		std::string popUpLabel = m_Label.CString();

		// Display Menu Item
		if (m_Flags & ChooseDirectory_Indented)
		{
			ImGui::SetCursorPosX(EditorUIService::s_TextLeftIndentOffset);
		}

		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(), EditorUIService::s_PrimaryTextWidth);
		EditorUIService::TruncateText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::s_SecondaryTextColor);
		EditorUIService::WriteMultilineText(m_CurrentOption.string(), EditorUIService::s_SecondaryTextLargeWidth, EditorUIService::s_SecondaryTextPosOne);
		ImGui::PopStyleColor();

		ImGui::SameLine();
		EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
			{
				const std::filesystem::path initialDirectory = m_CurrentOption.empty() ? std::filesystem::current_path() : m_CurrentOption;
				std::filesystem::path outputDirectory = Utility::FileDialogs::ChooseDirectory(initialDirectory);
				if (outputDirectory.empty())
				{
					KG_WARN("Empty path returned to ChooseDirectory");
					return;
				}
				m_CurrentOption = outputDirectory;
				if (m_ConfirmAction)
				{
					m_ConfirmAction(outputDirectory.string());
				}
			},
			EditorUIService::s_SmallEditButton, false, EditorUIService::s_DisabledColor);
	}
}
#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUIChooseDirectoryWidget.h"
#include "Modules/EditorUI/EditorUIContext.h"

#include "Kargono/Utility/FileDialogs.h"

namespace Kargono::EditorUI
{
	void ChooseDirectoryWidget::RenderChooseDir()
	{
		ResetChildID();

		// Display Menu Item
		if (m_Flags & ChooseDirectory_Indented)
		{
			ImGui::SetCursorPosX(EditorUIContext::m_ConfigSpacing.m_PrimaryTextIndent);
		}

		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(), EditorUIContext::m_ActiveWindowData.m_PrimaryTextWidth);
		EditorUIContext::RenderTruncatedText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_SecondaryTextColor);
		EditorUIContext::RenderMultiLineText(m_CurrentOption.string(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextLargeWidth, EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne);
		ImGui::PopStyleColor();

		ImGui::SameLine();
		EditorUIContext::RenderInlineButton(GetNextChildID(), [&]()
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
			EditorUIContext::m_UIPresets.m_SmallEditButton, false, EditorUIContext::m_ConfigColors.m_DisabledColor);
	}
}
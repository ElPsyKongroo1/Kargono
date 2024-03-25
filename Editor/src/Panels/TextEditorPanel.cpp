#include "Panels/TextEditorPanel.h"

#include "imgui_internal.h"
#include "Kargono.h"

namespace Kargono
{
	std::function<void()> s_OnOpenFile { nullptr };
	std::function<void()> s_OnCreateFile { nullptr };

	TextEditorPanel::TextEditorPanel()
	{
		s_OnOpenFile = [&]()
		{
			
		};
		s_OnCreateFile = [&]()
		{

		};
	}
	void TextEditorPanel::OnEditorUIRender()
	{
		UI::Editor::StartWindow("Text Editor", ImGuiWindowFlags_MenuBar);

		static char stringBuffer[4096];
		// Set up Menu Toolbar
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New File"))
				{
					// Load a new screen
				}

				if (ImGui::MenuItem("Open File"))
				{
					memset(stringBuffer, 0, sizeof(stringBuffer));
					std::string fileString = FileSystem::ReadFileString("asdf.txt");
					memcpy(stringBuffer, fileString.data(), fileString.size());
				}

				if (ImGui::MenuItem("Save File"))
				{
					FileSystem::WriteFileString("asdf.txt", stringBuffer);
				}
				ImGui::EndMenu();

			}

			ImGui::EndMenuBar();
		}

		ImGui::BeginTabBar("##TextTabBar");

		if (ImGui::BeginTabItem("Barkon.txt"))
		{
#if 0
			ImGui::PushFont(UI::Editor::s_AntaLarge);
			ImGui::Text("Barkon.txt");
			ImGui::PopFont();

			ImGui::SameLine(ImGui::GetWindowWidth() - 38);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			if (ImGui::ImageButtonEx(2342652332432,
				(ImTextureID)(uint64_t)UI::Editor::s_IconAddItem->GetRendererID(),
				ImVec2(20, 20), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
				ImVec4(0.0f, 0.0f, 0.0f, 0.0f),
				UI::Editor::s_PureWhite, 0))
			{

			}
			ImGui::PopStyleColor();

			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(UI::Editor::s_PearlBlue, false ? "Cancel Editing" : "Edit");
				ImGui::EndTooltip();
			}
#endif
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f);
			ImGui::InputTextMultiline("##textLabel", stringBuffer, sizeof(stringBuffer), ImGui::GetContentRegionAvail());

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();

		UI::Editor::EndWindow();
	}
}

#include "Panels/TextEditorPanel.h"

#include "imgui_internal.h"
#include "Kargono.h"

namespace Kargono
{
	static std::function<void()> s_OnOpenFile { nullptr };
	static std::function<void()> s_OnCreateFile { nullptr };
	static std::function<void()> s_OnSaveFile { nullptr };
	static std::function<void()> s_OnDeleteFile { nullptr };
	static std::filesystem::path s_CurrentPath { "" };
	static char stringBuffer[4096];

	static UI::WarningMessageSpec s_DeleteWarningSpec {};

	TextEditorPanel::TextEditorPanel()
	{
		s_OnCreateFile = [&]()
		{
			std::filesystem::path initialDirectory = Projects::Project::GetAssetDirectory();
			std::filesystem::path filepath = Utility::FileDialogs::SaveFile("", initialDirectory.string().c_str());
			if (!filepath.empty())
			{
				memset(stringBuffer, 0, sizeof(stringBuffer));
				FileSystem::WriteFileString(filepath, "");
				s_CurrentPath = filepath;
			}
		};
		s_OnOpenFile = [&]()
		{
			std::filesystem::path initialDirectory = Projects::Project::GetAssetDirectory();
			std::filesystem::path filepath = Utility::FileDialogs::OpenFile("", initialDirectory.string().c_str());
			if (!filepath.empty())
			{
				memset(stringBuffer, 0, sizeof(stringBuffer));
				std::string fileString = FileSystem::ReadFileString(filepath);
				memcpy(stringBuffer, fileString.data(), fileString.size());
				s_CurrentPath = filepath;
			}
		};
		s_OnSaveFile = [&]()
		{
			FileSystem::WriteFileString(s_CurrentPath, stringBuffer);
		};
		s_OnDeleteFile = [&]()
		{
			if (FileSystem::DeleteSelectedFile(s_CurrentPath))
			{
				memset(stringBuffer, 0, sizeof(stringBuffer));
				s_CurrentPath = "";
			}
			
		};


		s_DeleteWarningSpec.Label = "Delete File";
		s_DeleteWarningSpec.WidgetID = 0x7e3a1a046d694789;
		s_DeleteWarningSpec.ConfirmAction = s_OnDeleteFile;
		s_DeleteWarningSpec.WarningContents = [&]()
		{
			//UI::Editor::TitleText("Are you sure you want to delete this file?");
			ImGui::Text("Are you sure you want to delete this file?");
		};
	}
	void TextEditorPanel::OnEditorUIRender()
	{
		UI::Editor::StartWindow("Text Editor", ImGuiWindowFlags_MenuBar);

		if (s_CurrentPath == "")
		{
			UI::Editor::NewItemScreen("Open Existing File", s_OnOpenFile, "Create New File", s_OnCreateFile);
		}
		else
		{
			// Set up Menu Toolbar
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("New File"))
					{
						s_OnCreateFile();
					}

					if (ImGui::MenuItem("Open File"))
					{
						s_OnOpenFile();
					}

					if (ImGui::MenuItem("Save File"))
					{
						s_OnSaveFile();
					}
					if (ImGui::MenuItem("Delete File"))
					{
						s_DeleteWarningSpec.PopupActive = true;
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
			UI::Editor::WarningMessage(s_DeleteWarningSpec);

			ImGui::BeginTabBar("##TextTabBar");

			if (ImGui::BeginTabItem(s_CurrentPath.filename().string().c_str()))
			{
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f);
				ImGui::InputTextMultiline("##textLabel", stringBuffer, sizeof(stringBuffer), ImGui::GetContentRegionAvail(), ImGuiInputTextFlags_AllowTabInput);

				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		

		UI::Editor::EndWindow();
	}
}

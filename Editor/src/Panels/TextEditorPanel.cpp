#include "Panels/TextEditorPanel.h"

#include "imgui_internal.h"
#include "Kargono.h"

namespace Kargono
{
	static std::function<void()> s_OnOpenFile { nullptr };
	static std::function<void()> s_OnCreateFile { nullptr };
	static std::function<void()> s_OnSaveFile { nullptr };
	static std::function<void()> s_OnDeleteFile { nullptr };
	static std::function<void()> s_OnCloseFile { nullptr };
	static std::filesystem::path s_CurrentPath { "" };
	static char s_StringBuffer[4096];
	static bool s_DocumentEdited{ false };
	static bool s_DocumentOpen{ false };

	static UI::WarningMessageSpec s_DeleteWarningSpec {};

	TextEditorPanel::TextEditorPanel()
	{
		s_OnCreateFile = [&]()
		{
			std::filesystem::path initialDirectory = Projects::Project::GetAssetDirectory();
			std::filesystem::path filepath = Utility::FileDialogs::SaveFile("", initialDirectory.string().c_str());
			if (!filepath.empty())
			{
				memset(s_StringBuffer, 0, sizeof(s_StringBuffer));
				FileSystem::WriteFileString(filepath, "");
				s_CurrentPath = filepath;
				s_DocumentEdited = false;
				s_DocumentOpen = true;
			}
		};
		s_OnOpenFile = [&]()
		{
			std::filesystem::path initialDirectory = Projects::Project::GetAssetDirectory();
			std::filesystem::path filepath = Utility::FileDialogs::OpenFile("", initialDirectory.string().c_str());
			if (!filepath.empty())
			{
				memset(s_StringBuffer, 0, sizeof(s_StringBuffer));
				std::string fileString = FileSystem::ReadFileString(filepath);
				memcpy(s_StringBuffer, fileString.data(), fileString.size());
				s_CurrentPath = filepath;
				s_DocumentEdited = false;
				s_DocumentOpen = true;
			}
		};
		s_OnSaveFile = [&]()
		{
			FileSystem::WriteFileString(s_CurrentPath, s_StringBuffer);
			s_DocumentEdited = false;
		};
		s_OnDeleteFile = [&]()
		{
			if (FileSystem::DeleteSelectedFile(s_CurrentPath))
			{
				memset(s_StringBuffer, 0, sizeof(s_StringBuffer));
				s_CurrentPath = "";
				s_DocumentEdited = false;
				s_DocumentOpen = false;
			}
		};

		s_OnCloseFile = [&]()
		{
			memset(s_StringBuffer, 0, sizeof(s_StringBuffer));
			s_CurrentPath = "";
			s_DocumentEdited = false;
			s_DocumentOpen = false;
		};


		s_DeleteWarningSpec.Label = "Delete File";
		s_DeleteWarningSpec.WidgetID = 0x7e3a1a046d694789;
		s_DeleteWarningSpec.ConfirmAction = s_OnDeleteFile;
		s_DeleteWarningSpec.WarningContents = [&]()
		{
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
			bool tabColorModifed = false;
			if (s_DocumentEdited)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, UI::Editor::s_PearlBlue);
				tabColorModifed = true;
			}
			if (ImGui::BeginTabItem(s_CurrentPath.filename().string().c_str(), &s_DocumentOpen))
			{
				ImGui::PushStyleColor(ImGuiCol_Text, UI::Editor::s_PureWhite);
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f);
				ImGui::PushFont(UI::Editor::s_RobotoRegular);
				if (ImGui::InputTextMultiline("##textLabel", s_StringBuffer,
					sizeof(s_StringBuffer), ImGui::GetContentRegionAvail(),
					ImGuiInputTextFlags_AllowTabInput))
				{
					s_DocumentEdited = true;
				}
				ImGui::PopStyleColor();
				ImGui::PopFont();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
			if (tabColorModifed)
			{
				ImGui::PopStyleColor();
			}
			if (!s_CurrentPath.empty() && !s_DocumentOpen)
			{
				s_OnCloseFile();
			}
		}

		UI::Editor::EndWindow();
	}
}

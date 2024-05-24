#include "Panels/TextEditorPanel.h"

#include "EditorApp.h"
#include "API/EditorUI/ImGuiBackendAPI.h"
#include "Kargono.h"

namespace Kargono
{
	struct Document
	{
		Buffer TextBuffer{};
		std::filesystem::path FilePath { "" };
		bool Edited{ false };
		bool Opened{ false };
		bool SetActive{ false };
	};

	static EditorApp* s_EditorLayer { nullptr };

	static uint32_t s_ActiveDocument = 0;
	static std::vector<Document> s_AllDocuments {};
	static std::function<void()> s_OnOpenFile { nullptr };
	static std::function<void()> s_OnCreateFile { nullptr };
	static std::function<void()> s_OnSaveFile { nullptr };
	static std::function<void()> s_OnDeleteFile { nullptr };
	static std::function<void()> s_OnCloseFile { nullptr };

	static EditorUI::GenericPopupSpec s_DeleteWarningSpec {};
	static EditorUI::GenericPopupSpec s_DiscardChangesWarningSpec {};
	
	TextEditorPanel::TextEditorPanel()
	{
		s_EditorLayer = EditorApp::GetCurrentLayer();

		s_OnCreateFile = [&]()
		{
			const std::filesystem::path initialDirectory = Projects::Project::GetAssetDirectory();
			const std::filesystem::path filepath = Utility::FileDialogs::SaveFile("All Files\0*.*\0", initialDirectory.string().c_str());
			if (!filepath.empty())
			{
				Document newDocument{};
				newDocument.TextBuffer.Allocate(4096);
				newDocument.TextBuffer.SetDataToByte(0);
				Utility::FileSystem::WriteFileString(filepath, "");
				newDocument.FilePath = filepath;
				newDocument.Edited = false;
				newDocument.Opened = true;
				newDocument.SetActive = true;
				s_AllDocuments.push_back(newDocument);
				s_ActiveDocument = static_cast<uint32_t>(s_AllDocuments.size() - 1);
			}
		};
		s_OnOpenFile = [&]()
		{
			const std::filesystem::path initialDirectory = Projects::Project::GetAssetDirectory();
			const std::filesystem::path filepath = Utility::FileDialogs::OpenFile("All Files\0*.*\0", initialDirectory.string().c_str());
			OpenFile(filepath);
		};
		s_OnSaveFile = [&]()
		{
			Document& activeDocument = s_AllDocuments.at(s_ActiveDocument);

			Utility::FileSystem::WriteFileString(activeDocument.FilePath, std::string(activeDocument.TextBuffer.As<char>()));
			//Utility::FileSystem::WriteFileBinary(activeDocument.FilePath, activeDocument.TextBuffer);
			activeDocument.Edited = false;
		};
		s_OnDeleteFile = [&]()
		{
			Document& activeDocument = s_AllDocuments.at(s_ActiveDocument);
			if (Utility::FileSystem::DeleteSelectedFile(activeDocument.FilePath))
			{
				activeDocument.TextBuffer.Release();
				s_AllDocuments.erase(s_AllDocuments.begin() + s_ActiveDocument);
				s_ActiveDocument = 0;
				if (s_AllDocuments.size() > 0)
				{
					s_AllDocuments.at(0).SetActive = true;
				}
			}
		};
		s_OnCloseFile = [&]()
		{
			Document& activeDocument = s_AllDocuments.at(s_ActiveDocument);
			activeDocument.TextBuffer.Release();
			s_AllDocuments.erase(s_AllDocuments.begin() + s_ActiveDocument);
			s_ActiveDocument = 0;
			if (s_AllDocuments.size() > 0)
			{
				s_AllDocuments.at(0).SetActive = true;
			}
		};

		s_DeleteWarningSpec.Label = "Delete File";
		s_DeleteWarningSpec.ConfirmAction = s_OnDeleteFile;
		s_DeleteWarningSpec.PopupContents = [&]()
		{
			ImGui::Text("Are you sure you want to delete this file?");
		};

		s_DiscardChangesWarningSpec.Label = "Close File";
		s_DiscardChangesWarningSpec.ConfirmAction = s_OnCloseFile;
		s_DiscardChangesWarningSpec.PopupContents = [&]()
		{
			ImGui::Text("Are you sure you want to close this file without saving?");
		};


	}
	void TextEditorPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		ImGuiWindowFlags flags = 0;
		if (s_AllDocuments.size() != 0)
		{
			flags |= ImGuiWindowFlags_MenuBar;
		}
		EditorUI::Editor::StartWindow("Text Editor", &s_EditorLayer->m_ShowTextEditor, flags);

		if (s_AllDocuments.size() == 0)
		{
			EditorUI::Editor::NewItemScreen("Open Existing File", s_OnOpenFile, "Create New File", s_OnCreateFile);
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
			EditorUI::Editor::GenericPopup(s_DeleteWarningSpec);
			EditorUI::Editor::GenericPopup(s_DiscardChangesWarningSpec);

			ImGui::BeginTabBar("##TextTabBar", ImGuiTabBarFlags_AutoSelectNewTabs);
			uint32_t iteration{ 0 };
			for (auto& document : s_AllDocuments)
			{
				bool setColorBlue = false;
				if (document.Edited)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, EditorUI::Editor::s_PearlBlue);
					setColorBlue = true;
				}

				ImGuiTabItemFlags tabItemFlags = 0;
				if (document.SetActive)
				{
					tabItemFlags |= ImGuiTabItemFlags_SetSelected;
					document.SetActive = false;
				}

				if (ImGui::BeginTabItem((document.FilePath.filename().string() + "##" + std::to_string(iteration)).c_str(),
					&document.Opened, tabItemFlags))
				{
					s_ActiveDocument = iteration;
					ImGui::PushStyleColor(ImGuiCol_Text, EditorUI::Editor::s_PureWhite);
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f);

					ImGuiInputTextCallback typeCallback = [](ImGuiInputTextCallbackData* data)
					{
						if ((float)data->BufTextLen / (float)data->BufSize > 0.75f)
						{
							Document& activeDocument	= *(Document*)data->UserData;
							Buffer newBuffer{};
							newBuffer.Allocate(activeDocument.TextBuffer.Size * 2);
							newBuffer.SetDataToByte(0);
							memcpy(newBuffer.Data, activeDocument.TextBuffer.Data, activeDocument.TextBuffer.Size);
							activeDocument.TextBuffer.Release();
							activeDocument.TextBuffer = newBuffer;
						}
						return 0;
					};

					if (ImGui::InputTextMultiline("##textLabel", document.TextBuffer.As<char>(),
						document.TextBuffer.Size, ImGui::GetContentRegionAvail(),
						ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackEdit,
						typeCallback, (void*)&document))
					{
						document.Edited = true;
					}
					ImGui::PopStyleColor();
					ImGui::EndTabItem();
				}

				if (setColorBlue)
				{
					ImGui::PopStyleColor();
				}
				iteration++;
			}

			ImGui::EndTabBar();

			if (s_AllDocuments.size() != 0)
			{
				Document& activeDocument = s_AllDocuments.at(s_ActiveDocument);

				if (!activeDocument.FilePath.empty() && !activeDocument.Opened)
				{

					if (activeDocument.Edited)
					{
						activeDocument.Opened = true;
						s_DiscardChangesWarningSpec.PopupActive = true;
					}
					else
					{
						s_OnCloseFile();
					}
				}
			}
			
		}

		EditorUI::Editor::EndWindow();
	}
	void TextEditorPanel::OpenFile(const std::filesystem::path& filepath)
	{
		if (!filepath.empty())
		{
			if (!s_EditorLayer->m_ShowTextEditor)
			{
				s_EditorLayer->m_ShowTextEditor = true;
			}

			for (auto& document : s_AllDocuments)
			{
				if (std::filesystem::equivalent(document.FilePath, filepath))
				{
					KG_WARN("Attempt to open document that is already open");
					document.SetActive = true;
					return;
				}
			}

			Document newDocument{};
			std::string fileString = Utility::FileSystem::ReadFileString(filepath);
			newDocument.TextBuffer.Allocate(static_cast<uint64_t>(fileString.size() * 1.5f));
			newDocument.TextBuffer.SetDataToByte(0);
			memcpy(newDocument.TextBuffer.Data, fileString.data(), fileString.size());
			newDocument.FilePath = filepath;
			newDocument.Edited = false;
			newDocument.Opened = true;
			newDocument.SetActive = true;
			s_AllDocuments.push_back(newDocument);
			s_ActiveDocument = static_cast<uint32_t>(s_AllDocuments.size() - 1);
		}
	}
}

#include "Panels/TextEditorPanel.h"

#include "EditorApp.h"
#include "API/EditorUI/ImGuiBackendAPI.h"
#include "API/EditorUI/ImGuiColorTextEditorAPI.h"
#include "Kargono.h"
#include "Kargono/Scripting/ScriptCompiler.h"
#include "Kargono/Utility/Timers.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{
	struct Document
	{
		std::string TextBuffer {};
		std::filesystem::path FilePath { "" };
		bool Edited{ false };
		bool Opened{ false };
		bool SetActive{ false };
	};

	static TextEditor s_TextEditor;
	static uint32_t s_ActiveDocument = 0;
	static std::vector<Document> s_AllDocuments {};
	static std::function<void()> s_OnOpenFile { nullptr };
	static std::function<void()> s_OnCreateFile { nullptr };
	static std::function<void()> s_OnSaveFile { nullptr };
	static std::function<void()> s_OnDeleteFile { nullptr };
	static std::function<void()> s_OnCloseFile { nullptr };
	static std::function<void()> s_OnCloseAllFiles { nullptr };

	static EditorUI::GenericPopupSpec s_DeleteWarningSpec {};
	static EditorUI::GenericPopupSpec s_DiscardChangesWarningSpec {};
	
	TextEditorPanel::TextEditorPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName,
			KG_BIND_CLASS_FN(TextEditorPanel::OnKeyPressedEditor));

		s_TextEditor = {};
		s_TextEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::C());
		s_TextEditor.SetPalette(TextEditor::GetCurrentColorPalette());
		s_OnCreateFile = [&]()
		{
			const std::filesystem::path initialDirectory = Projects::ProjectService::GetActiveAssetDirectory();
			const std::filesystem::path filepath = Utility::FileDialogs::SaveFile("All Files\0*.*\0", initialDirectory.string().c_str());
			if (!filepath.empty())
			{
				Document newDocument{};
				Utility::FileSystem::WriteFileString(filepath, "");
				newDocument.TextBuffer = "";
				newDocument.FilePath = filepath;
				newDocument.Edited = false;
				newDocument.Opened = true;
				newDocument.SetActive = true;
				s_AllDocuments.push_back(newDocument);
				s_ActiveDocument = static_cast<uint32_t>(s_AllDocuments.size() - 1);
				s_TextEditor.SetText("");
			}
		};
		s_OnOpenFile = [&]()
		{
			const std::filesystem::path initialDirectory = Projects::ProjectService::GetActiveAssetDirectory();
			const std::filesystem::path filepath = Utility::FileDialogs::OpenFile("All Files\0*.*\0", initialDirectory.string().c_str());
			OpenFile(filepath);
		};
		s_OnSaveFile = [&]()
		{
			Document& activeDocument = s_AllDocuments.at(s_ActiveDocument);

			Utility::Operations::RemoveCharacterFromString(activeDocument.TextBuffer, '\r');

			KG_INFO("Saving file in Text Editor: {}", activeDocument.FilePath.filename());
			Utility::FileSystem::WriteFileString(activeDocument.FilePath, activeDocument.TextBuffer);
			activeDocument.Edited = false;
		};
		s_TextEditor.SetSaveCallback(s_OnSaveFile);

		s_OnDeleteFile = [&]()
		{
			Document& activeDocument = s_AllDocuments.at(s_ActiveDocument);
			if (Utility::FileSystem::DeleteSelectedFile(activeDocument.FilePath))
			{
				activeDocument.TextBuffer = "";
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
			activeDocument.TextBuffer = "";
			s_AllDocuments.erase(s_AllDocuments.begin() + s_ActiveDocument);
			s_ActiveDocument = 0;
			if (s_AllDocuments.size() > 0)
			{
				s_AllDocuments.at(0).SetActive = true;
			}
		};

		s_OnCloseAllFiles = [&]()
		{
			s_AllDocuments.clear();
			s_ActiveDocument = 0;
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
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowTextEditor, flags);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		if (s_AllDocuments.size() == 0)
		{
			EditorUI::EditorUIService::NewItemScreen("Open Existing File", s_OnOpenFile, "Create New File", s_OnCreateFile);
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
			EditorUI::EditorUIService::GenericPopup(s_DeleteWarningSpec);
			EditorUI::EditorUIService::GenericPopup(s_DiscardChangesWarningSpec);

			ImGui::BeginTabBar("##TextTabBar", ImGuiTabBarFlags_AutoSelectNewTabs);
			uint32_t iteration{ 0 };
			
			if (s_TextEditor.IsTextChanged())
			{
				std::string focusedWindow = EditorUI::EditorUIService::GetFocusedWindowName();
				std::string comparedWindow = m_EditorWindowName;
				Document& activeDocument = s_AllDocuments.at(s_ActiveDocument);
				activeDocument.TextBuffer = s_TextEditor.GetText();
				activeDocument.Edited = true;
				if (activeDocument.FilePath.extension().string() == ".kgscript")
				{
					CheckForErrors();
				}
				else
				{
					s_TextEditor.SetErrorMarkers({});
				}
			}

			for (auto& document : s_AllDocuments)
			{
				bool setColorHighlight = false;
				if (document.Edited)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, EditorUI::EditorUIService::s_HighlightColor2);
					setColorHighlight = true;
				}
				// Handle case 
				ImGuiTabItemFlags tabItemFlags = 0;
				if (document.SetActive)
				{
					tabItemFlags |= ImGuiTabItemFlags_SetSelected;
					s_ActiveDocument = iteration;
					Document& activeDocument = s_AllDocuments.at(s_ActiveDocument);
					s_TextEditor.SetText(activeDocument.TextBuffer);
					s_TextEditor.SetLanguageDefinitionByExtension(activeDocument.FilePath.extension().string());
					document.SetActive = false;
					if (activeDocument.FilePath.extension().string() == ".kgscript")
					{
						CheckForErrors();
					}
					else
					{
						s_TextEditor.SetErrorMarkers({});
					}
				}

				bool checkTab = true;
				if (ImGui::BeginTabItem((document.FilePath.filename().string() + "##" + std::to_string(iteration)).c_str(),
					&document.Opened, tabItemFlags))
				{
					s_TextEditor.Render(m_EditorWindowName.c_str());
					checkTab = false;
					ImGui::EndTabItem();
				}

				// Handle Case of switching tabs by clicking
				if (ImGui::IsItemClicked() && checkTab)
				{
					document.SetActive = true;
				}

				if (setColorHighlight)
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

		EditorUI::EditorUIService::EndWindow();
	}
	bool TextEditorPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		bool control = Input::InputService::IsKeyPressed(Key::LeftControl) || Input::InputService::IsKeyPressed(Key::RightControl);

		switch (event.GetKeyCode())
		{
			case Key::S:
			{
				if (control)
				{
					s_OnSaveFile();
					return true;
				}
				return false;
			}
			default:
			{
				return false;
			}
		}
	}
	void TextEditorPanel::OpenFile(const std::filesystem::path& filepath)
	{
		if (!filepath.empty())
		{
			if (!s_EditorApp->m_ShowTextEditor)
			{
				s_EditorApp->m_ShowTextEditor = true;
			}

			if (EditorUI::EditorUIService::GetFocusedWindowName() != m_PanelName)
			{
				EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);
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
			newDocument.TextBuffer = Utility::FileSystem::ReadFileString(filepath);
			newDocument.FilePath = filepath;
			newDocument.Edited = false;
			newDocument.Opened = true;
			newDocument.SetActive = true;
			s_AllDocuments.push_back(newDocument);
			s_ActiveDocument = static_cast<uint32_t>(s_AllDocuments.size() - 1);
			s_TextEditor.SetText(newDocument.TextBuffer);
			s_TextEditor.SetLanguageDefinitionByExtension(newDocument.FilePath.extension().string());
		}
	}
	void TextEditorPanel::ResetPanelResources()
	{
		s_OnCloseAllFiles();
	}
	void TextEditorPanel::CheckForErrors()
	{
		static int32_t countOfTimers{0};
		countOfTimers++;
		s_TextEditor.SetErrorMarkers({});
		Utility::PassiveTimer::CreateTimer(1.2f, [&]()
		{
			countOfTimers--;
			if (countOfTimers > 0)
			{
				return;
			}
			
			if (s_AllDocuments.size() <= 0)
			{
				return;
			}
			Document& activeDocument = s_AllDocuments.at(s_ActiveDocument);
			if (activeDocument.FilePath.extension().string() != ".kgscript")
			{
				return;
			}
			std::vector<Scripting::ParserError> errors = Scripting::ScriptCompiler::CheckForErrors(activeDocument.TextBuffer);

			if (errors.size() == 0)
			{
				s_TextEditor.SetErrorMarkers({});
				return;
			}

			TextEditor::ErrorMarkers markers{};
			for (Scripting::ParserError& error : errors)
			{
				if (markers.contains(error.CurrentToken.Line))
				{
					TextEditor::ErrorMarker& existingMarker = markers.at(error.CurrentToken.Line);
					existingMarker.Description = existingMarker.Description + error.ToString();
					TextEditor::ErrorLocation newLocation;
					newLocation.Column = error.CurrentToken.Column;
					newLocation.Length = (uint32_t)error.CurrentToken.Value.size();
					existingMarker.Locations.push_back(newLocation);
					continue;
				}
				TextEditor::ErrorMarker marker;
				marker.Description = error.ToString();
				TextEditor::ErrorLocation newLocation;
				newLocation.Column = error.CurrentToken.Column;
				newLocation.Length = (uint32_t)error.CurrentToken.Value.size();
				marker.Locations.push_back(newLocation);
				markers.insert_or_assign(error.CurrentToken.Line, marker);
			}
			s_TextEditor.SetErrorMarkers(markers);
		});
	}
}

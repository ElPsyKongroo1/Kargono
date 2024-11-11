#include "Panels/TextEditorPanel.h"

#include "EditorApp.h"
#include "API/EditorUI/ImGuiBackendAPI.h"
#include "Kargono.h"
#include "Kargono/Scripting/ScriptCompilerService.h"
#include "Kargono/Utility/Timers.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{
	TextEditorPanel::TextEditorPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(TextEditorPanel::OnKeyPressedEditor));

		m_TextEditor = {};
		m_TextEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::C());
		m_TextEditor.SetPalette(TextEditor::GetCurrentColorPalette());
	
		m_TextEditor.SetSaveCallback(KG_BIND_CLASS_FN(OnSaveFile));

		m_DeleteWarningSpec.Label = "Delete File";
		m_DeleteWarningSpec.ConfirmAction = KG_BIND_CLASS_FN(OnDeleteFile);
		m_DeleteWarningSpec.PopupContents = [&]()
		{
			ImGui::Text("Are you sure you want to delete this file?");
		};

		m_DiscardChangesWarningSpec.Label = "Close File";
		m_DiscardChangesWarningSpec.ConfirmAction = KG_BIND_CLASS_FN(OnCloseFile);
		m_DiscardChangesWarningSpec.PopupContents = [&]()
		{
			ImGui::Text("Are you sure you want to close this file without saving?");
		};

	}
	void TextEditorPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		ImGuiWindowFlags flags = 0;
		if (m_AllDocuments.size() != 0)
		{
			flags |= ImGuiWindowFlags_MenuBar;
		}
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowTextEditor, flags);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		if (m_AllDocuments.size() == 0)
		{
			EditorUI::EditorUIService::NewItemScreen("Open Existing File", KG_BIND_CLASS_FN(OnOpenFile), "Create New File", KG_BIND_CLASS_FN(OnCreateFile));
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
						OnCreateFile();
					}

					if (ImGui::MenuItem("Open File"))
					{
						OnOpenFile();
					}

					if (ImGui::MenuItem("Save File"))
					{
						OnSaveFile();
					}
					if (ImGui::MenuItem("Delete File"))
					{
						m_DeleteWarningSpec.OpenPopup = true;
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
			EditorUI::EditorUIService::GenericPopup(m_DeleteWarningSpec);
			EditorUI::EditorUIService::GenericPopup(m_DiscardChangesWarningSpec);

			ImGui::BeginTabBar("##TextTabBar", ImGuiTabBarFlags_AutoSelectNewTabs);
			uint32_t iteration{ 0 };
			
			if (m_TextEditor.IsTextChanged())
			{
				FixedString32 comparedWindow = m_EditorWindowName;
				Document& activeDocument = m_AllDocuments.at(m_ActiveDocument);
				activeDocument.TextBuffer = m_TextEditor.GetText();
				activeDocument.Edited = true;
				if (activeDocument.FilePath.extension().string() == ".kgscript")
				{
					CheckForErrors();
				}
				else
				{
					m_TextEditor.SetErrorMarkers({});
				}
			}

			for (auto& document : m_AllDocuments)
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
					m_ActiveDocument = iteration;
					Document& activeDocument = m_AllDocuments.at(m_ActiveDocument);
					m_TextEditor.SetText(activeDocument.TextBuffer);
					m_TextEditor.SetLanguageDefinitionByExtension(activeDocument.FilePath.extension().string());
					document.SetActive = false;
					if (activeDocument.FilePath.extension().string() == ".kgscript")
					{
						CheckForErrors();
					}
					else
					{
						m_TextEditor.SetErrorMarkers({});
					}
				}

				bool checkTab = true;
				if (ImGui::BeginTabItem((document.FilePath.filename().string() + "##" + std::to_string(iteration)).c_str(),
					&document.Opened, tabItemFlags))
				{
					m_TextEditor.Render(m_EditorWindowName);
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

			if (m_AllDocuments.size() != 0)
			{
				Document& activeDocument = m_AllDocuments.at(m_ActiveDocument);

				if (!activeDocument.FilePath.empty() && !activeDocument.Opened)
				{

					if (activeDocument.Edited)
					{
						activeDocument.Opened = true;
						m_DiscardChangesWarningSpec.OpenPopup = true;
					}
					else
					{
						OnCloseFile();
					}
				}
			}
			
		}

		EditorUI::EditorUIService::EndWindow();
	}

	void TextEditorPanel::RefreshKGScriptEditor()
	{
		Scripting::ScriptCompilerService::CreateKGScriptLanguageDefinition();
		CheckForErrors();

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
					OnSaveFile();
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

			for (auto& document : m_AllDocuments)
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
			m_AllDocuments.push_back(newDocument);
			m_ActiveDocument = static_cast<uint32_t>(m_AllDocuments.size() - 1);
			m_TextEditor.SetText(newDocument.TextBuffer);
			m_TextEditor.SetLanguageDefinitionByExtension(newDocument.FilePath.extension().string());
		}
	}
	void TextEditorPanel::ResetPanelResources()
	{
		OnCloseAllFiles();
	}
	void TextEditorPanel::CheckForErrors()
	{
		static int32_t countOfTimers{0};
		countOfTimers++;
		m_TextEditor.ClearErrorMarkers();
		Utility::PassiveTimer::CreateTimer(1.2f, [&]()
		{
			countOfTimers--;
			if (countOfTimers > 0)
			{
				return;
			}
			
			if (m_AllDocuments.size() <= 0)
			{
				return;
			}
			Document& activeDocument = m_AllDocuments.at(m_ActiveDocument);
			if (activeDocument.FilePath.extension().string() != ".kgscript")
			{
				return;
			}
			std::vector<Scripting::ParserError> errors = Scripting::ScriptCompilerService::CheckForErrors(activeDocument.TextBuffer);

			if (errors.size() == 0)
			{
				m_TextEditor.SetErrorMarkers({});
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
			m_TextEditor.SetErrorMarkers(markers);
		});
	}
	void TextEditorPanel::OpenCreateDialog(const std::filesystem::path& path)
	{
		// Open project component Window
		s_EditorApp->m_ShowTextEditor = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);
		OnCreateFile(path);
	}
	void TextEditorPanel::OnOpenFile()
	{
		const std::filesystem::path initialDirectory = Projects::ProjectService::GetActiveAssetDirectory();
		const std::filesystem::path filepath = Utility::FileDialogs::OpenFile("All Files\0*.*\0", initialDirectory.string().c_str());
		OpenFile(filepath);
	}
	void TextEditorPanel::OnCreateFile()
	{
		OnCreateFile(Projects::ProjectService::GetActiveAssetDirectory());
	}
	void TextEditorPanel::OnCreateFile(const std::filesystem::path& initialDirectory)
	{
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
			m_AllDocuments.push_back(newDocument);
			m_ActiveDocument = static_cast<uint32_t>(m_AllDocuments.size() - 1);
			m_TextEditor.SetText("");
		}
	}
	void TextEditorPanel::OnSaveFile()
	{
		Document& activeDocument = m_AllDocuments.at(m_ActiveDocument);

		Utility::Operations::RemoveCharacterFromString(activeDocument.TextBuffer, '\r');

		KG_INFO("Saving file in Text Editor: {}", activeDocument.FilePath.filename());
		Utility::FileSystem::WriteFileString(activeDocument.FilePath, activeDocument.TextBuffer);
		activeDocument.Edited = false;
	}
	void TextEditorPanel::OnDeleteFile()
	{
		Document& activeDocument = m_AllDocuments.at(m_ActiveDocument);
		if (Utility::FileSystem::DeleteSelectedFile(activeDocument.FilePath))
		{
			activeDocument.TextBuffer = "";
			m_AllDocuments.erase(m_AllDocuments.begin() + m_ActiveDocument);
			m_ActiveDocument = 0;
			if (m_AllDocuments.size() > 0)
			{
				m_AllDocuments.at(0).SetActive = true;
			}
		}
	}
	void TextEditorPanel::OnCloseFile()
	{
		Document& activeDocument = m_AllDocuments.at(m_ActiveDocument);
		activeDocument.TextBuffer = "";
		m_AllDocuments.erase(m_AllDocuments.begin() + m_ActiveDocument);
		m_ActiveDocument = 0;
		if (m_AllDocuments.size() > 0)
		{
			m_AllDocuments.at(0).SetActive = true;
		}
	}
	void TextEditorPanel::OnCloseAllFiles()
	{
		m_AllDocuments.clear();
		m_ActiveDocument = 0;
	}
}

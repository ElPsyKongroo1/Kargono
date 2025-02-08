#include "Windows/MainWindow/TextEditorPanel.h"

#include "EditorApp.h"

#include "Kargono/Scripting/ScriptCompilerService.h"
#include "Kargono/Input/InputService.h"
#include "Kargono/Utility/FileDialogs.h"
#include "Kargono/Utility/Operations.h"
#include "Kargono/Utility/Timers.h"

#include "API/EditorUI/ImGuiBackendAPI.h"

static Kargono::EditorApp* s_EditorApp { nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };

namespace Kargono::Panels
{
	TextEditorPanel::TextEditorPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
		s_MainWindow->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(TextEditorPanel::OnKeyPressedEditor));

		m_TextEditor = {};
		m_TextEditor.SetLanguageDefinition(API::EditorUI::TextEditorService::GenerateC());
		m_TextEditor.SetPalette(API::EditorUI::TextEditorService::GetCurrentColorPalette());
	
		m_TextEditor.SetSaveCallback(KG_BIND_CLASS_FN(OnSaveFile));

		m_DeleteWarningSpec.m_Label = "Delete File";
		m_DeleteWarningSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnDeleteFile);
		m_DeleteWarningSpec.m_PopupContents = [&]()
		{
			ImGui::Text("Are you sure you want to delete this file?");
		};

		m_DiscardChangesWarningSpec.m_Label = "Close File";
		m_DiscardChangesWarningSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnCloseFile);
		m_DiscardChangesWarningSpec.m_PopupContents = [&]()
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
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_MainWindow->m_ShowTextEditor, flags);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		if (m_AllDocuments.size() == 0)
		{
			EditorUI::EditorUIService::NewItemScreen("Open Existing File", KG_BIND_CLASS_FN(OnOpenFileDialog), "Create New File", KG_BIND_CLASS_FN(OnCreateFileDialog));
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
						OnCreateFileDialog();
					}

					if (ImGui::MenuItem("Open File"))
					{
						OnOpenFileDialog();
					}

					if (ImGui::MenuItem("Save File"))
					{
						OnSaveFile();
					}
					if (ImGui::MenuItem("Delete File"))
					{
						m_DeleteWarningSpec.m_OpenPopup = true;
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
				OnTextChanged();
			}

			for (Document& currentDocument : m_AllDocuments)
			{
				bool setColorHighlight = false;
				if (currentDocument.Edited)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, EditorUI::EditorUIService::s_HighlightColor2);
					setColorHighlight = true;
				}
				// Handle case 
				ImGuiTabItemFlags tabItemFlags = 0;
				if (currentDocument.SetActive)
				{
					tabItemFlags |= ImGuiTabItemFlags_SetSelected;

					SwitchToAnotherDocument(iteration);
				}

				bool checkTab = true;
				if (ImGui::BeginTabItem((currentDocument.FilePath.filename().string() + "##" + std::to_string(iteration)).c_str(),
					&currentDocument.Opened, tabItemFlags))
				{
					m_TextEditor.OnEditorUIRender(m_EditorWindowName);
					checkTab = false;
					ImGui::EndTabItem();
				}

				// Handle Case of switching tabs by clicking
				if (ImGui::IsItemClicked() && checkTab)
				{
					currentDocument.SetActive = true;
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
						m_DiscardChangesWarningSpec.m_OpenPopup = true;
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
	bool TextEditorPanel::OnAssetEvent(Events::Event* event)
	{
		// Ensure event is a manage asset event before casting
		if (event->GetEventType() != Events::EventType::ManageAsset)
		{
			return false;
		}

		// Only respond to post delete events
		Events::ManageAsset* manageEvent = (Events::ManageAsset*)event;
		Events::ManageAssetAction eventAction{ manageEvent->GetAction() };
		if (eventAction == Events::ManageAssetAction::PreDelete)
		{
			return false;
		}

		// Refresh scripting languange definition
		RefreshKGScriptEditor();
		return false;
	}
	void TextEditorPanel::OpenFile(const std::filesystem::path& filepath)
	{
		if (!filepath.empty())
		{
			if (!s_MainWindow->m_ShowTextEditor)
			{
				s_MainWindow->m_ShowTextEditor = true;
			}

			if (EditorUI::EditorUIService::GetFocusedWindowName() != m_PanelName)
			{
				EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);
			}

			for (Document& document : m_AllDocuments)
			{
				if (Utility::FileSystem::PathsEquivalent(document.FilePath, filepath))
				{
					KG_WARN("Attempt to open document that is already open");
					document.SetActive = true;
					return;
				}
			}

			// Store previously active document's undo buffer
			if (m_ActiveDocument < m_AllDocuments.size())
			{
				Document& oldDocument = m_AllDocuments.at(m_ActiveDocument);
				oldDocument.UndoBuffer = m_TextEditor.GetUndoBuffer();
				oldDocument.UndoIndex = m_TextEditor.GetUndoIndex();
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

			API::EditorUI::ErrorMarkers markers{};
			for (Scripting::ParserError& error : errors)
			{
				if (markers.contains(error.CurrentToken.Line))
				{
					API::EditorUI::ErrorMarker& existingMarker = markers.at(error.CurrentToken.Line);
					existingMarker.m_Description = existingMarker.m_Description + error.ToString();
					API::EditorUI::ErrorLocation newLocation;
					newLocation.m_Column = error.CurrentToken.Column;
					newLocation.m_Length = (uint32_t)error.CurrentToken.Value.size();
					existingMarker.m_Locations.push_back(newLocation);
					continue;
				}
				API::EditorUI::ErrorMarker marker;
				marker.m_Description = error.ToString();
				API::EditorUI::ErrorLocation newLocation;
				newLocation.m_Column = error.CurrentToken.Column;
				newLocation.m_Length = (uint32_t)error.CurrentToken.Value.size();
				marker.m_Locations.push_back(newLocation);
				markers.insert_or_assign(error.CurrentToken.Line, marker);
			}
			m_TextEditor.SetErrorMarkers(markers);
		});
	}
	void TextEditorPanel::OpenCreateDialog(const std::filesystem::path& path)
	{
		// Open project component Window
		s_MainWindow->m_ShowTextEditor = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);
		OnCreateFileDialog(path);
	}
	void TextEditorPanel::OnOpenFileDialog()
	{
		const std::filesystem::path initialDirectory = Projects::ProjectService::GetActiveAssetDirectory();
		const std::filesystem::path filepath = Utility::FileDialogs::OpenFile("All Files\0*.*\0", initialDirectory.string().c_str());
		OpenFile(filepath);
	}
	void TextEditorPanel::OnCreateFileDialog()
	{
		OnCreateFileDialog(Projects::ProjectService::GetActiveAssetDirectory());
	}
	void TextEditorPanel::OnCreateFileDialog(const std::filesystem::path& initialDirectory)
	{
		const std::filesystem::path filepath = Utility::FileDialogs::SaveFile("All Files\0*.*\0", initialDirectory.string().c_str());
		if (!filepath.empty())
		{
			OnCreateFile(filepath);
		}
		else
		{
			KG_WARN("Could not create file. Issue retreiving filepath from choose file dialog.");
		}
	}
	void TextEditorPanel::OnCreateFile(const std::filesystem::path& filePath)
	{
		// Store previously active document's undo buffer
		if (m_ActiveDocument < m_AllDocuments.size())
		{
			Document& oldDocument = m_AllDocuments.at(m_ActiveDocument);
			oldDocument.UndoBuffer = m_TextEditor.GetUndoBuffer();
			oldDocument.UndoIndex = m_TextEditor.GetUndoIndex();
		}

		// Add the file to the active text editor
		Document newDocument{};
		Utility::FileSystem::WriteFileString(filePath, "");
		newDocument.TextBuffer = "";
		newDocument.FilePath = filePath;
		newDocument.Edited = false;
		newDocument.Opened = true;
		newDocument.SetActive = true;
		m_AllDocuments.push_back(newDocument);
		m_ActiveDocument = static_cast<uint32_t>(m_AllDocuments.size() - 1);
		m_TextEditor.SetText("");
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
	void TextEditorPanel::OnTextChanged()
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
	void TextEditorPanel::SwitchToAnotherDocument(std::size_t documentIteration)
	{

		// Change to new active document index
		if (m_ActiveDocument != (uint32_t)documentIteration)
		{
			// Store previously active document's undo buffer
			if (m_ActiveDocument < m_AllDocuments.size())
			{
				Document& oldDocument = m_AllDocuments.at(m_ActiveDocument);
				oldDocument.UndoBuffer = m_TextEditor.GetUndoBuffer();
				oldDocument.UndoIndex = m_TextEditor.GetUndoIndex();
			}

			// Update active document
			m_ActiveDocument = (uint32_t)documentIteration;
		}

		Document& activeDocument = m_AllDocuments.at(m_ActiveDocument);

		// Load undo buffer from new active document
		m_TextEditor.SetUndoBuffer(activeDocument.UndoBuffer, activeDocument.UndoIndex);

		// Clear current selection
		m_TextEditor.ClearSelection();

		// Load active document text and settings into text editor
		m_TextEditor.SetText(activeDocument.TextBuffer);
		m_TextEditor.SetLanguageDefinitionByExtension(activeDocument.FilePath.extension().string());

		// Refresh error markers
		activeDocument.SetActive = false;
		if (activeDocument.FilePath.extension().string() == ".kgscript")
		{
			CheckForErrors();
		}
		else
		{
			m_TextEditor.SetErrorMarkers({});
		}
	}
}

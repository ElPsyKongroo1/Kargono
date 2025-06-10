#pragma once
#include "Modules/Events/KeyEvent.h"
#include "Modules/EditorUI/EditorUIInclude.h"

#include "Modules/EditorUI/ExternalAPI/ImGuiColorTextEditorAPI.h"

#include <filesystem>
#include <string>

namespace Kargono::Panels
{
	struct Document
	{
		std::string TextBuffer {};
		API::EditorUI::UndoBuffer UndoBuffer{};
		int UndoIndex{0};
		std::filesystem::path FilePath { "" };
		bool Edited{ false };
		bool Opened{ false };
		bool SetActive{ false };
	};

	class TextEditorPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		TextEditorPanel();

		//=========================
		// On Event Functions
		//=========================
		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
		bool OnAssetEvent(Events::Event* event);

		//=========================
		// External API
		//=========================
		void RefreshKGScriptEditor();
		void OpenFile(const std::filesystem::path& path);
		void ResetPanelResources();
		void CheckForErrors();
		void OpenCreateDialog(const std::filesystem::path& path);

	private:
		//=========================
		// Internal Functionality
		//=========================
		void OnOpenFileDialog();
		void OnCreateFileDialog();
		void OnCreateFileDialog(const std::filesystem::path& initialDirectory);
		void OnCreateFile(const std::filesystem::path& filePath);
		void OnSaveFile();
		void OnDeleteFile();
		void OnCloseFile();
		void OnCloseAllFiles(); 
		void OnTextChanged();
		void SwitchToAnotherDocument(std::size_t documentIteration);

	private:
		//=========================
		// Core Panel Data
		//=========================
		FixedString32 m_PanelName{ "Text Editor" };
		FixedString32 m_EditorWindowName { "TextEditorChild" };
		uint32_t m_ActiveDocument = 0;
		std::vector<Document> m_AllDocuments {};

	private:
		//=========================
		// Widgets
		//=========================
		API::EditorUI::TextEditorSpec m_TextEditor;
		EditorUI::GenericPopupWidget m_DeleteWarningSpec {};
		EditorUI::GenericPopupWidget m_DiscardChangesWarningSpec {};
	};
}

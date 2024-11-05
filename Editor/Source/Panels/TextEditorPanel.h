#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/EditorUI/EditorUI.h"
#include "API/EditorUI/ImGuiColorTextEditorAPI.h"

#include <filesystem>
#include <string>

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

		//=========================
		// External API
		//=========================
		void RefreshKGScriptEditor();
		void OpenFile(const std::filesystem::path& path);
		void ResetPanelResources();
		void CheckForErrors();

	private:
		//=========================
		// Internal Functionality
		//=========================
		void OnOpenFile();
		void OnCreateFile();
		void OnSaveFile();
		void OnDeleteFile();
		void OnCloseFile();
		void OnCloseAllFiles(); 

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
		TextEditor m_TextEditor;
		EditorUI::GenericPopupSpec m_DeleteWarningSpec {};
		EditorUI::GenericPopupSpec m_DiscardChangesWarningSpec {};
	};
}

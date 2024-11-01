#pragma once

#include "Kargono.h"
#include "Kargono/Events/KeyEvent.h"

#include "API/FileWatch/FileWatchAPI.h"

#include <filesystem>
#include <string>
#include <vector>

namespace Kargono::Panels
{
	enum class BrowserFileType
	{
		None = 0,
		Directory,
		Image,
		Audio,
		Binary,
		Registry,
		Scene,
		Font,
		UserInterface,
		Input
	};


	inline static std::array<FixedString32, 6> s_ContentBrowserPayloads
	{
		"CONTENT_BROWSER_IMAGE", "CONTENT_BROWSER_AUDIO", "CONTENT_BROWSER_FONT",
			"CONTENT_BROWSER_ITEM", "CONTENT_BROWSER_SCENE", "CONTENT_BROWSER_USERINTERFACE"
	};

	class ContentBrowserPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		ContentBrowserPanel();

	private:
		//=========================
		// Internal Initialization Functions
		//=========================
		void InitializeNavigationHeader();
		void InitializeFileFolderViewer();

	public:
		//=========================
		// On Event Functions
		//=========================
		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);

	public:
		//=========================
		// External API
		//=========================
		void ResetPanelResources();
		void RefreshCachedDirectoryEntries();
	private:
		//=========================
		// General Internal Functionality
		//=========================
		void OnFileWatchUpdate(const std::string&, const API::FileWatch::EventType change_type);
		void UpdateCurrentDirectory(const std::filesystem::path& newPath);

		//=========================
		// Manage Navigation Header Functionality
		//=========================
		void NavigateDirectoryBack();
		void NavigateDirectoryForward();
		void OnNavHeaderBackReceivePayload(const char* payloadName, void* dataPointer, std::size_t dataSize);
		void OnNavHeaderForwardReceivePayload(const char* payloadName, void* dataPointer, std::size_t dataSize);

		//=========================
		// Manage Directory Grid Functionality
		//=========================
		void OnGridCreatePayload(EditorUI::GridEntry& currentEntry, EditorUI::DragDropPayload& payload);
		void OnGridReceivePayload(EditorUI::GridEntry& currentEntry, 
			const char* payloadName, 
			void* dataPointer, 
			std::size_t dataSize);
		void OnGridDirectoryDoubleClick(EditorUI::GridEntry& currentEntry);
		void OnGridHandleRightClick(EditorUI::GridEntry& currentEntry);

	private:
		//=========================
		// Core Panel Data
		//=========================
		// Panel Name
		std::string m_PanelName{ "Content Browser" };

		// Manage content browser directory
		std::filesystem::path m_BaseDirectory {};
		std::filesystem::path m_CurrentDirectory {};
		std::filesystem::path m_LongestRecentPath {};
		std::filesystem::path m_FileToModifyCache{};

		// Tooltip handles
		UUID m_OpenFileInTextEditorEntry;

		//=========================
		// Widgets
		//=========================

		EditorUI::NavigationHeaderSpec m_NavigateAssetsHeader{};

		// Grid widgets
		EditorUI::GridSpec m_FileFolderViewer{};

		// Tooltip widgets
		EditorUI::TooltipSpec m_RightClickTooltip{};

		// Popup widgets
		EditorUI::GenericPopupSpec m_DeleteFilePopup{};
		EditorUI::GenericPopupSpec m_DeleteDirectoryPopup{};
		EditorUI::GenericPopupSpec m_RenameFilePopup{};
		EditorUI::EditTextSpec m_RenameFileEditName{};
		
	};
}

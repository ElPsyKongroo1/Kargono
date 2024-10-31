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
		None = 0, Directory = 1, Image = 2, Audio = 3,
		Binary = 4, Registry = 5, Scene = 6, ScriptProject = 7,
		Font = 8, UserInterface = 9, Input = 10
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

	private:
		//=========================
		// Core Panel Data
		//=========================
		std::string m_PanelName{ "Content Browser" };
		std::filesystem::path m_BaseDirectory {};
		std::filesystem::path m_CurrentDirectory {};
		std::filesystem::path m_LongestRecentPath {};
		std::vector<std::filesystem::path> m_CachedDirectoryEntries {};

		//=========================
		// Widgets
		//=========================
		EditorUI::NavigationHeaderSpec m_NavigateAssetsHeader{};
		EditorUI::GridSpec m_FileFolderViewer{};
		
	};
}

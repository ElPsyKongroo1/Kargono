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
		Input,
		GenericFile
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
		bool OnMouseButton(Events::MouseButtonPressedEvent event);

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
		std::filesystem::path m_BaseDirectory{};
		std::filesystem::path m_CurrentDirectory{};
		std::filesystem::path m_LongestRecentPath{};
		std::filesystem::path m_FileToModifyCache{};

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
		EditorUI::GenericPopupSpec m_CreateDirectoryPopup{};
		EditorUI::EditTextSpec m_CreateDirectoryEditName{};

	};
}

namespace Kargono::Utility
{
	inline Ref<Rendering::Texture2D> BrowserFileTypeToIcon(Panels::BrowserFileType type)
	{
		switch (type)
		{
		case Panels::BrowserFileType::Directory: { return EditorUI::EditorUIService::s_IconDirectory; }
		case Panels::BrowserFileType::Image: { return EditorUI::EditorUIService::s_IconImage; }
		case Panels::BrowserFileType::Audio: { return EditorUI::EditorUIService::s_IconAudio; }
		case Panels::BrowserFileType::Font: { return EditorUI::EditorUIService::s_IconFont; }
		case Panels::BrowserFileType::UserInterface: { return EditorUI::EditorUIService::s_IconUserInterface; }
		case Panels::BrowserFileType::Binary: { return EditorUI::EditorUIService::s_IconBinary; }
		case Panels::BrowserFileType::Registry: { return EditorUI::EditorUIService::s_IconRegistry; }
		case Panels::BrowserFileType::Scene: { return EditorUI::EditorUIService::s_IconScene; }
		case Panels::BrowserFileType::Input: { return EditorUI::EditorUIService::s_IconInput; }
		case Panels::BrowserFileType::GenericFile: { return EditorUI::EditorUIService::s_IconGenericFile; }
		case Panels::BrowserFileType::None: { return EditorUI::EditorUIService::s_IconCancel; }
		}
		KG_ERROR("Invalid BrowserFileType provided");
		return EditorUI::EditorUIService::s_IconGenericFile;
	}

	inline Panels::BrowserFileType DetermineFileType(const std::filesystem::path& entry)
	{
		if (std::filesystem::is_directory(entry)) { return Panels::BrowserFileType::Directory; }
		if (!entry.has_extension()) { return Panels::BrowserFileType::None; }
		auto extension = entry.extension();
		if (extension == ".jpg" || extension == ".png") { return Panels::BrowserFileType::Image; }
		if (extension == ".wav" || extension == ".mp3") { return Panels::BrowserFileType::Audio; }
		if (extension == ".kgreg") { return Panels::BrowserFileType::Registry; }
		if (extension == ".kgui") { return Panels::BrowserFileType::UserInterface; }
		if (extension == ".ttf") { return Panels::BrowserFileType::Font; }
		if (extension == ".kgscene") { return Panels::BrowserFileType::Scene; }
		if (extension == ".kginput") { return Panels::BrowserFileType::Input; }
		if (extension == ".kgaudio" || extension == ".kgtexture" || extension == ".kgfont" ||
			extension == ".kgshadervert" || extension == ".kgshaderfrag") {
			return Panels::BrowserFileType::Binary;
		}
		return Panels::BrowserFileType::GenericFile;
	}

	inline std::string BrowserFileTypeToPayloadString(Panels::BrowserFileType type)
	{
		switch (type)
		{
		case Panels::BrowserFileType::Directory: { return "CONTENT_BROWSER_DIRECTORY"; }
		case Panels::BrowserFileType::Image: { return "CONTENT_BROWSER_IMAGE"; }
		case Panels::BrowserFileType::Audio: { return "CONTENT_BROWSER_AUDIO"; }
		case Panels::BrowserFileType::UserInterface: { return "CONTENT_BROWSER_USERINTERFACE"; }
		case Panels::BrowserFileType::Font: { return "CONTENT_BROWSER_FONT"; }
		case Panels::BrowserFileType::Binary: { return "CONTENT_BROWSER_ITEM"; }
		case Panels::BrowserFileType::Registry: { return "CONTENT_BROWSER_ITEM"; }
		case Panels::BrowserFileType::Scene: { return "CONTENT_BROWSER_SCENE"; }
		case Panels::BrowserFileType::Input: { return "CONTENT_BROWSER_ITEM"; }
		case Panels::BrowserFileType::GenericFile: { return "CONTENT_BROWSER_ITEM"; }
		case Panels::BrowserFileType::None: { return "CONTENT_BROWSER_ITEM"; }
		}
		KG_ERROR("Invalid BrowserFileType provided");
		return "CONTENT_BROWSER_ITEM";
	}
}

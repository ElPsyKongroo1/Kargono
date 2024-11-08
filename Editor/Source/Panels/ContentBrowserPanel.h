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
		RawImage,
		RawAudio,
		Binary,
		Registry,
		Scene,
		RawFont,
		UserInterface,
		Input,
		GenericFile
	};


	inline static std::array<FixedString32, 6> s_ContentBrowserPayloads
	{
		"CONTENT_BROWSER_IMAGE", "CONTENT_BROWSER_RAWAUDIO", "CONTENT_BROWSER_RAWFONT",
			"CONTENT_BROWSER_ITEM", "CONTENT_BROWSER_SCENE", "CONTENT_BROWSER_RAWUSERINTERFACE"
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
		void OnHandleMoveAssetLocation(const std::filesystem::path& originalFilePath, const std::filesystem::path& newDirectory);

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
		// Popup functionality
		void OnHandleDeleteFile();

	private:
		//=========================
		// Core Panel Data
		//=========================
		// Panel Name
		FixedString32 m_PanelName{ "Content Browser" };

		// Manage content browser directory
		std::filesystem::path m_BaseDirectory{};
		std::filesystem::path m_CurrentDirectory{};
		std::filesystem::path m_LongestRecentPath{};
		std::filesystem::path m_CurrentFileToModifyCache{};
		BrowserFileType m_CurrentFileTypeCache{};

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
		case Panels::BrowserFileType::RawImage: { return EditorUI::EditorUIService::s_IconImage; }
		case Panels::BrowserFileType::RawAudio: { return EditorUI::EditorUIService::s_IconAudio; }
		case Panels::BrowserFileType::RawFont: { return EditorUI::EditorUIService::s_IconFont; }
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
		std::filesystem::path currentFileExtension = entry.extension();

		// Check for raw texture extension
		for (const std::string& textureExtension : Assets::AssetService::GetTexture2DValidImportExtensions())
		{
			if (textureExtension == currentFileExtension)
			{
				return Panels::BrowserFileType::RawImage;
			}
		}

		// Check for raw audio extension
		for (const std::string& audioExtension : Assets::AssetService::GetAudioBufferValidImportExtensions())
		{
			if (audioExtension == currentFileExtension)
			{
				return Panels::BrowserFileType::RawAudio;
			}
		}
		// Check for raw font extension
		for (const std::string& fontExtension : Assets::AssetService::GetFontValidImportExtensions())
		{
			if (fontExtension == currentFileExtension)
			{
				return Panels::BrowserFileType::RawFont;
			}
		}
		if (currentFileExtension == ".kgreg") { return Panels::BrowserFileType::Registry; }
		if (currentFileExtension == ".kgui") { return Panels::BrowserFileType::UserInterface; }
		if (currentFileExtension == ".kgscene") { return Panels::BrowserFileType::Scene; }
		if (currentFileExtension == ".kginput") { return Panels::BrowserFileType::Input; }
		if (currentFileExtension == ".kgaudio" || currentFileExtension == ".kgtexture" || currentFileExtension == ".kgfont" ||
			currentFileExtension == ".kgshadervert" || currentFileExtension == ".kgshaderfrag") {
			return Panels::BrowserFileType::Binary;
		}
		return Panels::BrowserFileType::GenericFile;
	}

	inline std::string BrowserFileTypeToPayloadString(Panels::BrowserFileType type)
	{
		switch (type)
		{
		case Panels::BrowserFileType::Directory: { return "CONTENT_BROWSER_DIRECTORY"; }
		case Panels::BrowserFileType::RawImage: { return "CONTENT_BROWSER_RAWIMAGE"; }
		case Panels::BrowserFileType::RawAudio: { return "CONTENT_BROWSER_RAWAUDIO"; }
		case Panels::BrowserFileType::UserInterface: { return "CONTENT_BROWSER_USERINTERFACE"; }
		case Panels::BrowserFileType::RawFont: { return "CONTENT_BROWSER_RAWFONT"; }
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

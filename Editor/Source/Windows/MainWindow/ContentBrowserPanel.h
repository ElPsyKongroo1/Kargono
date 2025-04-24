#pragma once

#include "EventsPlugin/KeyEvent.h"
#include "EventsPlugin/MouseEvent.h"
#include "EditorUIPlugin/EditorUI.h"
#include "AssetsPlugin/AssetService.h"

#include "API/FileWatch/FileWatchAPI.h"

#include <filesystem>
#include <string>
#include <vector>

namespace Kargono::Panels
{
	enum class BrowserFileType
	{
		None = 0,

		// Directory
		Directory,

		// Kargono managed files
		AIState,
		Audio,
		Binary,
		ColorPalette,
		EmitterConfig,
		Font,
		GameState,
		GlobalState,
		ProjectComponent,
		ProjectEnum,
		InputMap,
		Registry,
		Scene,
		Script,
		Texture,
		UserInterface,

		// Raw data files that can be imported
		RawTexture,
		RawAudio,
		RawFont,

		// All other files
		GenericFile
	};


	static inline std::vector<FixedString32> s_ContentBrowserPayloads
	{
		"CONTENT_BROWSER_RAWIMAGE", 
		"CONTENT_BROWSER_RAWAUDIO", 
		"CONTENT_BROWSER_RAWFONT",

		"CONTENT_BROWSER_ITEM", 

		"CONTENT_BROWSER_SCENE", 
		"CONTENT_BROWSER_USERINTERFACE"
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
		void OnFileWatchUpdate(std::string_view name, const API::FileWatch::EventType change_type);
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
		EditorUI::GenericPopupSpec m_CreateDirectoryPopup{};
		EditorUI::EditTextSpec m_CreateDirectoryEditName{};

	};
}

namespace Kargono::Utility
{

	inline Panels::BrowserFileType DetermineFileType(const std::filesystem::path& entry)
	{
		if (Utility::FileSystem::IsDirectory(entry)) { return Panels::BrowserFileType::Directory; }
		if (!entry.has_extension()) { return Panels::BrowserFileType::None; }
		std::filesystem::path currentFileExtension = entry.extension();

		// Check for raw texture extension
		for (std::string_view textureExtension : Assets::AssetService::GetTexture2DValidImportExtensions())
		{
			if (textureExtension == currentFileExtension)
			{
				return Panels::BrowserFileType::RawTexture;
			}
		}

		// Check for raw audio extension
		for (std::string_view audioExtension : Assets::AssetService::GetAudioBufferValidImportExtensions())
		{
			if (audioExtension == currentFileExtension)
			{
				return Panels::BrowserFileType::RawAudio;
			}
		}
		// Check for raw font extension
		for (std::string_view fontExtension : Assets::AssetService::GetFontValidImportExtensions())
		{
			if (fontExtension == currentFileExtension)
			{
				return Panels::BrowserFileType::RawFont;
			}
		}
		if (currentFileExtension == ".kgaistate") { return Panels::BrowserFileType::AIState; }
		if (currentFileExtension == ".kgaudio") { return Panels::BrowserFileType::Audio; }
		if (currentFileExtension == ".kgfont") { return Panels::BrowserFileType::Font; }
		if (currentFileExtension == ".kgpalette") { return Panels::BrowserFileType::ColorPalette; }
		if (currentFileExtension == ".kgcomponent") { return Panels::BrowserFileType::ProjectComponent; }
		if (currentFileExtension == ".kgenum") { return Panels::BrowserFileType::ProjectEnum; }
		if (currentFileExtension == ".kgreg") { return Panels::BrowserFileType::Registry; }
		if (currentFileExtension == ".kgui") { return Panels::BrowserFileType::UserInterface; }
		if (currentFileExtension == ".kgscript") { return Panels::BrowserFileType::Script; }
		if (currentFileExtension == ".kgstate") { return Panels::BrowserFileType::GameState; }
		if (currentFileExtension == ".kggstate") { return Panels::BrowserFileType::GlobalState; }
		if (currentFileExtension == ".kgparticle") { return Panels::BrowserFileType::EmitterConfig; }
		if (currentFileExtension == ".kgscene") { return Panels::BrowserFileType::Scene; }
		if (currentFileExtension == ".kgtexture") { return Panels::BrowserFileType::Texture; }
		if (currentFileExtension == ".kginput") { return Panels::BrowserFileType::InputMap; }
		if (currentFileExtension == ".kgbinary" || currentFileExtension == ".kgshadervert" || 
			currentFileExtension == ".kgshaderfrag") 
		{
			return Panels::BrowserFileType::Binary;
		}
		return Panels::BrowserFileType::GenericFile;
	}

	inline const char* BrowserFileTypeToPayloadString(Panels::BrowserFileType type)
	{
		switch (type)
		{
		case Panels::BrowserFileType::Directory: { return "CONTENT_BROWSER_DIRECTORY"; }

		case Panels::BrowserFileType::RawTexture: { return "CONTENT_BROWSER_RAWIMAGE"; }
		case Panels::BrowserFileType::RawAudio: { return "CONTENT_BROWSER_RAWAUDIO"; }
		case Panels::BrowserFileType::RawFont: { return "CONTENT_BROWSER_RAWFONT"; }

		case Panels::BrowserFileType::GenericFile: { return "CONTENT_BROWSER_ITEM"; }

		case Panels::BrowserFileType::AIState: { return "CONTENT_BROWSER_ITEM"; }
		case Panels::BrowserFileType::Audio: { return "CONTENT_BROWSER_ITEM"; }
		case Panels::BrowserFileType::Binary: { return "CONTENT_BROWSER_ITEM"; }
		case Panels::BrowserFileType::EmitterConfig: { return "CONTENT_BROWSER_ITEM"; }
		case Panels::BrowserFileType::ColorPalette: { return "CONTENT_BROWSER_ITEM"; }
		case Panels::BrowserFileType::Font: { return "CONTENT_BROWSER_ITEM"; }
		case Panels::BrowserFileType::GameState: { return "CONTENT_BROWSER_ITEM"; }
		case Panels::BrowserFileType::GlobalState: { return "CONTENT_BROWSER_ITEM"; }
		case Panels::BrowserFileType::ProjectComponent: { return "CONTENT_BROWSER_ITEM"; }
		case Panels::BrowserFileType::ProjectEnum: { return "CONTENT_BROWSER_ITEM"; }
		case Panels::BrowserFileType::InputMap: { return "CONTENT_BROWSER_ITEM"; }
		case Panels::BrowserFileType::Registry: { return "CONTENT_BROWSER_ITEM"; }
		case Panels::BrowserFileType::Scene: { return "CONTENT_BROWSER_SCENE"; }
		case Panels::BrowserFileType::Script: { return "CONTENT_BROWSER_ITEM"; }
		case Panels::BrowserFileType::Texture: { return "CONTENT_BROWSER_ITEM"; }
		case Panels::BrowserFileType::UserInterface: { return "CONTENT_BROWSER_USERINTERFACE"; }

		case Panels::BrowserFileType::None: { return "CONTENT_BROWSER_ITEM"; }
		}
		KG_ERROR("Invalid BrowserFileType provided");
		return "CONTENT_BROWSER_ITEM";
	}
}

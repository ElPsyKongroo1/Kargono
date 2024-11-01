#include "kgpch.h"

#include "Panels/ContentBrowserPanel.h"

#include "EditorApp.h"

namespace Kargono::Utility
{
	static Ref<Rendering::Texture2D> BrowserFileTypeToIcon(Panels::BrowserFileType type)
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
		case Panels::BrowserFileType::None: { return EditorUI::EditorUIService::s_IconGenericFile; }
		}
		KG_ERROR("Invalid BrowserFileType provided");
		return EditorUI::EditorUIService::s_IconGenericFile;
	}

	static Panels::BrowserFileType DetermineFileType(const std::filesystem::path& entry)
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
		return Panels::BrowserFileType::None;
	}

	static std::string BrowserFileTypeToPayloadString(Panels::BrowserFileType type)
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
		case Panels::BrowserFileType::None: { return "CONTENT_BROWSER_ITEM"; }
		}
		KG_ERROR("Invalid BrowserFileType provided");
		return "CONTENT_BROWSER_ITEM";
	}
}

static Kargono::EditorApp* s_EditorApp{ nullptr };

namespace Kargono::Panels
{

	void ContentBrowserPanel::OnFileWatchUpdate(const std::string&, const API::FileWatch::EventType change_type)
	{
		EngineService::SubmitToMainThread([&]()
		{
			s_EditorApp->m_ContentBrowserPanel->RefreshCachedDirectoryEntries();
		});
	}

	void ContentBrowserPanel::UpdateCurrentDirectory(const std::filesystem::path& newPath)
	{

		// Create function to 
		static std::filesystem::path currentPath;
		currentPath = newPath;
		EngineService::SubmitToMainThread([&]()
		{
				
			API::FileWatch::EndWatch(m_CurrentDirectory);
			m_CurrentDirectory = currentPath;
			API::FileWatch::StartWatch(m_CurrentDirectory, KG_BIND_CLASS_FN(OnFileWatchUpdate));
			RefreshCachedDirectoryEntries();
		});
	}

	void ContentBrowserPanel::NavigateDirectoryBack()
	{
		UpdateCurrentDirectory(m_CurrentDirectory.parent_path());
	}

	void ContentBrowserPanel::NavigateDirectoryForward()
	{
		if (Utility::FileSystem::DoesPathContainSubPath(m_CurrentDirectory, m_LongestRecentPath))
		{
			std::filesystem::path currentIterationPath{ m_LongestRecentPath };
			std::filesystem::path recentIterationPath{ m_LongestRecentPath };
			while (currentIterationPath != m_CurrentDirectory)
			{
				recentIterationPath = currentIterationPath;
				currentIterationPath = currentIterationPath.parent_path();
			}
			UpdateCurrentDirectory(recentIterationPath);
		}
	}

	void ContentBrowserPanel::OnNavHeaderBackReceivePayload(const char* payloadName, void* dataPointer, std::size_t dataSize)
	{
		const wchar_t* payloadPathPointer = (const wchar_t*)dataPointer;
		std::filesystem::path payloadPath(payloadPathPointer);
		Utility::FileSystem::MoveFileToDirectory(payloadPath, m_CurrentDirectory.parent_path());
	}

	void ContentBrowserPanel::OnNavHeaderForwardReceivePayload(const char* payloadName, void* dataPointer, std::size_t dataSize)
	{
		const wchar_t* payloadPathPointer = (const wchar_t*)dataPointer;
		std::filesystem::path payloadPath(payloadPathPointer);
		std::filesystem::path currentIterationPath{ m_LongestRecentPath };
		std::filesystem::path recentIterationPath{ m_LongestRecentPath };
		while (currentIterationPath != m_CurrentDirectory)
		{
			recentIterationPath = currentIterationPath;
			currentIterationPath = currentIterationPath.parent_path();
		}
		Utility::FileSystem::MoveFileToDirectory(payloadPath, recentIterationPath);
	}

	void ContentBrowserPanel::OnGridCreatePayload(EditorUI::GridEntry& currentEntry, EditorUI::DragDropPayload& payload)
	{
		// Append the file name to current content browser directory
		static std::filesystem::path relativePath;
		relativePath = m_CurrentDirectory / currentEntry.m_Label.CString();
		const wchar_t* itemPath = relativePath.c_str();

		// Load up payload
		payload.m_Label = Utility::BrowserFileTypeToPayloadString((BrowserFileType)currentEntry.m_ArchetypeID).c_str();
		payload.m_DataPointer = (void*)itemPath;
		payload.m_DataSize = (wcslen(itemPath) + 1) * sizeof(wchar_t);
	}

	void ContentBrowserPanel::OnGridReceivePayload(EditorUI::GridEntry& currentEntry, const char* payloadName, void* dataPointer, std::size_t dataSize)
	{
		std::filesystem::path currentEntryPath{ m_CurrentDirectory / currentEntry.m_Label.CString() };

		const wchar_t* payloadPathPointer = (const wchar_t*)dataPointer;
		std::filesystem::path payloadPath(payloadPathPointer);
		Utility::FileSystem::MoveFileToDirectory(payloadPath, currentEntryPath);
		
	}

	void ContentBrowserPanel::OnGridDirectoryDoubleClick(EditorUI::GridEntry& currentEntry)
	{
		UpdateCurrentDirectory(m_CurrentDirectory / currentEntry.m_Label.CString());
		if (!Utility::FileSystem::DoesPathContainSubPath(m_CurrentDirectory / currentEntry.m_Label.CString(), m_LongestRecentPath))
		{
			m_LongestRecentPath = m_CurrentDirectory / currentEntry.m_Label.CString();
		}
	}

	void ContentBrowserPanel::OnGridHandleRightClick(EditorUI::GridEntry& currentEntry)
	{
		// Store current file to modify in later function
		m_FileToModifyCache = m_CurrentDirectory / currentEntry.m_Label.CString();

		// Enable tooltip menu
		m_RightClickTooltip.TooltipActive = true;
		BrowserFileType fileType = (BrowserFileType)currentEntry.m_ArchetypeID;

		m_RightClickTooltip.ClearEntries();
		if (fileType != BrowserFileType::Directory)
		{
			// m_OpenFileInTextEditorEntry
		}
#if 0
		if (fileType == BrowserFileType::Scene)
		{
			if (ImGui::Selectable("Open Scene"))
			{
				EditorApp::GetCurrentApp()->OpenScene(directoryEntry);
			}
		}

		if (fileType == BrowserFileType::Font)
		{
			if (ImGui::Selectable("Use Font In Current User Interface"))
			{
				Assets::AssetHandle currentHandle = Assets::AssetService::ImportFontFromFile(directoryEntry);
				Ref<RuntimeUI::Font> font = Assets::AssetService::GetFont(currentHandle);
				if (font)
				{
					RuntimeUI::RuntimeUIService::SetActiveFont(font, currentHandle);
				}

				else { KG_WARN("Could not load font {0}", directoryEntry.filename().string()); }
			}
		}

		if (fileType != BrowserFileType::Binary &&
			fileType != BrowserFileType::Directory &&
			fileType != BrowserFileType::Registry &&
			fileType != BrowserFileType::Scene)
		{
			if (ImGui::Selectable("Rename File"))
			{
				openRenamePopup = true;
			}
		}

		if (fileType != BrowserFileType::Registry && fileType != BrowserFileType::Directory)
		{
			if (ImGui::Selectable("Delete File"))
			{
				openDeleteModal = true;
			}
		}

		if (fileType == BrowserFileType::Directory)
		{
			if (ImGui::Selectable("Delete Directory"))
			{
				openDeleteDirectory = true;
			}
		}
#endif

	}

	ContentBrowserPanel::ContentBrowserPanel()
		: m_BaseDirectory(Projects::ProjectService::GetActiveAssetDirectory()), m_CurrentDirectory(m_BaseDirectory)
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName,
			KG_BIND_CLASS_FN(ContentBrowserPanel::OnKeyPressedEditor));

		// Initialize widget resources
		InitializeNavigationHeader();
		InitializeFileFolderViewer();

		API::FileWatch::StartWatch(m_CurrentDirectory, KG_BIND_CLASS_FN(OnFileWatchUpdate));
		RefreshCachedDirectoryEntries();
	}

	void ContentBrowserPanel::InitializeNavigationHeader()
	{
		m_NavigateAssetsHeader.m_Label = "Assets";
		m_NavigateAssetsHeader.m_Flags = EditorUI::NavigationHeaderFlags::NavigationHeader_AllowDragDrop;

		// Copy over s_ContentBrowserPayloads into m_NavigateAssetHeader!!!!!
		std::vector<FixedString32>& navPayloadList = m_NavigateAssetsHeader.m_AcceptableOnReceivePayloads;
		navPayloadList.insert(navPayloadList.end(), &s_ContentBrowserPayloads[0], &s_ContentBrowserPayloads[s_ContentBrowserPayloads.size() - 1]);

		m_NavigateAssetsHeader.m_OnNavigateBack = KG_BIND_CLASS_FN(NavigateDirectoryBack);
		m_NavigateAssetsHeader.m_OnReceivePayloadBack = KG_BIND_CLASS_FN(OnNavHeaderBackReceivePayload);
		m_NavigateAssetsHeader.m_OnNavigateForward = KG_BIND_CLASS_FN(NavigateDirectoryForward);
		m_NavigateAssetsHeader.m_OnReceivePayloadForward = KG_BIND_CLASS_FN(OnNavHeaderForwardReceivePayload);

	}

	void ContentBrowserPanel::InitializeFileFolderViewer()
	{
		// Initialize file viewer grid
		m_FileFolderViewer.m_Label = "Assets File Viewer";

		// Initialize grid archetypes
		EditorUI::GridEntryArchetype directoryArch;
		directoryArch.m_Icon = EditorUI::EditorUIService::s_IconDirectory;
		directoryArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		directoryArch.m_OnDoubleLeftClick = KG_BIND_CLASS_FN(OnGridDirectoryDoubleClick);
		directoryArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		// Copy over s_ContentBrowserPayloads into directoryArch
		std::vector<FixedString32>& navPayloadList = directoryArch.m_AcceptableOnReceivePayloads;
		navPayloadList.insert(navPayloadList.end(), &s_ContentBrowserPayloads[0], &s_ContentBrowserPayloads[s_ContentBrowserPayloads.size() - 1]);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::Directory, directoryArch);

		EditorUI::GridEntryArchetype imageArch;
		imageArch.m_Icon = EditorUI::EditorUIService::s_IconImage;
		imageArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		imageArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::Image, imageArch);

		EditorUI::GridEntryArchetype audioArch;
		audioArch.m_Icon = EditorUI::EditorUIService::s_IconAudio;
		audioArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		audioArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::Audio, audioArch);

		EditorUI::GridEntryArchetype fontArch;
		fontArch.m_Icon = EditorUI::EditorUIService::s_IconFont;
		fontArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		fontArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::Font, fontArch);

		EditorUI::GridEntryArchetype userInterfaceArch;
		userInterfaceArch.m_Icon = EditorUI::EditorUIService::s_IconUserInterface;
		userInterfaceArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		userInterfaceArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::UserInterface, userInterfaceArch);

		EditorUI::GridEntryArchetype binaryArch;
		binaryArch.m_Icon = EditorUI::EditorUIService::s_IconBinary;
		binaryArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		binaryArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::Binary, binaryArch);

		EditorUI::GridEntryArchetype registryArch;
		registryArch.m_Icon = EditorUI::EditorUIService::s_IconRegistry;
		registryArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		registryArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::Registry, registryArch);

		EditorUI::GridEntryArchetype sceneArch;
		sceneArch.m_Icon = EditorUI::EditorUIService::s_IconScene;
		sceneArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		sceneArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::Scene, sceneArch);

		EditorUI::GridEntryArchetype inputArch;
		inputArch.m_Icon = EditorUI::EditorUIService::s_IconInput;
		inputArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		inputArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::Input, inputArch);

		EditorUI::GridEntryArchetype genericFileArch;
		genericFileArch.m_Icon = EditorUI::EditorUIService::s_IconGenericFile;
		genericFileArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		genericFileArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::None, genericFileArch);

		
		// Initialize Tooltip
		m_RightClickTooltip.m_Label = "Content browser tooltip";
		// Create open file in text editor entry inside tooltip menu
		EditorUI::TooltipEntry openFileEntry("Open File In Text Editor", [&](EditorUI::TooltipEntry& entry) 
		{
			s_EditorApp->m_TextEditorPanel->OpenFile(m_FileToModifyCache);
		});
		m_OpenFileInTextEditorEntry = m_RightClickTooltip.AddMenuItem(openFileEntry);


		// Initialize delete directory popup
		m_DeleteDirectoryPopup.Label = "Delete Directory";
		m_DeleteDirectoryPopup.PopupWidth = 420.0f;
		m_DeleteDirectoryPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text(m_FileToModifyCache.string().c_str());
		};
		m_DeleteDirectoryPopup.ConfirmAction = [&]()
		{
			Utility::FileSystem::DeleteSelectedDirectory(m_FileToModifyCache);
		};

		// Initialize delete file popup
		m_DeleteFilePopup.Label = "Delete File";
		m_DeleteFilePopup.PopupWidth = 420.0f;
		m_DeleteFilePopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text(m_FileToModifyCache.string().c_str());
		};
		m_DeleteFilePopup.ConfirmAction = [&]()
		{
			Utility::FileSystem::DeleteSelectedFile(m_FileToModifyCache);
		};

		// Initialize rename file popup
		m_RenameFilePopup.Label = "Rename File";
		m_RenameFilePopup.PopupWidth = 420.0f;
		m_RenameFilePopup.PopupAction = [&]() 
		{
			m_RenameFileEditName.CurrentOption = m_FileToModifyCache.filename().string();
		};
		m_RenameFilePopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_RenameFileEditName);
		};
		m_RenameFilePopup.ConfirmAction = [&]()
		{
			Utility::FileSystem::RenameFile(m_FileToModifyCache, m_RenameFileEditName.CurrentOption);
		};

		m_RenameFileEditName.Label = "File Name";

#if 0
#endif

	}

	void ContentBrowserPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowContentBrowser);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		// Draw navigation header draw header
		EditorUI::EditorUIService::NavigationHeader(m_NavigateAssetsHeader);

		// Draw content browser file grid
		EditorUI::EditorUIService::Grid(m_FileFolderViewer);

		static char buffer[256];
		bool openNewDirectoryPopup = false;

		if (!ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			ImGui::OpenPopup("RightClickWindowOptions");
		}

		if (ImGui::BeginPopup("RightClickWindowOptions"))
		{
			if (ImGui::Selectable("Open In File Explorer"))
			{
				Utility::OSCommands::OpenFileExplorer(m_CurrentDirectory);
			}
			if (ImGui::Selectable("Create New Directory"))
			{
				openNewDirectoryPopup = true;
			}
			ImGui::EndPopup();
		}

		if (openNewDirectoryPopup) { ImGui::OpenPopup("New Directory"); }

		if (ImGui::BeginPopup("New Directory"))
		{
			memset(buffer, 0, 256);
			ImGui::InputText("New Directory Name", buffer, sizeof(buffer));
			if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))
			{
				std::filesystem::path newPath = m_CurrentDirectory / std::string(buffer);
				Utility::FileSystem::CreateNewDirectory(newPath);
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		EditorUI::EditorUIService::EndWindow();

	}
	bool ContentBrowserPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
	void ContentBrowserPanel::RefreshCachedDirectoryEntries()
	{
		// Re-validate filesystem directory
		m_FileFolderViewer.ClearEntries();
		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			// Create entry and associate appropriate archetype
			EditorUI::GridEntry newEntry;
			BrowserFileType fileType = Utility::DetermineFileType(directoryEntry);
			newEntry.m_Label = directoryEntry.path().filename().string().c_str();
			newEntry.m_ArchetypeID = (uint32_t)fileType;
			m_FileFolderViewer.AddEntry(newEntry);
		}

		// Generate current title for navigation header
		std::filesystem::path activeDirectory = Utility::FileSystem::GetRelativePath(Projects::ProjectService::GetActiveProjectDirectory(), m_CurrentDirectory);
		std::vector<std::string> tokenizedDirectoryPath{};
		while (activeDirectory.filename() != "Assets")
		{
			tokenizedDirectoryPath.push_back(activeDirectory.filename().string());
			activeDirectory = activeDirectory.parent_path();
		}
		tokenizedDirectoryPath.push_back("Assets");

		FixedString64 newTitle;
		for (int32_t i = (int32_t)(tokenizedDirectoryPath.size()) - 1; i >= 0; --i)
		{
			newTitle.Append(tokenizedDirectoryPath.at(i).c_str());
			if (i != 0)
			{
				newTitle.Append(" / ");
			}
		}
		m_NavigateAssetsHeader.m_Label = newTitle;

		// Revalidate whether the header's direction buttons should be active
		m_NavigateAssetsHeader.m_IsBackActive = m_CurrentDirectory != std::filesystem::path(m_BaseDirectory);
		m_NavigateAssetsHeader.m_IsForwardActive = m_CurrentDirectory != m_LongestRecentPath && !m_LongestRecentPath.empty();
	}
	void ContentBrowserPanel::ResetPanelResources()
	{
		API::FileWatch::EndWatch(m_CurrentDirectory);
		m_BaseDirectory = Projects::ProjectService::GetActiveAssetDirectory();
		m_CurrentDirectory = m_BaseDirectory;
		m_LongestRecentPath.clear();
		m_NavigateAssetsHeader.m_Label = "Assets";
		m_FileFolderViewer.ClearEntries();
		API::FileWatch::StartWatch(m_CurrentDirectory, KG_BIND_CLASS_FN(OnFileWatchUpdate));
		RefreshCachedDirectoryEntries();
	}
}

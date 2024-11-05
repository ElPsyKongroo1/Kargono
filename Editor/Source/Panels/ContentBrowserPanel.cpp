#include "kgpch.h"

#include "Panels/ContentBrowserPanel.h"

#include "EditorApp.h"

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

		// Manage custom file type tooltips
		m_RightClickTooltip.ClearEntries();
		if (fileType == BrowserFileType::Scene)
		{
			EditorUI::TooltipEntry openSceneTooltipEntry{ "Open Scene", [&](EditorUI::TooltipEntry& currentEntry)
			{
				EditorApp::GetCurrentApp()->OpenScene(m_FileToModifyCache);
			} };
			m_RightClickTooltip.AddTooltipEntry(openSceneTooltipEntry);
		}
		if (fileType == BrowserFileType::Font)
		{
			EditorUI::TooltipEntry useFontTooltipEntry{ "Use Font In Current User Interface", [&](EditorUI::TooltipEntry& currentEntry)
			{
				Assets::AssetHandle currentHandle = Assets::AssetService::ImportFontFromFile(m_FileToModifyCache);
				Ref<RuntimeUI::Font> font = Assets::AssetService::GetFont(currentHandle);
				if (font)
				{
					RuntimeUI::RuntimeUIService::SetActiveFont(font, currentHandle);
				}
				else 
				{ 
					KG_WARN("Could not load font {0}", m_FileToModifyCache.filename().string()); 
				}
			} };
			m_RightClickTooltip.AddTooltipEntry(useFontTooltipEntry);
		}

		// Manage files tooltip options
		if (fileType != BrowserFileType::Directory)
		{
			EditorUI::TooltipEntry openFileTooltipEntry{ "Open File in Text Editor", [&](EditorUI::TooltipEntry& currentEntry)
			{
					// Open the file in the text editor
					s_EditorApp->m_TextEditorPanel->OpenFile(m_FileToModifyCache);
				} };
			m_RightClickTooltip.AddTooltipEntry(openFileTooltipEntry);
		}
		if (fileType != BrowserFileType::Binary &&
			fileType != BrowserFileType::Directory &&
			fileType != BrowserFileType::Registry &&
			fileType != BrowserFileType::Scene)
		{
			EditorUI::TooltipEntry renameFileTooltipEntry{ "Rename File", [&](EditorUI::TooltipEntry& currentEntry)
			{
				m_RenameFilePopup.PopupActive = true;
			} };
			m_RightClickTooltip.AddTooltipEntry(renameFileTooltipEntry);
		}
		if (fileType != BrowserFileType::Registry && fileType != BrowserFileType::Directory)
		{
			EditorUI::TooltipEntry deleteFileTooltipEntry{ "Delete File", [&](EditorUI::TooltipEntry& currentEntry)
			{
				m_DeleteFilePopup.PopupActive = true;
			} };
			m_RightClickTooltip.AddTooltipEntry(deleteFileTooltipEntry);
		}

		// Manage directories tooltip options
		if (fileType == BrowserFileType::Directory)
		{
			EditorUI::TooltipEntry deleteDirectoryTooltipEntry{ "Delete Directory", [&](EditorUI::TooltipEntry& currentEntry)
			{
				m_DeleteDirectoryPopup.PopupActive = true;
			} };
			m_RightClickTooltip.AddTooltipEntry(deleteDirectoryTooltipEntry);
		}

	}

	ContentBrowserPanel::ContentBrowserPanel()
		: m_BaseDirectory(Projects::ProjectService::GetActiveAssetDirectory()), m_CurrentDirectory(m_BaseDirectory)
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(ContentBrowserPanel::OnKeyPressedEditor));
		s_EditorApp->m_PanelToMousePressedInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(ContentBrowserPanel::OnMouseButton));

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
		m_FileFolderViewer.m_Flags = EditorUI::GridFlags::Grid_AllowDragDrop;

		// Initialize grid archetypes
		EditorUI::GridEntryArchetype directoryArch;
		directoryArch.m_Icon = EditorUI::EditorUIService::s_IconDirectory;
		directoryArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		directoryArch.m_OnDoubleLeftClick = KG_BIND_CLASS_FN(OnGridDirectoryDoubleClick);
		directoryArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		directoryArch.m_OnReceivePayload = KG_BIND_CLASS_FN(OnGridReceivePayload);
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
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::GenericFile, genericFileArch);

		
		// Initialize Tooltip
		m_RightClickTooltip.m_Label = "Content browser tooltip";
		// Create open file in text editor entry inside tooltip menu
		EditorUI::TooltipEntry openFileEntry("Open File In Text Editor", [&](EditorUI::TooltipEntry& entry) 
		{
			s_EditorApp->m_TextEditorPanel->OpenFile(m_FileToModifyCache);
		});

		// Initialize delete directory popup
		m_DeleteDirectoryPopup.Label = "Delete Directory";
		m_DeleteDirectoryPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::LabeledText("Directory Name:", m_FileToModifyCache.string().c_str());
		};
		m_DeleteDirectoryPopup.ConfirmAction = [&]()
		{
			Utility::FileSystem::DeleteSelectedDirectory(m_FileToModifyCache);
		};

		// Initialize delete file popup
		m_DeleteFilePopup.Label = "Delete File";
		m_DeleteFilePopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::LabeledText("File Name:", m_FileToModifyCache.string().c_str());
		};
		m_DeleteFilePopup.ConfirmAction = [&]()
		{
			Utility::FileSystem::DeleteSelectedFile(m_FileToModifyCache);
		};

		// Initialize rename file popup
		m_RenameFilePopup.Label = "Rename File";
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

		// Initialize create directory popup
		m_CreateDirectoryPopup.Label = "Create Directory";
		m_CreateDirectoryPopup.PopupAction = [&]()
		{
			m_CreateDirectoryEditName.CurrentOption = "NewName";
		};
		m_CreateDirectoryPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_CreateDirectoryEditName);
		};
		m_CreateDirectoryPopup.ConfirmAction = [&]()
		{
			std::filesystem::path newPath = m_CurrentDirectory / m_CreateDirectoryEditName.CurrentOption;
			Utility::FileSystem::CreateNewDirectory(newPath);
		};

		m_CreateDirectoryEditName.Label = "File Name";

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

		// Handle tooltip
		EditorUI::EditorUIService::Tooltip(m_RightClickTooltip);

		// Handle popups
		EditorUI::EditorUIService::GenericPopup(m_DeleteDirectoryPopup);
		EditorUI::EditorUIService::GenericPopup(m_DeleteFilePopup);
		EditorUI::EditorUIService::GenericPopup(m_RenameFilePopup);
		EditorUI::EditorUIService::GenericPopup(m_CreateDirectoryPopup);

		EditorUI::EditorUIService::EndWindow();

	}
	bool ContentBrowserPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
	bool ContentBrowserPanel::OnMouseButton(Events::MouseButtonPressedEvent event)
	{
		// Ensure right click is applicable
		if (!EditorUI::EditorUIService::IsAnyItemHovered() && event.GetMouseButton() == Mouse::ButtonRight)
		{
			m_RightClickTooltip.ClearEntries();

			// Initialize list of create file options
			std::vector<EditorUI::TooltipEntry> createFileOptions;
			createFileOptions.reserve(8);

			// Add create directory option
			EditorUI::TooltipEntry createDirectoryTooltipEntry{ "Directory", [&](EditorUI::TooltipEntry& currentEntry)
			{
				m_CreateDirectoryPopup.PopupActive = true;
			} };
			createFileOptions.push_back(createDirectoryTooltipEntry);

			// Add create ai state
			EditorUI::TooltipEntry createAIStateTooltipEntry{ "AI State", [&](EditorUI::TooltipEntry& currentEntry)
			{
				// Open create ai state dialog
				s_EditorApp->m_AIStatePanel->OpenCreateDialog(m_CurrentDirectory);
			} };
			createFileOptions.push_back(createAIStateTooltipEntry);
			// Add create game state
			EditorUI::TooltipEntry createGameStateTooltipEntry{ "Game State", [&](EditorUI::TooltipEntry& currentEntry)
			{
				// TODO: Add code to add Game State
				s_EditorApp->m_GameStatePanel->OpenCreateDialog(m_CurrentDirectory);
			} };
			createFileOptions.push_back(createGameStateTooltipEntry);

			// Add create Input Map
			EditorUI::TooltipEntry createInputMapTooltipEntry{ "Input Map", [&](EditorUI::TooltipEntry& currentEntry)
			{
				// TODO: Add code to add Input Map
				s_EditorApp->m_InputMapPanel->OpenCreateDialog(m_CurrentDirectory);
			}};
			createFileOptions.push_back(createInputMapTooltipEntry);

			// Add create Project Component
			EditorUI::TooltipEntry createProjectComponentTooltipEntry{ "Project Component", [&](EditorUI::TooltipEntry& currentEntry)
			{
				// TODO: Add code to add Project Component
				s_EditorApp->m_ProjectComponentPanel->OpenCreateDialog(m_CurrentDirectory);
			} };
			createFileOptions.push_back(createProjectComponentTooltipEntry);

			// Add create Scene
			EditorUI::TooltipEntry createSceneTooltipEntry{ "Scene", [&](EditorUI::TooltipEntry& currentEntry)
			{
				// TODO: Add code to add Scene
					s_EditorApp->NewScene();
			} };
			createFileOptions.push_back(createSceneTooltipEntry);

			// Add create text file
			EditorUI::TooltipEntry creatTextFileTooltipEntry{ "Text File", [&](EditorUI::TooltipEntry& currentEntry)
			{
				// TODO: Add code to add Text File
					s_EditorApp->m_TextEditorPanel->OpenCreateDialog(m_CurrentDirectory);
			} };
			createFileOptions.push_back(creatTextFileTooltipEntry);

			// Add create User Interface
			EditorUI::TooltipEntry createUserInterfaceTooltipEntry{ "User Interface", [&](EditorUI::TooltipEntry& currentEntry)
			{
				// TODO: Add code to add User Interface
				s_EditorApp->m_UIEditorPanel->OpenCreateDialog(m_CurrentDirectory);
			} };
			createFileOptions.push_back(createUserInterfaceTooltipEntry);

			// Create Menu
			EditorUI::TooltipEntry createFileTooltipMenu{ "Create", createFileOptions };
			m_RightClickTooltip.AddTooltipEntry(createFileTooltipMenu);

			// Add open current directory in file explorer
			EditorUI::TooltipEntry openCurrentDirectoryTooltipEntry{ "Open File Explorer", [&](EditorUI::TooltipEntry& currentEntry)
			{
				Utility::OSCommands::OpenFileExplorer(m_CurrentDirectory);
			} };
			m_RightClickTooltip.AddTooltipEntry(openCurrentDirectoryTooltipEntry);

			// Add open current directory in file explorer
			EditorUI::TooltipEntry openTerminalTooltipEntry{ "Open Terminal", [&](EditorUI::TooltipEntry& currentEntry)
			{
				Utility::OSCommands::OpenTerminal(m_CurrentDirectory);
			} };
			m_RightClickTooltip.AddTooltipEntry(openTerminalTooltipEntry);

			// Active tooltip
			m_RightClickTooltip.TooltipActive = true;
			return true;
		}

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

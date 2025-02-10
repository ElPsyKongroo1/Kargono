#include "kgpch.h"

#include "Windows/MainWindow/ContentBrowserPanel.h"

#include "Kargono/Utility/OSCommands.h"

#include "EditorApp.h"

static Kargono::EditorApp* s_EditorApp{ nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };

namespace Kargono::Panels
{

	void ContentBrowserPanel::OnFileWatchUpdate(std::string_view name, const API::FileWatch::EventType change_type)
	{
		UNREFERENCED_PARAMETER(name);
		UNREFERENCED_PARAMETER(change_type);
		EngineService::SubmitToMainThread([&]()
		{
			s_MainWindow->m_ContentBrowserPanel->RefreshCachedDirectoryEntries();
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
		if (!m_NavigateAssetsHeader.m_IsBackActive)
		{
			return;
		}
		
		UpdateCurrentDirectory(m_CurrentDirectory.parent_path());
	}

	void ContentBrowserPanel::NavigateDirectoryForward()
	{
		if (!m_NavigateAssetsHeader.m_IsForwardActive)
		{
			return;
		}

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
		UNREFERENCED_PARAMETER(payloadName);
		UNREFERENCED_PARAMETER(dataSize);
		// Get original file path from payload
		const wchar_t* payloadPathPointer = (const wchar_t*)dataPointer;
		std::filesystem::path payloadPath(payloadPathPointer);

		// Handle updating internal registry for assets if necessary
		OnHandleMoveAssetLocation(payloadPath, m_CurrentDirectory.parent_path());

		// Move file to new directory
		Utility::FileSystem::MoveFileToDirectory(payloadPath, m_CurrentDirectory.parent_path());
	}

	void ContentBrowserPanel::OnNavHeaderForwardReceivePayload(const char* payloadName, void* dataPointer, std::size_t dataSize)
	{
		UNREFERENCED_PARAMETER(dataSize);
		UNREFERENCED_PARAMETER(payloadName);

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
		static std::string relativePath;
		relativePath = (m_CurrentDirectory / currentEntry.m_Label.CString()).string();
		const char* itemPath = relativePath.c_str();

		// Load up payload
		payload.m_Label = Utility::BrowserFileTypeToPayloadString((BrowserFileType)currentEntry.m_ArchetypeID);
		payload.m_DataPointer = (void*)itemPath;
		payload.m_DataSize = (std::strlen(itemPath) + 1) * sizeof(char);
		
	}

	void ContentBrowserPanel::OnGridReceivePayload(EditorUI::GridEntry& currentEntry, const char* payloadName, void* dataPointer, std::size_t dataSize)
	{
		UNREFERENCED_PARAMETER(payloadName);
		UNREFERENCED_PARAMETER(dataSize);

		// Get absolute path of the in-grid directory
		std::filesystem::path newDirectoryPath{ m_CurrentDirectory / currentEntry.m_Label.CString() };

		// Handle receiving payload (Get directory of file...)
		const char* payloadPathPointer = (const char*)dataPointer;
		std::filesystem::path payloadPath(payloadPathPointer);

		// Handle updating internal registry for assets if necessary
		OnHandleMoveAssetLocation(payloadPath, newDirectoryPath);

		// Move the file
		Utility::FileSystem::MoveFileToDirectory(payloadPath, newDirectoryPath);
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
		m_CurrentFileToModifyCache = m_CurrentDirectory / currentEntry.m_Label.CString();

		// Enable tooltip menu
		m_RightClickTooltip.m_TooltipActive = true;
		BrowserFileType fileType = (BrowserFileType)currentEntry.m_ArchetypeID;
		m_CurrentFileTypeCache = fileType;

		// Manage custom file type tooltips
		m_RightClickTooltip.ClearEntries();
		if (fileType == BrowserFileType::AIState)
		{
			EditorUI::TooltipEntry openAIStateTooltipEntry{ "Open AI State", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				s_MainWindow->m_AIStatePanel->OpenAssetInEditor(m_CurrentFileToModifyCache);
			} };
			m_RightClickTooltip.AddTooltipEntry(openAIStateTooltipEntry);
		}
		else if (fileType == BrowserFileType::GameState)
		{
			EditorUI::TooltipEntry openGameStateTooltipEntry{ "Open Game State", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				s_MainWindow->m_GameStatePanel->OpenAssetInEditor(m_CurrentFileToModifyCache);
			} };
			m_RightClickTooltip.AddTooltipEntry(openGameStateTooltipEntry);
		}
		else if (fileType == BrowserFileType::EmitterConfig)
		{
			EditorUI::TooltipEntry openEmitterConfigTooltipEntry{ "Open Emitter Config", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				s_EditorApp->m_EmitterConfigEditorWindow->OpenAssetInEditor(m_CurrentFileToModifyCache);
			} };
			m_RightClickTooltip.AddTooltipEntry(openEmitterConfigTooltipEntry);
		}
		else if (fileType == BrowserFileType::InputMap)
		{
			EditorUI::TooltipEntry openInputMapTooltipEntry{ "Open Input Map", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				s_MainWindow->m_InputMapPanel->OpenAssetInEditor(m_CurrentFileToModifyCache);
			} };
			m_RightClickTooltip.AddTooltipEntry(openInputMapTooltipEntry);
		}
		else if (fileType == BrowserFileType::ProjectComponent)
		{
			EditorUI::TooltipEntry openProjectComponentTooltipEntry{ "Open Project Component", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				s_MainWindow->m_ProjectComponentPanel->OpenAssetInEditor(m_CurrentFileToModifyCache);
			} };
			m_RightClickTooltip.AddTooltipEntry(openProjectComponentTooltipEntry);
		}
		else if (fileType == BrowserFileType::ProjectEnum)
		{
			EditorUI::TooltipEntry openProjectEnumTooltipEntry{ "Open Project Enum", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				s_MainWindow->m_ProjectEnumPanel->OpenAssetInEditor(m_CurrentFileToModifyCache);
			} };
			m_RightClickTooltip.AddTooltipEntry(openProjectEnumTooltipEntry);
		}
		else if (fileType == BrowserFileType::Script)
		{
			EditorUI::TooltipEntry openScriptTooltipEntry{ "Open Script", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				// Open the file in the text editor
				s_MainWindow->m_TextEditorPanel->OpenFile(m_CurrentFileToModifyCache);
			} };
			m_RightClickTooltip.AddTooltipEntry(openScriptTooltipEntry);
		}

		else if (fileType == BrowserFileType::Scene)
		{
			EditorUI::TooltipEntry openSceneTooltipEntry{ "Open Scene", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				s_MainWindow->OpenScene(m_CurrentFileToModifyCache);
			} };
			m_RightClickTooltip.AddTooltipEntry(openSceneTooltipEntry);
		}

		else if (fileType == BrowserFileType::UserInterface)
		{
			EditorUI::TooltipEntry openUserInterfaceTooltipEntry{ "Open User Interface", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				s_EditorApp->m_UIEditorWindow->OpenAssetInEditor(m_CurrentFileToModifyCache);
			} };
			m_RightClickTooltip.AddTooltipEntry(openUserInterfaceTooltipEntry);
		}

		else if (fileType == BrowserFileType::RawAudio)
		{
			EditorUI::TooltipEntry importAudioTooltipEntry{ "Import Audio", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				// Open import font dialog
				s_MainWindow->OpenImportFileDialog(m_CurrentFileToModifyCache, Assets::AssetType::Audio);
			} };
			m_RightClickTooltip.AddTooltipEntry(importAudioTooltipEntry);
		}

		else if (fileType == BrowserFileType::RawTexture)
		{
			EditorUI::TooltipEntry importTextureTooltipEntry{ "Import Texture", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				// Open import font dialog
				s_MainWindow->OpenImportFileDialog(m_CurrentFileToModifyCache, Assets::AssetType::Texture);
			} };
			m_RightClickTooltip.AddTooltipEntry(importTextureTooltipEntry);
		}

		else if (fileType == BrowserFileType::RawFont)
		{
			EditorUI::TooltipEntry importFontTooltipEntry{ "Import Font", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				// Open import font dialog
				s_MainWindow->OpenImportFileDialog(m_CurrentFileToModifyCache, Assets::AssetType::Font);
			} };
			m_RightClickTooltip.AddTooltipEntry(importFontTooltipEntry);

			EditorUI::TooltipEntry useFontTooltipEntry{ "Use Font In Current User Interface", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				Assets::AssetHandle currentHandle = Assets::AssetService::ImportFontFromFile(m_CurrentFileToModifyCache);
				Ref<RuntimeUI::Font> font = Assets::AssetService::GetFont(currentHandle);
				if (font)
				{
					RuntimeUI::RuntimeUIService::SetActiveFont(font, currentHandle);
				}
				else 
				{ 
					KG_WARN("Could not load font {0}", m_CurrentFileToModifyCache.filename().string()); 
				}
			} };
			m_RightClickTooltip.AddTooltipEntry(useFontTooltipEntry);
		}

		// Manage files tooltip options
		if (fileType != BrowserFileType::Directory && fileType != BrowserFileType::Script && 
			fileType != BrowserFileType::RawTexture &&
			fileType != BrowserFileType::RawAudio &&
			fileType != BrowserFileType::RawFont)
		{
			EditorUI::TooltipEntry openFileTooltipEntry{ "Open File in Text Editor", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				// Open the file in the text editor
				s_MainWindow->m_TextEditorPanel->OpenFile(m_CurrentFileToModifyCache);
			} };
			m_RightClickTooltip.AddTooltipEntry(openFileTooltipEntry);
		}
		if (fileType != BrowserFileType::Registry && fileType != BrowserFileType::Directory)
		{
			EditorUI::TooltipEntry deleteFileTooltipEntry{ "Delete File", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				m_DeleteFilePopup.m_OpenPopup = true;
#if 0
				switch (fileType)
				{
					case BrowserFileType::
				case BrowserFileType::RawAudio:
				case BrowserFileType::RawFont:
				case BrowserFileType::RawTexture:
					m_DeleteFilePopup.m_Label = "Delete File";

				}
#endif
			} };
			m_RightClickTooltip.AddTooltipEntry(deleteFileTooltipEntry);
		}

		// Manage directories tooltip options
		if (fileType == BrowserFileType::Directory)
		{
			EditorUI::TooltipEntry deleteDirectoryTooltipEntry{ "Delete Directory", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				m_DeleteDirectoryPopup.m_OpenPopup = true;
			} };
			m_RightClickTooltip.AddTooltipEntry(deleteDirectoryTooltipEntry);
		}

	}

	void ContentBrowserPanel::OnHandleMoveAssetLocation(const std::filesystem::path& originalFilePath, const std::filesystem::path& newDirectory)
	{
		// Get file extension and relative path for payload file
		std::filesystem::path fileExtension = originalFilePath.extension();
		std::filesystem::path relativeToAssetsDirFilePath{ Utility::FileSystem::GetRelativePath(Projects::ProjectService::GetActiveAssetDirectory(), originalFilePath) };
		std::filesystem::path newRelativePath{ Utility::FileSystem::GetRelativePath(Projects::ProjectService::GetActiveAssetDirectory(), newDirectory / originalFilePath.filename()) };
		if (fileExtension == ".kgstate")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetGameStateHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If game state in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a game state, however, no game state could be found in registry. Moving the indicated file without updating registry.");
			}
			else
			{
				// Handle revalidating internal registry
				bool success = Assets::AssetService::SetGameStateFileLocation(resultHandle, newRelativePath);
				if (success)
				{
					KG_INFO("Updated location of game state asset {} to {}", relativeToAssetsDirFilePath.string(), newRelativePath.string());
				}
				else
				{
					KG_WARN("Could not update game state location in registry");
				}
			}
		}
		else if (fileExtension == ".kgaistate")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetAIStateHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If AI state in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a AI state, however, no AI state could be found in registry. Moving the indicated file without updating registry.");

			}
			else
			{
				// Handle revalidating internal registry
				bool success = Assets::AssetService::SetAIStateFileLocation(resultHandle, newRelativePath);
				if (success)
				{
					KG_INFO("Updated location of AI state asset {} to {}", relativeToAssetsDirFilePath.string(), newRelativePath.string());
				}
				else
				{
					KG_WARN("Could not update AI state location in registry");
				}
			}
		}
		else if (fileExtension == ".kgaudio")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetAudioBufferHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If audio buffer in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a audio buffer, however, no audio buffer could be found in registry. Moving the indicated file without updating registry.");

			}
			else
			{
				// Handle revalidating internal registry
				bool success = Assets::AssetService::SetAudioBufferFileLocation(resultHandle, newRelativePath);
				if (success)
				{
					KG_INFO("Updated location of audio buffer asset {} to {}", relativeToAssetsDirFilePath.string(), newRelativePath.string());
				}
				else
				{
					KG_WARN("Could not update audio buffer location in registry");
				}
			}
		}
		else if (fileExtension == ".kgparticle")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetEmitterConfigHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If EmitterConfig in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a EmitterConfig, however, no EmitterConfig could be found in registry. Moving the indicated file without updating registry.");
			}
			else
			{
				// Handle revalidating internal registry
				bool success = Assets::AssetService::SetEmitterConfigFileLocation(resultHandle, newRelativePath);
				if (success)
				{
					KG_INFO("Updated location of EmitterConfig asset {} to {}", relativeToAssetsDirFilePath.string(), newRelativePath.string());
				}
				else
				{
					KG_WARN("Could not update EmitterConfig location in registry");
				}
			}
		}
		else if (fileExtension == ".kgfont")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetFontHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If font in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a font, however, no font could be found in registry. Moving the indicated file without updating registry.");

			}
			else
			{
				// Handle revalidating internal registry
				bool success = Assets::AssetService::SetFontFileLocation(resultHandle, newRelativePath);
				if (success)
				{
					KG_INFO("Updated location of font asset {} to {}", relativeToAssetsDirFilePath.string(), newRelativePath.string());
				}
				else
				{
					KG_WARN("Could not update font location in registry");
				}
			}
		}
		else if (fileExtension == ".kginput")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetInputMapHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If input map in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a input map, however, no input map could be found in registry. Moving the indicated file without updating registry.");

			}
			else
			{
				// Handle revalidating internal registry
				bool success = Assets::AssetService::SetInputMapFileLocation(resultHandle, newRelativePath);
				if (success)
				{
					KG_INFO("Updated location of input map asset {} to {}", relativeToAssetsDirFilePath.string(), newRelativePath.string());
				}
				else
				{
					KG_WARN("Could not update input map location in registry");
				}
			}
		}
		else if (fileExtension == ".kgcomponent")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetProjectComponentHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If project component in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a project component, however, no project component could be found in registry. Moving the indicated file without updating registry.");

			}
			else
			{
				// Handle revalidating internal registry
				bool success = Assets::AssetService::SetProjectComponentFileLocation(resultHandle, newRelativePath);
				if (success)
				{
					KG_INFO("Updated location of project component asset {} to {}", relativeToAssetsDirFilePath.string(), newRelativePath.string());
				}
				else
				{
					KG_WARN("Could not update project component location in registry");
				}
			}
		}
		else if (fileExtension == ".kgscene")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetSceneHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If scene in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a scene, however, no scene could be found in registry. Moving the indicated file without updating registry.");

			}
			else
			{
				// Handle revalidating internal registry
				bool success = Assets::AssetService::SetSceneFileLocation(resultHandle, newRelativePath);
				if (success)
				{
					KG_INFO("Updated location of scene asset {} to {}", relativeToAssetsDirFilePath.string(), newRelativePath.string());
				}
				else
				{
					KG_WARN("Could not update scene location in registry");
				}
			}
		}
		else if (fileExtension == ".kgscript")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetScriptHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If script in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a script, however, no script could be found in registry. Moving the indicated file without updating registry.");

			}
			else
			{
				// Handle revalidating internal registry
				bool success = Assets::AssetService::SetScriptFileLocation(resultHandle, newRelativePath);
				if (success)
				{
					KG_INFO("Updated location of script asset {} to {}", relativeToAssetsDirFilePath.string(), newRelativePath.string());
				}
				else
				{
					KG_WARN("Could not update script location in registry");
				}
			}
		}
		else if (fileExtension == ".kgenum")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetProjectEnumHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If ProjectEnum in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a ProjectEnum, however, no ProjectEnum could be found in registry. Moving the indicated file without updating registry.");

			}
			else
			{
				// Handle revalidating internal registry
				bool success = Assets::AssetService::SetProjectEnumFileLocation(resultHandle, newRelativePath);
				if (success)
				{
					KG_INFO("Updated location of ProjectEnum asset {} to {}", relativeToAssetsDirFilePath.string(), newRelativePath.string());
				}
				else
				{
					KG_WARN("Could not update ProjectEnum location in registry");
				}
			}
		}
		else if (fileExtension == ".kgtexture")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetTexture2DHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If texture in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a texture, however, no texture could be found in registry. Moving the indicated file without updating registry.");

			}
			else
			{
				// Handle revalidating internal registry
				bool success = Assets::AssetService::SetTexture2DFileLocation(resultHandle, newRelativePath);
				if (success)
				{
					KG_INFO("Updated location of texture asset {} to {}", relativeToAssetsDirFilePath.string(), newRelativePath.string());
				}
				else
				{
					KG_WARN("Could not update texture location in registry");
				}
			}
		}

		else if (fileExtension == ".kgui")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetUserInterfaceHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If user interface in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a user interface, however, no user interface could be found in registry. Moving the indicated file without updating registry.");

			}
			else
			{
				// Handle revalidating internal registry
				bool success = Assets::AssetService::SetUserInterfaceFileLocation(resultHandle, newRelativePath);
				if (success)
				{
					KG_INFO("Updated location of user interface asset {} to {}", relativeToAssetsDirFilePath.string(), newRelativePath.string());
				}
				else
				{
					KG_WARN("Could not update user interface location in registry");
				}
			}
		}
	}

	void ContentBrowserPanel::OnHandleDeleteFile()
	{
		// TODO: Fix this implementation. Probably should use a switch
		std::filesystem::path currentExtension = m_CurrentFileToModifyCache.extension();
		std::filesystem::path relativeToAssetsDirFilePath{ Utility::FileSystem::GetRelativePath(Projects::ProjectService::GetActiveAssetDirectory(), m_CurrentFileToModifyCache) };
		if (currentExtension == ".kgstate")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetGameStateHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If game state in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a game state, however, no game state could be found in registry. Deleting the file provided.");
				Utility::FileSystem::DeleteSelectedFile(m_CurrentFileToModifyCache);
				return;
			}

			Assets::AssetService::DeleteGameState(resultHandle);
		}

		else if (currentExtension == ".kgaistate")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetAIStateHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If game state in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as an ai state, however, no ai state could be found in registry. Deleting the file provided.");
				Utility::FileSystem::DeleteSelectedFile(m_CurrentFileToModifyCache);
				return;
			}

			Assets::AssetService::DeleteAIState(resultHandle);
		}
		else if (currentExtension == ".kgaudio")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetAudioBufferHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If audio in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as an audio asset, however, no audio asset could be found in registry. Deleting the file provided.");
				Utility::FileSystem::DeleteSelectedFile(m_CurrentFileToModifyCache);
				return;
			}

			Assets::AssetService::DeleteAudioBuffer(resultHandle);
		}
		else if (currentExtension == ".kgparticle")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetEmitterConfigHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If EmitterConfig in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a EmitterConfig asset, however, no EmitterConfig asset could be found in registry. Deleting the file provided.");
				Utility::FileSystem::DeleteSelectedFile(m_CurrentFileToModifyCache);
				return;
			}

			Assets::AssetService::DeleteEmitterConfig(resultHandle);
		}

		else if (currentExtension == ".kgfont")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetFontHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If font in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a font asset, however, no font asset could be found in registry. Deleting the file provided.");
				Utility::FileSystem::DeleteSelectedFile(m_CurrentFileToModifyCache);
				return;
			}

			Assets::AssetService::DeleteFont(resultHandle);
		}
		else if (currentExtension == ".kgenum")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetProjectEnumHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If ProjectEnum in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a ProjectEnum asset, however, no ProjectEnum asset could be found in registry. Deleting the file provided.");
				Utility::FileSystem::DeleteSelectedFile(m_CurrentFileToModifyCache);
				return;
			}

			Assets::AssetService::DeleteProjectEnum(resultHandle);
		}
		else if (currentExtension == ".kginput")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetInputMapHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If input map in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a input map asset, however, no input map asset could be found in registry. Deleting the file provided.");
				Utility::FileSystem::DeleteSelectedFile(m_CurrentFileToModifyCache);
				return;
			}
			Assets::AssetService::DeleteInputMap(resultHandle);
		}
		else if (currentExtension == ".kgcomponent")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetProjectComponentHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If project component in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a project component asset, however, no project component asset could be found in registry. Deleting the file provided.");
				Utility::FileSystem::DeleteSelectedFile(m_CurrentFileToModifyCache);
				return;
			}
			Assets::AssetService::DeleteProjectComponent(resultHandle);
		}
		else if (currentExtension == ".kgscene")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetSceneHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If scene in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a scene asset, however, no scene asset could be found in registry. Deleting the file provided.");
				Utility::FileSystem::DeleteSelectedFile(m_CurrentFileToModifyCache);
				return;
			}
			Assets::AssetService::DeleteScene(resultHandle);
			s_MainWindow->LoadSceneParticleEmitters();
		}
		else if (currentExtension == ".kgscript")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetScriptHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If script in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a script asset, however, no script asset could be found in registry. Deleting the file provided.");
				Utility::FileSystem::DeleteSelectedFile(m_CurrentFileToModifyCache);
				return;
			}
			Assets::AssetService::DeleteScript(resultHandle);
		}
		else if (currentExtension == ".kgtexture")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetTexture2DHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If texture in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a texture asset, however, no texture asset could be found in registry. Deleting the file provided.");
				Utility::FileSystem::DeleteSelectedFile(m_CurrentFileToModifyCache);
				return;
			}
			Assets::AssetService::DeleteTexture2D(resultHandle);
		}
		else if (currentExtension == ".kgui")
		{
			// Search registry for asset with identical file location
			Assets::AssetHandle resultHandle = Assets::AssetService::GetUserInterfaceHandleFromFileLocation(relativeToAssetsDirFilePath);
			// If user interface in registry is not found, simply delete the file
			if (resultHandle == Assets::EmptyHandle)
			{
				KG_WARN("File extension recognized as a user interface asset, however, no user interface asset could be found in registry. Deleting the file provided.");
				Utility::FileSystem::DeleteSelectedFile(m_CurrentFileToModifyCache);
				return;
			}
			Assets::AssetService::DeleteUserInterface(resultHandle);
		}
		else
		{
			Utility::FileSystem::DeleteSelectedFile(m_CurrentFileToModifyCache);
		}
	}

	ContentBrowserPanel::ContentBrowserPanel()
		: m_BaseDirectory(Projects::ProjectService::GetActiveAssetDirectory()), m_CurrentDirectory(m_BaseDirectory)
	{

		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
		s_MainWindow->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(ContentBrowserPanel::OnKeyPressedEditor));
		s_MainWindow->m_PanelToMousePressedInput.insert_or_assign(m_PanelName.CString(),
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
		navPayloadList = s_ContentBrowserPayloads;

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
		navPayloadList = s_ContentBrowserPayloads;
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::Directory, directoryArch);

		EditorUI::GridEntryArchetype rawTextureArch;
		rawTextureArch.m_Icon = EditorUI::EditorUIService::s_IconTexture;
		rawTextureArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		rawTextureArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::RawTexture, rawTextureArch);

		EditorUI::GridEntryArchetype rawAudioArch;
		rawAudioArch.m_Icon = EditorUI::EditorUIService::s_IconAudio;
		rawAudioArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		rawAudioArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::RawAudio, rawAudioArch);

		EditorUI::GridEntryArchetype rawFontArch;
		rawFontArch.m_Icon = EditorUI::EditorUIService::s_IconFont;
		rawFontArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		rawFontArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::RawFont, rawFontArch);

		EditorUI::GridEntryArchetype aiStateArch;
		aiStateArch.m_Icon = EditorUI::EditorUIService::s_IconAI_KG;
		aiStateArch.m_IconColor = EditorUI::EditorUIService::s_HighlightColor1_Thin;
		aiStateArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		aiStateArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::AIState, aiStateArch);

		EditorUI::GridEntryArchetype audioArch;
		audioArch.m_Icon = EditorUI::EditorUIService::s_IconAudio_KG;
		audioArch.m_IconColor = EditorUI::EditorUIService::s_HighlightColor1_Thin;
		audioArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		audioArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::Audio, audioArch);

		EditorUI::GridEntryArchetype binaryArch;
		binaryArch.m_Icon = EditorUI::EditorUIService::s_IconBinary;
		binaryArch.m_IconColor = EditorUI::EditorUIService::s_HighlightColor4_Thin;
		binaryArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		binaryArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::Binary, binaryArch);

		EditorUI::GridEntryArchetype fontArch;
		fontArch.m_Icon = EditorUI::EditorUIService::s_IconFont_KG;
		fontArch.m_IconColor = EditorUI::EditorUIService::s_HighlightColor1_Thin;
		fontArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		fontArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::Font, fontArch);

		EditorUI::GridEntryArchetype gameStateArch;
		gameStateArch.m_Icon = EditorUI::EditorUIService::s_IconGameState;
		gameStateArch.m_IconColor = EditorUI::EditorUIService::s_HighlightColor1_Thin;
		gameStateArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		gameStateArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::GameState, gameStateArch);

		EditorUI::GridEntryArchetype emitterConfigArch;
		emitterConfigArch.m_Icon = EditorUI::EditorUIService::s_IconEmitterConfig;
		emitterConfigArch.m_IconColor = EditorUI::EditorUIService::s_HighlightColor1_Thin;
		emitterConfigArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		emitterConfigArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::EmitterConfig, emitterConfigArch);

		EditorUI::GridEntryArchetype inputMapArch;
		inputMapArch.m_Icon = EditorUI::EditorUIService::s_IconInput;
		inputMapArch.m_IconColor = EditorUI::EditorUIService::s_HighlightColor1_Thin;
		inputMapArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		inputMapArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::InputMap, inputMapArch);

		EditorUI::GridEntryArchetype projectComponentArch;
		projectComponentArch.m_Icon = EditorUI::EditorUIService::s_IconProjectComponent;
		projectComponentArch.m_IconColor = EditorUI::EditorUIService::s_HighlightColor1_Thin;
		projectComponentArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		projectComponentArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::ProjectComponent, projectComponentArch);
		
		EditorUI::GridEntryArchetype registryArch;
		registryArch.m_Icon = EditorUI::EditorUIService::s_IconRegistry;
		registryArch.m_IconColor = EditorUI::EditorUIService::s_HighlightColor4_Thin;
		registryArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		registryArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::Registry, registryArch);

		EditorUI::GridEntryArchetype sceneArch;
		sceneArch.m_Icon = EditorUI::EditorUIService::s_IconScene_KG;
		sceneArch.m_IconColor = EditorUI::EditorUIService::s_HighlightColor3_Thin;
		sceneArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		sceneArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::Scene, sceneArch);

		EditorUI::GridEntryArchetype scriptArch;
		scriptArch.m_Icon = EditorUI::EditorUIService::s_IconScript;
		scriptArch.m_IconColor = EditorUI::EditorUIService::s_HighlightColor2_Thin;
		scriptArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		scriptArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::Script, scriptArch);

		EditorUI::GridEntryArchetype projectEnumArch;
		projectEnumArch.m_Icon = EditorUI::EditorUIService::s_IconEnum;
		projectEnumArch.m_IconColor = EditorUI::EditorUIService::s_HighlightColor2_Thin;
		projectEnumArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		projectEnumArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::ProjectEnum, projectEnumArch);

		EditorUI::GridEntryArchetype textureArch;
		textureArch.m_Icon = EditorUI::EditorUIService::s_IconTexture_KG;
		textureArch.m_IconColor = EditorUI::EditorUIService::s_HighlightColor1_Thin;
		textureArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		textureArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::Texture, textureArch);

		EditorUI::GridEntryArchetype userInterfaceArch;
		userInterfaceArch.m_Icon = EditorUI::EditorUIService::s_IconUserInterface;
		userInterfaceArch.m_IconColor = EditorUI::EditorUIService::s_HighlightColor1_Thin;
		userInterfaceArch.m_OnRightClick = KG_BIND_CLASS_FN(OnGridHandleRightClick);
		userInterfaceArch.m_OnCreatePayload = KG_BIND_CLASS_FN(OnGridCreatePayload);
		m_FileFolderViewer.AddEntryArchetype((uint32_t)BrowserFileType::UserInterface, userInterfaceArch);

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
			UNREFERENCED_PARAMETER(entry);
			s_MainWindow->m_TextEditorPanel->OpenFile(m_CurrentFileToModifyCache);
		});

		// Initialize delete directory popup
		m_DeleteDirectoryPopup.m_Label = "Delete Directory";
		m_DeleteDirectoryPopup.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::LabeledText("Directory Name:", m_CurrentFileToModifyCache.string().c_str());
		};
		m_DeleteDirectoryPopup.m_ConfirmAction = [&]()
		{
			Utility::FileSystem::DeleteSelectedDirectory(m_CurrentFileToModifyCache);
		};

		// Initialize delete file popup
		m_DeleteFilePopup.m_Label = "Delete File";
		m_DeleteFilePopup.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::LabeledText("File Name:", m_CurrentFileToModifyCache.string().c_str());
		};
		m_DeleteFilePopup.m_ConfirmAction = KG_BIND_CLASS_FN(OnHandleDeleteFile);

		// Initialize create directory popup
		m_CreateDirectoryPopup.m_Label = "Create Directory";
		m_CreateDirectoryPopup.m_PopupAction = [&]()
		{
			m_CreateDirectoryEditName.m_CurrentOption = "NewName";
		};
		m_CreateDirectoryPopup.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_CreateDirectoryEditName);
		};
		m_CreateDirectoryPopup.m_ConfirmAction = [&]()
		{
			std::filesystem::path newPath = m_CurrentDirectory / m_CreateDirectoryEditName.m_CurrentOption;
			Utility::FileSystem::CreateNewDirectory(newPath);
		};

		m_CreateDirectoryEditName.m_Label = "File Name";

	}

	void ContentBrowserPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_MainWindow->m_ShowContentBrowser);

		// Early out of window if not visible
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
		EditorUI::EditorUIService::GenericPopup(m_CreateDirectoryPopup);

		EditorUI::EditorUIService::EndWindow();

	}
	bool ContentBrowserPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		if (event.GetKeyCode() == Key::Escape)
		{
			m_FileFolderViewer.ClearSelectedEntry();
		}

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
			createFileOptions.reserve(7);

			// Add create ai state
			createFileOptions.emplace_back("AI State", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				// Open create ai state dialog
				s_MainWindow->m_AIStatePanel->OpenCreateDialog(m_CurrentDirectory);
			});
			// Add create game state
			createFileOptions.emplace_back("Game State", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				// TODO: Add code to add Game State
				s_MainWindow->m_GameStatePanel->OpenCreateDialog(m_CurrentDirectory);
			});

			// Add create Input Map
			EditorUI::TooltipEntry createInputMapTooltipEntry{ "Input Map", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				// TODO: Add code to add Input Map
				s_MainWindow->m_InputMapPanel->OpenCreateDialog(m_CurrentDirectory);
			}};
			createFileOptions.push_back(createInputMapTooltipEntry);

			// Add create Project Component
			EditorUI::TooltipEntry createProjectComponentTooltipEntry{ "Project Component", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				// TODO: Add code to add Project Component
				s_MainWindow->m_ProjectComponentPanel->OpenCreateDialog(m_CurrentDirectory);
			} };
			createFileOptions.push_back(createProjectComponentTooltipEntry);

			// Add create Project Enum
			EditorUI::TooltipEntry createProjectEnumTooltipEntry{ "Project Enum", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				// TODO: Add code to add Project Enum
				s_MainWindow->m_ProjectEnumPanel->OpenCreateDialog(m_CurrentDirectory);
			} };
			createFileOptions.push_back(createProjectEnumTooltipEntry);

			// Add create Scene
			EditorUI::TooltipEntry createSceneTooltipEntry{ "Scene", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				// TODO: Add code to add Scene
				s_MainWindow->NewSceneDialog();
			} };
			createFileOptions.push_back(createSceneTooltipEntry);

			// Add create text file
			EditorUI::TooltipEntry creatTextFileTooltipEntry{ "Text File", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				// TODO: Add code to add Text File
				s_MainWindow->m_TextEditorPanel->OpenCreateDialog(m_CurrentDirectory);
			} };
			createFileOptions.push_back(creatTextFileTooltipEntry);

			// Add create User Interface
			EditorUI::TooltipEntry createUserInterfaceTooltipEntry{ "User Interface", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				// TODO: Add code to add User Interface
				s_EditorApp->m_UIEditorWindow->OpenCreateDialog(m_CurrentDirectory);
			} };
			createFileOptions.push_back(createUserInterfaceTooltipEntry);

			// Add asset creation menu
			EditorUI::TooltipEntry createFileTooltipMenu{ "Create Asset", createFileOptions };
			m_RightClickTooltip.AddTooltipEntry(createFileTooltipMenu);

			// Add create directory option
			EditorUI::TooltipEntry createDirectoryTooltipEntry{ "Create Directory", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				m_CreateDirectoryPopup.m_OpenPopup = true;
			} };
			m_RightClickTooltip.AddTooltipEntry(createDirectoryTooltipEntry);

			// Add seperator
			m_RightClickTooltip.AddSeperator(EditorUI::EditorUIService::s_HighlightColor1_Thin);

			// Add open current directory in file explorer
			EditorUI::TooltipEntry openCurrentDirectoryTooltipEntry{ "Open File Explorer", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				Utility::OSCommands::OpenFileExplorer(m_CurrentDirectory);
			} };
			m_RightClickTooltip.AddTooltipEntry(openCurrentDirectoryTooltipEntry);

			// Add open current directory in file explorer
			EditorUI::TooltipEntry openTerminalTooltipEntry{ "Open Terminal", [&](EditorUI::TooltipEntry& currentEntry)
			{
				UNREFERENCED_PARAMETER(currentEntry);
				Utility::OSCommands::OpenTerminal(m_CurrentDirectory);
			} };
			m_RightClickTooltip.AddTooltipEntry(openTerminalTooltipEntry);

			// Active tooltip
			m_RightClickTooltip.m_TooltipActive = true;
			return true;
		}

		// Handle navigating forward and back directories with mouse
		if (event.GetMouseButton() == Mouse::ButtonBack)
		{
			NavigateDirectoryBack();
		}
		else if (event.GetMouseButton() == Mouse::ButtonForward)
		{
			NavigateDirectoryForward();
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

#include "kgpch.h"

#include "Panels/ContentBrowserPanel.h"

#include "Kargono.h"
#include "EditorApp.h"
#include "API/FileWatch/FileWatchAPI.h"


namespace Kargono::Utility
{
	static Ref<Rendering::Texture2D> BrowserFileTypeToIcon(Panels::BrowserFileType type)
	{
		switch (type)
		{
		case Panels::BrowserFileType::Directory: { return EditorUI::Editor::s_DirectoryIcon; }
		case Panels::BrowserFileType::Image: { return EditorUI::Editor::s_ImageIcon; }
		case Panels::BrowserFileType::Audio: { return EditorUI::Editor::s_AudioIcon; }
		case Panels::BrowserFileType::Font: { return EditorUI::Editor::s_FontIcon; }
		case Panels::BrowserFileType::UserInterface: { return EditorUI::Editor::s_UserInterfaceIcon; }
		case Panels::BrowserFileType::Binary: { return EditorUI::Editor::s_BinaryIcon; }
		case Panels::BrowserFileType::Registry: { return EditorUI::Editor::s_RegistryIcon; }
		case Panels::BrowserFileType::Scene: { return EditorUI::Editor::s_SceneIcon; }
		case Panels::BrowserFileType::ScriptProject: { return EditorUI::Editor::s_ScriptProjectIcon; }
		case Panels::BrowserFileType::Input: { return EditorUI::Editor::s_InputIcon; }
		case Panels::BrowserFileType::None: { return EditorUI::Editor::s_GenericFileIcon; }
		}
		KG_ERROR("Invalid BrowserFileType provided");
		return EditorUI::Editor::s_GenericFileIcon;
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
		if (extension == ".csproj") { return Panels::BrowserFileType::ScriptProject; }
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
		case Panels::BrowserFileType::ScriptProject: { return "CONTENT_BROWSER_ITEM"; }
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
	static std::vector<std::filesystem::path> s_CachedDirectoryEntries {};

	void OnFileWatchUpdate(const std::string&, const API::FileWatch::EventType change_type)
	{
		EngineCore::GetCurrentEngineCore().SubmitToMainThread([&]()
		{
			s_EditorApp->m_ContentBrowserPanel->RefreshCachedDirectoryEntries();
		});
	}

	void ContentBrowserPanel::UpdateCurrentDirectory(const std::filesystem::path& newPath)
	{
		static std::filesystem::path currentPath;
		currentPath = newPath;
		EngineCore::GetCurrentEngineCore().SubmitToMainThread([&]()
		{
			API::FileWatch::EndWatch(m_CurrentDirectory);
			m_CurrentDirectory = currentPath;
			API::FileWatch::StartWatch(m_CurrentDirectory, OnFileWatchUpdate);
			RefreshCachedDirectoryEntries();
		});
	}

	ContentBrowserPanel::ContentBrowserPanel()
		: m_BaseDirectory(Projects::Project::GetAssetDirectory()), m_CurrentDirectory(m_BaseDirectory)
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName,
			KG_BIND_CLASS_FN(ContentBrowserPanel::OnKeyPressedEditor));

		API::FileWatch::StartWatch(m_CurrentDirectory, OnFileWatchUpdate);
		RefreshCachedDirectoryEntries();
	}

	void ContentBrowserPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::Editor::StartWindow(m_PanelName, &s_EditorApp->m_ShowContentBrowser);

		static std::filesystem::path s_LongestRecentPath {};
		static float padding = 25.0f;
		static float thumbnailSize = 140;
		float cellSize = thumbnailSize + padding;
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		columnCount = columnCount > 0 ? columnCount : 1;

		bool backActive = m_CurrentDirectory != std::filesystem::path(m_BaseDirectory);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		if (!backActive)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
		}


		if (ImGui::ImageButton((ImTextureID)(uint64_t)(backActive ? EditorUI::Editor::s_BackIcon : EditorUI::Editor::s_BackInactiveIcon)->GetRendererID(),
			{ 24.0f, 24.0f }, { 0, 1 }, { 1, 0 }))
		{
			if (backActive)
			{
				UpdateCurrentDirectory(m_CurrentDirectory.parent_path());
			}
		}
		if (!backActive)
		{
			ImGui::PopStyleColor(2);
		}
		bool forwardActive = m_CurrentDirectory != s_LongestRecentPath && !s_LongestRecentPath.empty();

		ImGui::SameLine();
		if (!forwardActive)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
		}
		if (ImGui::ImageButton((ImTextureID)(uint64_t)(forwardActive ? EditorUI::Editor::s_ForwardIcon : EditorUI::Editor::s_ForwardInactiveIcon)->GetRendererID(), { 24.0f, 24.0f }, { 0, 1 }, { 1, 0 }))
		{
			if (forwardActive && Utility::FileSystem::DoesPathContainSubPath(m_CurrentDirectory, s_LongestRecentPath))
			{
				std::filesystem::path currentIterationPath{s_LongestRecentPath};
				std::filesystem::path recentIterationPath{s_LongestRecentPath};
				while (currentIterationPath != m_CurrentDirectory)
				{
					recentIterationPath = currentIterationPath;
					currentIterationPath = currentIterationPath.parent_path();
				}
				UpdateCurrentDirectory(recentIterationPath);
			}
		}
		if (!forwardActive)
		{
			ImGui::PopStyleColor(2);
		}
		ImGui::PopStyleColor();
		if (ImGui::BeginDragDropTarget())
		{
			static std::array<std::string, 6> acceptablePayloads
			{
				"CONTENT_BROWSER_IMAGE", "CONTENT_BROWSER_AUDIO", "CONTENT_BROWSER_FONT",
					"CONTENT_BROWSER_ITEM", "CONTENT_BROWSER_SCENE", "CONTENT_BROWSER_USERINTERFACE"
			};

			for (auto& payloadName : acceptablePayloads)
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadName.c_str()))
				{
					const wchar_t* payloadPathPointer = (const wchar_t*)payload->Data;
					std::filesystem::path payloadPath(payloadPathPointer);
					KG_CRITICAL(payloadPath.string());
					KG_CRITICAL(m_CurrentDirectory.parent_path());
					Utility::FileSystem::MoveFileToDirectory(payloadPath, m_CurrentDirectory.parent_path());
					KG_CRITICAL("-----------------");
					break;
				}
			}
			ImGui::EndDragDropTarget();
		}

		std::filesystem::path activeDirectory = Utility::FileSystem::GetRelativePath(Projects::Project::GetProjectDirectory(), m_CurrentDirectory);

		std::vector<std::string> tokenizedDirectoryPath{};

		while (activeDirectory.filename() != "Assets")
		{
			tokenizedDirectoryPath.push_back(activeDirectory.filename().string());
			activeDirectory = activeDirectory.parent_path();
		}
		tokenizedDirectoryPath.push_back("Assets");

		ImGui::PushFont(EditorUI::Editor::s_PlexBold);
		for (int32_t i = (int32_t)(tokenizedDirectoryPath.size()) - 1; i >= 0; --i)
		{
			ImGui::SameLine();
			ImGui::Text(tokenizedDirectoryPath.at(i).c_str());
			if (i != 0)
			{
				ImGui::SameLine();
				ImGui::Text("/");
			}
		}
		ImGui::PopFont();

		ImGui::Separator();

		ImGui::Columns(columnCount, 0, false);
		for (auto& directoryEntry: s_CachedDirectoryEntries)
		{
			std::string filenameString = directoryEntry.filename().string();
			BrowserFileType fileType = Utility::DetermineFileType(directoryEntry);
			ImGui::PushID(filenameString.c_str());
			Ref<Rendering::Texture2D> icon = Utility::BrowserFileTypeToIcon(fileType);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), {thumbnailSize, thumbnailSize}, {0, 1}, {1, 0});
			if (ImGui::BeginDragDropSource())
			{
				std::filesystem::path relativePath(directoryEntry);
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload(Utility::BrowserFileTypeToPayloadString(fileType).c_str(),
					itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();

			if (fileType == BrowserFileType::Directory)
			{
				if (ImGui::BeginDragDropTarget())
				{
					static std::array<std::string, 6> acceptablePayloads
					{
						"CONTENT_BROWSER_IMAGE", "CONTENT_BROWSER_AUDIO", "CONTENT_BROWSER_FONT",
							"CONTENT_BROWSER_ITEM", "CONTENT_BROWSER_SCENE", "CONTENT_BROWSER_USERINTERFACE"
					};

					for (auto& payloadName : acceptablePayloads)
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadName.c_str()))
						{
							const wchar_t* payloadPathPointer = (const wchar_t*)payload->Data;
							std::filesystem::path payloadPath(payloadPathPointer);
							Utility::FileSystem::MoveFileToDirectory(payloadPath, directoryEntry);
							break;
						}
					}
					ImGui::EndDragDropTarget();
				}
			}
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if ( std::filesystem::is_directory(directoryEntry))
				{
					UpdateCurrentDirectory(m_CurrentDirectory / directoryEntry.filename());
					if (!Utility::FileSystem::DoesPathContainSubPath(m_CurrentDirectory / directoryEntry.filename(), s_LongestRecentPath))
					{
						s_LongestRecentPath = m_CurrentDirectory / directoryEntry.filename();
					}
				}
			}

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsItemHovered())
			{
				ImGui::OpenPopup("RightClickItemOptions");
			}

			bool openRenamePopup = false;
			bool openDeleteModal = false;
			bool openDeleteDirectory = false;
			static char buffer[256];

			if (ImGui::BeginPopup("RightClickItemOptions"))
			{
				if (fileType != BrowserFileType::Directory)
				{
					if (ImGui::Selectable("Open File In Text Editor"))
					{
						s_EditorApp->m_TextEditorPanel->OpenFile(directoryEntry);
					}
				}

				if (fileType == BrowserFileType::ScriptProject)
				{
					if (ImGui::Selectable("Open Scripting Project"))
					{
						Utility::OSCommands::OpenScriptProject(directoryEntry);
					}
				}

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
						Assets::AssetHandle currentHandle = Assets::AssetManager::ImportNewFontFromFile(directoryEntry);
						Ref<RuntimeUI::Font> font = Assets::AssetManager::GetFont(currentHandle);
						if (font)
						{
							RuntimeUI::Runtime::SetFont(font, currentHandle);
						}

						else { KG_WARN("Could not load font {0}", directoryEntry.filename().string()); }
					}
				}

				if (fileType != BrowserFileType::ScriptProject && fileType != BrowserFileType::Binary &&
					fileType != BrowserFileType::Directory && fileType != BrowserFileType::Registry &&
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

				ImGui::EndPopup();
			}

			if (openDeleteModal) { ImGui::OpenPopup("Delete File"); }
			if (openDeleteDirectory) { ImGui::OpenPopup("Delete Directory"); }
				

			// Always center this window when appearing
			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			if (ImGui::BeginPopupModal("Delete File", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Are you sure you want to delete this file?\n");
				ImGui::Text("%s", directoryEntry.string().c_str());
				ImGui::Separator();

				if (ImGui::Button("OK", ImVec2(120, 0)))
				{
					Utility::FileSystem::DeleteSelectedFile(directoryEntry);
					ImGui::CloseCurrentPopup();
				}
				ImGui::SetItemDefaultFocus();
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}

			if (ImGui::BeginPopupModal("Delete Directory", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Are you sure you want to delete this directory?\n");
				ImGui::Text("%s", directoryEntry.string().c_str());
				ImGui::Separator();

				if (ImGui::Button("OK", ImVec2(120, 0)))
				{
					Utility::FileSystem::DeleteSelectedDirectory(directoryEntry);
					ImGui::CloseCurrentPopup();
				}
				ImGui::SetItemDefaultFocus();
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}


			if (openRenamePopup) { ImGui::OpenPopup("NewFileName"); }
			if (ImGui::BeginPopup("NewFileName"))
			{
				strcpy_s(buffer, directoryEntry.filename().string().c_str());
				ImGui::InputText("New File Name", buffer, sizeof(buffer));
				if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))
				{
					Utility::FileSystem::RenameFile(directoryEntry, std::string(buffer));
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			ImGui::TextWrapped(filenameString.c_str());

			ImGui::NextColumn();

			ImGui::PopID();
			
		}
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
		ImGui::Columns(1);
		

		EditorUI::Editor::EndWindow();

	}
	bool ContentBrowserPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
	void ContentBrowserPanel::RefreshCachedDirectoryEntries()
	{
		s_CachedDirectoryEntries.clear();
		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			s_CachedDirectoryEntries.push_back(directoryEntry.path());
		}
	}
}

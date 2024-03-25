#include "kgpch.h"

#include "Kargono.h"

#include "EditorLayer.h"
#include "Panels/ContentBrowserPanel.h"



namespace Kargono::Utility
{
	static BrowserFileType DetermineFileType(const std::filesystem::directory_entry& entry)
	{
		if (entry.is_directory()) { return BrowserFileType::Directory; }
		if (!entry.path().has_extension()) { return BrowserFileType::None; }
		auto extension = entry.path().extension();
		if (extension == ".jpg" || extension == ".png") { return BrowserFileType::Image; }
		if (extension == ".wav" || extension == ".mp3") { return BrowserFileType::Audio; }
		if (extension == ".kgreg") { return BrowserFileType::Registry; }
		if (extension == ".kgui") { return BrowserFileType::UserInterface; }
		if (extension == ".ttf") { return BrowserFileType::Font; }
		if (extension == ".kgscene") { return BrowserFileType::Scene; }
		if (extension == ".csproj") { return BrowserFileType::ScriptProject; }
		if (extension == ".kginput") { return BrowserFileType::Input; }
		if (extension == ".kgaudio" || extension == ".kgtexture" || extension == ".kgfont" ||
			extension == ".kgshadervert" || extension == ".kgshaderfrag") {
			return BrowserFileType::Binary;
		}
		return BrowserFileType::None;
	}

	static std::string BrowserFileTypeToPayloadString(BrowserFileType type)
	{
		switch (type)
		{
		case BrowserFileType::Directory: { return "CONTENT_BROWSER_DIRECTORY"; }
		case BrowserFileType::Image: { return "CONTENT_BROWSER_IMAGE"; }
		case BrowserFileType::Audio: { return "CONTENT_BROWSER_AUDIO"; }
		case BrowserFileType::UserInterface: { return "CONTENT_BROWSER_USERINTERFACE"; }
		case BrowserFileType::Font: { return "CONTENT_BROWSER_FONT"; }
		case BrowserFileType::Binary: { return "CONTENT_BROWSER_ITEM"; }
		case BrowserFileType::Registry: { return "CONTENT_BROWSER_ITEM"; }
		case BrowserFileType::Scene: { return "CONTENT_BROWSER_SCENE"; }
		case BrowserFileType::ScriptProject: { return "CONTENT_BROWSER_ITEM"; }
		case BrowserFileType::Input: { return "CONTENT_BROWSER_ITEM"; }
		case BrowserFileType::None: { return "CONTENT_BROWSER_ITEM"; }
		}
		KG_ASSERT(false, "Invalid BrowserFileType provided");
		return "CONTENT_BROWSER_ITEM";
	}
}

namespace Kargono
{

	ContentBrowserPanel::ContentBrowserPanel()
		: m_BaseDirectory(Projects::Project::GetAssetDirectory()), m_CurrentDirectory(m_BaseDirectory)
	{
		
	}

	void ContentBrowserPanel::OnEditorUIRender()
	{
		UI::Editor::StartWindow("Content Browser");

		static float padding = 25.0f;
		static float thumbnailSize = 140;
		float cellSize = thumbnailSize + padding;
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		columnCount = columnCount > 0 ? columnCount : 1;

		if (m_CurrentDirectory != std::filesystem::path(m_BaseDirectory))
		{
			if (ImGui::ImageButton((ImTextureID)(uint64_t)UI::Editor::s_BackIcon->GetRendererID(), { 22.4f, 22.4f }, { 0, 1 }, { 1, 0 }))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
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
						FileSystem::MoveFileToDirectory(payloadPath, m_CurrentDirectory.parent_path());
						KG_CRITICAL("-----------------");
						break;
					}
				}
				ImGui::EndDragDropTarget();
			}
		}

		ImGui::SameLine();
		if (ImGui::BeginPopup("Options"))
		{
			ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
			ImGui::SliderFloat("Padding", &padding, 0, 32);
			ImGui::EndPopup();
		}
		// Main window
		if (ImGui::Button("Options", { 70, 28 }))
			ImGui::OpenPopup("Options");

		std::string outputDirectory = "Current Directory: " + FileSystem::GetRelativePath(Projects::Project::GetProjectDirectory(), m_CurrentDirectory).string();

		ImGui::SameLine();
		ImGui::TextWrapped(outputDirectory.c_str());

		ImGui::Separator();

		ImGui::Columns(columnCount, 0, false);

		for (auto& directoryEntry: std::filesystem::directory_iterator(m_CurrentDirectory))
		{

			const auto& path = directoryEntry.path();
			std::string filenameString = path.filename().string();
			BrowserFileType fileType = Utility::DetermineFileType(directoryEntry);
			ImGui::PushID(filenameString.c_str());
			Ref<Texture2D> icon = BrowserFileTypeToIcon(fileType);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), {thumbnailSize, thumbnailSize}, {0, 1}, {1, 0});
			if (ImGui::BeginDragDropSource())
			{
				std::filesystem::path relativePath(path);
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
							FileSystem::MoveFileToDirectory(payloadPath, path);
							break;
						}
					}
					ImGui::EndDragDropTarget();
				}
			}
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (directoryEntry.is_directory()) { m_CurrentDirectory /= path.filename(); }
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
				if (fileType == BrowserFileType::ScriptProject)
				{
					if (ImGui::Selectable("Open Scripting Project"))
					{
						Utility::OSCommands::OpenScriptProject(path);
					}
				}

				if (fileType == BrowserFileType::Scene)
				{
					if (ImGui::Selectable("Open Scene"))
					{
						EditorLayer::GetCurrentLayer()->OpenScene(path);
					}
				}

				if (fileType == BrowserFileType::Font)
				{
					if (ImGui::Selectable("Use Font In Current User Interface"))
					{
						Assets::AssetHandle currentHandle = Assets::AssetManager::ImportNewFontFromFile(path);
						Ref<UI::Font> font = Assets::AssetManager::GetFont(currentHandle);
						if (font)
						{
							UI::Runtime::SetFont(font, currentHandle);
						}

						else { KG_WARN("Could not load font {0}", path.filename().string()); }
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

				if (fileType != BrowserFileType::Registry && fileType != BrowserFileType::Directory &&
					fileType != BrowserFileType::None)
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
				ImGui::Text("%s", path.string().c_str());
				ImGui::Separator();

				if (ImGui::Button("OK", ImVec2(120, 0)))
				{
					FileSystem::DeleteSelectedFile(path);
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
				ImGui::Text("%s", path.string().c_str());
				ImGui::Separator();

				if (ImGui::Button("OK", ImVec2(120, 0)))
				{
					FileSystem::DeleteSelectedDirectory(path);
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
				strcpy_s(buffer, path.filename().string().c_str());
				ImGui::InputText("New File Name", buffer, sizeof(buffer));
				if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))
				{
					FileSystem::RenameFile(path, std::string(buffer));
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
				FileSystem::CreateNewDirectory(newPath);
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::Columns(1);
		

		UI::Editor::EndWindow();

	}
	Ref<Texture2D> ContentBrowserPanel::BrowserFileTypeToIcon(BrowserFileType type)
	{
		switch (type)
		{
			case BrowserFileType::Directory: { return UI::Editor::s_DirectoryIcon; }
			case BrowserFileType::Image: { return UI::Editor::s_ImageIcon; }
			case BrowserFileType::Audio: { return UI::Editor::s_AudioIcon; }
			case BrowserFileType::Font: { return UI::Editor::s_FontIcon; }
			case BrowserFileType::UserInterface: { return UI::Editor::s_UserInterfaceIcon; }

			case BrowserFileType::Binary: { return UI::Editor::s_BinaryIcon; }
			case BrowserFileType::Registry: { return UI::Editor::s_RegistryIcon; }
			case BrowserFileType::Scene: { return UI::Editor::s_SceneIcon; }
			case BrowserFileType::ScriptProject: { return UI::Editor::s_ScriptProjectIcon; }
			case BrowserFileType::Input: { return UI::Editor::s_InputIcon; }

			case BrowserFileType::None: { return UI::Editor::s_GenericFileIcon; }
		}
		KG_ASSERT(false, "Invalid BrowserFileType provided");
		return UI::Editor::s_GenericFileIcon;
	}
}

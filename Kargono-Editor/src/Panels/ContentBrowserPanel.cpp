#include "Kargono/Project/Project.h"
#include "Kargono/Core/Application.h"
#include "Kargono/Core/FileSystem.h"
#include "Kargono/Utils/PlatformUtils.h"
#include "Kargono/Text/TextEngine.h"
#include "EditorLayer.h"

#include "Panels/ContentBrowserPanel.h"

#include <imgui.h>

#include "Kargono/UI/RuntimeUI.h"

#include <sstream>
#include <vector>
#include <array>


namespace Kargono
{

	enum class BrowserFileType
	{
		None = 0, Directory = 1, Image = 2, Audio = 3,
		Binary = 4, Registry = 5, Scene = 6, ScriptProject = 7,
		Font = 8, UserInterface = 9
	};

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
		if (extension == ".kgaudio" || extension == ".kgtexture" || extension == ".kgfont" ||
			extension == ".kgshadervert" || extension == ".kgshaderfrag") { return BrowserFileType::Binary; }
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
		case BrowserFileType::None: { return "CONTENT_BROWSER_ITEM"; }
		}
		KG_CORE_ASSERT(false, "Invalid BrowserFileType provided");
		return "CONTENT_BROWSER_ITEM";
	}

	ContentBrowserPanel::ContentBrowserPanel()
		: m_BaseDirectory(Project::GetAssetDirectory()), m_CurrentDirectory(m_BaseDirectory)
	{
		m_DirectoryIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/directory_icon.png");
		m_GenericFileIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/generic_file_icon.png");
		m_BackIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/back_icon.png");
		m_AudioIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/audio_icon.png");
		m_ImageIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/image_icon.png");
		m_BinaryIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/binary_icon.png");
		m_SceneIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/scene_icon.png");
		m_RegistryIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/registry_icon.png");
		m_ScriptProjectIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/script_project.png");
		m_FontIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/font.png");
		m_UserInterfaceIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/user_interface_icon.png");
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		static float padding = 25.0f;
		static float thumbnailSize = 140;
		float cellSize = thumbnailSize + padding;
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		columnCount = columnCount > 0 ? columnCount : 1;

		if (m_CurrentDirectory != std::filesystem::path(m_BaseDirectory))
		{
			if (ImGui::ImageButton((ImTextureID)(uint64_t)m_BackIcon->GetRendererID(), { 22.4f, 22.4f }, { 0, 1 }, { 1, 0 }))
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

		std::string outputDirectory = "Current Directory: " + FileSystem::GetRelativePath(Project::GetProjectDirectory(), m_CurrentDirectory).string();

		ImGui::SameLine();
		ImGui::TextWrapped(outputDirectory.c_str());

		ImGui::Separator();

		ImGui::Columns(columnCount, 0, false);

		for (auto& directoryEntry: std::filesystem::directory_iterator(m_CurrentDirectory))
		{

			const auto& path = directoryEntry.path();
			std::string filenameString = path.filename().string();
			BrowserFileType fileType = DetermineFileType(directoryEntry);
			ImGui::PushID(filenameString.c_str());
			Ref<Texture2D> icon = BrowserFileTypeToIcon(fileType);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), {thumbnailSize, thumbnailSize}, {0, 1}, {1, 0});
			if (ImGui::BeginDragDropSource())
			{
				std::filesystem::path relativePath(path);
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload(BrowserFileTypeToPayloadString(fileType).c_str(),
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
						FileExplorer::OpenScriptProject(path);
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
						AssetHandle currentHandle = AssetManager::ImportNewFontFromFile(path);
						Ref<Font> font = AssetManager::GetFont(currentHandle);
						if (font)
						{
							UIEngine::SetFont(font, currentHandle);
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
				FileExplorer::OpenFileExplorer(m_CurrentDirectory);
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
		

		// TODO: Status bar
		ImGui::End();

	}
	Ref<Texture2D> ContentBrowserPanel::BrowserFileTypeToIcon(BrowserFileType type)
	{
		switch (type)
		{
			case BrowserFileType::Directory: { return m_DirectoryIcon; }
			case BrowserFileType::Image: { return m_ImageIcon; }
			case BrowserFileType::Audio: { return m_AudioIcon; }
			case BrowserFileType::Font: { return m_FontIcon; }
			case BrowserFileType::UserInterface: { return m_UserInterfaceIcon; }

			case BrowserFileType::Binary: { return m_BinaryIcon; }
			case BrowserFileType::Registry: { return m_RegistryIcon; }
			case BrowserFileType::Scene: { return m_SceneIcon; }
			case BrowserFileType::ScriptProject: { return m_ScriptProjectIcon; }

			case BrowserFileType::None: { return m_GenericFileIcon; }
		}
		KG_CORE_ASSERT(false, "Invalid BrowserFileType provided");
		return m_GenericFileIcon;
	}
}

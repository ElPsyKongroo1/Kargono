#include "Kargono/Project/Project.h"
#include "Kargono/Core/Application.h"
#include "Kargono/Core/FileSystem.h"
#include "Kargono/Utils/PlatformUtils.h"

#include "Panels/ContentBrowserPanel.h"

#include <imgui.h>


namespace Kargono
{

	enum class BrowserFileType
	{
		None = 0, Directory = 1, Image = 2, Audio = 3, Binary = 4, Registry = 5, Scene = 6, ScriptProject = 7
	};

	static BrowserFileType DetermineFileType(const std::filesystem::directory_entry& entry)
	{
		if (entry.is_directory()) { return BrowserFileType::Directory; }
		if (!entry.path().has_extension()) { return BrowserFileType::None; }
		auto extension = entry.path().extension();
		if (extension == ".jpg" || extension == ".png") { return BrowserFileType::Image; }
		if (extension == ".wav" || extension == ".mp3") { return BrowserFileType::Audio; }
		if (extension == ".kgreg") { return BrowserFileType::Registry; }
		if (extension == ".kgscene") { return BrowserFileType::Scene; }
		if (extension == ".csproj") { return BrowserFileType::ScriptProject; }
		if (extension == ".kgaudio" || extension == ".kgtexture" ||
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
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (directoryEntry.is_directory()) { m_CurrentDirectory /= path.filename(); }
			}
			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup("RightClickOptions");
			}

			bool openRenamePopup = false;
			static char buffer[256];

			if (ImGui::BeginPopup("RightClickOptions"))
			{
				if (ImGui::Selectable("Open In File Explorer"))
				{
					FileExplorer::OpenFileExplorer(path.parent_path());
				}
				if (fileType == BrowserFileType::ScriptProject)
				{
					if (ImGui::Selectable("Open Scripting Project"))
					{
						FileExplorer::OpenScriptProject(path);
					}
				}
				if (fileType != BrowserFileType::ScriptProject && fileType != BrowserFileType::Binary &&
					fileType != BrowserFileType::Directory && fileType != BrowserFileType::Registry)
				{
					if (ImGui::Selectable("Rename File"))
					{
						openRenamePopup = true;
					}

				}
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
				}
				ImGui::EndPopup();
			}

			ImGui::TextWrapped(filenameString.c_str());

			ImGui::NextColumn();

			ImGui::PopID();
			
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

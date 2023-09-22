#include "Kargono/Project/Project.h"
#include "Kargono/Core/Application.h"
#include "Kargono/Core/FileSystem.h"
#include "Panels/ContentBrowserPanel.h"

#include <imgui.h>


namespace Kargono
{

	ContentBrowserPanel::ContentBrowserPanel()
		: m_BaseDirectory(Project::GetAssetDirectory()), m_CurrentDirectory(m_BaseDirectory)
	{

		m_DirectoryIcon = Texture2D::Create(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/directory_icon.png");
		m_GenericFileIcon = Texture2D::Create(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/generic_file_icon.png");
		m_BackIcon = Texture2D::Create(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/back_icon.png");
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

			ImGui::PushID(filenameString.c_str());
			Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_GenericFileIcon;
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), {thumbnailSize, thumbnailSize}, {0, 1}, {1, 0});

			if (ImGui::BeginDragDropSource())
			{
				std::filesystem::path relativePath(path);
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (directoryEntry.is_directory()) { m_CurrentDirectory /= path.filename(); }
			}
			ImGui::TextWrapped(filenameString.c_str());

			ImGui::NextColumn();

			ImGui::PopID();
			
		}
		ImGui::Columns(1);
		

		// TODO: Status bar
		ImGui::End();

	}
}

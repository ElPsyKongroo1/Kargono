#include "Panels/ToolbarPanel.h"

#include "EditorLayer.h"

#include "Kargono.h"

namespace Kargono
{
	void ToolbarPanel::OnEditorUIRender()
	{
		EditorLayer* editorLayer = EditorLayer::GetCurrentLayer();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

		ImGui::Begin("##toolbar", nullptr, 0);

		bool toolbarEnabled = (bool)Scene::GetActiveScene();

		ImVec4 tintColor = ImVec4(1, 1, 1, 1);

		if (!toolbarEnabled) { tintColor.w = 0.5f; }

		float size = ImGui::GetWindowHeight() - 4.0f;
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));

		bool hasPlayButton = editorLayer->m_SceneState == SceneState::Edit || editorLayer->m_SceneState == SceneState::Play;
		bool hasSimulateButton = editorLayer->m_SceneState == SceneState::Edit || editorLayer->m_SceneState == SceneState::Simulate;
		bool hasPauseButton = editorLayer->m_SceneState != SceneState::Edit;

		if (hasPlayButton)
		{
			Ref<Texture2D> icon = (editorLayer->m_SceneState == SceneState::Edit || editorLayer->m_SceneState == SceneState::Simulate) ? UI::Editor::s_IconPlay : UI::Editor::s_IconStop;
			if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor)
				&& toolbarEnabled)
			{
				if (editorLayer->m_SceneState == SceneState::Edit || editorLayer->m_SceneState == SceneState::Simulate) { editorLayer->OnPlay(); }
				else if (editorLayer->m_SceneState == SceneState::Play) { editorLayer->OnStop(); }
			}
		}
		if (hasSimulateButton)
		{
			if (hasPlayButton) { ImGui::SameLine(); }

			Ref<Texture2D> icon = (editorLayer->m_SceneState == SceneState::Edit || editorLayer->m_SceneState == SceneState::Play) ? UI::Editor::s_IconSimulate : UI::Editor::s_IconStop;
			//ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
			if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor)
				&& toolbarEnabled)
			{
				if (editorLayer->m_SceneState == SceneState::Edit || editorLayer->m_SceneState == SceneState::Play) { editorLayer->OnSimulate(); }
				else if (editorLayer->m_SceneState == SceneState::Simulate) { editorLayer->OnStop(); }
			}

		}

		if (hasPauseButton)
		{
			ImGui::SameLine();
			{
				Ref<Texture2D> icon = UI::Editor::s_IconPause;
				if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor)
					&& toolbarEnabled)
				{
					editorLayer->m_IsPaused = !editorLayer->m_IsPaused;
				}
			}
			if (editorLayer->m_IsPaused)
			{
				ImGui::SameLine();
				{
					Ref<Texture2D> icon = UI::Editor::s_IconStep;
					if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor)
						&& toolbarEnabled)
					{
						editorLayer->Step(1);
					}
				}
			}
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		ImGui::End();
	}
}

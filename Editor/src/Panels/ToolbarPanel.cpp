#include "Panels/ToolbarPanel.h"

#include "EditorApp.h"

#include "Kargono.h"

namespace Kargono
{
	static EditorApp* s_EditorLayer{ nullptr };

	ToolbarPanel::ToolbarPanel()
	{
		s_EditorLayer = EditorApp::GetCurrentLayer();
	}
	void ToolbarPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

		EditorUI::Editor::StartWindow("##toolbar", &s_EditorLayer->m_ShowToolbar);

		bool toolbarEnabled = (bool)Scene::GetActiveScene();

		ImVec4 tintColor = ImVec4(1, 1, 1, 1);

		if (!toolbarEnabled) { tintColor.w = 0.5f; }

		float size = ImGui::GetWindowHeight() - 4.0f;
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));

		bool hasPlayButton = s_EditorLayer->m_SceneState == SceneState::Edit || s_EditorLayer->m_SceneState == SceneState::Play;
		bool hasSimulateButton = s_EditorLayer->m_SceneState == SceneState::Edit || s_EditorLayer->m_SceneState == SceneState::Simulate;
		bool hasPauseButton = s_EditorLayer->m_SceneState != SceneState::Edit;

		if (hasPlayButton)
		{
			Ref<Texture2D> icon = (s_EditorLayer->m_SceneState == SceneState::Edit || s_EditorLayer->m_SceneState == SceneState::Simulate) ? EditorUI::Editor::s_IconPlay : EditorUI::Editor::s_IconStop;
			if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor)
				&& toolbarEnabled)
			{
				if (s_EditorLayer->m_SceneState == SceneState::Edit || s_EditorLayer->m_SceneState == SceneState::Simulate) { s_EditorLayer->OnPlay(); }
				else if (s_EditorLayer->m_SceneState == SceneState::Play) { s_EditorLayer->OnStop(); }
			}
		}
		if (hasSimulateButton)
		{
			if (hasPlayButton) { ImGui::SameLine(); }

			Ref<Texture2D> icon = (s_EditorLayer->m_SceneState == SceneState::Edit || s_EditorLayer->m_SceneState == SceneState::Play) ? EditorUI::Editor::s_IconSimulate : EditorUI::Editor::s_IconStop;
			//ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
			if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor)
				&& toolbarEnabled)
			{
				if (s_EditorLayer->m_SceneState == SceneState::Edit || s_EditorLayer->m_SceneState == SceneState::Play) { s_EditorLayer->OnSimulate(); }
				else if (s_EditorLayer->m_SceneState == SceneState::Simulate) { s_EditorLayer->OnStop(); }
			}

		}

		if (hasPauseButton)
		{
			ImGui::SameLine();
			{
				Ref<Texture2D> icon = EditorUI::Editor::s_IconPause;
				if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor)
					&& toolbarEnabled)
				{
					s_EditorLayer->m_IsPaused = !s_EditorLayer->m_IsPaused;
				}
			}
			if (s_EditorLayer->m_IsPaused)
			{
				ImGui::SameLine();
				{
					Ref<Texture2D> icon = EditorUI::Editor::s_IconStep;
					if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor)
						&& toolbarEnabled)
					{
						s_EditorLayer->Step(1);
					}
				}
			}
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		EditorUI::Editor::EndWindow();
	}
}

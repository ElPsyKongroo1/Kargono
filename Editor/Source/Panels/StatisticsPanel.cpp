#include "Panels/StatisticsPanel.h"

#include "EditorApp.h"
#include "Kargono.h"

namespace Kargono
{
	static EditorApp* s_EditorLayer { nullptr };

	StatisticsPanel::StatisticsPanel()
	{
		s_EditorLayer = EditorApp::GetCurrentLayer();
	}
	void StatisticsPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::Editor::StartWindow("Statistics", &s_EditorLayer->m_ShowStats);

		ImGui::Text("Scene");
		ImGui::Separator();
		std::string name = "None";
		if (*Scene::GetActiveScene()->GetHoveredEntity())
		{
			name = Scene::GetActiveScene()->GetHoveredEntity()->GetComponent<TagComponent>().Tag;
		}
		ImGui::Text("Hovered Entity: %s", name.c_str());
		ImGui::NewLine();

		ImGui::Text("Renderer");
		ImGui::Separator();
		auto stats = Renderer::GetStats();
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::NewLine();

		ImGui::Text("Time");
		ImGui::Separator();
		ImGui::Text("Editor Runtime: %s", Utility::Time::GetStringFromSeconds(static_cast<uint64_t>(Utility::Time::GetTime())).c_str());
		ImGui::Text("Total Frame Count: %d", static_cast<int32_t>(EngineCore::GetCurrentEngineCore().GetUpdateCount()));
		if (Scene::GetActiveScene()->IsRunning())
		{
			ImGui::Text("Application Runtime: %s", Utility::Time::GetStringFromSeconds(static_cast<uint64_t>(Utility::Time::GetTime() - EngineCore::GetCurrentEngineCore().GetAppStartTime())).c_str());
		}
		else
		{
			ImGui::Text("Application Runtime: %s", "Application is not running");
		}

		EditorUI::Editor::EndWindow();
	}
}
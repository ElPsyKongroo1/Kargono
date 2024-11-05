#include "Panels/StatisticsPanel.h"

#include "EditorApp.h"
#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{
	StatisticsPanel::StatisticsPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(StatisticsPanel::OnKeyPressedEditor));
	}
	void StatisticsPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowStats);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		ImGui::Text("Scene");
		ImGui::Separator();
		std::string name = "None";
		if (*Scenes::SceneService::GetActiveScene()->GetHoveredEntity())
		{
			name = Scenes::SceneService::GetActiveScene()->GetHoveredEntity()->GetComponent<ECS::TagComponent>().Tag;
		}
		ImGui::Text("Hovered Entity: %s", name.c_str());
		ImGui::NewLine();

		ImGui::Text("Renderer");
		ImGui::Separator();
		auto stats = Rendering::RenderingService::GetStats();
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::NewLine();

		ImGui::Text("Time");
		ImGui::Separator();
		ImGui::Text("Editor Runtime: %s", Utility::Time::GetStringFromSeconds(static_cast<uint64_t>(Utility::Time::GetTime())).c_str());
		ImGui::Text("Total Frame Count: %d", static_cast<int32_t>(EngineService::GetActiveEngine().GetUpdateCount()));
		if (Scenes::SceneService::GetActiveScene()->IsRunning())
		{
			ImGui::Text("Application Runtime: %s", Utility::Time::GetStringFromSeconds(static_cast<uint64_t>(Utility::Time::GetTime() - EngineService::GetActiveEngine().GetAppStartTime())).c_str());
		}
		else
		{
			ImGui::Text("Application Runtime: %s", "Application is not running");
		}

		EditorUI::EditorUIService::EndWindow();
	}
	bool StatisticsPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
}

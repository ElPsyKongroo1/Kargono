#include "Panels/TestingPanel.h"

#include "EditorApp.h"
#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{

	TestingPanel::TestingPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName,
			KG_BIND_CLASS_FN(TestingPanel::OnKeyPressedEditor));
	}
	void TestingPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowTesting);
		// Exit window early if window is not visible
		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		ImGui::DragFloat4("Primary Color", (float*)&EditorUI::EditorUIService::s_PrimaryColor, 0.01f);
		ImGui::DragFloat4("Secondary Color", (float*)&EditorUI::EditorUIService::s_SecondaryColor, 0.01f);
		ImGui::DragFloat4("Disabled Color", (float*)&EditorUI::EditorUIService::s_DisabledColor, 0.01f);
		ImGui::DragFloat4("Highlight Color 1", (float*)&EditorUI::EditorUIService::s_HighlightColor1, 0.01f);
		ImGui::DragFloat4("Highlight Color 1 Thin", (float*)&EditorUI::EditorUIService::s_HighlightColor1_Thin, 0.01f);
		ImGui::DragFloat4("Highlight Color 2", (float*)&EditorUI::EditorUIService::s_HighlightColor2, 0.01f);
		ImGui::DragFloat4("Highlight Color 3", (float*)&EditorUI::EditorUIService::s_HighlightColor3, 0.01f);


		EditorUI::EditorUIService::EndWindow();
	}
	bool TestingPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
}

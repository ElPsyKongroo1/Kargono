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

		if (ImGui::DragFloat("Small Button Offset", (float*)&EditorUI::EditorUIService::s_SmallButtonRightOffset, 0.01f))
		{
			EditorUI::EditorUIService::SetButtonDefaults();
		}

		if (ImGui::DragFloat("Lefthand Indenting", (float*)&EditorUI::EditorUIService::s_TextLeftIndentOffset, 0.01f))
		{
			EditorUI::EditorUIService::SetButtonDefaults();
		}

		if (ImGui::DragFloat4("Background Color", (float*)&EditorUI::EditorUIService::s_BackgroundColor, 0.01f))
		{
			EditorUI::EditorUIService::SetColorDefaults();
		}
		if (ImGui::DragFloat4("Accent Color", (float*)&EditorUI::EditorUIService::s_AccentColor, 0.01f))
		{
			EditorUI::EditorUIService::SetColorDefaults();
		}
		if (ImGui::DragFloat4("Primary Text Color", (float*)&EditorUI::EditorUIService::s_PrimaryTextColor, 0.01f))
		{
			EditorUI::EditorUIService::SetColorDefaults();
		}
		if (ImGui::DragFloat4("Secondary Text Color", (float*)&EditorUI::EditorUIService::s_SecondaryTextColor, 0.01f))
		{
			EditorUI::EditorUIService::SetColorDefaults();
		}
		if (ImGui::DragFloat4("Disabled Color", (float*)&EditorUI::EditorUIService::s_DisabledColor, 0.01f))
		{
			EditorUI::EditorUIService::SetColorDefaults();
		}
		if (ImGui::DragFloat4("Active Color", (float*)&EditorUI::EditorUIService::s_ActiveColor, 0.01f))
		{
			EditorUI::EditorUIService::SetColorDefaults();
		}
		if (ImGui::DragFloat4("Hovered Color", (float*)&EditorUI::EditorUIService::s_HoveredColor, 0.01f))
		{
			EditorUI::EditorUIService::SetColorDefaults();
		}
		if (ImGui::DragFloat4("Highlight Color 1", (float*)&EditorUI::EditorUIService::s_HighlightColor1, 0.01f))
		{
			EditorUI::EditorUIService::s_HighlightColor1_Thin =
			{
				EditorUI::EditorUIService::s_HighlightColor1.x,
				EditorUI::EditorUIService::s_HighlightColor1.y,
				EditorUI::EditorUIService::s_HighlightColor1.z,
				EditorUI::EditorUIService::s_HighlightColor1.w * 0.75f
			};
			EditorUI::EditorUIService::SetColorDefaults();
		}

		if (ImGui::DragFloat4("Highlight Color 2", (float*)&EditorUI::EditorUIService::s_HighlightColor2, 0.01f))
		{
			EditorUI::EditorUIService::SetColorDefaults();
		}
		if (ImGui::DragFloat4("Highlight Color 3", (float*)&EditorUI::EditorUIService::s_HighlightColor3, 0.01f))
		{
			EditorUI::EditorUIService::SetColorDefaults();
		}

		EditorUI::EditorUIService::EndWindow();
	}
	bool TestingPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
}

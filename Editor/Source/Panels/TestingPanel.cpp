#include "Panels/TestingPanel.h"

#include "EditorApp.h"
#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{
	ImVec4 TestingPanel::s_TestColor {1.0f, 1.0f, 1.0f, 1.0f};
	ImVec4 TestingPanel::s_TestColor2 {1.0f, 1.0f, 1.0f, 1.0f};

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
		if (ImGui::Button("Compile brokentest"))
		{
			KG_INFO(Scripting::ScriptCompiler::CompileScriptFile("./../Projects/Pong/Assets/NewScripting/brokentest.kgscript"));
		}

		ImGui::ColorPicker4("TestColor1", (float*)&s_TestColor);
		ImGui::ColorPicker4("TestColor2", (float*)&s_TestColor2);

		Math::vec3 focalPoint = s_EditorApp->m_ViewportPanel->m_EditorCamera.GetFocalPoint();
		float distance = s_EditorApp->m_ViewportPanel->m_EditorCamera.GetDistance();
		float pitch = s_EditorApp->m_ViewportPanel->m_EditorCamera.GetPitch();
		float yaw = s_EditorApp->m_ViewportPanel->m_EditorCamera.GetYaw();

		ImGui::Text("Camera Focal Point %f %f %f", focalPoint.x, focalPoint.y, focalPoint.z);
		ImGui::Text("Camera Distance, Pitch, Yaw %f %f %f", distance , pitch, yaw);

		EditorUI::EditorUIService::EndWindow();
	}
	bool TestingPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
}

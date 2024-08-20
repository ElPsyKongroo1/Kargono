#include "Panels/TestingPanel.h"

#include "EditorApp.h"
#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{

	static EditorUI::EditTextSpec s_TestText {};

	TestingPanel::TestingPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName,
			KG_BIND_CLASS_FN(TestingPanel::OnKeyPressedEditor));

		s_TestText.Label = "File to Compile";
		s_TestText.CurrentOption = "NewScripting/test.kgscript";
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

		EditorUI::EditorUIService::EditText(s_TestText);

		if (ImGui::Button("Compile File"))
		{
			Scripting::ScriptCompiler::CompileScriptFile("./../Projects/Pong/Assets/" + s_TestText.CurrentOption);
		}

		EditorUI::EditorUIService::EndWindow();
	}
	bool TestingPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
}

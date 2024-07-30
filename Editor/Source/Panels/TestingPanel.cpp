#include "Panels/TestingPanel.h"

#include "EditorApp.h"
#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{

	static EditorUI::TreeSpec s_TreeSpec {};

	TestingPanel::TestingPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName,
			KG_BIND_CLASS_FN(TestingPanel::OnKeyPressedEditor));

		s_TreeSpec.Label = "HAHAH ITS A SPEC";
		EditorUI::TreeEntry newEntry {};
		newEntry.Label = "HAHAH brahhhhahahaha";
		newEntry.IconHandle = EditorUI::EditorUIService::s_IconCameraActive;

		for (uint64_t size {0}; size < 8; size++)
		{
			newEntry.SubEntries.push_back(newEntry);
		}

		s_TreeSpec.InsertEntry(newEntry);
		s_TreeSpec.InsertEntry(newEntry);
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

		EditorUI::EditorUIService::Tree(s_TreeSpec);

		EditorUI::EditorUIService::EndWindow();
	}
	bool TestingPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
}

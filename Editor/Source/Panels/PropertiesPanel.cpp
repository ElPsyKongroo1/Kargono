#include "Panels/PropertiesPanel.h"

#include "EditorApp.h"
#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{
	PropertiesPanel::PropertiesPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
	}
	void PropertiesPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow("Properties", &s_EditorApp->m_ShowProperties, 
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoFocusOnAppearing);
		if (*Scenes::SceneService::GetActiveScene()->GetSelectedEntity())
		{
			s_EditorApp->m_SceneHierarchyPanel->DrawComponents(*Scenes::SceneService::GetActiveScene()->GetSelectedEntity());
		}
		EditorUI::EditorUIService::EndWindow();
	}
}

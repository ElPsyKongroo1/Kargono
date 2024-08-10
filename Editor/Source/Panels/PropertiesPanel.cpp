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
		EditorUI::EditorUIService::StartWindow("Properties", &s_EditorApp->m_ShowProperties, ImGuiWindowFlags_NoFocusOnAppearing);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		if (*Scenes::SceneService::GetActiveScene()->GetSelectedEntity())
		{
			if (s_EditorApp->m_SceneEditorPanel->m_DisplayedComponent == Scenes::ComponentType::None)
			{
				s_EditorApp->m_SceneEditorPanel->DrawAllComponents(*Scenes::SceneService::GetActiveScene()->GetSelectedEntity());
			}
			else
			{
				s_EditorApp->m_SceneEditorPanel->DrawSingleComponent(*Scenes::SceneService::GetActiveScene()->GetSelectedEntity());
			}
		}
		EditorUI::EditorUIService::EndWindow();
	}
}

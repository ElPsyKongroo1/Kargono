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

		if (m_ActiveParent == s_EditorApp->m_SceneEditorPanel->m_PanelName)
		{
			if (*Scenes::SceneService::GetActiveScene()->GetSelectedEntity())
			{
				if (s_EditorApp->m_SceneEditorPanel->m_DisplayedComponent == ECS::ComponentType::None)
				{
					s_EditorApp->m_SceneEditorPanel->DrawAllComponents(*Scenes::SceneService::GetActiveScene()->GetSelectedEntity());
				}
				else
				{
					s_EditorApp->m_SceneEditorPanel->DrawSingleComponent(*Scenes::SceneService::GetActiveScene()->GetSelectedEntity());
				}
			}
		}

		if (m_ActiveParent == s_EditorApp->m_UIEditorPanel->m_PanelName)
		{
			switch (s_EditorApp->m_UIEditorPanel->m_CurrentDisplay)
			{
			case UIPropertiesDisplay::None:
				break;
			case UIPropertiesDisplay::Window:
				s_EditorApp->m_UIEditorPanel->DrawWindowOptions();
				break;
			case UIPropertiesDisplay::Widget:
				s_EditorApp->m_UIEditorPanel->DrawWidgetOptions();
				break;
			}
		}

		EditorUI::EditorUIService::EndWindow();
	}
}

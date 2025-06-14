#include "Windows/MainWindow/PropertiesPanel.h"

#include "EditorApp.h"

#include "Kargono/Scenes/Scene.h"

static Kargono::EditorApp* s_EditorApp { nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };

namespace Kargono::Panels
{
	PropertiesPanel::PropertiesPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
	}
	void PropertiesPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIContext::StartRenderWindow("Properties", &s_MainWindow->m_ShowProperties, ImGuiWindowFlags_NoFocusOnAppearing);

		if (!EditorUI::EditorUIContext::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIContext::EndRenderWindow();
			return;
		}

		if (m_ActiveParent == s_MainWindow->m_SceneEditorPanel->m_PanelName)
		{
			if (s_MainWindow->m_SceneEditorPanel->m_CurrentDisplayed == ScenePropertiesDisplay::Entity)
			{
				if (*Scenes::SceneService::GetActiveScene()->GetSelectedEntity())
				{
					if (s_MainWindow->m_SceneEditorPanel->m_DisplayedComponent == ECS::ComponentType::None)
					{
						s_MainWindow->m_SceneEditorPanel->DrawAllComponents(*Scenes::SceneService::GetActiveScene()->GetSelectedEntity());
					}
					else
					{
						s_MainWindow->m_SceneEditorPanel->DrawSingleComponent(*Scenes::SceneService::GetActiveScene()->GetSelectedEntity());
					}
				}
			}
			if (s_MainWindow->m_SceneEditorPanel->m_CurrentDisplayed == ScenePropertiesDisplay::Scene)
			{
				s_MainWindow->m_SceneEditorPanel->DrawSceneOptions();
			}
			
		}

		EditorUI::EditorUIContext::EndRenderWindow();
	}
}

#include "Windows/MainWindow/PropertiesPanel.h"

#include "EditorApp.h"

#include "Modules/Scenes/Assets/Scene.h"

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
				ECS::Entity selectedEntity = Scenes::SceneService::GetActiveContext().GetActiveScene()->GetSelectedEntity();
				if (selectedEntity.IsValid())
				{
					if (s_MainWindow->m_SceneEditorPanel->m_DisplayedComponent == ECSInternal::k_InvalidComponentIdentifier)
					{
						s_MainWindow->m_SceneEditorPanel->DrawAllComponents(selectedEntity);
					}
					else
					{
						s_MainWindow->m_SceneEditorPanel->DrawSingleComponent(selectedEntity);
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

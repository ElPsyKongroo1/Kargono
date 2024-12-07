#include "Windows/MainWindow/PropertiesPanel.h"

#include "EditorApp.h"
#include "Kargono.h"

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
		EditorUI::EditorUIService::StartWindow("Properties", &s_MainWindow->m_ShowProperties, ImGuiWindowFlags_NoFocusOnAppearing);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		if (m_ActiveParent == s_MainWindow->m_SceneEditorPanel->m_PanelName)
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

		// TODO: Migrate ui editor panel to new system
		/*if (m_ActiveParent == s_MainWindow->m_UIEditorPanel->m_PanelName)
		{
			switch (s_MainWindow->m_UIEditorPanel->m_CurrentDisplay)
			{
			case UIPropertiesDisplay::None:
				break;
			case UIPropertiesDisplay::Window:
				s_MainWindow->m_UIEditorPanel->DrawWindowOptions();
				break;
			case UIPropertiesDisplay::Widget:
				s_MainWindow->m_UIEditorPanel->DrawWidgetOptions();
				break;
			}
		}*/

		EditorUI::EditorUIService::EndWindow();
	}
}

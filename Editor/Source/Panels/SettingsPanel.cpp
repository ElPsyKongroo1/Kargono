#include "Panels/SettingsPanel.h"
#include "EditorApp.h"

#include "Kargono.h"

namespace Kargono
{
	static EditorUI::CheckboxSpec s_PrimaryCameraCheckboxSpec {};
	static EditorUI::CheckboxSpec s_DisplayCameraFrustrumSpec {};
	static EditorUI::CheckboxSpec s_DisplayRuntimeUISpec {};
	static EditorUI::CheckboxSpec s_FullscreenRuntimeSpec {};
	static EditorApp* s_EditorApp { nullptr };

	SettingsPanel::SettingsPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName,
			KG_BIND_CLASS_FN(SettingsPanel::OnKeyPressedEditor));

		s_PrimaryCameraCheckboxSpec.Label = "Display Physics Colliders";
		s_PrimaryCameraCheckboxSpec.Flags = 0;
		s_PrimaryCameraCheckboxSpec.ConfirmAction = [&](bool value)
		{
			s_EditorApp->m_ShowPhysicsColliders = value;
		};

		s_DisplayCameraFrustrumSpec.Label = "Display Camera Frustrum";
		s_DisplayCameraFrustrumSpec.Flags = 0;
		s_DisplayCameraFrustrumSpec.ConfirmAction = [&](bool value)
		{
			s_EditorApp->m_ShowCameraFrustrums = value;
		};

		s_DisplayRuntimeUISpec.Label = "Display Runtime UI";
		s_DisplayRuntimeUISpec.Flags = 0;
		s_DisplayRuntimeUISpec.ConfirmAction = [&](bool value)
		{
			s_EditorApp->m_ShowUserInterface = value;
		};

		s_FullscreenRuntimeSpec.Label = "Fullscreen While Running";
		s_FullscreenRuntimeSpec.Flags = 0;
		s_FullscreenRuntimeSpec.ConfirmAction = [&](bool value)
		{
			s_EditorApp->m_RuntimeFullscreen = value;
		};
	}
	void SettingsPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::Editor::StartWindow(m_PanelName, &s_EditorApp->m_ShowSettings);

		// Toggle Physics Colliders
		s_PrimaryCameraCheckboxSpec.ToggleBoolean = s_EditorApp->m_ShowPhysicsColliders;
		EditorUI::Editor::Checkbox(s_PrimaryCameraCheckboxSpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Toggle Camera Frustrums
		s_DisplayCameraFrustrumSpec.ToggleBoolean = s_EditorApp->m_ShowCameraFrustrums;
		EditorUI::Editor::Checkbox(s_DisplayCameraFrustrumSpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Toggle Runtime User Interface
		s_DisplayRuntimeUISpec.ToggleBoolean = s_EditorApp->m_ShowUserInterface;
		EditorUI::Editor::Checkbox(s_DisplayRuntimeUISpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Toggle Runtime Fullscreen
		s_FullscreenRuntimeSpec.ToggleBoolean = s_EditorApp->m_RuntimeFullscreen;
		EditorUI::Editor::Checkbox(s_FullscreenRuntimeSpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		static float musicVolume = 10.0f;
		ImGui::Separator();
		static int32_t* choice = (int32_t*)&s_EditorApp->m_ViewportPanel->m_EditorCamera.GetMovementType();
		ImGui::Text("Editor Camera Movement:");
		if (ImGui::RadioButton("Model Viewer", choice, (int32_t)EditorCamera::MovementType::ModelView))
		{
			s_EditorApp->m_ViewportPanel->m_EditorCamera.SetMovementType(EditorCamera::MovementType::ModelView);
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("FreeFly", choice, (int32_t)EditorCamera::MovementType::FreeFly))
		{
			s_EditorApp->m_ViewportPanel->m_EditorCamera.SetMovementType(EditorCamera::MovementType::FreeFly);
		}
		ImGui::SameLine();
		ImGui::TextDisabled("(Tab)");

		if (*choice == (int32_t)EditorCamera::MovementType::FreeFly)
		{
			ImGui::DragFloat("Speed", &s_EditorApp->m_ViewportPanel->m_EditorCamera.GetMovementSpeed(), 0.5f,
				s_EditorApp->m_ViewportPanel->m_EditorCamera.GetMinMovementSpeed(), s_EditorApp->m_ViewportPanel->m_EditorCamera.GetMaxMovementSpeed());
		}

		ImGui::Text("Physics Settings:");
		if (ImGui::DragFloat2("Gravity", glm::value_ptr(s_EditorApp->m_EditorScene->GetPhysicsSpecification().Gravity), 0.05f))
		{
			if (Scene::GetActiveScene()->GetPhysicsWorld())
			{
				Scene::GetActiveScene()->GetPhysicsSpecification().Gravity = s_EditorApp->m_EditorScene->GetPhysicsSpecification().Gravity;
				Scene::GetActiveScene()->GetPhysicsWorld()->SetGravity(s_EditorApp->m_EditorScene->GetPhysicsSpecification().Gravity);
			}
		}
		EditorUI::Editor::EndWindow();
	}
	bool SettingsPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
}

#include "Panels/SettingsPanel.h"
#include "EditorLayer.h"

#include "Kargono.h"

namespace Kargono
{
	static EditorUI::CheckboxSpec s_PrimaryCameraCheckboxSpec {};
	static EditorUI::CheckboxSpec s_DisplayCameraFrustrumSpec {};
	static EditorUI::CheckboxSpec s_DisplayRuntimeUISpec {};
	static EditorUI::CheckboxSpec s_FullscreenRuntimeSpec {};
	static EditorLayer* s_EditorLayer { nullptr };

	SettingsPanel::SettingsPanel()
	{
		s_EditorLayer = EditorLayer::GetCurrentLayer();

		s_PrimaryCameraCheckboxSpec.Label = "Display Physics Colliders";
		s_PrimaryCameraCheckboxSpec.LeftLean = false;
		s_PrimaryCameraCheckboxSpec.ConfirmAction = [&](bool value)
		{
			s_EditorLayer->m_ShowPhysicsColliders = value;
		};

		s_DisplayCameraFrustrumSpec.Label = "Display Camera Frustrum";
		s_DisplayCameraFrustrumSpec.LeftLean = false;
		s_DisplayCameraFrustrumSpec.ConfirmAction = [&](bool value)
		{
			s_EditorLayer->m_ShowCameraFrustrums = value;
		};

		s_DisplayRuntimeUISpec.Label = "Display Runtime UI";
		s_DisplayRuntimeUISpec.LeftLean = false;
		s_DisplayRuntimeUISpec.ConfirmAction = [&](bool value)
		{
			s_EditorLayer->m_ShowUserInterface = value;
		};

		s_FullscreenRuntimeSpec.Label = "Fullscreen While Running";
		s_FullscreenRuntimeSpec.LeftLean = false;
		s_FullscreenRuntimeSpec.ConfirmAction = [&](bool value)
		{
			s_EditorLayer->m_RuntimeFullscreen = value;
		};
	}
	void SettingsPanel::OnEditorUIRender()
	{
		EditorUI::Editor::StartWindow("Settings", &s_EditorLayer->m_ShowSettings);

		// Toggle Physics Colliders
		s_PrimaryCameraCheckboxSpec.ToggleBoolean = s_EditorLayer->m_ShowPhysicsColliders;
		EditorUI::Editor::Checkbox(s_PrimaryCameraCheckboxSpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Toggle Camera Frustrums
		s_DisplayCameraFrustrumSpec.ToggleBoolean = s_EditorLayer->m_ShowCameraFrustrums;
		EditorUI::Editor::Checkbox(s_DisplayCameraFrustrumSpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Toggle Runtime User Interface
		s_DisplayRuntimeUISpec.ToggleBoolean = s_EditorLayer->m_ShowUserInterface;
		EditorUI::Editor::Checkbox(s_DisplayRuntimeUISpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Toggle Runtime Fullscreen
		s_FullscreenRuntimeSpec.ToggleBoolean = s_EditorLayer->m_RuntimeFullscreen;
		EditorUI::Editor::Checkbox(s_FullscreenRuntimeSpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		static float musicVolume = 10.0f;
		ImGui::Separator();
		static int32_t* choice = (int32_t*)&s_EditorLayer->m_ViewportPanel->m_EditorCamera.GetMovementType();
		ImGui::Text("Editor Camera Movement:");
		if (ImGui::RadioButton("Model Viewer", choice, (int32_t)EditorCamera::MovementType::ModelView))
		{
			s_EditorLayer->m_ViewportPanel->m_EditorCamera.SetMovementType(EditorCamera::MovementType::ModelView);
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("FreeFly", choice, (int32_t)EditorCamera::MovementType::FreeFly))
		{
			s_EditorLayer->m_ViewportPanel->m_EditorCamera.SetMovementType(EditorCamera::MovementType::FreeFly);
		}
		ImGui::SameLine();
		ImGui::TextDisabled("(Tab)");

		if (*choice == (int32_t)EditorCamera::MovementType::FreeFly)
		{
			ImGui::DragFloat("Speed", &s_EditorLayer->m_ViewportPanel->m_EditorCamera.GetMovementSpeed(), 0.5f,
				s_EditorLayer->m_ViewportPanel->m_EditorCamera.GetMinMovementSpeed(), s_EditorLayer->m_ViewportPanel->m_EditorCamera.GetMaxMovementSpeed());
		}

		ImGui::Text("Physics Settings:");
		if (ImGui::DragFloat2("Gravity", glm::value_ptr(s_EditorLayer->m_EditorScene->GetPhysicsSpecification().Gravity), 0.05f))
		{
			if (Scene::GetActiveScene()->GetPhysicsWorld())
			{
				Scene::GetActiveScene()->GetPhysicsSpecification().Gravity = s_EditorLayer->m_EditorScene->GetPhysicsSpecification().Gravity;
				Scene::GetActiveScene()->GetPhysicsWorld()->SetGravity(s_EditorLayer->m_EditorScene->GetPhysicsSpecification().Gravity);
			}
		}
		EditorUI::Editor::EndWindow();
	}
}

#include "Windows/EmitterConfig/EmitterConfigPropertiesPanel.h"

#include "EditorApp.h"

static Kargono::EditorApp* s_EditorApp{ nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };
static Kargono::Windows::EmitterConfigWindow* s_EmitterConfigWindow{ nullptr };

namespace Kargono::Panels
{
	EmitterConfigPropertiesPanel::EmitterConfigPropertiesPanel()
	{
		// Get the main window and UI window
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
		s_EmitterConfigWindow = s_EditorApp->m_EmitterConfigEditorWindow.get();

		// Initialize properties panel widget resources
		InitializeEmitterConfigOptions();
	}
	void EmitterConfigPropertiesPanel::InitializeEmitterConfigOptions()
	{
		// Set up widget to modify the particle emitter's starting color
		m_ColorBeginSpec.m_Label = "Starting Color";
		m_ColorBeginSpec.m_Flags |= EditorUI::EditVec4_RGBA;
		m_ColorBeginSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyColorBegin);

		// Set up widget to modify the particle emitter's end color
		m_ColorEndSpec.m_Label = "Ending Color";
		m_ColorEndSpec.m_Flags |= EditorUI::EditVec4_RGBA;
		m_ColorEndSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyColorEnd);

		// Set up widget to modify the particle emitter's Start size
		m_SizeBeginSpec.m_Label = "Starting Size";
		m_SizeBeginSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySizeBegin);

		// Set up widget to modify the particle emitter's end size
		m_SizeEndSpec.m_Label = "Ending Size";
		m_SizeEndSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySizeEnd);

	}
	void EmitterConfigPropertiesPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EmitterConfigWindow->m_ShowProperties);

		// Early out if the window is not visible
		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}
		// Draw header
		EditorUI::EditorUIService::PanelHeader(s_EmitterConfigWindow->m_MainHeader);

		// Draw configuration options
		DrawEmitterConfigOptions();

		// End the window
		EditorUI::EditorUIService::EndWindow();
	}

	void EmitterConfigPropertiesPanel::DrawEmitterConfigOptions()
	{
		if (!s_EmitterConfigWindow->m_EditorEmitterConfig)
		{
			return;
		}

		// Draw color begin/end specs
		m_ColorBeginSpec.m_CurrentVec4 = s_EmitterConfigWindow->m_EditorEmitterConfig->m_ColorBegin;
		EditorUI::EditorUIService::EditVec4(m_ColorBeginSpec);
		m_ColorEndSpec.m_CurrentVec4 = s_EmitterConfigWindow->m_EditorEmitterConfig->m_ColorEnd;
		EditorUI::EditorUIService::EditVec4(m_ColorEndSpec);

		// Draw size begin/end specs
		m_SizeBeginSpec.m_CurrentVec3 = s_EmitterConfigWindow->m_EditorEmitterConfig->m_SizeBegin;
		EditorUI::EditorUIService::EditVec3(m_SizeBeginSpec);
		m_SizeEndSpec.m_CurrentVec3 = s_EmitterConfigWindow->m_EditorEmitterConfig->m_SizeEnd;
		EditorUI::EditorUIService::EditVec3(m_SizeEndSpec);
	}

	void EmitterConfigPropertiesPanel::ClearPanelData()
	{
	}
	void EmitterConfigPropertiesPanel::OnModifyColorBegin(EditorUI::EditVec4Spec& spec)
	{
		// Update the starting color for the current emitter config
		s_EmitterConfigWindow->m_EditorEmitterConfig->m_ColorBegin = spec.m_CurrentVec4;

		// Set the active editor UI as edited
		s_EmitterConfigWindow->m_MainHeader.m_EditColorActive = true;
	}
	void EmitterConfigPropertiesPanel::OnModifyColorEnd(EditorUI::EditVec4Spec& spec)
	{
		// Update the ending color for the current emitter config
		s_EmitterConfigWindow->m_EditorEmitterConfig->m_ColorEnd = spec.m_CurrentVec4;

		// Set the active editor UI as edited
		s_EmitterConfigWindow->m_MainHeader.m_EditColorActive = true;
	}
	void EmitterConfigPropertiesPanel::OnModifySizeBegin(EditorUI::EditVec3Spec& spec)
	{
		// Update the starting size for the current emitter config
		s_EmitterConfigWindow->m_EditorEmitterConfig->m_SizeBegin = spec.m_CurrentVec3;

		// Set the active editor UI as edited
		s_EmitterConfigWindow->m_MainHeader.m_EditColorActive = true;
	}
	void EmitterConfigPropertiesPanel::OnModifySizeEnd(EditorUI::EditVec3Spec& spec)
	{
		// Update the ending size for the current emitter config
		s_EmitterConfigWindow->m_EditorEmitterConfig->m_SizeEnd = spec.m_CurrentVec3;

		// Set the active editor UI as edited
		s_EmitterConfigWindow->m_MainHeader.m_EditColorActive = true;
	}
}


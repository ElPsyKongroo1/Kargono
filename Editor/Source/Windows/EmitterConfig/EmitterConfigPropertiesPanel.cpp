#include "Windows/EmitterConfig/EmitterConfigPropertiesPanel.h"

#include "Kargono/Math/Interpolation.h"

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
		// General options header
		m_GeneralOptionsHeaderSpec.m_Label = "General Options";
		m_GeneralOptionsHeaderSpec.m_Expanded = true;
		m_GeneralOptionsHeaderSpec.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;

		// Set up widget to modify the particle lifetimes
		m_LifetimeSpec.m_Label = "Lifetime";
		m_LifetimeSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyLifetime);
		m_LifetimeSpec.m_Flags |= EditorUI::EditFloat_Indented;
		m_LifetimeSpec.m_Bounds = { 0.0f, 10'000.0f };

		// Set up widget to modify the particle emitter's buffer size
		m_BufferSizeSpec.m_Label = "Buffer Size";
		m_BufferSizeSpec.m_Flags |= EditorUI::EditInteger_Indented;
		m_BufferSizeSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyBufferSize);
		m_BufferSizeSpec.m_Bounds = { 1, 100'000 };

		// Set up widget to modify the particle spawn rate per second
		m_SpawnPerSecSpec.m_Label = "Spawn Rate Per Second";
		m_SpawnPerSecSpec.m_Flags |= EditorUI::EditInteger_Indented;
		m_SpawnPerSecSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySpawnRate);
		m_SpawnPerSecSpec.m_Bounds = { 1, 1'000 };

		// Color options header
		m_ColorOptionsHeaderSpec.m_Label = "Particle Color Options";
		m_ColorOptionsHeaderSpec.m_Expanded = true;
		m_ColorOptionsHeaderSpec.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;

		// Set up widget to modify the particle emitter's starting color
		m_ColorBeginSpec.m_Label = "Starting Color";
		m_ColorBeginSpec.m_Flags |= EditorUI::EditVec4_RGBA | EditorUI::EditVec4_Indented;
		m_ColorBeginSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyColorBegin);
		m_ColorBeginSpec.m_Bounds = { 0.0f, 1.0f };

		// Set up widget to modify the particle emitter's end color
		m_ColorEndSpec.m_Label = "Ending Color";
		m_ColorEndSpec.m_Flags |= EditorUI::EditVec4_RGBA | EditorUI::EditVec4_Indented;
		m_ColorEndSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyColorEnd);
		m_ColorEndSpec.m_Bounds = { 0.0f, 1.0f };

		// Set up widget to modify the particle emitter's Color interpolation function
		m_SelectColorInterpSpec.m_Label = "Color Curve";
		m_SelectColorInterpSpec.m_Flags |= EditorUI::SelectOption_Indented;
		m_SelectColorInterpSpec.m_LineCount = 3;
		m_SelectColorInterpSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyColorCurve);
		m_SelectColorInterpSpec.ClearOptions();
		for (Math::InterpolationType type : Math::s_AllInterpolationTypes)
		{
			if (type == Math::InterpolationType::None)
			{
				continue;
			}
			m_SelectColorInterpSpec.AddToOptions
			(
				Utility::InterpolationTypeToCategory(type),
				Utility::InterpolationTypeToString(type),
				(uint64_t)type
			);
		}

		// Size options header
		m_SizeOptionsHeaderSpec.m_Label = "Particle Size Options";
		m_SizeOptionsHeaderSpec.m_Expanded = true;
		m_SizeOptionsHeaderSpec.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;

		// Set up widget to modify the particle emitter's Start size
		m_SizeBeginSpec.m_Label = "Starting Size";
		m_SizeBeginSpec.m_Flags |= EditorUI::EditVec3_Indented;
		m_SizeBeginSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySizeBegin);
		m_SizeBeginSpec.m_Bounds = { 0.0f, 100'000.0f };

		// Set up widget to modify the particle emitter's end size
		m_SizeEndSpec.m_Label = "Ending Size";
		m_SizeEndSpec.m_Flags |= EditorUI::EditVec3_Indented;
		m_SizeEndSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySizeEnd);
		m_SizeEndSpec.m_Bounds = { 0.0f, 100'000.0f };

		// Set up widget to modify the particle emitter's size interpolation function
		m_SelectSizeInterpSpec.m_Label = "Size Curve";
		m_SelectSizeInterpSpec.m_Flags |= EditorUI::SelectOption_Indented;
		m_SelectSizeInterpSpec.m_LineCount = 3;
		m_SelectSizeInterpSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySizeCurve);
		m_SelectSizeInterpSpec.ClearOptions();
		for (Math::InterpolationType type : Math::s_AllInterpolationTypes)
		{
			if (type == Math::InterpolationType::None)
			{
				continue;
			}
			m_SelectSizeInterpSpec.AddToOptions
			(
				Utility::InterpolationTypeToCategory(type),
				Utility::InterpolationTypeToString(type),
				(uint64_t)type
			);
		}

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

		// General emitter/particle options section
		EditorUI::EditorUIService::CollapsingHeader(m_GeneralOptionsHeaderSpec);
		if (m_GeneralOptionsHeaderSpec.m_Expanded)
		{
			// Draw buffer size widget
			m_BufferSizeSpec.m_CurrentInteger = (int32_t)s_EmitterConfigWindow->m_EditorEmitterConfig->m_BufferSize;
			EditorUI::EditorUIService::EditInteger(m_BufferSizeSpec);

			// Draw spawn rate widget
			m_SpawnPerSecSpec.m_CurrentInteger = (int32_t)s_EmitterConfigWindow->m_EditorEmitterConfig->m_SpawnRatePerSec;
			EditorUI::EditorUIService::EditInteger(m_SpawnPerSecSpec);

			// Draw particle lifetime widget
			m_LifetimeSpec.m_CurrentFloat = s_EmitterConfigWindow->m_EditorEmitterConfig->m_ParticleLifetime;
			EditorUI::EditorUIService::EditFloat(m_LifetimeSpec);
		}

		// Particle color section
		EditorUI::EditorUIService::CollapsingHeader(m_ColorOptionsHeaderSpec);
		if (m_ColorOptionsHeaderSpec.m_Expanded)
		{
			// Draw color begin/end specs
			m_ColorBeginSpec.m_CurrentVec4 = s_EmitterConfigWindow->m_EditorEmitterConfig->m_ColorBegin;
			EditorUI::EditorUIService::EditVec4(m_ColorBeginSpec);
			m_ColorEndSpec.m_CurrentVec4 = s_EmitterConfigWindow->m_EditorEmitterConfig->m_ColorEnd;
			EditorUI::EditorUIService::EditVec4(m_ColorEndSpec);

			// Draw option to select color interpolation curve
			Math::InterpolationType currentColorInterp{ s_EmitterConfigWindow->m_EditorEmitterConfig->m_ColorInterpolationType };
			m_SelectColorInterpSpec.m_CurrentOption =
			{
				Utility::InterpolationTypeToString(currentColorInterp),
				(uint64_t)currentColorInterp
			};
			EditorUI::EditorUIService::SelectOption(m_SelectColorInterpSpec);

		}

		// Particle size section
		EditorUI::EditorUIService::CollapsingHeader(m_SizeOptionsHeaderSpec);
		if (m_SizeOptionsHeaderSpec.m_Expanded)
		{
			// Draw size begin/end specs
			m_SizeBeginSpec.m_CurrentVec3 = s_EmitterConfigWindow->m_EditorEmitterConfig->m_SizeBegin;
			EditorUI::EditorUIService::EditVec3(m_SizeBeginSpec);
			m_SizeEndSpec.m_CurrentVec3 = s_EmitterConfigWindow->m_EditorEmitterConfig->m_SizeEnd;
			EditorUI::EditorUIService::EditVec3(m_SizeEndSpec);

			// Draw option to select size interpolation curve
			Math::InterpolationType currentSizeInterp{ s_EmitterConfigWindow->m_EditorEmitterConfig->m_SizeInterpolationType };
			m_SelectSizeInterpSpec.m_CurrentOption =
			{
				Utility::InterpolationTypeToString(currentSizeInterp),
				(uint64_t)currentSizeInterp
			};
			EditorUI::EditorUIService::SelectOption(m_SelectSizeInterpSpec);
		}
		
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
		s_EmitterConfigWindow->LoadEditorEmitterIntoParticleService();

	}
	void EmitterConfigPropertiesPanel::OnModifyColorEnd(EditorUI::EditVec4Spec& spec)
	{
		// Update the ending color for the current emitter config
		s_EmitterConfigWindow->m_EditorEmitterConfig->m_ColorEnd = spec.m_CurrentVec4;

		// Set the active editor UI as edited
		s_EmitterConfigWindow->m_MainHeader.m_EditColorActive = true;
		s_EmitterConfigWindow->LoadEditorEmitterIntoParticleService();
	}
	void EmitterConfigPropertiesPanel::OnModifySizeBegin(EditorUI::EditVec3Spec& spec)
	{
		// Update the starting size for the current emitter config
		s_EmitterConfigWindow->m_EditorEmitterConfig->m_SizeBegin = spec.m_CurrentVec3;

		// Set the active editor UI as edited
		s_EmitterConfigWindow->m_MainHeader.m_EditColorActive = true;
		s_EmitterConfigWindow->LoadEditorEmitterIntoParticleService();
	}
	void EmitterConfigPropertiesPanel::OnModifySizeEnd(EditorUI::EditVec3Spec& spec)
	{
		// Update the ending size for the current emitter config
		s_EmitterConfigWindow->m_EditorEmitterConfig->m_SizeEnd = spec.m_CurrentVec3;

		// Set the active editor UI as edited
		s_EmitterConfigWindow->m_MainHeader.m_EditColorActive = true;
		s_EmitterConfigWindow->LoadEditorEmitterIntoParticleService();
	}
	void EmitterConfigPropertiesPanel::OnModifySizeCurve(const EditorUI::OptionEntry& entry)
	{
		KG_ASSERT(entry.m_Handle != Assets::EmptyHandle);
		// Update the ending size for the current emitter config
		s_EmitterConfigWindow->m_EditorEmitterConfig->m_SizeInterpolationType = 
			(Math::InterpolationType)(uint64_t)entry.m_Handle;

		// Set the active editor UI as edited
		s_EmitterConfigWindow->m_MainHeader.m_EditColorActive = true;
		s_EmitterConfigWindow->LoadEditorEmitterIntoParticleService();
	}
	void EmitterConfigPropertiesPanel::OnModifyColorCurve(const EditorUI::OptionEntry& entry)
	{
		KG_ASSERT(entry.m_Handle != Assets::EmptyHandle);
		// Update the ending color for the current emitter config
		s_EmitterConfigWindow->m_EditorEmitterConfig->m_ColorInterpolationType =
			(Math::InterpolationType)(uint64_t)entry.m_Handle;

		// Set the active editor UI as edited
		s_EmitterConfigWindow->m_MainHeader.m_EditColorActive = true;
		s_EmitterConfigWindow->LoadEditorEmitterIntoParticleService();
	}
	void EmitterConfigPropertiesPanel::OnModifyLifetime(EditorUI::EditFloatSpec& spec)
	{
		// Update the lifetime for the current emitter config
		s_EmitterConfigWindow->m_EditorEmitterConfig->m_ParticleLifetime = spec.m_CurrentFloat;

		// Set the active editor UI as edited
		s_EmitterConfigWindow->m_MainHeader.m_EditColorActive = true;
		s_EmitterConfigWindow->LoadEditorEmitterIntoParticleService();
	}
	void EmitterConfigPropertiesPanel::OnModifySpawnRate(EditorUI::EditIntegerSpec& spec)
	{
		// Update the spawn rate for the current emitter config
		s_EmitterConfigWindow->m_EditorEmitterConfig->m_SpawnRatePerSec = (size_t)spec.m_CurrentInteger;

		// Set the active editor UI as edited
		s_EmitterConfigWindow->m_MainHeader.m_EditColorActive = true;
		s_EmitterConfigWindow->LoadEditorEmitterIntoParticleService();
	}
	void EmitterConfigPropertiesPanel::OnModifyBufferSize(EditorUI::EditIntegerSpec& spec)
	{
		// Update the buffer size for the current emitter config
		s_EmitterConfigWindow->m_EditorEmitterConfig->m_BufferSize = (size_t)spec.m_CurrentInteger;

		// Set the active editor UI as edited
		s_EmitterConfigWindow->m_MainHeader.m_EditColorActive = true;
		s_EmitterConfigWindow->LoadEditorEmitterIntoParticleService();
	}
}


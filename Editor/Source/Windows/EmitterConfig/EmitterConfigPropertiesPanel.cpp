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
		InitializeGeneralEmitterConfigOptions();
		InitializeSpawningOptions();
		InitializeParticleColorOptions();
		InitializeParticleSizeOptions();
	}

	
	void EmitterConfigPropertiesPanel::InitializeGeneralEmitterConfigOptions()
	{
		// General options header
		m_GeneralOptionsHeaderSpec.m_Label = "General Options";
		m_GeneralOptionsHeaderSpec.m_Expanded = true;
		m_GeneralOptionsHeaderSpec.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;

		// Set up widget to modify the emitter's lifecycle type
		m_EmitterLifecycleSpec.m_Label = "Emitter Lifecycle Type";
		m_EmitterLifecycleSpec.m_Flags |= EditorUI::RadioSelector_Indented;
		m_EmitterLifecycleSpec.m_FirstOptionLabel = "Immortal";
		m_EmitterLifecycleSpec.m_SecondOptionLabel = "Fixed Time";
		m_EmitterLifecycleSpec.m_SelectAction = KG_BIND_CLASS_FN(OnModifyEmitterLifecycleType);
		
		// Set up widget to modify the emitter lifetimes
		m_EmitterLifetimeSpec.m_Label = "Emitter Lifetime";
		m_EmitterLifetimeSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyEmitterLifetime);
		m_EmitterLifetimeSpec.m_Flags |= EditorUI::EditFloat_Indented;
		m_EmitterLifetimeSpec.m_Bounds = { 0.0f, 10'000.0f };

		// Set up widget to modify the particle lifetimes
		m_ParticleLifetimeSpec.m_Label = "Particle Lifetime";
		m_ParticleLifetimeSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyParticleLifetime);
		m_ParticleLifetimeSpec.m_Flags |= EditorUI::EditFloat_Indented;
		m_ParticleLifetimeSpec.m_Bounds = { 0.0f, 10'000.0f };

		// Set up widget to modify the particle emitter's buffer size
		m_BufferSizeSpec.m_Label = "Buffer Size";
		m_BufferSizeSpec.m_Flags |= EditorUI::EditInteger_Indented;
		m_BufferSizeSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyBufferSize);
		m_BufferSizeSpec.m_Bounds = { 1, 100'000 };

		// Set up widget to specific if gravity is used for the emitter
		m_UseGravitySpec.m_Label = "Use Gravity";
		m_UseGravitySpec.m_Flags |= EditorUI::Checkbox_Indented;
		m_UseGravitySpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyUseGravity);

		// Set up widget to modify the particle emitter's gravity acceleration
		m_GravityAccelerationSpec.m_Label = "Gravity Acceleration";
		m_GravityAccelerationSpec.m_Flags |= EditorUI::EditVec3_Indented;
		m_GravityAccelerationSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyGravityAcceleration);
		m_GravityAccelerationSpec.m_Bounds = { -10'000.0f, 10'000.0f };
	}
	void EmitterConfigPropertiesPanel::InitializeSpawningOptions()
	{
		// Color options header
		m_SpawningOptionsHeaderSpec.m_Label = "Particle Spawning Options";
		m_SpawningOptionsHeaderSpec.m_Expanded = true;
		m_SpawningOptionsHeaderSpec.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;

		// Set up widget to modify the particle spawn rate per second
		m_SpawnPerSecSpec.m_Label = "Spawn Rate Per Second";
		m_SpawnPerSecSpec.m_Flags |= EditorUI::EditInteger_Indented;
		m_SpawnPerSecSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifySpawnRate);
		m_SpawnPerSecSpec.m_Bounds = { 1, 1'000 };

		// Set up widget to modify the particle emitter's spawning lower bounds
		m_SpawningLowerBounds.m_Label = "Spawning Lower Bounds";
		m_SpawningLowerBounds.m_Flags |= EditorUI::EditVec3_Indented;
		m_SpawningLowerBounds.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyLowerSpawningBounds);
		m_SpawningLowerBounds.m_Bounds = { -10'000.0f, 10'000.0f };

		// Set up widget to modify the particle emitter's spawning lower bounds
		m_SpawningUpperBounds.m_Label = "Spawning Upper Bounds";
		m_SpawningUpperBounds.m_Flags |= EditorUI::EditVec3_Indented;
		m_SpawningUpperBounds.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyUpperSpawningBounds);
		m_SpawningUpperBounds.m_Bounds = { -10'000.0f, 10'000.0f };
	}
	void EmitterConfigPropertiesPanel::InitializeParticleColorOptions()
	{
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
	}
	void EmitterConfigPropertiesPanel::InitializeParticleSizeOptions()
	{
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

		// Early out if no emitter config is selected
		if (!s_EmitterConfigWindow->m_EditorEmitterConfig)
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		// Draw configuration options
		DrawGeneralEmitterConfigOptions();
		DrawSpawningOptions();
		DrawParticleColorOptions();
		DrawParticleSizeOptions();

		// End the window
		EditorUI::EditorUIService::EndWindow();
	}

	void EmitterConfigPropertiesPanel::DrawGeneralEmitterConfigOptions()
	{
		// General emitter/particle options section
		EditorUI::EditorUIService::CollapsingHeader(m_GeneralOptionsHeaderSpec);
		if (m_GeneralOptionsHeaderSpec.m_Expanded)
		{
			// Draw buffer size widget
			Particles::EmitterLifecycle lifeCycleOption = s_EmitterConfigWindow->m_EditorEmitterConfig->m_EmitterLifecycle;
			if (lifeCycleOption == Particles::EmitterLifecycle::Immortal)
			{
				m_EmitterLifecycleSpec.m_SelectedOption = 0;
			}
			else if (lifeCycleOption == Particles::EmitterLifecycle::FixedTime)
			{
				m_EmitterLifecycleSpec.m_SelectedOption = 1;
			}
			else
			{
				KG_WARN("Invalid lifecycle type provided when rendering emitter config UI");
			}
			EditorUI::EditorUIService::RadioSelector(m_EmitterLifecycleSpec);

			// Draw emitter lifetime widget
			m_EmitterLifetimeSpec.m_CurrentFloat = s_EmitterConfigWindow->m_EditorEmitterConfig->m_EmitterLifetime;
			EditorUI::EditorUIService::EditFloat(m_EmitterLifetimeSpec);

			// Draw particle lifetime widget
			m_ParticleLifetimeSpec.m_CurrentFloat = s_EmitterConfigWindow->m_EditorEmitterConfig->m_ParticleLifetime;
			EditorUI::EditorUIService::EditFloat(m_ParticleLifetimeSpec);

			// Draw buffer size widget
			m_BufferSizeSpec.m_CurrentInteger = (int32_t)s_EmitterConfigWindow->m_EditorEmitterConfig->m_BufferSize;
			EditorUI::EditorUIService::EditInteger(m_BufferSizeSpec);

			// Draw (use gravity) widget
			m_UseGravitySpec.m_CurrentBoolean= s_EmitterConfigWindow->m_EditorEmitterConfig->m_UseGravity;
			EditorUI::EditorUIService::Checkbox(m_UseGravitySpec);

			// Draw gravity acceleration widget
			m_GravityAccelerationSpec.m_CurrentVec3 = s_EmitterConfigWindow->m_EditorEmitterConfig->m_GravityAcceleration;
			EditorUI::EditorUIService::EditVec3(m_GravityAccelerationSpec);
		}

	}

	void EmitterConfigPropertiesPanel::DrawSpawningOptions()
	{
		// General emitter/particle options section
		EditorUI::EditorUIService::CollapsingHeader(m_SpawningOptionsHeaderSpec);
		if (m_SpawningOptionsHeaderSpec.m_Expanded)
		{
			// Draw spawn rate widget
			m_SpawnPerSecSpec.m_CurrentInteger = (int32_t)s_EmitterConfigWindow->m_EditorEmitterConfig->m_SpawnRatePerSec;
			EditorUI::EditorUIService::EditInteger(m_SpawnPerSecSpec);

			// Draw lower spawning bounds widget
			m_SpawningLowerBounds.m_CurrentVec3 = s_EmitterConfigWindow->m_EditorEmitterConfig->m_SpawningBounds[0];
			EditorUI::EditorUIService::EditVec3(m_SpawningLowerBounds);

			// Draw upper spawning bounds widget
			m_SpawningUpperBounds.m_CurrentVec3 = s_EmitterConfigWindow->m_EditorEmitterConfig->m_SpawningBounds[1];
			EditorUI::EditorUIService::EditVec3(m_SpawningUpperBounds);
		}
	}

	void EmitterConfigPropertiesPanel::DrawParticleColorOptions()
	{
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
	}

	void EmitterConfigPropertiesPanel::DrawParticleSizeOptions()
	{
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
	void EmitterConfigPropertiesPanel::OnModifyParticleLifetime(EditorUI::EditFloatSpec& spec)
	{
		// Update the particle lifetime for the current emitter config
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
	void EmitterConfigPropertiesPanel::OnModifyLowerSpawningBounds(EditorUI::EditVec3Spec& spec)
	{
		// Update the lower spawning bounds for the current emitter config
		s_EmitterConfigWindow->m_EditorEmitterConfig->m_SpawningBounds[0] = spec.m_CurrentVec3;

		// Set the active editor UI as edited
		s_EmitterConfigWindow->m_MainHeader.m_EditColorActive = true;
		s_EmitterConfigWindow->LoadEditorEmitterIntoParticleService();
	}
	void EmitterConfigPropertiesPanel::OnModifyUpperSpawningBounds(EditorUI::EditVec3Spec& spec)
	{
		// Update the upper spawning bounds for the current emitter config
		s_EmitterConfigWindow->m_EditorEmitterConfig->m_SpawningBounds[1] = spec.m_CurrentVec3;

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
	void EmitterConfigPropertiesPanel::OnModifyEmitterLifecycleType()
	{
		// Update the emitter lifecycle type for the current emitter config
		if (m_EmitterLifecycleSpec.m_SelectedOption == 0)
		{
			// Immortal option selected
			s_EmitterConfigWindow->m_EditorEmitterConfig->m_EmitterLifecycle = Particles::EmitterLifecycle::Immortal;
		}
		else if (m_EmitterLifecycleSpec.m_SelectedOption == 1)
		{
			// Fixed time option selected
			s_EmitterConfigWindow->m_EditorEmitterConfig->m_EmitterLifecycle = Particles::EmitterLifecycle::FixedTime;
		}
		else
		{
			KG_ERROR("Invalid option provided when modifying lifecycle type of an emitter");
		}
		

		// Set the active editor UI as edited
		s_EmitterConfigWindow->m_MainHeader.m_EditColorActive = true;
		s_EmitterConfigWindow->LoadEditorEmitterIntoParticleService();
	}
	void EmitterConfigPropertiesPanel::OnModifyEmitterLifetime(EditorUI::EditFloatSpec& spec)
	{
		// Update the emitter lifetime for the current emitter config
		s_EmitterConfigWindow->m_EditorEmitterConfig->m_EmitterLifetime = spec.m_CurrentFloat;

		// Set the active editor UI as edited
		s_EmitterConfigWindow->m_MainHeader.m_EditColorActive = true;
		s_EmitterConfigWindow->LoadEditorEmitterIntoParticleService();
	}
	void EmitterConfigPropertiesPanel::OnModifyUseGravity(EditorUI::CheckboxSpec& spec)
	{
		// Update the use gravity for the current emitter config
		s_EmitterConfigWindow->m_EditorEmitterConfig->m_UseGravity = spec.m_CurrentBoolean;

		// Set the active editor UI as edited
		s_EmitterConfigWindow->m_MainHeader.m_EditColorActive = true;
		s_EmitterConfigWindow->LoadEditorEmitterIntoParticleService();
	}
	void EmitterConfigPropertiesPanel::OnModifyGravityAcceleration(EditorUI::EditVec3Spec& spec)
	{
		// Update the gravity acceleration for the current emitter config
		s_EmitterConfigWindow->m_EditorEmitterConfig->m_GravityAcceleration = spec.m_CurrentVec3;

		// Set the active editor UI as edited
		s_EmitterConfigWindow->m_MainHeader.m_EditColorActive = true;
		s_EmitterConfigWindow->LoadEditorEmitterIntoParticleService();
	}
}


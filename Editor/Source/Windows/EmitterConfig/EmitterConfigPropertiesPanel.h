#pragma once
#include "Modules/Events/KeyEvent.h"
#include "Modules/EditorUI/EditorUIInclude.h"
#include "Kargono/Core/FixedString.h"
#include "Modules/Particles/ParticleService.h"

#include <string>

namespace Kargono::Panels
{
	class EmitterConfigPropertiesPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		EmitterConfigPropertiesPanel();

	private:
		//=========================
		// Internal Initialization Functions
		//=========================
		void InitializeGeneralEmitterConfigOptions();
		void InitializeSpawningOptions();
		void InitializeParticleColorOptions();
		void InitializeParticleSizeOptions();

	public:
		//=========================
		// On Event Functions
		//=========================
		void OnEditorUIRender();

		//=========================
		// Draw Properties Panel Functions
		//=========================
		void DrawGeneralEmitterConfigOptions();
		void DrawSpawningOptions();
		void DrawParticleColorOptions();
		void DrawParticleSizeOptions();

		//=========================
		// Modify Panel Data
		//=========================
		void ClearPanelData();

	private:

		//=========================
		// Change Emitter Data
		//=========================
		
		// General options modification
		void OnModifyBufferSize(EditorUI::EditIntegerWidget& spec);
		void OnModifyEmitterLifecycleType();
		void OnModifyEmitterLifetime(EditorUI::EditFloatWidget& spec);
		void OnModifyParticleLifetime(EditorUI::EditFloatWidget& spec);
		void OnModifyUseGravity(EditorUI::CheckboxWidget& spec);
		void OnModifyGravityAcceleration(EditorUI::EditVec3Widget& spec);
		// Spawning options modification
		void OnModifySpawnRate(EditorUI::EditIntegerWidget& spec);
		void OnModifyLowerSpawningBounds(EditorUI::EditVec3Widget& spec);
		void OnModifyUpperSpawningBounds(EditorUI::EditVec3Widget& spec);
		// Color modification
		void OnModifyColorBegin(EditorUI::EditVec4Widget& spec);
		void OnModifyColorEnd(EditorUI::EditVec4Widget& spec);
		void OnModifyColorCurve(const EditorUI::OptionEntry& entry);
		// Size modification
		void OnModifySizeBegin(EditorUI::EditVec3Widget& spec);
		void OnModifySizeEnd(EditorUI::EditVec3Widget& spec);
		void OnModifySizeCurve(const EditorUI::OptionEntry& entry);
	public:
		//=========================
		// Core Panel Data
		//=========================
		FixedString32 m_PanelName{ "Particle Emitter Properties" };

	public:
		//=========================
		// Widgets
		//=========================
		// General options widgets
		EditorUI::CollapsingHeaderWidget m_GeneralOptionsHeaderSpec;
		EditorUI::EditIntegerWidget m_BufferSizeSpec;
		EditorUI::EditIntegerWidget m_SpawnPerSecSpec;
		EditorUI::EditFloatWidget m_ParticleLifetimeSpec;
		EditorUI::EditFloatWidget m_EmitterLifetimeSpec;
		EditorUI::CheckboxWidget m_UseGravitySpec;
		EditorUI::EditVec3Widget m_GravityAccelerationSpec;
		EditorUI::RadioSelectWidget m_EmitterLifecycleSpec;
		// Particle spawning options widgets
		EditorUI::CollapsingHeaderWidget m_SpawningOptionsHeaderSpec;
		EditorUI::EditVec3Widget m_SpawningLowerBounds;
		EditorUI::EditVec3Widget m_SpawningUpperBounds;
		// Particle color options widgets
		EditorUI::CollapsingHeaderWidget m_ColorOptionsHeaderSpec;
		EditorUI::SelectOptionWidget m_SelectColorInterpSpec;
		EditorUI::EditVec4Widget m_ColorBeginSpec;
		EditorUI::EditVec4Widget m_ColorEndSpec;
		// Particle size options widgets
		EditorUI::CollapsingHeaderWidget m_SizeOptionsHeaderSpec;
		EditorUI::SelectOptionWidget m_SelectSizeInterpSpec;
		EditorUI::EditVec3Widget m_SizeBeginSpec;
		EditorUI::EditVec3Widget m_SizeEndSpec;

	};
}

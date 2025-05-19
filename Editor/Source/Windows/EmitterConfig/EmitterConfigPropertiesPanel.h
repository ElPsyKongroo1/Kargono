#pragma once
#include "Modules/Events/KeyEvent.h"
#include "Modules/EditorUI/EditorUI.h"
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
		void OnModifyBufferSize(EditorUI::EditIntegerSpec& spec);
		void OnModifyEmitterLifecycleType();
		void OnModifyEmitterLifetime(EditorUI::EditFloatSpec& spec);
		void OnModifyParticleLifetime(EditorUI::EditFloatSpec& spec);
		void OnModifyUseGravity(EditorUI::CheckboxSpec& spec);
		void OnModifyGravityAcceleration(EditorUI::EditVec3Spec& spec);
		// Spawning options modification
		void OnModifySpawnRate(EditorUI::EditIntegerSpec& spec);
		void OnModifyLowerSpawningBounds(EditorUI::EditVec3Spec& spec);
		void OnModifyUpperSpawningBounds(EditorUI::EditVec3Spec& spec);
		// Color modification
		void OnModifyColorBegin(EditorUI::EditVec4Spec& spec);
		void OnModifyColorEnd(EditorUI::EditVec4Spec& spec);
		void OnModifyColorCurve(const EditorUI::OptionEntry& entry);
		// Size modification
		void OnModifySizeBegin(EditorUI::EditVec3Spec& spec);
		void OnModifySizeEnd(EditorUI::EditVec3Spec& spec);
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
		EditorUI::CollapsingHeaderSpec m_GeneralOptionsHeaderSpec;
		EditorUI::EditIntegerSpec m_BufferSizeSpec;
		EditorUI::EditIntegerSpec m_SpawnPerSecSpec;
		EditorUI::EditFloatSpec m_ParticleLifetimeSpec;
		EditorUI::EditFloatSpec m_EmitterLifetimeSpec;
		EditorUI::CheckboxSpec m_UseGravitySpec;
		EditorUI::EditVec3Spec m_GravityAccelerationSpec;
		EditorUI::RadioSelectorSpec m_EmitterLifecycleSpec;
		// Particle spawning options widgets
		EditorUI::CollapsingHeaderSpec m_SpawningOptionsHeaderSpec;
		EditorUI::EditVec3Spec m_SpawningLowerBounds;
		EditorUI::EditVec3Spec m_SpawningUpperBounds;
		// Particle color options widgets
		EditorUI::CollapsingHeaderSpec m_ColorOptionsHeaderSpec;
		EditorUI::SelectOptionSpec m_SelectColorInterpSpec;
		EditorUI::EditVec4Spec m_ColorBeginSpec;
		EditorUI::EditVec4Spec m_ColorEndSpec;
		// Particle size options widgets
		EditorUI::CollapsingHeaderSpec m_SizeOptionsHeaderSpec;
		EditorUI::SelectOptionSpec m_SelectSizeInterpSpec;
		EditorUI::EditVec3Spec m_SizeBeginSpec;
		EditorUI::EditVec3Spec m_SizeEndSpec;

	};
}

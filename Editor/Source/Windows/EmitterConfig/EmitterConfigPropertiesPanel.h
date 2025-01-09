#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/EditorUI/EditorUI.h"
#include "Kargono/Core/FixedString.h"
#include "Kargono/Particles/ParticleService.h"

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
		void InitializeEmitterConfigOptions();

	public:
		//=========================
		// On Event Functions
		//=========================
		void OnEditorUIRender();

		//=========================
		// Draw Properties Panel Functions
		//=========================
		void DrawEmitterConfigOptions();

		//=========================
		// Modify Panel Data
		//=========================
		void ClearPanelData();

	private:

		//=========================
		// Change Emitter Data
		//=========================
		void OnModifyColorBegin(EditorUI::EditVec4Spec& spec);
		void OnModifyColorEnd(EditorUI::EditVec4Spec& spec);

		void OnModifySizeBegin(EditorUI::EditVec3Spec& spec);
		void OnModifySizeEnd(EditorUI::EditVec3Spec& spec);
		void OnModifySizeCurve(const EditorUI::OptionEntry& entry);
		void OnModifyColorCurve(const EditorUI::OptionEntry& entry);


		void OnModifyLifetime(EditorUI::EditFloatSpec& spec);
		void OnModifySpawnRate(EditorUI::EditIntegerSpec& spec);
		void OnModifyBufferSize(EditorUI::EditIntegerSpec& spec);
	public:
		//=========================
		// Core Panel Data
		//=========================
		FixedString32 m_PanelName{ "Particle Emitter Properties" };

	public:
		//=========================
		// Widgets
		//=========================

		EditorUI::CollapsingHeaderSpec m_GeneralOptionsHeaderSpec;
		EditorUI::EditIntegerSpec m_BufferSizeSpec;
		EditorUI::EditIntegerSpec m_SpawnPerSecSpec;
		EditorUI::EditFloatSpec m_LifetimeSpec;

		EditorUI::CollapsingHeaderSpec m_ColorOptionsHeaderSpec;
		EditorUI::SelectOptionSpec m_SelectColorInterpSpec;
		EditorUI::EditVec4Spec m_ColorBeginSpec;
		EditorUI::EditVec4Spec m_ColorEndSpec;

		EditorUI::CollapsingHeaderSpec m_SizeOptionsHeaderSpec;
		EditorUI::SelectOptionSpec m_SelectSizeInterpSpec;
		EditorUI::EditVec3Spec m_SizeBeginSpec;
		EditorUI::EditVec3Spec m_SizeEndSpec;
	};
}

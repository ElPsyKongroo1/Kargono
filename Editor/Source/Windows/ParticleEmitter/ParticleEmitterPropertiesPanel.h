#if 0

#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/EditorUI/EditorUI.h"
#include "Kargono/Core/FixedString.h"
#include "Kargono/Particles/ParticleService.h"

#include <string>

namespace Kargono::Panels
{

	class ParticleEmitterPropertiesPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		ParticleEmitterPropertiesPanel();

	private:
		//=========================
		// Internal Initialization Functions
		//=========================
		void InitializeParticleEmitterOptions();

	public:
		//=========================
		// On Event Functions
		//=========================
		void OnEditorUIRender();

		//=========================
		// Draw Properties Panel Functions
		//=========================
		void DrawParticleEmitterOptions();

		//=========================
		// Modify Panel Data
		//=========================
		void ClearPanelData();

	private:

		//=========================
		// Change Emitter Data
		//=========================

	public:
		//=========================
		// Core Panel Data
		//=========================
		FixedString32 m_PanelName{ "Particle Emitter Properties" };

	public:
		//=========================
		// Widgets
		//=========================
	};
}

#endif

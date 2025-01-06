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

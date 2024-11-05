#pragma once
#include "Kargono/Events/KeyEvent.h"

#include <string>

namespace Kargono::Panels
{
	class StatisticsPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		StatisticsPanel();

		//=========================
		// On Event Functions
		//=========================
		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
	private:
		//=========================
		// Core Panel Data
		//=========================
		FixedString32 m_PanelName{ "Statistics" };
	};
}

#pragma once
#include "Kargono/Events/KeyEvent.h"

#include <string>

namespace Kargono::Panels
{
	class UIEditorViewportPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		UIEditorViewportPanel();

		//=========================
		// On Event Functions
		//=========================
		void OnEditorUIRender();
	public:
		//=========================
		// Core Panel Data
		//=========================
		FixedString32 m_PanelName{ "User Interface Viewport" };
	};
}

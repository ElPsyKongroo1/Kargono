#pragma once
#include "Kargono/Events/KeyEvent.h"

#include <string>

namespace Kargono::Panels
{
	class UIEditorPropertiesPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		UIEditorPropertiesPanel();

		//=========================
		// On Event Functions
		//=========================
		void OnEditorUIRender();
	public:
		//=========================
		// Core Panel Data
		//=========================
		FixedString32 m_PanelName{ "UIEditorPropertiesPanel" };
	};
}

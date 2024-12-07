#pragma once

#include "Kargono/Core/FixedString.h"

#include <string>

namespace Kargono::Panels
{
	class PropertiesPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		PropertiesPanel();

		//=========================
		// On Event Functions
		//=========================
		void OnEditorUIRender();

	private:
		//=========================
		// Core Panel Data
		//=========================
		FixedString32 m_ActiveParent;
		FixedString32 m_PanelName{ "Properties" };
	private:
		friend class SceneEditorPanel;
		friend class UIEditorPanel;
	};
}

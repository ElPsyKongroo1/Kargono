#pragma once

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
		std::string m_ActiveParent{};
		std::string m_PanelName{ "Properties" };
	private:
		friend class SceneEditorPanel;
		friend class UIEditorPanel;
	};
}

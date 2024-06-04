#pragma once
#include "Kargono/Events/KeyEvent.h"

namespace Kargono
{
	class SettingsPanel
	{
	public:
		SettingsPanel();

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
	private:
		std::string m_PanelName{ "Settings" };
	};
}

#pragma once
#include "Kargono/Events/KeyEvent.h"

#include <string>

namespace Kargono::Panels
{
	class ScriptEditorPanel
	{
	public:
		ScriptEditorPanel();

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
	private:
		std::string m_PanelName{ "Scripts" };
	};
}

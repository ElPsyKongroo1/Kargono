#pragma once
#include "Kargono/Events/KeyEvent.h"

#include <string>

namespace Kargono
{
	class ToolbarPanel
	{
	public:
		ToolbarPanel();

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
	private:
		std::string m_PanelName{ "##toolbar" };

	};
}

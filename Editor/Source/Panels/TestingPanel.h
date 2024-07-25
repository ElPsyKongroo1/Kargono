#pragma once
#include "Kargono/Events/KeyEvent.h"

#include <string>

namespace Kargono::Panels
{
	class TestingPanel
	{
	public:
		TestingPanel();

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
	private:
		std::string m_PanelName{ "Testing" };
	};
}

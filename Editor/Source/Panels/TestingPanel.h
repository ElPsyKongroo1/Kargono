#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono.h"

#include <string>

namespace Kargono::Panels
{
	class TestingPanel
	{
	public:
		TestingPanel();

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
	public:
		static ImVec4 s_TestColor;
		static ImVec4 s_TestColor2;
	private:
		std::string m_PanelName{ "Testing" };
	};
}

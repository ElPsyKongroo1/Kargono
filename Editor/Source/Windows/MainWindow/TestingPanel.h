#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Core/FixedString.h"
#include "Kargono/EditorUI/EditorUI.h"

#include <string>

namespace Kargono::Panels
{
	class TestingPanel
	{
	public:
		TestingPanel();

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
		bool OnInputEvent(Events::Event* event);
	private:
		FixedString32 m_PanelName{ "Testing" };

		//==================================
		// Content browser widget test
		//==================================
	public:

		EditorUI::NavigationHeaderSpec m_TestHeader{};
	};
}

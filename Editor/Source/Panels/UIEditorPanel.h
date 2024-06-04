#pragma once
#include "Kargono/Events/KeyEvent.h"

#include <string>

namespace Kargono
{
	class UIEditorPanel
	{
	public:
		UIEditorPanel();

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
	private:
		std::string m_PanelName{ "User Interface Editor" };

	};
}

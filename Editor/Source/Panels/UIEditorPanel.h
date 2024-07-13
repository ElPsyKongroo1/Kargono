#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/RuntimeUI/RuntimeUI.h"

#include <string>

namespace Kargono { class EditorApp; }


namespace Kargono::Panels
{
	class UIEditorPanel
	{
	public:
		UIEditorPanel();

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
	private:
		std::string m_PanelName{ "User Interface Editor" };
	private:
		friend Kargono::EditorApp;
	};
}

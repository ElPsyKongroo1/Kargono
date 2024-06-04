#pragma once
#include "Kargono/Events/KeyEvent.h"

#include <filesystem>
#include <string>

namespace Kargono
{
	class TextEditorPanel
	{
	public:
		TextEditorPanel();

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);

		void OpenFile(const std::filesystem::path& path);
	private:
		std::string m_PanelName{ "Text Editor" };
	};
}

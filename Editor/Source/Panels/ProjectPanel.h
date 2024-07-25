#pragma once
#include "Kargono/Events/KeyEvent.h"

#include <string>

namespace Kargono::Panels
{
	class ProjectPanel
	{
	public:
		ProjectPanel();

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);

		void ResetPanelResources();
	private:
		std::string m_PanelName{ "Project" };
	};
}

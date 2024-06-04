#pragma once
#include "Kargono/Events/KeyEvent.h"

#include <string>

namespace Kargono
{
	class StatisticsPanel
	{
	public:
		StatisticsPanel();

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
	private:
		std::string m_PanelName{ "Statistics" };
	};
}

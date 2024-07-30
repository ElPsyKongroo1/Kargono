#pragma once

#include <string>

namespace Kargono::Panels
{
	class PropertiesPanel
	{
	public:
		PropertiesPanel();

		void OnEditorUIRender();
	private:
		std::string m_PanelName{ "Properties" };
	private:
		friend class SceneEditorPanel;
	};
}

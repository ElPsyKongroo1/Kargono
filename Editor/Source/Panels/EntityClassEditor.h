#pragma once
#include "Kargono.h"

#include <string>

namespace Kargono::Panels
{
	class EntityClassEditor
	{
	public:
		EntityClassEditor();

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
	private:
		void InitializeDisplayEntityClassScreen();

	public:
		void RefreshEntityScripts(Assets::AssetHandle handle);

	private:
		std::string m_PanelName{ "Class Editor" };

	};
}

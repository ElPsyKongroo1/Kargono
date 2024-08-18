#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/RuntimeUI/RuntimeUI.h"

#include <string>

namespace Kargono { class EditorApp; }


namespace Kargono::Panels
{
	enum class UIPropertiesDisplay : uint16_t
	{
		None = 0,
		Window,
		Widget
	};

	class UIEditorPanel
	{
	public:
		UIEditorPanel();

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);

	private:
		void DrawWindowOptions();
		void DrawWidgetOptions();
	private:
		void InitializeOpeningScreen();
		void InitializeUIHeader();
		void InitializeMainContent();
		void InitializeWindowOptions();
		void InitializeWidgetOptions();
	public:
		Ref<RuntimeUI::UserInterface> m_EditorUI{ nullptr };
		Assets::AssetHandle m_EditorUIHandle { Assets::EmptyHandle };
	private:
		std::string m_PanelName{ "User Interface Editor" };
		UIPropertiesDisplay m_CurrentDisplay{ UIPropertiesDisplay::None };
		RuntimeUI::Widget* m_ActiveWidget {nullptr};
		RuntimeUI::Window* m_ActiveWindow {nullptr};
	private:
		friend class PropertiesPanel;
		friend Kargono::EditorApp;
	};
}

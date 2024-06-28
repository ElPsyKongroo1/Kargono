#pragma once
#include "Kargono/Assets/Asset.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Input/InputMode.h"

#include <string>


namespace Kargono::Panels
{
	class InputModePanel
	{
	public:
		InputModePanel();

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
	private:
		void InitializeOpeningScreen();
		void InitializeInputModeHeader();
		void InitializeKeyboardScreen();
	public:
		Ref<Input::InputMode> m_EditorInputMode{ nullptr };
		Assets::AssetHandle m_EditorInputModeHandle { Assets::EmptyHandle };
	private:
		std::string m_PanelName{ "Input Mode Editor" };

	};
}

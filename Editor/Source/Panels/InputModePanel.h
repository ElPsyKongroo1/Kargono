#pragma once
#include "Kargono/Assets/Asset.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Input/InputMode.h"
#include "Kargono/EditorUI/EditorUI.h"

#include <string>


namespace Kargono::Panels
{
	class InputModePanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		InputModePanel();
	private:
		void InitializeOpeningScreen();
		void InitializeInputModeHeader();
		void InitializeKeyboardScreen();

	public:
		//=========================
		// On Event Functions
		//=========================
		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);

		//=========================
		// External API
		//=========================
		void ResetPanelResources();

	private:
		//=========================
		// Internal Functionality
		//=========================
		void OnOpenInputMode();
		void OnCreateInputMode();
		void OnRefreshData();
	public:
		//=========================
		// Core Panel Data
		//=========================
		Ref<Input::InputMode> m_EditorInputMode{ nullptr };
		Assets::AssetHandle m_EditorInputModeHandle { Assets::EmptyHandle };
	private:
		std::string m_PanelName{ "Input Mode Editor" };
		uint32_t m_KeyboardOnUpdateActiveSlot {};
		uint32_t m_KeyboardOnKeyPressedActiveSlot {};
		uint32_t m_KeyboardPollingActiveSlot {};

		//=========================
		// Widgets
		//=========================
		// Opening Panel w/ Popups
		EditorUI::SelectOptionSpec m_OpenInputModePopupSpec {};
		EditorUI::GenericPopupSpec m_CreateInputModePopupSpec {};
		EditorUI::EditTextSpec m_SelectInputModeNameSpec {};

		// Input Mode Header
		EditorUI::PanelHeaderSpec m_MainHeader {};
		EditorUI::GenericPopupSpec m_DeleteInputModeWarning {};
		EditorUI::GenericPopupSpec m_CloseInputModeWarning {};

		// Keyboard Panel
		//	OnUpdate
		EditorUI::TableSpec m_KeyboardOnUpdateTable {};
		EditorUI::GenericPopupSpec m_KeyboardOnUpdateAddSlot {};
		EditorUI::SelectOptionSpec m_KeyboardOnUpdateAddKeyCode {};
		EditorUI::SelectOptionSpec m_KeyboardOnUpdateAddFunction {};
		EditorUI::GenericPopupSpec m_KeyboardOnUpdateEditSlot {};
		EditorUI::SelectOptionSpec m_KeyboardOnUpdateEditKeyCode {};
		EditorUI::SelectOptionSpec m_KeyboardOnUpdateEditFunction {};

		//	OnKeyPressed
		EditorUI::TableSpec m_KeyboardOnKeyPressedTable {};
		EditorUI::GenericPopupSpec m_KeyboardOnKeyPressedAddSlot {};
		EditorUI::SelectOptionSpec m_KeyboardOnKeyPressedAddKeyCode {};
		EditorUI::SelectOptionSpec m_KeyboardOnKeyPressedAddFunction {};
		EditorUI::GenericPopupSpec m_KeyboardOnKeyPressedEditSlot {};
		EditorUI::SelectOptionSpec m_KeyboardOnKeyPressedEditKeyCode {};
		EditorUI::SelectOptionSpec m_KeyboardOnKeyPressedEditFunction {};

		//  Polling
		EditorUI::TableSpec m_KeyboardPollingTable {};
		EditorUI::GenericPopupSpec m_KeyboardPollingAddSlot {};
		EditorUI::SelectOptionSpec m_KeyboardPollingAddKeyCode {};
		EditorUI::GenericPopupSpec m_KeyboardPollingEditSlot {};
		EditorUI::SelectOptionSpec m_KeyboardPollingEditKeyCode {};

	};
}

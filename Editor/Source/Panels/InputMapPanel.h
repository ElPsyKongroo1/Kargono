#pragma once
#include "Kargono/Assets/Asset.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Input/InputMap.h"
#include "Kargono/EditorUI/EditorUI.h"

#include <string>


namespace Kargono::Panels
{
	class InputMapPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		InputMapPanel();
	private:
		//=========================
		// Internal Initialization Functions
		//=========================
		void InitializeOpeningScreen();
		void InitializeInputMapHeader();
		void InitializeKeyboardScreen();

	public:
		//=========================
		// On Event Functions
		//=========================
		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
		bool OnAssetEvent(Events::Event* event);

		//=========================
		// External API
		//=========================
		void ResetPanelResources();
		void OpenCreateDialog(std::filesystem::path& createLocation);
		void OpenAssetInEditor(std::filesystem::path& assetLocation);

	private:
		//=========================
		// Internal Functionality
		//=========================
		void OnOpenInputMapDialog();
		void OnCreateInputMapDialog();
		void OnOpenInputMap(Assets::AssetHandle newHandle);
		void OnRefreshData();
	public:
		//=========================
		// Core Panel Data
		//=========================
		Ref<Input::InputMap> m_EditorInputMap{ nullptr };
		Assets::AssetHandle m_EditorInputMapHandle { Assets::EmptyHandle };
	private:
		FixedString32 m_PanelName{ "Input Map Editor" };
		uint32_t m_KeyboardOnUpdateActiveSlot {};
		uint32_t m_KeyboardOnKeyPressedActiveSlot {};
		uint32_t m_KeyboardPollingActiveSlot {};

		//=========================
		// Widgets
		//=========================
		// Opening Panel w/ Popups
		EditorUI::SelectOptionSpec m_OpenInputMapPopupSpec {};
		EditorUI::GenericPopupSpec m_CreateInputMapPopupSpec {};
		EditorUI::EditTextSpec m_SelectInputMapNameSpec {};
		EditorUI::ChooseDirectorySpec m_SelectInputMapLocationSpec{};

		// Input Map Header
		EditorUI::PanelHeaderSpec m_MainHeader {};
		EditorUI::GenericPopupSpec m_DeleteInputMapWarning {};
		EditorUI::GenericPopupSpec m_CloseInputMapWarning {};

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

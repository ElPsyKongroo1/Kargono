#pragma once
#include "Modules/Assets/Asset.h"
#include "Modules/Events/KeyEvent.h"
#include "Modules/InputMap/InputMap.h"
#include "Modules/EditorUI/EditorUIInclude.h"

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
		EditorUI::SelectOptionWidget m_OpenInputMapPopupSpec {};
		EditorUI::GenericPopupWidget m_CreateInputMapPopupSpec {};
		EditorUI::EditTextSpec m_SelectInputMapNameSpec {};
		EditorUI::ChooseDirectoryWidget m_SelectInputMapLocationSpec{};

		// Input Map Header
		EditorUI::PanelHeaderWidget m_MainHeader {};
		EditorUI::GenericPopupWidget m_DeleteInputMapWarning {};
		EditorUI::GenericPopupWidget m_CloseInputMapWarning {};

		// Keyboard Panel
		//	OnUpdate
		EditorUI::ListWidget m_KeyboardOnUpdateTable {};
		EditorUI::TooltipWidget m_SelectScriptTooltip{};
		EditorUI::GenericPopupWidget m_KeyboardOnUpdateAddPopup {};
		EditorUI::SelectOptionWidget m_KeyboardOnUpdateAddKeyCode {};
		EditorUI::SelectOptionWidget m_KeyboardOnUpdateAddFunction {};
		EditorUI::GenericPopupWidget m_KeyboardOnUpdateEditPopup {};
		EditorUI::SelectOptionWidget m_KeyboardOnUpdateEditKeyCode {};
		EditorUI::SelectOptionWidget m_KeyboardOnUpdateEditFunction {};

		//	OnKeyPressed
		EditorUI::ListWidget m_KeyboardOnKeyPressedTable {};
		EditorUI::GenericPopupWidget m_KeyboardOnKeyPressedAddPopup {};
		EditorUI::SelectOptionWidget m_KeyboardOnKeyPressedAddKeyCode {};
		EditorUI::SelectOptionWidget m_KeyboardOnKeyPressedAddFunction {};
		EditorUI::GenericPopupWidget m_KeyboardOnKeyPressedEditPopup {};
		EditorUI::SelectOptionWidget m_KeyboardOnKeyPressedEditKeyCode {};
		EditorUI::SelectOptionWidget m_KeyboardOnKeyPressedEditFunction {};

		//  Polling
		EditorUI::ListWidget m_KeyboardPollingTable {};
		EditorUI::GenericPopupWidget m_KeyboardPollingAddSlot {};
		EditorUI::SelectOptionWidget m_KeyboardPollingAddKeyCode {};
		EditorUI::GenericPopupWidget m_KeyboardPollingEditSlot {};
		EditorUI::SelectOptionWidget m_KeyboardPollingEditKeyCode {};

	};
}

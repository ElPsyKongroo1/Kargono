#pragma once

#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/EditorUI/EditorUI.h"

#include <string>

namespace Kargono::Panels
{
	class UIEditorTreePanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		UIEditorTreePanel();

	private:
		//=========================
		// Internal Initialization Functions
		//=========================
		void InitializeOpeningScreen();
		void InitializeUIHeader();
		void InitializeMainContent();

	public:
		//=========================
		// On Event Functions
		//=========================
		void OnEditorUIRender();

	public:
		//=========================
		// Manage Panel State
		//=========================
		void OnRefreshData();
		void OnRefreshUITree();
		void RecalculateTreeIndexData();
		void SelectTreeNode(uint16_t windowID, uint16_t widgetID);

		//=========================
		// On Action Functions
		//=========================
		void OnOpenUIDialog();
		void OnCreateUIDialog();
		void OnOpenUI(Assets::AssetHandle newHandle);

		//=========================
		// Manage UserInterface
		//=========================
		void SelectUI(EditorUI::TreeEntry& entry);

		//=========================
		// Manage Windows
		//=========================
		void AddWindow();
		void AddWindow(EditorUI::TreeEntry& entry);
		void SelectWindow(EditorUI::TreeEntry& entry);
		void DeleteWindow(EditorUI::TreeEntry& entry);

		//=========================
		// Manage Widgets
		//=========================
		void AddTextWidget(EditorUI::TreeEntry& entry);
		void SelectTextWidget(EditorUI::TreeEntry& entry);
		void DeleteWidget(EditorUI::TreeEntry& entry);
	public:
		//=========================
		// Core Panel Data
		//=========================
		FixedString32 m_PanelName{ "User Interface Tree" };

		//=========================
		// Widgets
		//=========================
		// Main Content
		EditorUI::TreeSpec m_UITree{};

		// Opening Panel w/ Popups
		EditorUI::SelectOptionSpec m_OpenUIPopupSpec{};
		EditorUI::GenericPopupSpec m_CreateUIPopupSpec{};
		EditorUI::EditTextSpec m_SelectUINameSpec{};
		EditorUI::ChooseDirectorySpec m_SelectUILocationSpec{};
		EditorUI::TooltipSpec m_SelectScriptTooltip{};

		// User Interface Header
		EditorUI::PanelHeaderSpec m_MainHeader{};
		EditorUI::GenericPopupSpec m_DeleteUIWarning{};
		EditorUI::GenericPopupSpec m_CloseUIWarning{};
	};
}

#pragma once

#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/EditorUI/EditorUI.h"
#include "Kargono/Rendering/Texture.h"
#include "Kargono/RuntimeUI/RuntimeUI.h"


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
		void SelectTreeNode(RuntimeUI::IDType idType, int32_t id);

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
		void AddWindow(EditorUI::TreeEntry& entry);
		void SelectWindow(EditorUI::TreeEntry& entry);
		void DeleteWindow(EditorUI::TreeEntry& entry);

		//=========================
		// Manage Widgets
		//=========================
		void AddTextWidget(EditorUI::TreeEntry& entry);
		void AddButtonWidget(EditorUI::TreeEntry& entry);
		void AddImageWidget(EditorUI::TreeEntry& entry);
		void AddImageButtonWidget(EditorUI::TreeEntry& entry);
		void AddCheckboxWidget(EditorUI::TreeEntry& entry);
		void AddContainerWidget(EditorUI::TreeEntry& entry);
		void AddInputTextWidget(EditorUI::TreeEntry& entry);
		void AddSliderWidget(EditorUI::TreeEntry& entry);
		void AddDropDownWidget(EditorUI::TreeEntry& entry);
		void SelectWidget(EditorUI::TreeEntry& entry);
		void DeleteWidget(EditorUI::TreeEntry& entry);

	private:
		//=========================
		// Manage Widgets (Internal)
		//=========================
		EditorUI::TreeEntry* AddWidgetInternal(EditorUI::TreeEntry& parentEntry, Ref<RuntimeUI::Widget> newWidget, Ref<Rendering::Texture2D> widgetIcon);

		//=========================
		// Manage Tree (Internal)
		//=========================
		void CreateWindowSelectionOptions(EditorUI::TreeEntry& windowEntry);
		void CreateAddWidgetsSelectionOptions(EditorUI::TreeEntry& entry);
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
		EditorUI::TooltipSpec m_SelectTooltip{};

		// User Interface Header
		EditorUI::PanelHeaderSpec m_MainHeader{};
		EditorUI::GenericPopupSpec m_DeleteUIWarning{};
		EditorUI::GenericPopupSpec m_CloseUIWarning{};
	};
}

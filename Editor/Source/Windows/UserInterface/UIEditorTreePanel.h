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
		void RightClickUIEntry(EditorUI::TreeEntry& entry);

		//=========================
		// Manage Windows
		//=========================
		void AddWindow(EditorUI::TooltipEntry& entry);
		void SelectWindow(EditorUI::TreeEntry& entry);
		void ToggleWindowVisibility(EditorUI::TooltipEntry& entry);
		void RenameWindow(EditorUI::TooltipEntry& entry);
		void DeleteWindow(EditorUI::TooltipEntry& entry);
		void RightClickWindowEntry(EditorUI::TreeEntry& entry);

		//=========================
		// Manage Widgets
		//=========================
		void RightClickWidgetEntry(EditorUI::TreeEntry& entry);
		void AddTextWidget(EditorUI::TooltipEntry& entry);
		void AddButtonWidget(EditorUI::TooltipEntry& entry);
		void AddImageWidget(EditorUI::TooltipEntry& entry);
		void AddImageButtonWidget(EditorUI::TooltipEntry& entry);
		void AddCheckboxWidget(EditorUI::TooltipEntry& entry);
		void AddContainerWidget(EditorUI::TooltipEntry& entry);
		void AddHorizontalContainerWidget(EditorUI::TooltipEntry& entry);
		void AddVerticalContainerWidget(EditorUI::TooltipEntry& entry);
		void AddInputTextWidget(EditorUI::TooltipEntry& entry);
		void AddSliderWidget(EditorUI::TooltipEntry& entry);
		void AddDropDownWidget(EditorUI::TooltipEntry& entry);
		void SelectWidget(EditorUI::TreeEntry& entry);
		void RenameWidget(EditorUI::TooltipEntry& entry);
		void DeleteWidget(EditorUI::TooltipEntry& entry);

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
		void CreateWidgetSpecificSelectionOptions(EditorUI::TreeEntry& widgetEntry, RuntimeUI::WidgetTypes widgetType);
		void CreateContainerDataWidgets(EditorUI::TreeEntry& parentEntry, RuntimeUI::ContainerData* container);
		void CreateWidgetTreeEntry(EditorUI::TreeEntry& parentEntry, Ref<RuntimeUI::Widget> currentWidget);
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

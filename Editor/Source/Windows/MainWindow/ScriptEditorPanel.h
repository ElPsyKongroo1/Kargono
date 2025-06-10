#pragma once
#include "Modules/Events/KeyEvent.h"
#include "Modules/EditorUI/EditorUIInclude.h"

#include <string>

namespace Kargono::Panels
{
	struct CreateWidgets
	{
		EditorUI::GenericPopupWidget m_MainPopup{};
		EditorUI::EditTextSpec m_EditName{};
		EditorUI::SelectOptionWidget m_SelectSectionLabel{};
		EditorUI::SelectOptionWidget m_SelectReturnType{};
		EditorUI::ListWidget m_ParameterList{};
		EditorUI::GenericPopupWidget m_CreateParameterPopup{};
		EditorUI::EditTextSpec m_CreateParameterName{};
		EditorUI::SelectOptionWidget m_CreateParameterType{};
		EditorUI::GenericPopupWidget m_EditParameterPopup{};
		EditorUI::EditTextSpec m_EditParameterName{};
		EditorUI::SelectOptionWidget m_EditParameterType{};
	};

	struct EditWidgets
	{
		EditorUI::GenericPopupWidget m_MainPopup{};
		EditorUI::EditTextSpec m_EditName{};
		EditorUI::SelectOptionWidget m_SelectSectionLabel{};
		EditorUI::GenericPopupWidget m_DeleteWarning{};
		EditorUI::GenericPopupWidget m_EditWarning{};
	};

	class ScriptEditorPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		ScriptEditorPanel();

	private:
		//=========================
		// Internal Initialization Functions
		//=========================
		void InitializeAllScriptsList();
		void InitializeCreateScriptPopup();
		void InitializeEditScriptPopup();
		void InitializeGroupList();

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
		void OpenCreateScriptDialogFromUsagePoint(WrappedFuncType scriptType, std::function<void(Assets::AssetHandle)> onConfirm, const std::vector<FixedString32>& parameterNames, bool openScriptEditor = true);
		void DrawOnCreatePopup();
		

	private:
		//=========================
		// Internal Functionality
		//=========================
		void UpdateScript();
		void OnOpenScriptDialog(EditorUI::ListEntry& entry, std::size_t iteration);
		void OnCreateScriptDialog();

		void OnCreateScriptEditParameter(EditorUI::ListEntry& entry, std::size_t iteration);

	private:
		//=========================
		// Core Panel Data
		//=========================
		FixedString32 m_PanelName{ "Scripts" };
		Assets::AssetHandle m_ActiveScriptHandle {Assets::EmptyHandle};
		std::string m_ActiveLabel {};
		std::size_t m_ActiveParameterLocation{ EditorUI::k_ListIndex };
		std::function<void(Assets::AssetHandle)> m_OnCreateScriptConfirm{ nullptr };
		WrappedFuncType m_OnCreateFunctionType{ WrappedFuncType::None };
		std::vector<FixedString32> m_OnCreateParameterNames;

		//=========================
		// Widgets
		//=========================
		// Script List
		EditorUI::ListWidget m_AllScriptsList {};
		EditorUI::TooltipWidget m_ScriptTooltip{};
		// Script List (Create)
		CreateWidgets m_CreateWidgets{};

		// Script List (Edit)
		EditWidgets m_EditWidgets{};

		// Group Label List
		EditorUI::ListWidget m_GroupLabelsTable {};
		EditorUI::EditTextSpec m_CreateGroupLabelPopup {};
		EditorUI::GenericPopupWidget m_EditGroupLabelPopup {};
		EditorUI::EditTextSpec m_EditGroupLabelText {};
	};
}

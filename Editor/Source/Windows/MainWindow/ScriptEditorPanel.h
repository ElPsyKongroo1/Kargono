#pragma once
#include "EventModule/KeyEvent.h"
#include "EditorUIModule/EditorUI.h"

#include <string>

namespace Kargono::Panels
{
	struct CreateWidgets
	{
		EditorUI::GenericPopupSpec m_MainPopup{};
		EditorUI::EditTextSpec m_EditName{};
		EditorUI::SelectOptionSpec m_SelectSectionLabel{};
		EditorUI::SelectOptionSpec m_SelectReturnType{};
		EditorUI::ListSpec m_ParameterList{};
		EditorUI::GenericPopupSpec m_CreateParameterPopup{};
		EditorUI::EditTextSpec m_CreateParameterName{};
		EditorUI::SelectOptionSpec m_CreateParameterType{};
		EditorUI::GenericPopupSpec m_EditParameterPopup{};
		EditorUI::EditTextSpec m_EditParameterName{};
		EditorUI::SelectOptionSpec m_EditParameterType{};
	};

	struct EditWidgets
	{
		EditorUI::GenericPopupSpec m_MainPopup{};
		EditorUI::EditTextSpec m_EditName{};
		EditorUI::SelectOptionSpec m_SelectSectionLabel{};
		EditorUI::GenericPopupSpec m_DeleteWarning{};
		EditorUI::GenericPopupSpec m_EditWarning{};
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
		EditorUI::ListSpec m_AllScriptsList {};
		EditorUI::TooltipSpec m_ScriptTooltip{};
		// Script List (Create)
		CreateWidgets m_CreateWidgets{};

		// Script List (Edit)
		EditWidgets m_EditWidgets{};

		// Group Label List
		EditorUI::ListSpec m_GroupLabelsTable {};
		EditorUI::EditTextSpec m_CreateGroupLabelPopup {};
		EditorUI::GenericPopupSpec m_EditGroupLabelPopup {};
		EditorUI::EditTextSpec m_EditGroupLabelText {};
	};
}

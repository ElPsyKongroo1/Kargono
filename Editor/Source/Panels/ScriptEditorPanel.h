#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/EditorUI/EditorUI.h"

#include <string>

namespace Kargono::Panels
{
	struct ScriptWidgets
	{
		EditorUI::GenericPopupSpec m_MainPopup{};
		EditorUI::EditTextSpec m_EditName{};
		EditorUI::SelectOptionSpec m_SelectFuncType{};
		EditorUI::SelectOptionSpec m_SelectSectionLabel{};
		EditorUI::SelectOptionSpec m_SelectReturnType{};
		EditorUI::ListSpec m_ParameterList{};
		EditorUI::GenericPopupSpec m_CreateParameterPopup{};
		EditorUI::EditTextSpec m_CreateParameterName{};
		EditorUI::SelectOptionSpec m_CreateParameterType{};
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
		void InitializeScriptPanel();

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
		void OpenCreateScriptDialogFromUsagePoint(WrappedFuncType scriptType, std::function<void(Assets::AssetHandle)> onConfirm, bool openScriptEditor = true);
		void DrawOnCreatePopup();
		

	private:
		//=========================
		// Internal Functionality
		//=========================
		void UpdateScript();
		void OnOpenScriptDialog(EditorUI::ListEntry& entry, std::size_t iteration);
		void OnCreateScriptDialog();

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

		//=========================
		// Widgets
		//=========================
		// Script List
		EditorUI::ListSpec m_AllScriptsList {};
		EditorUI::TooltipSpec m_ScriptTooltip{};
		// Script List (Create)
		ScriptWidgets m_CreateWidgets{};

		// Script List (Edit)
		EditorUI::GenericPopupSpec m_EditScriptPopup {};
		EditorUI::GenericPopupSpec m_DeleteScriptWarning {};
		EditorUI::GenericPopupSpec m_EditScriptFuncTypeWarning {};
		EditorUI::EditTextSpec m_EditScriptName{};
		EditorUI::SelectOptionSpec m_EditScriptFuncType{};
		EditorUI::SelectOptionSpec m_EditScriptSectionLabel{};

		// Group Label List
		EditorUI::ListSpec m_GroupLabelsTable {};
		EditorUI::EditTextSpec m_CreateGroupLabelPopup {};
		EditorUI::GenericPopupSpec m_EditGroupLabelPopup {};
		EditorUI::EditTextSpec m_EditGroupLabelText {};
	};
}

#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/EditorUI/EditorUI.h"

#include <string>

namespace Kargono::Panels
{
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
		void OpenCreateScriptDialogFromUsagePoint(WrappedFuncType scriptType, std::function<void(Assets::AssetHandle)> onConfirm);

	private:
		//=========================
		// Internal Functionality
		//=========================
		void UpdateScript();
		void OnOpenScriptDialog(EditorUI::TableEntry& entry);
		void OnCreateScriptDialog();

	private:
		//=========================
		// Core Panel Data
		//=========================
		FixedString32 m_PanelName{ "Scripts" };
		Assets::AssetHandle m_ActiveScriptHandle {Assets::EmptyHandle};
		std::string m_ActiveLabel {};
		std::function<void(Assets::AssetHandle)> m_OnCreateScriptConfirm{ nullptr };
		WrappedFuncType m_OnCreateFunctionType{ WrappedFuncType::None };

		//=========================
		// Widgets
		//=========================
		// Script Table
		EditorUI::TableSpec m_AllScriptsTable {};
		// Script Table (Create)
		EditorUI::GenericPopupSpec m_CreateScriptPopup {};
		EditorUI::EditTextSpec m_CreateScriptName{};
		EditorUI::SelectOptionSpec m_CreateScriptFuncType{};
		EditorUI::SelectOptionSpec m_CreateScriptSectionLabel{};
		// Script Table (Edit)
		EditorUI::GenericPopupSpec m_EditScriptPopup {};
		EditorUI::GenericPopupSpec m_DeleteScriptWarning {};
		EditorUI::GenericPopupSpec m_EditScriptFuncTypeWarning {};
		EditorUI::EditTextSpec m_EditScriptName{};
		EditorUI::SelectOptionSpec m_EditScriptFuncType{};
		EditorUI::SelectOptionSpec m_EditScriptSectionLabel{};

		// Group Label Table
		EditorUI::TableSpec m_GroupLabelsTable {};
		EditorUI::EditTextSpec m_CreateGroupLabelPopup {};
		EditorUI::GenericPopupSpec m_EditGroupLabelPopup {};
		EditorUI::EditTextSpec m_EditGroupLabelText {};
	};
}

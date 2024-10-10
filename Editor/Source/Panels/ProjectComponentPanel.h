#pragma once
#include "Kargono.h"

namespace Kargono::Panels
{
	class ProjectComponentPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		ProjectComponentPanel();

	private:
		//=========================
		// Internal Initialization Functions
		//=========================
		void InitializeOpeningPanel();
		void InitializeComponentFieldsSection();

	public:
		//=========================
		// On Event Functions
		//=========================
		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);

	private:
		//=========================
		// Internal Functionality
		//=========================
		void OpenComponent();
		void CreateComponent();
		void RefreshData();

	private:
		//=========================
		// Core Panel Data
		//=========================
		std::string m_PanelName{ "Project Component" };
		Ref<ECS::ProjectComponent> m_EditorProjectComponent { nullptr };
		Assets::AssetHandle m_EditorProjectComponentHandle { Assets::EmptyHandle };
		size_t m_ActiveField{ 0 };

	private:
		//=========================
		// Widgets
		//=========================
		// Opening Panel Widgets
		EditorUI::GenericPopupSpec m_CreateComponentPopup {};
		EditorUI::SelectOptionSpec m_OpenComponentPopup {};
		EditorUI::EditTextSpec m_SelectComponentName {};
		// Header
		EditorUI::PanelHeaderSpec m_TagHeader {};
		EditorUI::GenericPopupSpec m_DeleteComponentWarning {};
		EditorUI::GenericPopupSpec m_CloseComponentWarning {};
		// Fields Table
		EditorUI::TableSpec m_FieldsTable {};
		EditorUI::GenericPopupSpec m_AddFieldPopup {};
		EditorUI::EditTextSpec m_AddFieldName {};
		EditorUI::SelectOptionSpec m_AddFieldType {};
		EditorUI::GenericPopupSpec m_EditFieldPopup {};
		EditorUI::EditTextSpec m_EditFieldName {};
		EditorUI::SelectOptionSpec m_EditFieldType {};
	};
}



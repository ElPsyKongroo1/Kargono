#pragma once

#include "Modules/Events/KeyEvent.h"
#include "Modules/Assets/Asset.h"
#include "Modules/EditorUI/EditorUIInclude.h"
#include "Modules/ECS/ProjectComponent.h"

#include <filesystem>

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
		bool OnAssetEvent(Events::Event* event);

	public:
		//=========================
		// External Functionality
		//=========================
		void ResetPanelResources();
		void OpenCreateDialog(std::filesystem::path& createLocation);
		void OpenAssetInEditor(std::filesystem::path& assetLocation);

	private:
		//=========================
		// Internal Functionality
		//=========================
		void OpenComponentDialog();
		void CreateComponentDialog();
		void RefreshData();

		void OnOpenComponent(Assets::AssetHandle newHandle);

	private:
		//=========================
		// Core Panel Data
		//=========================
		FixedString32 m_PanelName{ "Project Component" };
		Ref<ECS::ProjectComponent> m_EditorProjectComponent { nullptr };
		Assets::AssetHandle m_EditorProjectComponentHandle { Assets::EmptyHandle };
		size_t m_ActiveField{ 0 };

	private:
		//=========================
		// Widgets
		//=========================
		// Opening Panel Widgets
		EditorUI::GenericPopupWidget m_CreateComponentPopup {};
		EditorUI::SelectOptionWidget m_OpenComponentPopup {};
		EditorUI::EditTextSpec m_SelectComponentName {};
		EditorUI::ChooseDirectoryWidget m_SelectProjectComponentLocationSpec{};
		// Header
		EditorUI::PanelHeaderWidget m_MainHeader {};
		EditorUI::GenericPopupWidget m_DeleteComponentWarning {};
		EditorUI::GenericPopupWidget m_CloseComponentWarning {};
		// Fields List
		EditorUI::ListWidget m_FieldsTable {};
		EditorUI::GenericPopupWidget m_AddFieldPopup {};
		EditorUI::EditTextSpec m_AddFieldName {};
		EditorUI::SelectOptionWidget m_AddFieldType {};
		EditorUI::GenericPopupWidget m_EditFieldPopup {};
		EditorUI::EditTextSpec m_EditFieldName {};
		EditorUI::SelectOptionWidget m_EditFieldType {};
	};
}



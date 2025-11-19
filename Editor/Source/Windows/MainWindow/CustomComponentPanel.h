#pragma once

#include "Modules/Events/KeyEvent.h"
#include "Modules/Assets/AssetsCommon.h"
#include "Modules/EditorUI/EditorUIInclude.h"
#include "Modules/ECSInternal/CustomComponent.h"

#include <filesystem>

namespace Kargono::Panels
{
	class CustomComponentPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		CustomComponentPanel();
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
		FixedBufStr32 m_PanelName{ "Custom Component" };
		Ref<ECSInternal::CustomComponent> m_EditorCustomComponent { nullptr };
		Assets::AssetHandle m_EditorCustomComponentHandle { Assets::k_EmptyHandle };
		size_t m_ActiveField{ 0 };

	private:
		//=========================
		// Widgets
		//=========================
		// Opening Panel Widgets
		EditorUI::GenericPopupWidget m_CreateComponentPopup {};
		EditorUI::SelectOptionWidget m_OpenComponentPopup {};
		EditorUI::EditTextSpec m_SelectComponentName {};
		EditorUI::ChooseDirectoryWidget m_SelectCustomComponentLocationSpec{};
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



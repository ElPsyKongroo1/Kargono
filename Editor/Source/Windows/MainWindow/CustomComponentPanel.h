#pragma once

#include "Modules/Events/KeyEvent.h"
#include "Modules/Assets/Asset.h"
#include "Modules/EditorUI/EditorUI.h"
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
		Assets::AssetHandle m_EditorCustomComponentHandle { Assets::EmptyHandle };
		size_t m_ActiveField{ 0 };

	private:
		//=========================
		// Widgets
		//=========================
		// Opening Panel Widgets
		EditorUI::GenericPopupSpec m_CreateComponentPopup {};
		EditorUI::SelectOptionSpec m_OpenComponentPopup {};
		EditorUI::EditTextSpec m_SelectComponentName {};
		EditorUI::ChooseDirectorySpec m_SelectCustomComponentLocationSpec{};
		// Header
		EditorUI::PanelHeaderSpec m_MainHeader {};
		EditorUI::GenericPopupSpec m_DeleteComponentWarning {};
		EditorUI::GenericPopupSpec m_CloseComponentWarning {};
		// Fields List
		EditorUI::ListSpec m_FieldsTable {};
		EditorUI::GenericPopupSpec m_AddFieldPopup {};
		EditorUI::EditTextSpec m_AddFieldName {};
		EditorUI::SelectOptionSpec m_AddFieldType {};
		EditorUI::GenericPopupSpec m_EditFieldPopup {};
		EditorUI::EditTextSpec m_EditFieldName {};
		EditorUI::SelectOptionSpec m_EditFieldType {};
	};
}



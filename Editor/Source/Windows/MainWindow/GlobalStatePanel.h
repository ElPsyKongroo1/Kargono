#pragma once
#include "Kargono/Assets/Asset.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/ProjectData/GlobalState.h"
#include "Kargono/EditorUI/EditorUI.h"

#include <string>

namespace Kargono::Panels
{
	class GlobalStatePanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		GlobalStatePanel();
	private:
		//=========================
		// Internal Initialization Functions
		//=========================
		void InitializeOpeningScreen();
		void InitializeDisplayGlobalStateScreen();
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
		void OpenCreateDialog(std::filesystem::path& createLocation);
		void OpenAssetInEditor(std::filesystem::path& assetLocation);
	private:
		//=========================
		// Internal Functionality
		//=========================
		void OnOpenGlobalStateDialog();
		void OnCreateGlobalStateDialog();
		void OnRefreshData();
		void OnOpenGlobalState(Assets::AssetHandle newHandle);

	public:
		//=========================
		// Core Panel Data
		//=========================
		Ref<ProjectData::GlobalState> m_EditorGlobalState{ nullptr };
		Assets::AssetHandle m_EditorGlobalStateHandle{ 0 };
	private:
		FixedString32 m_PanelName{ "Global State Editor" };

		//=========================
		// Widgets
		//=========================
		// Opening menu
		EditorUI::GenericPopupSpec m_CreateGlobalStatePopupSpec{};
		EditorUI::SelectOptionSpec m_OpenGlobalStatePopupSpec{};
		EditorUI::ChooseDirectorySpec m_SelectGlobalStateLocationSpec{};
		// Header
		EditorUI::EditTextSpec m_SelectGlobalStateNameSpec{};
		EditorUI::PanelHeaderSpec m_MainHeader{};
		EditorUI::GenericPopupSpec m_DeleteGlobalStateWarning{};
		EditorUI::GenericPopupSpec m_CloseGlobalStateWarning{};

	};
}

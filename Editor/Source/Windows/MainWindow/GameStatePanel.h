#pragma once
#include "Modules/Assets/Asset.h"
#include "Modules/Events/KeyEvent.h"
#include "Kargono/Scenes/GameState.h"
#include "Modules/EditorUI/EditorUIInclude.h"

#include <string>

namespace Kargono::Panels
{
	class GameStatePanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		GameStatePanel();
	private:
		//=========================
		// Internal Initialization Functions
		//=========================
		void InitializeOpeningScreen();
		void InitializeDisplayGameStateScreen();
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
		void OnOpenGameStateDialog();
		void OnCreateGameStateDialog();
		void OnRefreshData();

		void OnOpenGameState(Assets::AssetHandle newHandle);
	public:
		//=========================
		// Core Panel Data
		//=========================
		Ref<Scenes::GameState> m_EditorGameState {nullptr};
		Assets::AssetHandle m_EditorGameStateHandle {0};
	private:
		FixedString32 m_PanelName{ "Game State Editor" };
		std::string m_CurrentField {};

		//=========================
		// Widgets
		//=========================
		// Opening menu
		EditorUI::GenericPopupWidget m_CreateGameStatePopupSpec {};
		EditorUI::SelectOptionWidget m_OpenGameStatePopupSpec {};
		EditorUI::ChooseDirectoryWidget m_SelectGameStateLocationSpec{};
		// Header
		EditorUI::EditTextSpec m_SelectGameStateNameSpec {};
		EditorUI::PanelHeaderWidget m_MainHeader {};
		EditorUI::GenericPopupWidget m_DeleteGameStateWarning {};
		EditorUI::GenericPopupWidget m_CloseGameStateWarning {};
		// Fields Panel
		EditorUI::ListWidget m_FieldsTable {};
		EditorUI::SelectOptionWidget m_AddFieldPopup {};
		EditorUI::GenericPopupWidget m_EditFieldPopup {};
		EditorUI::EditTextSpec m_EditFieldName {};
		EditorUI::SelectOptionWidget m_EditFieldType {};
		EditorUI::EditVariableWidget m_EditFieldValue {};

	};
}

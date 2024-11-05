#pragma once
#include "Kargono/Assets/Asset.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Scenes/GameState.h"
#include "Kargono/EditorUI/EditorUI.h"

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

		//=========================
		// External API
		//=========================
		void ResetPanelResources();
		void OpenCreateDialog(std::filesystem::path& createLocation);
	private:
		//=========================
		// Internal Functionality
		//=========================
		void OpenGameState();
		void CreateGameState();
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
		EditorUI::GenericPopupSpec m_CreateGameStatePopupSpec {};
		EditorUI::SelectOptionSpec m_OpenGameStatePopupSpec {};
		EditorUI::ChooseDirectorySpec m_SelectGameStateLocationSpec{};
		// Header
		EditorUI::EditTextSpec m_SelectGameStateNameSpec {};
		EditorUI::PanelHeaderSpec m_TagHeader {};
		EditorUI::GenericPopupSpec m_DeleteGameStateWarning {};
		EditorUI::GenericPopupSpec m_CloseGameStateWarning {};
		// Fields Panel
		EditorUI::TableSpec m_FieldsTable {};
		EditorUI::SelectOptionSpec m_AddFieldPopup {};
		EditorUI::GenericPopupSpec m_EditFieldPopup {};
		EditorUI::EditTextSpec m_EditFieldName {};
		EditorUI::SelectOptionSpec m_EditFieldType {};
		EditorUI::EditVariableSpec m_EditFieldValue {};

	};
}

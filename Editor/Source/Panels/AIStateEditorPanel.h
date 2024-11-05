#pragma once
#include "Kargono/Assets/Asset.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/AI/AIService.h"
#include "Kargono/EditorUI/EditorUI.h"

#include <string>


namespace Kargono::Panels
{
	class AIStateEditorPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		AIStateEditorPanel();
	private:
		void InitializeOpeningScreen();
		void InitializeAIStateHeader();
		void InitializeMainPanel();

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
		void OpenCreateAIWindow(std::filesystem::path& createLocation);

	private:
		//=========================
		// Internal Functionality
		//=========================
		void OnOpenAIState();
		void OnCreateAIState();
		void OnRefreshData();
	public:
		//=========================
		// Core Panel Data
		//=========================
		Ref<AI::AIState> m_EditorAIState{ nullptr };
		Assets::AssetHandle m_EditorAIStateHandle { Assets::EmptyHandle };
		FixedString32 m_PanelName{ "AI State Editor" };
	private:
		//=========================
		// Widgets
		//=========================
		// Opening Panel w/ Popups
		EditorUI::SelectOptionSpec m_OpenAIStatePopupSpec {};
		EditorUI::GenericPopupSpec m_CreateAIStatePopupSpec {};
		EditorUI::EditTextSpec m_SelectAIStateNameSpec {};
		EditorUI::ChooseDirectorySpec m_SelectAIStateLocationSpec {};

		// Input Mode Header
		EditorUI::PanelHeaderSpec m_MainHeader {};
		EditorUI::GenericPopupSpec m_DeleteAIStateWarning {};
		EditorUI::GenericPopupSpec m_CloseAIStateWarning {};

		// Edit Scripts Options
		EditorUI::SelectOptionSpec m_SelectOnUpdateScript {};
		EditorUI::SelectOptionSpec m_SelectOnEnterStateScript {};
		EditorUI::SelectOptionSpec m_SelectOnExitStateScript {};
		EditorUI::SelectOptionSpec m_SelectOnAIMessageScript {};

	};
}

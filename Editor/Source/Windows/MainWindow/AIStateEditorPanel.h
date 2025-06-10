#pragma once
#include "Modules/Assets/Asset.h"
#include "Modules/Events/KeyEvent.h"
#include "Modules/AI/AIService.h"
#include "Modules/EditorUI/EditorUIInclude.h"

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
		void OnOpenAIStateDialog();
		void OnCreateAIStateDialog();
		void OnRefreshData();

		void OnOpenAIState(Assets::AssetHandle newHandle);
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
		EditorUI::SelectOptionWidget m_OpenAIStatePopupSpec {};
		EditorUI::GenericPopupWidget m_CreateAIStatePopupSpec {};
		EditorUI::EditTextSpec m_SelectAIStateNameSpec {};
		EditorUI::ChooseDirectoryWidget m_SelectAIStateLocationSpec {};

		// Input Mode Header
		EditorUI::PanelHeaderWidget m_MainHeader {};
		EditorUI::GenericPopupWidget m_DeleteAIStateWarning {};
		EditorUI::GenericPopupWidget m_CloseAIStateWarning {};

		// Edit Scripts Options
		EditorUI::SelectOptionWidget m_SelectOnUpdateScript {};
		EditorUI::SelectOptionWidget m_SelectOnEnterStateScript {};
		EditorUI::SelectOptionWidget m_SelectOnExitStateScript {};
		EditorUI::SelectOptionWidget m_SelectOnAIMessageScript{};
		EditorUI::TooltipWidget m_SelectScriptTooltip{};

	};
}

#pragma once
#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Events/KeyEvent.h"
#include "Modules/AI/StatesContext.h"
#include "Modules/EditorUI/EditorUIInclude.h"

#include <string>


namespace Kargono::Panels
{
	class StateEditorPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		StateEditorPanel();
	private:
		void InitializeOpeningScreen();
		void InitializeStateHeader();
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
		void OnOpenStateDialog();
		void OnCreateStateDialog();
		void OnRefreshData();

		void OnOpenState(Assets::AssetHandle newHandle);
	public:
		//=========================
		// Core Panel Data
		//=========================
		Ref<States::State> m_EditorState{ nullptr };
		Assets::AssetHandle m_EditorStateHandle { Assets::k_EmptyHandle };
		FixedBufStr32 m_PanelName{ "AI State Editor" };
	private:
		//=========================
		// Widgets
		//=========================
		// Opening Panel w/ Popups
		EditorUI::SelectOptionWidget m_OpenStatePopupSpec {};
		EditorUI::GenericPopupWidget m_CreateStatePopupSpec {};
		EditorUI::EditTextSpec m_SelectStateNameSpec {};
		EditorUI::ChooseDirectoryWidget m_SelectStateLocationSpec {};

		// Input Mode Header
		EditorUI::PanelHeaderWidget m_MainHeader {};
		EditorUI::GenericPopupWidget m_DeleteStateWarning {};
		EditorUI::GenericPopupWidget m_CloseStateWarning {};

		// Edit Scripts Options
		EditorUI::SelectOptionWidget m_SelectOnUpdateScript {};
		EditorUI::SelectOptionWidget m_SelectOnEnterStateScript {};
		EditorUI::SelectOptionWidget m_SelectOnExitStateScript {};
		EditorUI::SelectOptionWidget m_SelectOnAIMessageScript{};
		EditorUI::TooltipWidget m_SelectScriptTooltip{};

	};
}

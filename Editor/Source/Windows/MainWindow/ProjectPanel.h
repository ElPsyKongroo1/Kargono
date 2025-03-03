#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/EditorUI/EditorUI.h"

#include <string>

namespace Kargono::Panels
{
	class ProjectPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		ProjectPanel();

	private:
		//=========================
		// Internal Initialization Functions
		//=========================
		void InitializeStaticResources();

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

	private:
		//=========================
		// Internal Functionality
		//=========================
		void OnRefresh();

		//=========================
		// Core Panel Data
		//=========================
		FixedString32 m_PanelName{ "Project Settings" };

	private:
		//=========================
		// Widgets
		//=========================
		EditorUI::PanelHeaderSpec m_ProjectSettingsHeader{};
		EditorUI::SelectOptionSpec m_SelectStartSceneSpec {};
		EditorUI::CheckboxSpec m_DefaultFullscreenSpec {};
		EditorUI::CheckboxSpec m_ToggleNetworkSpec {};
		EditorUI::SelectOptionSpec m_SelectResolutionSpec {};
		EditorUI::SelectOptionSpec m_SelectStartGameStateSpec {};
		EditorUI::SelectOptionSpec m_SelectRuntimeStartSpec {};
		EditorUI::TooltipSpec m_SelectScriptTooltip{};

		// Session init
		EditorUI::SelectOptionSpec m_SelectSessionInitSpec {};
		EditorUI::SelectOptionSpec m_SelectStartSessionSpec {};
		EditorUI::SelectOptionSpec m_SelectSessionReadyCheckSpec {};
		
		// Session users
		EditorUI::SelectOptionSpec m_SelectApproveJoinSessionSpec {};
		EditorUI::SelectOptionSpec m_SelectUpdateSessionSlotSpec {};
		EditorUI::SelectOptionSpec m_SelectUpdateUserCountSpec {};
		EditorUI::SelectOptionSpec m_SelectUserLeftSessionSpec {};

		// Handle signals
		EditorUI::SelectOptionSpec m_SelectReceiveSignalSpec {};

		// Handle server connections
		EditorUI::SelectOptionSpec m_SelectConnectionTerminatedSpec {};

		// Editor AI Message Types
		EditorUI::EditTextSpec m_CreateMessageTypePopup {};
		EditorUI::GenericPopupSpec m_EditMessageTypePopup {};
		EditorUI::EditTextSpec m_EditMessageTypeText {};
	};
}

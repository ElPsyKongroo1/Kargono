#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/EditorUI/EditorUI.h"

#include <string>

namespace Kargono::Panels
{
	class ClientOptions
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		ClientOptions() = default;
		~ClientOptions() = default;

		//=========================
		// Lifecycle Functions
		//=========================
		void InitWidgets(EditorUI::TooltipSpec* parentTooltipSpec);

		//=========================
		// On Event
		//=========================
		void OnEditorUIRender();
		bool OnAssetEvent(Events::Event* event);

	private:
		//=========================
		// Widgets
		//=========================
		// Command Widgets
		EditorUI::CollapsingHeaderSpec m_GeneralCommandsHeader;

		// Status Widgets
		EditorUI::CollapsingHeaderSpec m_GeneralStatusHeader;

		// Scripts Widgets
		EditorUI::CollapsingHeaderSpec m_GeneralScriptsHeader{};
		EditorUI::CollapsingHeaderSpec m_SessionScriptsHeader{};
		EditorUI::CollapsingHeaderSpec m_ConnectionScriptsHeader{};
		EditorUI::CollapsingHeaderSpec m_QueryServerScriptsHeader{};
		EditorUI::SelectOptionSpec m_SelectSessionInitSpec{};
		EditorUI::SelectOptionSpec m_SelectStartSessionSpec{};
		EditorUI::SelectOptionSpec m_SelectSessionReadyCheckSpec{};
		EditorUI::SelectOptionSpec m_SelectApproveJoinSessionSpec{};
		EditorUI::SelectOptionSpec m_SelectUpdateSessionSlotSpec{};
		EditorUI::SelectOptionSpec m_SelectUpdateUserCountSpec{};
		EditorUI::SelectOptionSpec m_SelectUserLeftSessionSpec{};
		EditorUI::SelectOptionSpec m_SelectReceiveSignalSpec{};
		EditorUI::SelectOptionSpec m_SelectConnectionTerminatedSpec{};

		// Parent Panel Widgets
		EditorUI::TooltipSpec* m_ParentTooltip{ nullptr };
	};

	class ServerOptions
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		ServerOptions() = default;
		~ServerOptions() = default;

		//=========================
		// Lifecycle Functions
		//=========================
		void InitWidgets();

		//=========================
		// On Event
		//=========================
		void OnEditorUIRender();
	private:
		//=========================
		// Widgets
		//=========================
		// Command Widgets
		EditorUI::CollapsingHeaderSpec m_GeneralCommandsHeader;
		EditorUI::ButtonBarSpec m_LifecycleOptions;

		// Status Widgets
		EditorUI::CollapsingHeaderSpec m_GeneralStatusHeader;

		// Config Widgets
		EditorUI::CollapsingHeaderSpec m_GeneralConfigHeader;
		EditorUI::EditIVec4Spec m_ServerIP{};
		EditorUI::EditIntegerSpec m_ServerPort{};
		EditorUI::CheckboxSpec m_ServerLocation{};
		EditorUI::EditIVec4Spec m_ServerSecrets{};
	};

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
		EditorUI::SelectOptionSpec m_SelectStartSceneSpec{};
		EditorUI::CheckboxSpec m_DefaultFullscreenSpec{};
		EditorUI::CheckboxSpec m_ToggleNetworkSpec{};
		EditorUI::SelectOptionSpec m_SelectResolutionSpec{};
		EditorUI::SelectOptionSpec m_SelectStartGameStateSpec{};
		EditorUI::SelectOptionSpec m_SelectRuntimeStartSpec{};
		EditorUI::TooltipSpec m_SelectScriptTooltip{};

		//=========================
		// Sub Panels
		//=========================
		ServerOptions m_ServerOptions{};
		ClientOptions m_ClientOptions{};
	};
}

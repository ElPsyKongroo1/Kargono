#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/EditorUI/EditorUI.h"

#include "Kargono/Network/Server.h"

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
		void Init(EditorUI::TooltipSpec* parentTooltipSpec);
	private:
		void InitWidgets();

	public:
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
		EditorUI::CollapsingHeaderSpec m_CommandsHeader;

		// Status Widgets
		EditorUI::CollapsingHeaderSpec m_StatusHeader;

		// Scripts Widgets
		EditorUI::CollapsingHeaderSpec m_AppScriptsHeader{};
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

	constexpr float k_MaxRoundTripTime{ 220.0f };
	
	class ConnectionUI
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		ConnectionUI(Network::ClientIndex index, size_t bufferSize);
		ConnectionUI();
		~ConnectionUI() = default;
	private:
		// Constructor helper(s)
		void InitWidgets();
	public:
		//=========================
		// OnEvent
		//=========================
		void OnEditorUIRender();
	public:
		//=========================
		// On Observer Notification
		//=========================
		void OnNotifySendServerPacket(Network::ClientIndex clientIndex, Network::PacketSequence seq);
		void OnNotifyAckServerPacket(Network::ClientIndex clientIndex, Network::PacketSequence seq, float rtt);

		//=========================
		// Getters/Setters
		//=========================
		Network::ClientIndex GetClientIndex() const
		{
			return m_ClientIndex;
		}
	private:
		//=========================
		// Internal Fields
		//=========================
		// State
		Network::ClientIndex m_ClientIndex{ Network::k_InvalidClientIndex };
		Network::PacketSequence m_LastSequence{ 0 };
		// Widgets
		EditorUI::PlotSpec m_PacketRTTPlot{};
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
		void Init();
	private:
		void InitWidgets();
		void RegisterObservers();

	public:
		//=========================
		// On Event
		//=========================
		void OnEditorUIRender();

		//=========================
		// On Observer Notification
		//=========================
		void OnNotifySendServerPacket(Network::ClientIndex clientIndex, Network::PacketSequence seq);
		void OnNotifyAckServerPacket(Network::ClientIndex clientIndex, Network::PacketSequence seq, float rtt);
		void OnNotifyClientConnect(Network::ClientIndex clientIndex);
		void OnNotifyClientDisconnect(Network::ClientIndex clientIndex);
		void OnNotifyServerInit();
		void OnNotifyServerTerminate();
	private:

		//=========================
		// Internal Fields
		//=========================
		// State fields
		bool m_ActiveState{ false };
		// Command widgets
		EditorUI::CollapsingHeaderSpec m_CommandsHeader;
		EditorUI::ButtonBarSpec m_LifecycleOptions;

		// Status widgets
		EditorUI::CollapsingHeaderSpec m_StatusHeader;

		// Config widgets
		EditorUI::CollapsingHeaderSpec m_GeneralConfigHeader;
		EditorUI::EditIVec4Spec m_ServerIP{};
		EditorUI::EditIntegerSpec m_ServerPort{};
		EditorUI::CheckboxSpec m_ServerLocation{};
		EditorUI::EditIVec4Spec m_ServerSecrets{};

		// Connection widgets
		std::vector<ConnectionUI> m_ConnectionUIs{};
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

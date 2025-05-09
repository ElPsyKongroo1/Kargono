#pragma once
#include "Modules/Events/KeyEvent.h"
#include "Modules/EditorUI/EditorUI.h"

#include "Modules/Network/Server.h"
#include "Modules/Network/Client.h"

#include <string>

namespace Kargono::Panels
{
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
		void OnNotifySendPacket(Network::PacketSequence seq);
		void OnNotifyAckPacket(Network::PacketSequence seq, float rtt);

		//=========================
		// External Fields
		//=========================
		Network::ClientIndex m_ClientIndex{ Network::k_InvalidClientIndex };
		Network::ConnectionStatus m_ConnectionStatus{ Network::ConnectionStatus::Disconnected };
		float m_AverageRTT{ 0.0f };
		bool m_IsCongested{ false };

	private:
		//=========================
		// Internal Fields
		//=========================
		// State
		Network::PacketSequence m_LastSequence{ 0 };
		// Widgets
		EditorUI::PlotSpec m_PacketRTTPlot{};
	};

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
		void RegisterObservers();
	public:
		//=========================
		// On Event
		//=========================
		void OnEditorUIRender();
		bool OnAssetEvent(Events::Event* event);

		//=========================
		// On Observer Notification
		//=========================
		void OnNotifySendClientPacket(Network::ClientIndex index, Network::PacketSequence seq);
		void OnNotifyAckClientPacket(Network::ClientIndex index, Network::PacketSequence seq, float rtt);
		void OnNotifyReliabilityState(Network::ClientIndex index, bool congested, float rtt);
		void OnNotifyConnectStatus(Network::ConnectionStatus status, Network::ClientIndex clientIndex);
		void OnNotifyClientActive(bool active);

	private:
		//=========================
		// Internal Fields
		//=========================
		// State fields
		bool m_ActiveState{ false };

		//=========================
		// Widgets
		//=========================
		// Command Widgets
		EditorUI::CollapsingHeaderSpec m_CommandsHeader;
		EditorUI::ButtonBarSpec m_LifecycleOptions;
		// Status Widgets
		EditorUI::CollapsingHeaderSpec m_StatusHeader;
		EditorUI::CollapsingHeaderSpec m_ConnectionHeader;
		ConnectionUI m_ClientConnectionUI{};

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
		void OnNotifyReliabilityState(Network::ClientIndex index, bool congested, float rtt);
		void OnNotifyClientConnect(Network::ClientIndex clientIndex);
		void OnNotifyClientDisconnect(Network::ClientIndex clientIndex);
		void OnNotifyServerActive(bool active);
	private:

		//=========================
		// Internal Fields
		//=========================
		// State fields
		bool m_ActiveState{ false };

		//=========================
		// Widgets
		//=========================
		// Status section
		EditorUI::CollapsingHeaderSpec m_StatusHeader;

		// Command widgets
		EditorUI::CollapsingHeaderSpec m_CommandsHeader;
		EditorUI::ButtonBarSpec m_LifecycleOptions;

		// Connection widgets
		EditorUI::CollapsingHeaderSpec m_ConnectionsHeader;
		std::vector<ConnectionUI> m_ConnectionUIs{};

		// Config section
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

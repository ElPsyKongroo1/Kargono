#pragma once
#include "Modules/Events/KeyEvent.h"
#include "Modules/EditorUI/EditorUIInclude.h"

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
		EditorUI::PlotWidget m_PacketRTTPlot{};
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
		void Init(EditorUI::TooltipWidget* parentTooltipSpec);
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
		EditorUI::CollapsingHeaderWidget m_CommandsHeader;
		EditorUI::ButtonBarWidget m_LifecycleOptions;
		// Status Widgets
		EditorUI::CollapsingHeaderWidget m_StatusHeader;
		EditorUI::CollapsingHeaderWidget m_ConnectionHeader;
		ConnectionUI m_ClientConnectionUI{};

		// Scripts Widgets
		EditorUI::CollapsingHeaderWidget m_AppScriptsHeader{};
		EditorUI::CollapsingHeaderWidget m_SessionScriptsHeader{};
		EditorUI::CollapsingHeaderWidget m_ConnectionScriptsHeader{};
		EditorUI::CollapsingHeaderWidget m_QueryServerScriptsHeader{};
		EditorUI::SelectOptionWidget m_SelectSessionInitSpec{};
		EditorUI::SelectOptionWidget m_SelectStartSessionSpec{};
		EditorUI::SelectOptionWidget m_SelectSessionReadyCheckSpec{};
		EditorUI::SelectOptionWidget m_SelectApproveJoinSessionSpec{};
		EditorUI::SelectOptionWidget m_SelectUpdateSessionSlotSpec{};
		EditorUI::SelectOptionWidget m_SelectUpdateUserCountSpec{};
		EditorUI::SelectOptionWidget m_SelectUserLeftSessionSpec{};
		EditorUI::SelectOptionWidget m_SelectReceiveSignalSpec{};
		EditorUI::SelectOptionWidget m_SelectConnectionTerminatedSpec{};

		// Parent Panel Widgets
		EditorUI::TooltipWidget* m_ParentTooltip{ nullptr };
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
		EditorUI::CollapsingHeaderWidget m_StatusHeader;

		// Command widgets
		EditorUI::CollapsingHeaderWidget m_CommandsHeader;
		EditorUI::ButtonBarWidget m_LifecycleOptions;

		// Connection widgets
		EditorUI::CollapsingHeaderWidget m_ConnectionsHeader;
		std::vector<ConnectionUI> m_ConnectionUIs{};

		// Config section
		EditorUI::CollapsingHeaderWidget m_GeneralConfigHeader;
		EditorUI::EditIVec4Widget m_ServerIP{};
		EditorUI::EditIntegerWidget m_ServerPort{};
		EditorUI::CheckboxWidget m_ServerLocation{};
		EditorUI::EditIVec4Widget m_ServerSecrets{};

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
		EditorUI::SelectOptionWidget m_SelectStartSceneSpec{};
		EditorUI::CheckboxWidget m_DefaultFullscreenSpec{};
		EditorUI::CheckboxWidget m_ToggleNetworkSpec{};
		EditorUI::SelectOptionWidget m_SelectResolutionSpec{};
		EditorUI::SelectOptionWidget m_SelectStartGameStateSpec{};
		EditorUI::SelectOptionWidget m_SelectRuntimeStartSpec{};
		EditorUI::TooltipWidget m_SelectScriptTooltip{};

		//=========================
		// Sub Panels
		//=========================
		ServerOptions m_ServerOptions{};
		ClientOptions m_ClientOptions{};
	};
}

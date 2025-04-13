#pragma once

#include "Kargono/Network/Session.h"
#include "Kargono/Events/Event.h"
#include "Kargono/Events/NetworkingEvent.h"
#include "Kargono/Core/Base.h"

#include "Kargono/Network/Socket.h"
#include "Kargono/Network/Connection.h"
#include "Kargono/Network/Session.h"
#include "Kargono/Network/ServerConfig.h"

#include "Kargono/Utility/PassiveLoopTimer.h"
#include "Kargono/Utility/LoopTimer.h"
#include "Kargono/Core/Thread.h"
#include "Kargono/Events/EventQueue.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Core/Observable.h"
#include "Kargono/Core/FunctionQueue.h"

#include <unordered_map>

namespace Kargono::Network
{
	class Server
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Server() = default;
		~Server() = default;

		//==============================
		// Lifecycle Functions
		//==============================
		bool InitServer(const ServerConfig& initConfig);
		bool TerminateServer(bool withinNetworkThread = false);

		// Allows other threads to wait on the server to close
		void WaitOnServerTerminate();
	public:
		//==============================
		// Run Threads
		//==============================
		// Run socket/packet handling thread
		void RunNetworkThread();
		void RunNetworkEventThread();

	private:
		// Helper functions
		bool ManageConnections();
		void HandleConsoleInput(Events::KeyPressedEvent event);

	public:
		//==============================
		// On Event
		//==============================
		void OnEvent(Events::Event* event);

		//==============================
		// Manage Events
		//==============================
		void SubmitEvent(Ref<Events::Event> event);

		//==============================
		// Send Packets
		//==============================
		bool SendToConnection(ClientIndex clientIndex, Message& msg);
		bool SendToAllConnections(Message& msg, ClientIndex ignoreClient = k_InvalidClientIndex);

	public:
		//==============================
		// Receive Messages
		//==============================
		// Receive messages from client(s)
		void OpenMessageFromClient(ClientIndex client, Message& incomingMessage);
		// Handle specific message types
		void OpenServerPingMessage(ClientIndex client, Message& msg);
		void OpenMessageAllClientsMessage(ClientIndex client, Message& msg);
		void OpenMessageClientChatMessage(ClientIndex client, Message& msg);
		void OpenRequestClientJoinMessage(ClientIndex client, Message& msg);
		void OpenRequestClientCountMessage(ClientIndex client);
		void OpenNotifyAllLeaveMessage(ClientIndex client);
		void OpenSyncPingMessage(ClientIndex client);
		void OpenStartReadyCheckMessage(ClientIndex client);
		void OpenEnableReadyCheckMessage();
		void OpenSendAllClientsLocationMessage(ClientIndex client, Message& msg);
		void OpenSendAllClientsPhysicsMessage(ClientIndex client, Message& msg);
		void OpenSendAllClientsSignalMessage(ClientIndex client, Message& msg);
		void OpenKeepAliveMessage(ClientIndex client);
		void OpenCheckUDPConnectionMessage(ClientIndex client);

		//==============================
		// Send Messages
		//==============================
		// Send message to client(s)

		// Handle specific message types
		void SendClientLeftMessageToAll(uint16_t removedClientSlot);
		void SendServerPingMessage(ClientIndex client, Message& msg);
		void SendGenericMessageAllClients(ClientIndex sendingClient, Message& msg);
		void SendServerChatMessageAllClients(ClientIndex sendingClient, Message& msg);
		void SendDenyClientJoinMessage(ClientIndex receivingClient);
		void SendApproveClientJoinMessage(ClientIndex receivingClient, uint16_t clientSlot);
		void SendUpdateClientSlotMessage(ClientIndex receivingClient, uint16_t clientSlot);
		void SendReceiveClientCountMessage(ClientIndex receivingClient, uint32_t clientCount);
		void SendReceiveClientCountToAllMessage(ClientIndex receivingClient, uint32_t clientCount);
		void SendClientLeftMessage(ClientIndex receivingClient, uint16_t removedClientSlot);
		void SendSyncPingMessage(ClientIndex receivingClient);
		void SendConfirmReadyCheckMessage(ClientIndex receivingClient, float waitTime);
		void SendUpdateLocationMessage(ClientIndex receivingClient, Message& msg);
		void SendUpdatePhysicsMessage(ClientIndex receivingClient, Message& msg);
		void SendSignalMessage(ClientIndex receivingClient, Message& msg);
		void SendKeepAliveMessage(ClientIndex receivingClient);
		void SendCheckUDPConnectionMessage(ClientIndex receivingClient);
		void SendAcceptConnectionMessage(ClientIndex receivingClient, uint32_t clientCount);
		void SendSessionInitMessage(ClientIndex receivingClient);

		//==============================
		// Manage Session
		//==============================
		void SessionClock();
		void StartSession();

		//==============================
		// Manage Observer(s)
		//==============================
		ObserverIndex AddSendPacketObserver(std::function<void(ClientIndex, PacketSequence)> func);
		bool RemoveSendPacketObserver(ObserverIndex index);

	private:
		//==============================
		// Manage Clients Connections
		//==============================
		void OnClientValidated(ClientIndex client);
		bool OnClientConnect(ClientIndex client);
		void OnClientDisconnect(ClientIndex client);
		void CheckConnectionsValid();


	private:
		//==============================
		// Internal Data
		//==============================
		// Main server network context
		std::atomic<bool> m_ServerActive{ false };
		Socket m_ServerSocket;
		ServerConfig m_Config;
		KGThread m_NetworkThread;
		KGThread m_NetworkEventThread;
		Utility::LoopTimer m_ManageConnectionTimer;
		Utility::PassiveLoopTimer m_KeepAliveTimer;
		ConnectionList m_AllConnections;
		Events::EventQueue m_NetworkEventQueue;
		FunctionQueue m_NetworkFunctionQueue;

		// Session Data
		Session m_OnlySession{};
		Scope<std::thread> m_TimingThread{ nullptr };
		bool m_StopTimingThread = false;
		std::atomic<uint64_t> m_UpdateCount{ 0 };
		bool m_ManageConnections{ false };

		// Observable contexts
		Observable<ClientIndex, PacketSequence> m_SendPacketNotifier{};

	private:
		friend class ServerService;
	};

	class ServerService
	{
	public:
		//==============================
		// LifeCycle Functions
		//==============================
		static bool Init();
		static bool Terminate();
		static bool IsServerActive();

		//==============================
		// Getters/Setters
		//==============================
		static Server& GetActiveServer();

		//==============================
		// Submit Server Events 
		//==============================
		static void SubmitToNetworkEventQueue(Ref<Events::Event> e);
		static void SubmitToNetworkFunctionQueue(const std::function<void()>& func);

	private:
		//==============================
		// Process Events
		//==============================
		// Handle generic events
		static void OnEvent(Events::Event* e);
		// Handle specific events
		static bool OnStartSession(Events::StartSession event);

	private:
		//==============================
		// Internal Fields
		//==============================
		static inline Network::Server s_Server{};
	};
}

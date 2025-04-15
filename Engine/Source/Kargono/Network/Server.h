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
#include "Kargono/Core/Notifier.h"
#include "Kargono/Core/FunctionQueue.h"

namespace Kargono::Network
{
	// Forward declarations
	class ServerEventThread;

	class ServerNotifiers
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ServerNotifiers() = default;
		~ServerNotifiers() = default;
	private:
		void Init(std::atomic<bool>* serverActive);
	public:
		// Server state observers
		ObserverIndex AddServerInitObserver(std::function<void()> func);
		bool RemoveServerInitObserver(ObserverIndex index);
		ObserverIndex AddServerTerminateObserver(std::function<void()> func);
		bool RemoveServerTerminateObserver(ObserverIndex index);

	private:
		//==============================
		// Internal Fields
		//==============================
		Notifier<> m_ServerInitNotifier{};
		Notifier<> m_ServerTerminateNotifier{};

		//==============================
		// Injected Dependencies
		//==============================
		std::atomic<bool>* i_ServerActive{ nullptr };
	private:
		friend class Server;
	};

	class NetworkThreadNotifiers
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		NetworkThreadNotifiers() = default;
		~NetworkThreadNotifiers() = default;
	private:
		void Init(std::atomic<bool>* serverActive);
	public:
		// Client connection observers
		ObserverIndex AddClientConnectObserver(std::function<void(ClientIndex)> func);
		bool RemoveClientConnectObserver(ObserverIndex index);
		ObserverIndex AddClientDisconnectObserver(std::function<void(ClientIndex)> func);
		bool RemoveClientDisconnectObserver(ObserverIndex index);
		// Packet sent observers
		ObserverIndex AddSendPacketObserver(std::function<void(ClientIndex, PacketSequence)> func);
		bool RemoveSendPacketObserver(ObserverIndex index);
		ObserverIndex AddAckPacketObserver(std::function<void(ClientIndex, PacketSequence, float)> func);
		bool RemoveAckPacketObserver(ObserverIndex index);
	private:
		//==============================
		// Internal Fields
		//==============================
		// Client notifiers
		Notifier<ClientIndex> m_ClientConnectNotifier{};
		Notifier<ClientIndex> m_ClientDisconnectNotifier{};
		// Packet notifiers
		Notifier<ClientIndex, PacketSequence> m_SendPacketNotifier{};
		Notifier<ClientIndex, PacketSequence, float> m_AckPacketNotifier{};

		//==============================
		// Injected Dependencies
		//==============================
		std::atomic<bool>* i_ServerActive{ nullptr };
	private:
		friend class ServerNetworkThread;
	};

	class ServerNetworkThread
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ServerNetworkThread() = default;
		~ServerNetworkThread() = default;
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		bool Init
		(
			ServerConfig* serverConfig, 
			Socket* serverSocket, 
			std::atomic<bool>* serverActive,
			ServerEventThread* eventThread
		);
		void Terminate();
		void WaitOnThread();
	public:
		//==============================
		// Job Queue
		//==============================
		void SubmitFunction(const std::function<void()>& workFunction);

	public:
		//==============================
		// Manage Thread
		//==============================
		void ResumeThread(bool withinThread);
		void SuspendThread(bool withinThread);

		//==============================
		// Getters/Setters
		//==============================
		NetworkThreadNotifiers& GetNotifiers()
		{
			return m_Notifiers;
		}
	private:
		//==============================
		// Thread Work Functions
		//==============================
		void RunThread();

		//==============================
		// Manage Session
		//==============================
		void SessionClock();
		void StartSession();

		//==============================
		// Manage Clients Connections
		//==============================
		void OnClientValidated(ClientIndex client);
		bool OnClientConnect(ClientIndex client);
		void OnClientDisconnect(ClientIndex client);
		void CheckConnectionsValid();
		void HandleConnectionKeepAlive();
		void HandleConnectionTimeouts(float deltaTime);

	private:
		//==============================
		// Send Packets
		//==============================
		bool SendToConnection(ClientIndex clientIndex, Message& msg);
		bool SendToAllConnections(Message& msg, ClientIndex ignoreClient = k_InvalidClientIndex);

	private:
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
	private:
		//==============================
		// Internal Fields
		//==============================
		// Thread
		KGThread m_Thread;
		// Thread queues
		FunctionQueue m_FunctionQueue;
		// Notifiers
		NetworkThreadNotifiers m_Notifiers{};
		// Connections
		ConnectionList m_AllConnections{};
		// Timers
		Utility::LoopTimer m_ManageConnectionTimer{};
		uint32_t m_CongestionCounter{ 0 };
		// Sessions
		Session m_OnlySession{};
		Scope<std::thread> m_TimingThread{ nullptr };
		bool m_StopTimingThread{ false };
		std::atomic<uint64_t> m_UpdateCount{ 0 };
		
		//==============================
		// Injected Dependencies
		//==============================
		std::atomic<bool>* i_ServerActive{ nullptr };
		ServerConfig* i_ServerConfig{ nullptr };
		Socket* i_ServerSocket{ nullptr };
		ServerEventThread* i_EventThread{ nullptr };
	};

	class ServerEventThread
	{
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		bool Init(Socket* serverSocket, ServerNetworkThread* networkThread);
		void Terminate();
		void WaitOnThread();

		//==============================
		// Manage Server-Event Config
		//==============================
		void SetSyncPingFreq(size_t frequency /*ms*/);

		//==============================
		// Thread Work Functions
		//==============================
		void RunThread();
	private:
		//==============================
		// Internal Fields
		//==============================
		// Thread context and queues
		KGThread m_Thread{};
		FunctionQueue m_WorkQueue{};
		// OS handles
		HANDLE m_NetworkEventHandle{};
		// Config
		size_t m_ActiveSyncPingFreq{ 1'000 /*1 sec*/};

		//==============================
		// Injected Dependencies
		//==============================
		Socket* i_ServerSocket{ nullptr };
		ServerNetworkThread* i_NetworkThread{ nullptr };
	};

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
		bool Init(const ServerConfig& initConfig);
		bool Terminate(bool withinNetworkThread);
	private:
		// Allows other threads to wait on the server to close
		void WaitOnServerThreads();
	public:
		//==============================
		// Getters/Setters
		//==============================
		ServerNotifiers& GetServerNotifiers()
		{
			return m_Notifiers;
		}

		ServerNetworkThread& GetNetworkThread()
		{
			return m_NetworkThread;
		}

		ServerEventThread& GetEventThread()
		{
			return m_EventThread;
		}

	private:
		//==============================
		// Internal Data
		//==============================
		// Main server network context
		std::atomic<bool> m_ServerActive{ false };
		Socket m_ServerSocket{};
		ServerConfig m_Config{};

		// Server thread contexts
		ServerNetworkThread m_NetworkThread{};
		ServerEventThread m_EventThread{};

		// Server state notifiers
		ServerNotifiers m_Notifiers{};
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

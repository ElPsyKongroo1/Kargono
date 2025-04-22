#pragma once

#include "Kargono/Network/Session.h"
#include "Kargono/Events/Event.h"
#include "Kargono/Events/NetworkingEvent.h"
#include "Kargono/Core/Base.h"

#include "Kargono/Network/Socket.h"
#include "Kargono/Network/Connection.h"
#include "Kargono/Network/Session.h"
#include "Kargono/Network/ServerConfig.h"

#include "Kargono/Utility/Timers.h"
#include "Kargono/Core/Thread.h"
#include "Kargono/Events/EventQueue.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Core/Notifier.h"
#include "Kargono/Core/FunctionQueue.h"

namespace Kargono::Network
{
	// Forward declarations
	class ServerEventThread;
	class Session;

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
		ObserverIndex AddServerActiveObserver(std::function<void(bool)> func);
		bool RemoveServerActiveObserver(ObserverIndex index);

	private:
		//==============================
		// Internal Fields
		//==============================
		Notifier<bool> m_ServerActiveNotifier{};

		//==============================
		// Injected Dependencies
		//==============================
		std::atomic<bool>* i_ServerActive{ nullptr };
	private:
		friend class Server;
	};

	class ServerNetworkNotifiers
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ServerNetworkNotifiers() = default;
		~ServerNetworkNotifiers() = default;
	private:
		void Init(std::atomic<bool>* serverActive);
	public:
		// Client connection observers
		ObserverIndex AddConnectObserver(std::function<void(ClientIndex)> func);
		bool RemoveConnectObserver(ObserverIndex index);
		ObserverIndex AddDisconnectObserver(std::function<void(ClientIndex)> func);
		bool RemoveDisconnectObserver(ObserverIndex index);
	private:
		//==============================
		// Internal Fields
		//==============================
		// Connection notifiers
		Notifier<ClientIndex> m_ConnectNotifier{};
		Notifier<ClientIndex> m_DisconnectNotifier{};

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
		void SubmitEvent(Ref<Events::Event> event);

	private:
		void OnEvent(Events::Event* event);

	public:
		//==============================
		// Manage Thread
		//==============================
		void ResumeThread(bool withinThread);
		void SuspendThread(bool withinThread);

		//==============================
		// Getters/Setters
		//==============================
		ServerNetworkNotifiers& GetNotifiers()
		{
			return m_Notifiers;
		}
		ReliabilityContextNotifiers& GetReliabilityNotifiers()
		{
			return m_ReliabilityNotifiers;
		}

	private:
		//==============================
		// Manage Session
		//==============================
		void StartSession();

		//==============================
		// Thread Work Functions
		//==============================
		void RunThread();

		//==============================
		// Manage Clients Connections
		//==============================
		void OnClientValidated(ClientIndex client);
		bool OnClientConnect(ClientIndex client);
		void OnClientDisconnect(ClientIndex client);
		void HandleConnectionKeepAlive();
		void HandleNewConnectionPacket(MessageType type, Address address);
		void HandleConnectionTimeouts(float deltaTime);

	private:
		//==============================
		// Receive Messages
		//==============================
		// Receive messages from client(s)
		void OpenMessageFromClient(ClientIndex client, Message& incomingMessage);
		// Handle specific message types
		void OpenRequestClientJoinMessage(ClientIndex client, Message& msg);
		void OpenRequestClientCountMessage(ClientIndex client);
		void OpenNotifyAllLeaveMessage(ClientIndex client);
		void OpenStartReadyCheckMessage(ClientIndex client);
		void OpenEnableReadyCheckMessage();
		void OpenSendAllClientsLocationMessage(ClientIndex client, Message& msg);
		void OpenSendAllClientsPhysicsMessage(ClientIndex client, Message& msg);
		void OpenSendAllClientsSignalMessage(ClientIndex client, Message& msg);

		//==============================
		// Send Messages
		//==============================
		// Send message to client(s)
		bool SendToConnection(ClientIndex clientIndex, Message& msg);
		bool SendToAllConnections(Message& msg, ClientIndex ignoreClient = k_InvalidClientIndex);

		// Handle specific message types
		void SendClientLeftMessageToAll(SessionIndex removedClientSlot);
		void SendServerPingMessage(ClientIndex client, Message& msg);
		void SendDenyClientJoinMessage(ClientIndex receivingClient);
		void SendApproveClientJoinMessage(ClientIndex receivingClient, SessionIndex clientSlot);
		void SendUpdateClientSlotMessage(ClientIndex receivingClient, SessionIndex clientSlot);
		void SendReceiveClientCountMessage(ClientIndex receivingClient, size_t clientCount);
		void SendReceiveClientCountToAllMessage(ClientIndex receivingClient, size_t clientCount);
		void SendClientLeftMessage(ClientIndex receivingClient, SessionIndex removedClientSlot);
		void SendConfirmReadyCheckMessage(ClientIndex receivingClient, float waitTime);
		void SendUpdateLocationMessage(ClientIndex receivingClient, Message& msg);
		void SendUpdatePhysicsMessage(ClientIndex receivingClient, Message& msg);
		void SendSignalMessage(ClientIndex receivingClient, Message& msg);
		void SendKeepAliveMessage(ClientIndex receivingClient);
		void SendAcceptConnectionMessage(ClientIndex receivingClient, size_t clientCount);
		void SendSessionInitMessage(ClientIndex receivingClient);
		void SendStartSessionMessage(ClientIndex receivingClient, float waitTime);
	private:
		//==============================
		// Internal Fields
		//==============================
		// Thread
		KGThread m_Thread;
		// Thread queues
		FunctionQueue m_FunctionQueue;
		Events::EventQueue m_EventQueue;
		// Notifiers
		ServerNetworkNotifiers m_Notifiers{};
		ReliabilityContextNotifiers m_ReliabilityNotifiers{};
		// Connections
		ConnectionList m_AllConnections{};
		// Timers
		Utility::LoopTimer m_ManageConnectionTimer{};
		uint32_t m_CongestionCounter{ 0 };
		// Sessions
		Session m_OnlySession{};
		
		//==============================
		// Injected Dependencies
		//==============================
		std::atomic<bool>* i_ServerActive{ nullptr };
		ServerConfig* i_ServerConfig{ nullptr };
		Socket* i_ServerSocket{ nullptr };
		ServerEventThread* i_EventThread{ nullptr };
	private:
		friend class Session;
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
		void WaitOnThreads();
	public:
		//==============================
		// Getters/Setters
		//==============================
		ServerNotifiers& GetNotifiers()
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
		static void SubmitToNetworkFunctionQueue(const std::function<void()>& func);
		static void SubmitToNetworkEventQueue(Ref<Events::Event> event);

	private:
		//==============================
		// Internal Fields
		//==============================
		static inline Network::Server s_Server{};
	};
}

#pragma once

#include "Kargono/Events/NetworkingEvent.h"
#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Core/Base.h"
#include "Kargono/Core/FunctionQueue.h"
#include "Kargono/Network/NetworkCommon.h"
#include "Kargono/Network/Socket.h"
#include "Kargono/Network/ServerConfig.h"
#include "Kargono/Network/Connection.h"
#include "Kargono/Utility/Timers.h"
#include "Kargono/Core/Thread.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Events/EventQueue.h"
#include "Kargono/Core/Notifier.h"

#include <string>
#include <atomic>
#include <thread>
#include <limits>
#include <cstdint>
#include <mutex>
#include <condition_variable>


namespace Kargono::Network
{
	class ClientEventThread;
	class Client;

	enum ConnectionStatus : uint8_t
	{
		Disconnected,
		Connecting,
		Connected
	};

	class ClientNotifiers
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ClientNotifiers() = default;
		~ClientNotifiers() = default;
	private:
		void Init(std::atomic<bool>* clientActive);
	public:
		// Client state observers
		ObserverIndex AddClientActiveObserver(std::function<void(bool)> func);
		bool RemoveClientActiveObserver(ObserverIndex index);

	private:
		//==============================
		// Internal Fields
		//==============================
		Notifier<bool> m_ClientStatusNotifier{};

		//==============================
		// Injected Dependencies
		//==============================
		std::atomic<bool>* i_ClientActive{ nullptr };
	private:
		friend class Client;
	};

	class ClientNetworkNotifiers
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ClientNetworkNotifiers() = default;
		~ClientNetworkNotifiers() = default;
	private:
		void Init(std::atomic<bool>* clientActive);
	public:
		// Client connection observers
		ObserverIndex AddConnectStatusObserver(std::function<void(ConnectionStatus, ClientIndex)> func);
		// Packet sent observers
	private:
		//==============================
		// Internal Fields
		//==============================
		// Connection notifiers
		Notifier<ConnectionStatus, ClientIndex> m_ConnectStatusNotifier{};
		//==============================
		// Injected Dependencies
		//==============================
		std::atomic<bool>* i_ClientActive{ nullptr };
	private:
		friend class ClientNetworkThread;
	};

	class ConnectionToServer
	{
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		void Init(const ServerConfig& config);
		void Terminate();
	public:
		//==============================
		// Public Fields
		//==============================
		Connection m_Connection{};
		ClientIndex m_ClientIndex{};
		ConnectionStatus m_Status{ Disconnected };
	};

	class ClientNetworkThread
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ClientNetworkThread() = default;
		~ClientNetworkThread() = default;
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		bool Init
		(
			ServerConfig* serverConfig,
			Socket* clientSocket,
			std::atomic<bool>* clientActive,
			ClientEventThread* eventThread,
			Client* client
		);
		void Terminate(bool withinNetworkThread);
		void WaitOnThread();

	private:
		//==============================
		// Run Thread
		//==============================
		void RunThread();
		void RequestConnection();

	public:
		//==============================
		// Manage Thread
		//==============================
		void ResumeThread(bool withinThread);
		void SuspendThread(bool withinThread);

	private:
		//==============================
		// Manage Connection
		//==============================
		bool StartConnection(ClientIndex index, bool withinNetworkThread);
		void HandleConnectionKeepAlive();
		bool HandleConnectionTimeout(float deltaTime);

	public:
		//==============================
		// Work Queues
		//==============================
		void SubmitFunction(const std::function<void()> workFunction);
		void SubmitEvent(Ref<Events::Event> event);

	private:
		void OnEvent(Events::Event* event);
	public:
		//==============================
		// Getters/Setters
		//==============================
		ClientNetworkNotifiers& GetNotifiers()
		{
			return m_Notifiers;
		}

		ReliabilityContextNotifiers& GetReliabilityNotifiers()
		{
			return m_ReliabilityNotifiers;
		}

		SessionIndex GetSessionIndex() const
		{
			return m_SessionIndex;
		}

	public:
		//==============================
		// Receive Messages from Server
		//==============================
		// Get messages from the server
		bool OpenManagementPacket(MessageType type, ClientIndex index);
		void OpenMessageFromServer(Message& msg);
		// All specific message type handlers
		void OpenAcceptConnectionMessage(Message& msg);
		void OpenReceiveUserCountMessage(Message& msg);
		void OpenApproveJoinSessionMessage(Message& msg);
		void OpenUpdateSessionUserSlotMessage(Message& msg);
		void OpenUserLeftSessionMessage(Message& msg);
		void OpenDenyJoinSessionMessage(Message& msg);
		void OpenCurrentSessionInitMessage(Message& msg);
		void OpenStartSessionMessage(Message& msg);
		void OpenSessionReadyCheckConfirmMessage(Message& msg);
		void OpenUpdateEntityLocationMessage(Message& msg);
		void OpenUpdateEntityPhysicsMessage(Message& msg);
		void OpenReceiveSignalMessage(Message& msg);

		//==============================
		// Send Messages to Server
		//==============================
		// Send message to the server
		bool SendToServer(Message& msg);

		// All specific message handlers
		void SendRequestUserCountMessage();
		void SendAllEntityLocation(Events::SendAllEntityLocation& event);
		void SendRequestJoinSessionMessage();
		void SendEnableReadyCheckMessage();
		void SendSessionReadyCheckMessage();
		void SendAllClientsSignalMessage(Events::SignalAll& event);
		void SendAllEntityPhysicsMessage(Events::SendAllEntityPhysics& event);
		void SendLeaveCurrentSessionMessage();
		void SendRequestConnectionMessage();
		void SendKeepAliveMessage();

	private:
		//==============================
		// Internal Fields
		//==============================
		// Thread
		KGThread m_Thread;
		// Thread queues
		FunctionQueue m_WorkQueue;
		Events::EventQueue m_EventQueue;
		// Timers
		Utility::LoopTimer m_ManageConnectionTimer{};
		Utility::PassiveLoopTimer m_RequestConnectionTimer{};
		uint32_t m_CongestionCounter{ 0 };
		// Server connection
		ConnectionToServer m_ServerConnection;
		// Session
		UpdateCount m_SessionStartFrame{ 0 };
		std::atomic<SessionIndex> m_SessionIndex{ k_InvalidSessionIndex };
		// Notifiers
		ClientNetworkNotifiers m_Notifiers{};
		ReliabilityContextNotifiers m_ReliabilityNotifiers{};

		//==============================
		// Injected Dependencies
		//==============================
		std::atomic<bool>* i_ClientActive{ nullptr };
		ServerConfig* i_ServerConfig{ nullptr };
		Socket* i_ClientSocket{ nullptr };
		ClientEventThread* i_EventThread{ nullptr };
		Client* i_Client{ nullptr };

	private:
		friend class ClientService;
	};

	class ClientEventThread
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ClientEventThread() = default;
		~ClientEventThread() = default;
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		bool Init(Socket* clientSocket, ClientNetworkThread* networkThread);
		void Terminate(bool withinEventThread);
		void WaitOnThread();

		//==============================
		// Manage Server-Event Config
		//==============================
		void SetSyncPingFreq(size_t frequency /*ms*/);

	private:
		//==============================
		// Run Thread
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
		size_t m_ActiveSyncPingFreq{ 1'000 /*1 sec*/ };

		//==============================
		// Injected Dependencies
		//==============================
		Socket* i_ClientSocket{ nullptr };
		ClientNetworkThread* i_NetworkThread{ nullptr };
	};

	class Client
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Client() = default;
		~Client() = default;

		//==============================
		// Lifecycle Functions
		//==============================
		bool Init(const ServerConfig& initConfig);
		bool Terminate(bool withinNetworkThread);

		// Allows other threads to wait on the client to close
		void WaitOnThreads();

		//==============================
		// Getters/Setters
		//==============================
		ClientNotifiers& GetNotifiers()
		{
			return m_Notifiers;
		}
		ClientNetworkThread& GetNetworkThread()
		{
			return m_NetworkThread;
		}
		ClientEventThread& GetEventThread()
		{
			return m_EventThread;
		}
	private:
		//==============================
		// Internal Data
		//==============================
		std::atomic<bool> m_ClientActive{ false };
		Socket m_ClientSocket{};
		ServerConfig m_Config;
		// Notifier(s)
		ClientNotifiers m_Notifiers{};
		// Threads
		ClientNetworkThread m_NetworkThread{};
		ClientEventThread m_EventThread{};
	private:
		friend class ClientService;
	};
	
	class ClientService
	{
	public:
		//==============================
		// LifeCycle Functions
		//==============================
		static bool Init();
		static bool Terminate();
		static bool IsClientActive();

		//==============================
		// Getters/Setters
		//==============================
		static Client& GetActiveClient()
		{
			return s_Client;
		}

	public:
		//==============================
		// External API
		//==============================
		static uint16_t GetActiveSessionSlot();
		static void SendAllEntityLocation(UUID entityID, Math::vec3 location);
		static void SendAllEntityPhysics(UUID entityID, Math::vec3 translation, Math::vec2 linearVelocity);
		static void EnableReadyCheck();
		static void SessionReadyCheck();
		static void RequestUserCount();
		static void RequestJoinSession();
		static void LeaveCurrentSession();
		static void SignalAll(uint16_t signal);

		//==============================
		// Submit Client Events & Functions
		//==============================
		static void SubmitToNetworkFunctionQueue(const std::function<void()>& function);
		static void SubmitToNetworkEventQueue(Ref<Events::Event> e);
	private:
		//==============================
		// Internal Fields
		//==============================
		static inline Network::Client s_Client{};
	};
	
}


namespace Kargono::Utility
{
	//==============================
	// AssetType <-> String Conversions
	//==============================
	inline const char* ConnectionStatusToString(Network::ConnectionStatus type)
	{
		switch (type)
		{
		case Network::ConnectionStatus::Disconnected: return "Disconnected";
		case Network::ConnectionStatus::Connecting: return "Connecting";
		case Network::ConnectionStatus::Connected: return "Connected";
		}
		KG_ERROR("Unknown connection status type {}", (int)type);
		return "";
	}
}

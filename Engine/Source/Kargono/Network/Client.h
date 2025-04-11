#pragma once

#include "Kargono/Events/NetworkingEvent.h"
#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Core/Base.h"
#include "Kargono/Core/FunctionQueue.h"
#include "Kargono/Network/NetworkCommon.h"
#include "Kargono/Network/Socket.h"
#include "Kargono/Network/ServerConfig.h"
#include "Kargono/Network/Connection.h"
#include "Kargono/Utility/LoopTimer.h"
#include "Kargono/Utility/PassiveLoopTimer.h"
#include "Kargono/Core/Thread.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Events/EventQueue.h"

#include <string>
#include <atomic>
#include <thread>
#include <limits>
#include <cstdint>
#include <mutex>
#include <condition_variable>


namespace Kargono::Network
{
#if 0
	class Client
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Client() = default;
		~Client() = default;
	public:
		//==============================
		// Manage Connection to Server
		//==============================
		bool ConnectToServer(const std::string& serverIP, uint16_t serverPort, bool remote = false);
		void DisconnectFromServer();
		bool IsConnectedToServer();

	public:

	private:
		// Asio Thread and Context. This thread handles asynchronous calls from Asio itself

		// Function and Event Queue for m_NetworkThread to handle
		FunctionQueue m_WorkQueue;
		Events::EventQueue m_EventQueue;

	private:
		friend class ClientService;
	};
#endif

	
	enum ConnectionStatus : uint8_t
	{
		Disconnected,
		Connecting,
		Connected
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
		bool InitClient(const ServerConfig& initConfig);
		bool StartConnection(bool withinNetworkThread = false);
		bool TerminateClient(bool withinNetworkThread = false);

		// Allows other threads to wait on the client to close
		void WaitOnClientTerminate();

		//==============================
		// On Event
		//==============================
		void OnEvent(Events::Event* event);

		//==============================
		// Manage Events
		//==============================
		void SubmitEvent(Ref<Events::Event> event);
	private:
		// Manage the server connection
		void RequestConnection();
	public:
		//==============================
		// Run Threads
		//==============================
		// Run socket/packet handling thread
		void RunNetworkThread();
		void RunNetworkEventThread();

	private:
		// Helper functions
		bool HandleConsoleInput(Events::KeyPressedEvent event);
	public:
		//==============================
		// Send Packets
		//==============================
		bool SendToServer(Message& msg);

		//==============================
		// Receive Messages from Server
		//==============================
		// Get messages from the server
		void OpenMessageFromServer(Message& msg);
		// All specific message type handlers
		void OpenAcceptConnectionMessage(Message& msg);
		void OpenReceiveUserCountMessage(Message& msg);
		void OpenServerChatMessage(Message& msg);
		void OpenApproveJoinSessionMessage(Message& msg);
		void OpenUpdateSessionUserSlotMessage(Message& msg);
		void OpenUserLeftSessionMessage(Message& msg);
		void OpenDenyJoinSessionMessage(Message& msg);
		void OpenCurrentSessionInitMessage(Message& msg);
		void OpenInitSyncPingMessage(Message& msg);
		void OpenStartSessionMessage(Message& msg);
		void OpenSessionReadyCheckConfirmMessage(Message& msg);
		void OpenUpdateEntityLocationMessage(Message& msg);
		void OpenUpdateEntityPhysicsMessage(Message& msg);
		void OpenReceiveSignalMessage(Message& msg);
		void OpenKeepAliveMessage(Message& msg);
		void OpenUDPInitMessage(Message& msg);

		//==============================
		// Send Messages to Server
		//==============================
		// Send message to the server

		// All specific message handlers
		void SendRequestUserCountMessage();
		void SendAllEntityLocation(Events::SendAllEntityLocation& event);
		void SendInitSyncPingMessage();
		void SendRequestJoinSessionMessage();
		void SendEnableReadyCheckMessage();
		void SendSessionReadyCheckMessage();
		void SendAllClientsSignalMessage(Events::SignalAll& event);
		void SendKeepAliveMessage();
		void SendAllEntityPhysicsMessage(Events::SendAllEntityPhysics& event);
		void SendLeaveCurrentSessionMessage();
		void SendCheckUDPConnectionMessage();

	private:
		//==============================
		// Internal Data
		//==============================
		std::atomic<bool> s_ClientActive{ false };
		Socket m_ClientSocket{};
		KGThread m_NetworkThread;
		KGThread m_NetworkEventThread;
		ServerConfig m_Config;
		Utility::LoopTimer m_NetworkThreadTimer;
		Utility::PassiveLoopTimer m_RequestConnectionTimer;
		Utility::PassiveLoopTimer m_KeepAliveTimer;
		Events::EventQueue m_NetworkEventQueue;
		FunctionQueue m_WorkQueue;

		// Server connection
		ConnectionToServer m_ServerConnection;

		// Cached active session information
		uint64_t m_SessionStartFrame{ 0 };
		std::atomic<uint16_t> m_SessionSlot{ std::numeric_limits<uint16_t>::max() };
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
	private:
		static void Run();
		static void EndRun();
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
		// Handle Events as the Network Thread
		//==============================
		// Receive events and pass them along to event handlers
		static void OnEvent(Events::Event* e);
		// Specific event type handlers
		static bool OnRequestUserCount(Events::RequestUserCount event);
		static bool OnStartSession(Events::StartSession event);
		static bool OnConnectionTerminated(Events::ConnectionTerminated event);
		static bool OnRequestJoinSession(Events::RequestJoinSession event);
		static bool OnEnableReadyCheck(Events::EnableReadyCheck event);
		static bool OnSessionReadyCheck(Events::SessionReadyCheck event);
		static bool OnSendAllEntityLocation(Events::SendAllEntityLocation event);
		static bool OnSignalAll(Events::SignalAll event);
		static bool OnSendAllEntityPhysics(Events::SendAllEntityPhysics event);
		static bool OnLeaveCurrentSession(Events::LeaveCurrentSession event);

	private:
		//==============================
		// Internal Functionality
		//==============================
		// Call to process events or functions in queue's
		static void ProcessFunctionQueue();
		static void ProcessEventQueue();

	private:
		//==============================
		// Internal Fields
		//==============================
		static inline Network::Client s_Client{};
	};
	
}



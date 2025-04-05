#pragma once

#include "Kargono/Events/NetworkingEvent.h"
#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Core/Base.h"

#include "Kargono/Network/NetworkCommon.h"

#include <string>
#include <atomic>
#include <thread>
#include <limits>
#include <cstdint>
#include <mutex>
#include <condition_variable>


namespace Kargono::Network
{
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
		//==============================
		// Receive Messages from Server
		//==============================
		// Get messages from the server
		void CheckMessagesFromServer(size_t maxMessages = k_MaxMessageCount);
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
		void SendUDPToServer(Message& msg);
		void SendChatToServer(const std::string& text);
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
		// Asio Thread and Context. This thread handles asynchronous calls from Asio itself

		// Function and Event Queue for m_NetworkThread to handle
		std::vector<std::function<void()>> m_FunctionQueue;
		std::mutex m_FunctionQueueMutex;
		std::vector<Ref<Events::Event>> m_EventQueue {};
		std::mutex m_EventQueueMutex {};

		// Cached active session information
		uint64_t m_SessionStartFrame{ 0 };
		std::atomic<uint16_t> m_SessionSlot{std::numeric_limits<uint16_t>::max()};

	private:
		friend class ClientService;
	};

	class ClientService
	{
	public:
		//==============================
		// LifeCycle Functions
		//==============================
		static void Init();
		static void Terminate();
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
		static bool OnAppTickEvent(Events::AppTickEvent event);
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
		static inline Ref<Network::Client> s_Client{ nullptr };
	};
	
}



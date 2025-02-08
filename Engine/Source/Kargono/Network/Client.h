#pragma once

#include "Kargono/Events/NetworkingEvent.h"
#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Network/UDPConnection.h"
#include "Kargono/Network/TCPConnection.h"
#include "Kargono/Core/Base.h"

#include "API/Network/AsioAPI.h"

#include <string>
#include <atomic>
#include <thread>
#include <limits>
#include <cstdint>
#include <mutex>
#include <condition_variable>


namespace Kargono::Network
{

	class ClientTCPConnection;

	class ClientUDPConnection : public UDPConnection
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ClientUDPConnection(NetworkContext* networkContext, asio::ip::udp::socket&& socket, Ref<ClientTCPConnection> connection)
			: UDPConnection(networkContext, std::move(socket)), m_ActiveTCPConnection(connection) {}
		virtual ~ClientUDPConnection() override = default;
	public:
		//==============================
		// Client Connection Specific Functionality
		//==============================
		virtual void Disconnect(asio::ip::udp::endpoint key) override;
		virtual void AddMessageToIncomingMessageQueue() override;
	private:
		Ref<ClientTCPConnection> m_ActiveTCPConnection{ nullptr };
	};

	//============================================================
	// TCP Client Connection Class
	//============================================================

	class ClientTCPConnection : public TCPConnection
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ClientTCPConnection(NetworkContext* networkContext, asio::ip::tcp::socket&& socket) : TCPConnection(networkContext, std::move(socket)) {}
		virtual ~ClientTCPConnection() override {}

		//==============================
		// LifeCycle Functions
		//==============================
		bool Connect(const asio::ip::tcp::resolver::results_type& endpoints);
		virtual void Disconnect() override;
		bool IsConnected() const 
		{ 
			return m_TCPSocket.is_open(); 
		}

	private:
		//==============================
		// Client Connection Specific Functionality
		//==============================
		virtual void AddMessageToIncomingMessageQueue() override;

		//==============================
		// Client Validation Functions
		//==============================
		void WriteValidationAsync();
		void ReadValidationAsync();

	};

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
		bool ConnectToServer(const std::string& serverIP, const uint16_t serverPort, bool remote = false);
		void DisconnectFromServer();
		bool IsConnected();

		//==============================
		// Receive Messages from Server
		//==============================
		void CheckForMessages(size_t nMaxMessages = -1);
		void OpenMessageFromServer(Kargono::Network::Message& msg);

		//==============================
		// Send Messages to Server
		//==============================
		void SendTCP(const Message& msg);
		void SendUDP(Message& msg);
		void SendChat(const std::string& text);

		//==============================
		// Manage Main Network Thread
		//==============================
		void NetworkThreadSleep();
		void NetworkThreadWakeUp();

	private:
		// Asio Thread and Context. This thread handles asynchronous calls from Asio itself
		NetworkContext m_NetworkContext{};

		// Function and Event Queue for m_NetworkThread to handle
		std::vector<std::function<void()>> m_FunctionQueue;
		std::mutex m_FunctionQueueMutex;
		std::vector<Ref<Events::Event>> m_EventQueue {};
		std::mutex m_EventQueueMutex {};

		// Cached active session information
		uint64_t m_SessionStartFrame{ 0 };
		std::atomic<uint16_t> m_SessionSlot{std::numeric_limits<uint16_t>::max()};

		// TCP and UDP connection objects, which handles reliable and unreliable data transfer
		Ref<ClientTCPConnection> m_ClientTCPConnection { nullptr };
		Ref<ClientUDPConnection> m_ClientUDPConnection { nullptr };
		std::atomic<bool> m_UDPConnectionSuccessful;

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
		static void SubmitToFunctionQueue(const std::function<void()>& function);
		static void SubmitToEventQueue(Ref<Events::Event> e);

	private:
		//==============================
		// Process Events
		//==============================
		static void OnEvent(Events::Event* e);
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
		static void ProcessFunctionQueue();
		static void ProcessEventQueue();

	private:
		//==============================
		// Internal Fields
		//==============================
		static inline Ref<Network::Client> s_Client{ nullptr };
	};
	
}



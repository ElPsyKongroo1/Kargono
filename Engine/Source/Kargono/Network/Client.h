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
#include <atomic>
#include <condition_variable>


namespace Kargono::Network
{

	class ClientTCPConnection;

	class ClientUDPConnection : public std::enable_shared_from_this<ClientUDPConnection>, public UDPConnection
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ClientUDPConnection(asio::io_context& asioContext, asio::ip::udp::socket&& socket, TSQueue<owned_message>& qIn,
			std::condition_variable& newCV, std::mutex& newMutex, Ref<ClientTCPConnection> connection)
			: UDPConnection(asioContext, std::move(socket), qIn, newCV, newMutex), m_ActiveTCPConnection(connection)
		{

		}
		virtual ~ClientUDPConnection() override = default;
	public:

		virtual void Disconnect(asio::ip::udp::endpoint key) override;

		virtual void AddToIncomingMessageQueue() override;
	private:
		Ref<ClientTCPConnection> m_ActiveTCPConnection{ nullptr };
	};

	//============================================================
	// TCP Client Connection Class
	//============================================================

	class ClientTCPConnection : public std::enable_shared_from_this<ClientTCPConnection>, public TCPConnection
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ClientTCPConnection(asio::io_context& asioContext, asio::ip::tcp::socket&& socket,
			TSQueue<owned_message>& qIn, std::condition_variable& newCV,
			std::mutex& newMutex);
		virtual ~ClientTCPConnection() override {}

		//==============================
		// LifeCycle Functions
		//==============================

		bool Connect(const asio::ip::tcp::resolver::results_type& endpoints);

		virtual void Disconnect() override;

		bool IsConnected() const { return m_TCPSocket.is_open(); }

	private:
		//==============================
		// Extra Internal Functionality
		//==============================

		virtual void AddToIncomingMessageQueue() override;

		//==============================
		// ASIO ASYNC Functions
		//==============================
		// ASYNC - Used to write validation packets to server
	private:
		void WriteValidation();
		void ReadValidation();

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
		bool ConnectToServer(const std::string& host, const uint16_t port, bool remote = false);
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
		void Wait();
		void WakeUpNetworkThread();

	private:
		// Asio Thread and Context. This thread handles asynchronous calls from Asio itself
		asio::io_context m_AsioContext;
		std::thread m_AsioThread;

		// Main network thread that continously processes incoming network data from the m_AsioContext 
		//		and outgoing network data from this client's engine code
		Ref<std::thread> m_NetworkThread { nullptr };

		// These atomic variables help manage the network thread
		std::atomic<bool> m_Quit = false;
		std::condition_variable m_BlockThreadCV {};
		std::mutex m_BlockThreadMutex {};

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

		// This is the thread safe queue of incoming messages from the server
		TSQueue<owned_message> m_MessageInQueue;

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
		static Ref<Network::Client> s_Client;
	};
	
}



#pragma once

#include "Kargono/Network/Session.h"
#include "Kargono/Events/Event.h"
#include "Kargono/Events/NetworkingEvent.h"
#include "Kargono/Core/DataStructures.h"
#include "Kargono/Network/Message.h"
#include "Kargono/Network/UDPConnection.h"
#include "Kargono/Network/TCPConnection.h"
#include "Kargono/Core/Base.h"

#include "API/Network/AsioAPI.h"

#include <thread>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <unordered_map>


namespace Kargono::Network
{
	class ServerTCPConnection;

	class ServerUDPConnection : public std::enable_shared_from_this<ServerUDPConnection>, public UDPConnection
	{
	public:
		ServerUDPConnection(asio::io_context& asioContext, asio::ip::udp::socket&& socket, TSQueue<owned_message>& qIn,
			std::condition_variable& newCV, std::mutex& newMutex,
			std::unordered_map<asio::ip::udp::endpoint, Ref<ServerTCPConnection>>& ipMap)
			: UDPConnection(asioContext, std::move(socket), qIn, newCV, newMutex), m_IPAddressToConnection(ipMap)
		{
		}
		virtual ~ServerUDPConnection() override = default;
	public:
		virtual void AddToIncomingMessageQueue() override;

		virtual void Disconnect(asio::ip::udp::endpoint key) override;

	protected:
		std::unordered_map<asio::ip::udp::endpoint, Ref<ServerTCPConnection>>& m_IPAddressToConnection;
	};


	//============================================================
	// TCP Server Connection Class
	//============================================================

	class ServerTCPConnection : public std::enable_shared_from_this<ServerTCPConnection>, public TCPConnection
	{
	public:

		//==============================
		// Constructors/Destructors
		//==============================

		ServerTCPConnection(asio::io_context& asioContext, asio::ip::tcp::socket&& socket,
			TSQueue<owned_message>& qIn, std::condition_variable& newCV,
			std::mutex& newMutex);

		virtual ~ServerTCPConnection() override {}

		//==============================
		// Getter/Setters
		//==============================

		float GetTCPLatency() const { return m_TCPLatency; }
		void SetTCPLatency(float newLatency) { m_TCPLatency = newLatency; }

		uint32_t GetID() const { return m_ID; }

		//==============================
		// LifeCycle Functions
		//==============================

		void Connect(uint32_t uid = 0);

		virtual void Disconnect() override;

		bool IsConnected() const { return m_TCPSocket.is_open(); }

		//==============================
		// Extra Internal Functionality
		//==============================

		virtual void AddToIncomingMessageQueue() override;

		//==============================
		// ASIO ASYNC Functions
		//==============================

	private:
		// ASYNC - Used by both client and server to write validation packet
		void WriteValidation();

		void ReadValidation();

	protected:
		uint32_t m_ID { 0 };

		// Handshake Validation
		uint64_t m_ValidationCache { 0 };

		// Latency Variables
		float m_TCPLatency { 0.0f };

	};

	class Server
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Server(uint16_t nPort, bool isLocal = false);
		~Server() = default;

		//==============================
		// LifeCycle Functions
		//==============================
		bool StartServer();
		void StopServer();

	public:
		//==============================
		// Send Messages
		//==============================
		void SendTCPMessage(Ref<ServerTCPConnection> client, const Message& msg);
		void SendUDPMessage(Ref<ServerTCPConnection> client, Message& msg);
		void SendTCPMessageAll(const Message& msg, Ref<ServerTCPConnection> ignoreClient = nullptr);

		//==============================
		// Send Messages
		//==============================
		void CheckForMessages(size_t nMaxMessages = -1);
		void OpenMessageFromClient(std::shared_ptr<Kargono::Network::ServerTCPConnection> client, Kargono::Network::Message& incomingMessage);

		//==============================
		// Manage Session
		//==============================
		void SessionClock();

		//==============================
		// Manage Server Network Thread
		//==============================
		void NetworkThreadSleep();
		void NetworkThreadWakeUp();

	private:
		//==============================
		// Manage Clients Connections
		//==============================
		void OnClientValidated(Ref<ServerTCPConnection> client) {}
		bool OnClientConnect(std::shared_ptr<Kargono::Network::ServerTCPConnection> client);
		void OnClientDisconnect(std::shared_ptr<Kargono::Network::ServerTCPConnection> client);
		void CheckConnectionsValid();
		void WaitForClientConnection();

	private:
		// Session Data
		Session m_OnlySession{};
		Scope<std::thread> m_TimingThread { nullptr };
		bool m_StopThread = false;
		std::atomic<uint64_t> m_UpdateCount{ 0 };

		// Event queue
		std::vector<Ref<Events::Event>> m_EventQueue {};
		std::mutex m_EventQueueMutex {};

		// Thread safe Queue for incoming message packets
		TSQueue<owned_message> m_IncomingMessageQueue;

		// Storage for all connections and ancillary data
		std::deque<Ref<ServerTCPConnection>> m_AllClientConnections {};
		Ref<ServerUDPConnection> m_UDPServer { nullptr };
		std::unordered_map<asio::ip::udp::endpoint, Ref<ServerTCPConnection>> m_IPAddressToConnection {};
		uint32_t m_ClientIDCounter = 10000;

		// Order of declaration is important = it is also the order of initialization
		asio::io_context m_AsioContext;
		std::thread m_AsioThread;
		asio::ip::tcp::acceptor m_TCPAcceptor;

		// Variables used to manage active server thread. Currently, the active thread is the main thread.
		std::condition_variable m_BlockThreadCV {};
		std::mutex m_BlockThreadMutex {};
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
		static void Terminate();
		static void Run();

		//==============================
		// Submit Server Events 
		//==============================
		static void SubmitToEventQueue(Ref<Events::Event> e);

	private:
		//==============================
		// Process Events
		//==============================
		static void OnEvent(Events::Event* e);
		static bool OnStartSession(Events::StartSession event);

		//==============================
		// Internal Functionality
		//==============================
		static void ProcessEventQueue();

	private:
		//==============================
		// Internal Fields
		//==============================
		static Ref<Network::Server> s_Server;
	};
}

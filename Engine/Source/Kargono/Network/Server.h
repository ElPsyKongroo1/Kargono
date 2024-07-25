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
	class TCPServerConnection;

	class UDPServerConnection : public std::enable_shared_from_this<UDPServerConnection>, public UDPConnection
	{
	public:
		UDPServerConnection(asio::io_context& asioContext, asio::ip::udp::socket&& socket, TSQueue<owned_message>& qIn,
			std::condition_variable& newCV, std::mutex& newMutex,
			std::unordered_map<asio::ip::udp::endpoint, Ref<TCPServerConnection>>& ipMap)
			: UDPConnection(asioContext, std::move(socket), qIn, newCV, newMutex), m_IPAddressToConnection(ipMap)
		{
		}
		virtual ~UDPServerConnection() override = default;
	public:
		virtual void AddToIncomingMessageQueue() override;

		virtual void Disconnect(asio::ip::udp::endpoint key) override;

	protected:
		std::unordered_map<asio::ip::udp::endpoint, Ref<TCPServerConnection>>& m_IPAddressToConnection;
	};


	//============================================================
	// TCP Server Connection Class
	//============================================================

	class TCPServerConnection : public std::enable_shared_from_this<TCPServerConnection>, public TCPConnection
	{
	public:

		//==============================
		// Constructors/Destructors
		//==============================

		TCPServerConnection(asio::io_context& asioContext, asio::ip::tcp::socket&& socket,
			TSQueue<owned_message>& qIn, std::condition_variable& newCV,
			std::mutex& newMutex);

		virtual ~TCPServerConnection() override {}

		//==============================
		// Getter/Setters
		//==============================

		float GetTCPLatency() const { return m_TCPLatency; }
		void SetTCPLatency(float newLatency) { m_TCPLatency = newLatency; }

		uint32_t GetID() const { return id; }

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
		uint32_t id = 0;

		// Handshake Validation
		uint64_t m_nHandshakeCheck = 0;

		// Latency Variables
		float m_TCPLatency{};

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
		bool Start();

		void Stop();

		void Update(size_t nMaxMessages = -1);

		void Wait();

		void WakeUpNetworkThread();

		void CheckConnectionsValid();

	public:
		//==============================
		// Main API (Send Messages)
		//==============================

		// Send a message to a specific client
		void MessageClient(Ref<TCPServerConnection> client, const Message& msg);

		void MessageClientUDP(Ref<TCPServerConnection> client, Message& msg);

		// Send message to all clients
		void MessageAllClients(const Message& msg, Ref<TCPServerConnection> pIgnoreClient = nullptr);

	public:

		bool StartServer();

		void RunServer();

		void StopServer();

		void SessionClock();

		void SubmitToEventQueue(Ref<Events::Event> e);


	private:
		//==============================
		// Internal Functionality
		//==============================

		// Called when a client is validated
		void OnClientValidated(Ref<TCPServerConnection> client) {}

		// Called when a client connects, you can veto the connection by returning false
		bool OnClientConnect(std::shared_ptr<Kargono::Network::TCPServerConnection> client);

		// Called when a client appears to have disconnected
		void OnClientDisconnect(std::shared_ptr<Kargono::Network::TCPServerConnection> client);

		// Called when a message arrives
		void OnMessage(std::shared_ptr<Kargono::Network::TCPServerConnection> client, Kargono::Network::Message& incomingMessage);

		void ProcessEventQueue();
		void OnEvent(Events::Event& e);
		bool OnStartSession(Events::StartSession event);

		// ASYNC - Instruct asio to wait for connection
		void WaitForClientConnection();
	private:
		// TODO: This is very temporary. Only support one session currently!
		Session m_OnlySession{};
		Scope<std::thread> m_TimingThread { nullptr };
		bool m_StopThread = false;
		std::atomic<uint64_t> m_UpdateCount{ 0 };

		std::vector<Ref<Events::Event>> m_EventQueue {};
		std::mutex m_EventQueueMutex {};

		// Thread safe Queue for incoming message packets
		TSQueue<owned_message> m_qMessagesIn;

		std::deque<Ref<TCPServerConnection>> m_Connections;
		Ref<UDPServerConnection> m_UDPServer { nullptr };
		std::unordered_map<asio::ip::udp::endpoint, Ref<TCPServerConnection>> m_IPAddressToConnection{};

		// Order of declaration is important = it is also the order of initialization
		asio::io_context m_Context;
		std::thread m_threadContext;

		// These things need an asio context
		asio::ip::tcp::acceptor m_TCPAcceptor;

		// Clients will be identified in the "wider system" via an ID
		uint32_t nIDCounter = 10000;

		// Variables for sleeping thread until a notify command is received
		std::condition_variable m_BlockThreadCV {};
		std::mutex m_BlockThreadMx {};
	};

	class ServerService
	{
	public:
		//==============================
		// Getters/Setters
		//==============================
		static Ref<Network::Server> GetActiveServer()
		{
			return s_Server;
		}
		static void SetActiveServer(Ref<Network::Server> newServer)
		{
			s_Server = newServer;
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		static Ref<Network::Server> s_Server;
	};
}

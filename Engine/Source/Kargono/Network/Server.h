#pragma once

#include "Kargono/Network/Session.h"
#include "Kargono/Events/Event.h"
#include "Kargono/Events/NetworkingEvent.h"
#include "Kargono/Core/DataStructures.h"
#include "Kargono/Network/Message.h"
#include "Kargono/Network/ConnectionToClient.h"
#include "Kargono/Network/UDPServerConnection.h"
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
		void MessageClient(Ref<ConnectionToClient> client, const Message& msg);

		void MessageClientUDP(Ref<ConnectionToClient> client, Message& msg);

		// Send message to all clients
		void MessageAllClients(const Message& msg, Ref<ConnectionToClient> pIgnoreClient = nullptr);

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
		void OnClientValidated(Ref<ConnectionToClient> client) {}

		// Called when a client connects, you can veto the connection by returning false
		bool OnClientConnect(std::shared_ptr<Kargono::Network::ConnectionToClient> client);

		// Called when a client appears to have disconnected
		void OnClientDisconnect(std::shared_ptr<Kargono::Network::ConnectionToClient> client);

		// Called when a message arrives
		void OnMessage(std::shared_ptr<Kargono::Network::ConnectionToClient> client, Kargono::Network::Message& incomingMessage);

		void ProcessEventQueue();
		void OnEvent(Events::Event& e);
		bool OnStartSession(Events::StartSession event);

		// ASYNC - Instruct asio to wait for connection
		void WaitForClientConnection();
	public:
		static Ref<Network::Server> GetActiveServer() { return s_Server; }
		static void SetActiveServer(Ref<Network::Server> newServer) { s_Server = newServer; }
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

		std::deque<Ref<ConnectionToClient>> m_Connections;
		Ref<UDPServerConnection> m_UDPServer { nullptr };
		std::unordered_map<asio::ip::udp::endpoint, Ref<ConnectionToClient>> m_IPAddressToConnection{};

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


	private:
		static Ref<Network::Server> s_Server;
	};
}

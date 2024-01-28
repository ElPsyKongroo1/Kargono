#pragma once

#include "Kargono/Network/NetworkCommon.h"
#include "Kargono/Network/DataStructures.h"
#include "Kargono/Network/Message.h"
#include "Kargono/Network/ConnectionToClient.h"
#include "Kargono/Network/UDPServer.h"

#include "Kargono/Core/Base.h"

#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <utility>



namespace Kargono::Network
{
	//============================================================
	// Server Interface Class
	//============================================================
	class ServerInterface
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ServerInterface(uint16_t port, bool isLocal);

		virtual ~ServerInterface();

		//==============================
		// LifeCycle Functions
		//==============================
		bool Start();

		void Stop();

		void Update(size_t nMaxMessages = -1);

		void Wait();

		void WakeUpNetworkThread();

		void CheckConnectionsValid();

		//==============================
		// Main API (Send Messages)
		//==============================

		// Send a message to a specific client
		void MessageClient(Ref<ConnectionToClient> client, const Message& msg);

		void MessageClientUDP(Ref<ConnectionToClient> client, Message& msg);

		// Send message to all clients
		void MessageAllClients(const Message& msg, Ref<ConnectionToClient> pIgnoreClient = nullptr);



	protected:

		//==============================
		// Injectable Functionality
		//==============================

		// Called when a client connects, you can veto the connection by returning false
		virtual bool OnClientConnect(Ref<ConnectionToClient> client) { return false; }

		// Called when a client appears to have disconnected
		virtual void OnClientDisconnect(Ref<ConnectionToClient> client) {}

		// Called when a message arrives
		virtual void OnMessage(Ref<ConnectionToClient> client, Message& msg) {}

		// Called when a client is validated
		virtual void OnClientValidated(Ref<ConnectionToClient> client) {}

	private:

		//==============================
		// Internal Functionality
		//==============================

		// ASYNC - Instruct asio to wait for connection
		void WaitForClientConnection();

	protected:
		// Thread safe Queue for incoming message packets
		tsqueue<owned_message> m_qMessagesIn;

		std::deque<Ref<ConnectionToClient>> m_Connections;
		Ref<UDPServer> m_UDPServer { nullptr };
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
	};
}

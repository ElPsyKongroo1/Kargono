#pragma once

#include "Kargono/Network/DataStructures.h"
#include "Kargono/Network/Message.h"
#include "Kargono/Network/ConnectionToServer.h"
#include "Kargono/Network/UDPClient.h"

#include "API/Network/AsioAPI.h"

#include <cstdint>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace Kargono::Network
{
	//============================================================
	// Client Interface Class
	//============================================================
	class ClientInterface
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================

		ClientInterface();

		virtual ~ClientInterface();
	public:

		//==============================
		// LifeCycle Functions
		//==============================

		// Connect to server with hostname/ip-address and port
		bool Connect(const std::string& host, const uint16_t port, bool remote = false);

		// Disconnect from the server
		void Disconnect();

		// Check if client is actually connected to a server
		bool IsConnected();

		void Update(size_t nMaxMessages = -1);

		// Send message to server
		void Send(const Message& msg);

		void SendUDP(Message& msg);

		// Called when a message arrives
		virtual void OnMessage(Message& msg) {}

		void Wait();

		void WakeUpNetworkThread();

		//==============================
		// Getters
		//==============================

		tsqueue<owned_message>& Incoming(){ return m_qMessagesIn; }

	protected:
		// asio context handles the data transfer...
		asio::io_context m_context;
		// ... but needs a thread of its own to execute its word commands
		std::thread thrContext;

		std::atomic<bool> m_UDPConnectionSuccessful;

		// The client has a single instance of a "connection" object, which handles data transfer
		Ref<ConnectionToServer> m_connection;

		Ref<UDPClient> m_UDPClient {nullptr};

	private:
		// This is the thread safe queue of incoming messages from the server
		tsqueue<owned_message> m_qMessagesIn;

		// Variables for sleeping thread until a notify command is received
		std::condition_variable m_BlockThreadCV {};
		std::mutex m_BlockThreadMx {};
	};
}

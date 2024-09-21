#pragma once

#include "Kargono/Network/Session.h"
#include "Kargono/Events/Event.h"
#include "Kargono/Events/NetworkingEvent.h"
#include "Kargono/Network/UDPConnection.h"
#include "Kargono/Network/TCPConnection.h"
#include "Kargono/Core/Base.h"

#include <unordered_map>


namespace Kargono::Network
{
	class ServerTCPConnection;

	class ServerUDPConnection : public UDPConnection
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ServerUDPConnection(NetworkContext* networkContext, asio::ip::udp::socket&& socket,
			std::unordered_map<asio::ip::udp::endpoint, Ref<ServerTCPConnection>>& ipMap)
			: UDPConnection(networkContext, std::move(socket)), m_IPAddressToConnection(ipMap) {}
		virtual ~ServerUDPConnection() override = default;
	public:
		//==============================
		// Server Connection Specific Functionality
		//==============================
		virtual void AddMessageToIncomingMessageQueue() override;
		virtual void Disconnect(asio::ip::udp::endpoint key) override;

	protected:
		std::unordered_map<asio::ip::udp::endpoint, Ref<ServerTCPConnection>>& m_IPAddressToConnection;
	};


	//============================================================
	// TCP Server Connection Class
	//============================================================

	class ServerTCPConnection : public TCPConnection
	{
	public:

		//==============================
		// Constructors/Destructors
		//==============================
		ServerTCPConnection(NetworkContext* networkContext, asio::ip::tcp::socket&& socket);
		virtual ~ServerTCPConnection() override {}

		//==============================
		// LifeCycle Functions
		//==============================
		void Connect(uint32_t uid = 0);
		virtual void Disconnect() override;
		bool IsConnected() const 
		{ 
			return m_TCPSocket.is_open(); 
		}

		//==============================
		// Server Connection Specific Functionality
		//==============================
		virtual void AddMessageToIncomingMessageQueue() override;

		//==============================
		// Getter/Setters
		//==============================
		float GetTCPLatency() const 
		{ 
			return m_TCPLatency; 
		}
		void SetTCPLatency(float newLatency) 
		{ 
			m_TCPLatency = newLatency; 
		}
		uint32_t GetID() const 
		{ 
			return m_ID;
		}

	private:
		//==============================
		// Client Validation Functions
		//==============================
		void WriteValidationAsync();
		void ReadValidationAsync();

	protected:
		// Uniquely identify connection
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
		void SendTCPMessage(ServerTCPConnection* client, const Message& msg);
		void SendUDPMessage(ServerTCPConnection* client, Message& msg);
		void SendTCPMessageAll(const Message& msg, ServerTCPConnection* ignoreClient = nullptr);

		//==============================
		// Send Messages
		//==============================
		void CheckForMessages(size_t nMaxMessages = -1);
		void OpenMessageFromClient(ServerTCPConnection* client, Kargono::Network::Message& incomingMessage);

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
		bool OnClientConnect(Ref<Kargono::Network::ServerTCPConnection> client);
		void OnClientDisconnect(Ref<Kargono::Network::ServerTCPConnection> client);
		void CheckConnectionsValid();
		void WaitForClientConnectionAsync();

	private:
		// Main server network context
		NetworkContext m_NetworkContext{};

		// Session Data
		Session m_OnlySession{};
		Scope<std::thread> m_TimingThread { nullptr };
		bool m_StopTimingThread = false;
		std::atomic<uint64_t> m_UpdateCount{ 0 };

		// Event queue
		std::vector<Ref<Events::Event>> m_EventQueue {};
		std::mutex m_EventQueueMutex {};

		// Storage for all connections and ancillary data
		std::deque<Ref<ServerTCPConnection>> m_AllClientConnections {};
		Ref<ServerUDPConnection> m_UDPServer { nullptr };
		std::unordered_map<asio::ip::udp::endpoint, Ref<ServerTCPConnection>> m_IPAddressToConnection {};
		uint32_t m_ClientIDCounter = 10000;
		asio::ip::tcp::acceptor m_TCPAcceptor;
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

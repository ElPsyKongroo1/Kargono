#pragma once

#include "Kargono/Network/Session.h"
#include "Kargono/Events/Event.h"
#include "Kargono/Events/NetworkingEvent.h"
#include "Kargono/Core/Base.h"

#include <unordered_map>

namespace Kargono::Network
{
	class Server
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Server(uint16_t nPort);
		~Server() = default;

		//==============================
		// LifeCycle Functions
		//==============================
		bool StartServer(bool isLocal);
		void StopServer();
	public:
		//==============================
		// Receive Messages
		//==============================
		// Receive messages from client(s)
		void CheckForMessages(size_t maxMessages = k_MaxMessageCount);
		void OpenMessageFromClient(ServerTCPConnection* client, Message& incomingMessage);
		// Handle specific message types
		void OpenServerPingMessage(ServerTCPConnection* client, Message& msg);
		void OpenMessageAllClientsMessage(ServerTCPConnection* client, Message& msg);
		void OpenMessageClientChatMessage(ServerTCPConnection* client, Message& msg);
		void OpenRequestClientJoinMessage(ServerTCPConnection* client, Message& msg);
		void OpenRequestClientCountMessage(ServerTCPConnection* client);
		void OpenNotifyAllLeaveMessage(ServerTCPConnection* client);
		void OpenSyncPingMessage(ServerTCPConnection* client);
		void OpenStartReadyCheckMessage(ServerTCPConnection* client);
		void OpenEnableReadyCheckMessage();
		void OpenSendAllClientsLocationMessage(ServerTCPConnection* client, Message& msg);
		void OpenSendAllClientsPhysicsMessage(ServerTCPConnection* client, Message& msg);
		void OpenSendAllClientsSignalMessage(ServerTCPConnection* client, Message& msg);
		void OpenKeepAliveMessage(ServerTCPConnection* client);
		void OpenCheckUDPConnectionMessage(ServerTCPConnection* client);

		//==============================
		// Send Messages
		//==============================
		// Send message to client(s)
		void SendUDPMessage(ServerTCPConnection* client, Message& msg);
		void SendUDPMessageAll(const Message& msg, ServerTCPConnection* ignoreClient = nullptr);
		// Handle specific message types
		void SendClientLeftMessageToAll(uint16_t removedClientSlot);
		void SendServerPingMessage(ServerTCPConnection* client, Message& msg);
		void SendGenericMessageAllClients(ServerTCPConnection* sendingClient, Message& msg);
		void SendServerChatMessageAllClients(ServerTCPConnection* sendingClient, Message& msg);
		void SendDenyClientJoinMessage(ServerTCPConnection* receivingClient);
		void SendApproveClientJoinMessage(ServerTCPConnection* receivingClient, uint16_t clientSlot);
		void SendUpdateClientSlotMessage(ServerTCPConnection* receivingClient, uint16_t clientSlot);
		void SendReceiveClientCountMessage(ServerTCPConnection* receivingClient, uint32_t clientCount);
		void SendReceiveClientCountToAllMessage(ServerTCPConnection* receivingClient, uint32_t clientCount);
		void SendClientLeftMessage(ServerTCPConnection* receivingClient, uint16_t removedClientSlot);
		void SendSyncPingMessage(ServerTCPConnection* receivingClient);
		void SendConfirmReadyCheckMessage(ServerTCPConnection* receivingClient, float waitTime);
		void SendUpdateLocationMessage(ServerTCPConnection* receivingClient, Message& msg);
		void SendUpdatePhysicsMessage(ServerTCPConnection* receivingClient, Message& msg);
		void SendSignalMessage(ServerTCPConnection* receivingClient, Message& msg);
		void SendKeepAliveMessage(ServerTCPConnection* receivingClient);
		void SendCheckUDPConnectionMessage(ServerTCPConnection* receivingClient);
		void SendAcceptConnectionMessage(ServerTCPConnection* receivingClient, uint32_t clientCount);
		void SendSessionInitMessage(ServerTCPConnection* receivingClient);

		//==============================
		// Manage Session
		//==============================
		void SessionClock();
		void StartSession();

	private:
		//==============================
		// Manage Clients Connections
		//==============================
		void OnClientValidated(Ref<ServerTCPConnection> client);
		bool OnClientConnect(Ref<Kargono::Network::ServerTCPConnection> client);
		void OnClientDisconnect(Ref<Kargono::Network::ServerTCPConnection> client);
		void CheckConnectionsValid();

	private:
		// Main server network context

		// Session Data
		Session m_OnlySession{};
		Scope<std::thread> m_TimingThread { nullptr };
		bool m_StopTimingThread = false;
		std::atomic<uint64_t> m_UpdateCount{ 0 };

		// Event queue
		std::vector<Ref<Events::Event>> m_EventQueue {};
		std::mutex m_EventQueueMutex {};
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
		// Getters/Setters
		//==============================
		static Ref<Server> GetActiveServer();

		//==============================
		// Submit Server Events 
		//==============================
		static void SubmitToNetworkEventQueue(Ref<Events::Event> e);

	private:
		//==============================
		// Process Events
		//==============================
		// Handle generic events
		static void OnEvent(Events::Event* e);
		// Handle specific events
		static bool OnStartSession(Events::StartSession event);

		//==============================
		// Internal Functionality
		//==============================
		static void ProcessEventQueue();

	private:
		//==============================
		// Internal Fields
		//==============================
		static inline Ref<Network::Server> s_Server{ nullptr };
	};
}

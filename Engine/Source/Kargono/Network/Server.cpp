#include "kgpch.h"

#include "Kargono/Network/Server.h"

#include "Kargono/Projects/Project.h"
#include "Kargono/Core/Engine.h"

namespace Kargono::Network
{
	Server::Server(uint16_t nPort)
	{
		// TODO: Store the port

		// TODO: Initialize the server context maybe?
	}

	bool Server::StartServer(bool isLocal)
	{

		// TODO: Start the network-event thread so it can wait for the above message types
		
		// The network thread should be running

		KG_INFO("[SERVER] Started!");
		return true;
	}

	void Server::CheckForMessages(size_t maxMessages)
	{
		// TODO: Clean up dead connections

		// TODO: Allow the server main thread to sleep when no work is provided

		// TODO: Handle valid messages
		// TODO: Handle all messages until message queue is empty or maxMessages limit is reached
	}
	void Server::OpenMessageFromClient(ServerTCPConnection* client, Kargono::Network::Message& incomingMessage)
	{
		// Handle messages based on their type
		switch (incomingMessage.m_Header.m_MessageType)
		{
		case MessageType::ManageConnection_ServerPing:
			OpenServerPingMessage(client, incomingMessage);
			break;
		case MessageType::GenericMessage_MessageAllClients:
			OpenMessageAllClientsMessage(client, incomingMessage);
			break;
		case MessageType::GenericMessage_ClientChat:
			OpenMessageClientChatMessage(client, incomingMessage);
			break;
		case MessageType::ManageSession_RequestClientJoin:
			OpenRequestClientJoinMessage(client, incomingMessage);
			break;
		case MessageType::ServerQuery_RequestClientCount:
			OpenRequestClientCountMessage(client);
			break;
		case MessageType::ManageSession_NotifyAllLeave:
			OpenNotifyAllLeaveMessage(client);
			break;
		case MessageType::ManageSession_SyncPing:
			OpenSyncPingMessage(client);
			break;
		case MessageType::ManageSession_StartReadyCheck:
			OpenStartReadyCheckMessage(client);
			break;
		case MessageType::ManageSession_EnableReadyCheck:
			OpenEnableReadyCheckMessage();
			break;
		case MessageType::ManageSceneEntity_SendAllClientsLocation:
			OpenSendAllClientsLocationMessage(client, incomingMessage);
			break;
		case MessageType::ManageSceneEntity_SendAllClientsPhysics:
			OpenSendAllClientsPhysicsMessage(client, incomingMessage);
			break;
		case MessageType::ScriptMessaging_SendAllClientsSignal:
			OpenSendAllClientsSignalMessage(client, incomingMessage);
			break;
		case MessageType::ManageConnection_KeepAlive:
			OpenKeepAliveMessage(client);
			break;
		case MessageType::ManageConnection_CheckUDPConnection:
			OpenCheckUDPConnectionMessage(client);
			break;
		default:
			KG_ERROR("Invalid message type sent to server");
			break;
		}
	}
	void Server::OpenServerPingMessage(ServerTCPConnection* client, Kargono::Network::Message& msg)
	{
		SendServerPingMessage(client, msg);
	}
	void Server::OpenMessageAllClientsMessage(ServerTCPConnection* client, Kargono::Network::Message& msg)
	{
		KG_INFO("[{}]: Message All", client->GetID());
		SendGenericMessageAllClients(client, msg);
	}
	void Server::OpenMessageClientChatMessage(ServerTCPConnection* client, Kargono::Network::Message& msg)
	{
		KG_INFO("[{}]: Sent Chat", client->GetID());
		SendServerChatMessageAllClients(client, msg);
	}
	void Server::OpenRequestClientJoinMessage(ServerTCPConnection* client, Kargono::Network::Message& msg)
	{
		// Deny client join if session slots are full
		if (m_OnlySession.GetClientCount() >= k_MaxSessionClients)
		{
			SendDenyClientJoinMessage(client);
			return;
		}

		// Add client to session and ensure slot is valid
		uint16_t clientSlot = m_OnlySession.AddClient(client);
		if (clientSlot == k_InvalidSessionSlot)
		{
			SendDenyClientJoinMessage(client);
			return;
		}

		// Send approval message to the new client
		SendApproveClientJoinMessage(client, clientSlot);

		// Notify all other session clients that new client has been added
		for (auto [clientID, connection] : m_OnlySession.GetAllClients())
		{
			// Skip the current client (it already knows from approval)
			if (clientID == client->GetID()) 
			{ 
				continue;
			}
			
			// Send update message
			SendUpdateClientSlotMessage(connection, clientSlot);
		}

		// Updated new client with all other client data
		for (auto [slot, clientID] : m_OnlySession.GetAllSlots())
		{
			if (clientID == client->GetID()) 
			{ 
				continue; 
			}

			// Send update message
			SendUpdateClientSlotMessage(client, slot);
		}

		// If enough clients are connected, start the session
		if (m_OnlySession.GetClientCount() == k_MaxSessionClients)
		{
			// TODO: Probably should expose this to the scripts instead of automatically starting the session
			m_OnlySession.InitSession();
		}
	}
	void Server::OpenRequestClientCountMessage(ServerTCPConnection* client)
	{
#if 0
		KG_INFO("[{}]: User Count Request", client->GetID());

		// Send return message to client
		SendReceiveClientCountMessage(client, (uint32_t)m_AllClientConnections.size());
#endif
	}
	void Server::OpenNotifyAllLeaveMessage(ServerTCPConnection* client)
	{
		KG_INFO("[{}]: User Leaving Session", client->GetID());

		// Remove the client from the session
		uint16_t removedClient = m_OnlySession.RemoveClient(client->GetID());

		// Ensure the client removal was successful
		if (removedClient == k_InvalidSessionSlot)
		{
			KG_WARN("Failed to remove client from the active session");
			return;
		}

		// Notify all users in the same session that a client left
		for (auto [clientID, connection] : m_OnlySession.GetAllClients())
		{
			SendClientLeftMessage(connection, removedClient);
		}

		// Notify the removed client it is removed 
		// (note this is necessary since the client no longer exists in the session list)
		SendClientLeftMessage(client, removedClient);
	}
	void Server::OpenSyncPingMessage(ServerTCPConnection* client)
	{
		m_OnlySession.ReceiveSyncPing(client->GetID());
	}
	void Server::OpenStartReadyCheckMessage(ServerTCPConnection* client)
	{
		m_OnlySession.StoreClientReadyCheck(client->GetID());
	}
	void Server::OpenEnableReadyCheckMessage()
	{
		m_OnlySession.EnableReadyCheck();
	}
	void Server::OpenSendAllClientsLocationMessage(ServerTCPConnection* client, Message& msg)
	{
		// Forward entity location to all other clients
		for (auto [clientID, connection] : m_OnlySession.GetAllClients())
		{
			// Do not forward the message back to the original client
			if (clientID == client->GetID()) 
			{
				continue; 
			}

			SendUpdateLocationMessage(connection, msg);
		}
	}
	void Server::OpenSendAllClientsPhysicsMessage(ServerTCPConnection* client, Message& msg)
	{
		
		// Forward entity Physics to all other clients
		for (auto [clientID, connection] : m_OnlySession.GetAllClients())
		{
			// Do not forward the message back to the original client
			if (clientID == client->GetID()) 
			{ 
				continue;
			}

			SendUpdatePhysicsMessage(connection, msg);
		}
	}
	void Server::OpenSendAllClientsSignalMessage(ServerTCPConnection* client, Message& msg)
	{
		// Forward signal to all other session clients
		for (auto [clientID, connection] : m_OnlySession.GetAllClients())
		{
			// Do not forward the message back to the original client
			if (clientID == client->GetID()) 
			{ 
				continue; 
			}

			SendSignalMessage(connection, msg);
		}
	}
	void Server::OpenKeepAliveMessage(ServerTCPConnection* client)
	{
		SendKeepAliveMessage(client);
	}
	void Server::OpenCheckUDPConnectionMessage(ServerTCPConnection* client)
	{
		SendCheckUDPConnectionMessage(client);
	}
	void Server::CheckConnectionsValid()
	{
		// TODO: Remove client connection if set-to-be-disconnected
		// TODO: Remove client-connection-ref from data structure
#if 0
		bool invalidClientExists = false;
		for (Ref<ServerTCPConnection> client : m_AllClientConnections)
		{
			// Check client is connected...
			if (!(client && client->IsConnected()))
			{
				// The client couldn't be contacted, so assume it has disconnected.
				// Disconnect the client's TCP/UDP connection
				OnClientDisconnect(client);

				// Revalidate the IP -> Connection map
				m_IPAddressToConnection.erase(client->GetUDPRemoteReceiveEndpoint());
				client.reset();

				// Indicate clients objects need to be cleaned up after this loop
				invalidClientExists = true;
			}
		}
		if (invalidClientExists)
		{
			// Remove all clients that have been determined disconnected (and set to nullptr)
			m_AllClientConnections.erase(
				std::remove(m_AllClientConnections.begin(), m_AllClientConnections.end(), nullptr), m_AllClientConnections.end());
		}
#endif
	}

	void Server::SendUDPMessage(ServerTCPConnection* client, Message& msg)
	{
		// TODO: Identify/validate the client and send the message
	}



	void Server::SendUDPMessageAll(const Message& msg, ServerTCPConnection* ignoreClient)
	{
		// TODO: Send UDP message to all clients other than the ignored client
	}
	void Server::SendClientLeftMessageToAll(uint16_t removedClientSlot)
	{
		Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::ManageSession_ClientLeft;
		newMessage << removedClientSlot;

		// Notify all users in the same session that a client left
		for (auto& [clientID, connection] : m_OnlySession.GetAllClients())
		{
			connection->SendUDPMessage(newMessage);
		}
	}
	void Server::SendServerPingMessage(ServerTCPConnection* client, Kargono::Network::Message& msg)
	{
		// Return the message back to the client to calculate the client's ping
		KG_INFO("[{}]: Server Ping", client->GetID());
		client->SendUDPMessage(msg);
	}
	void Server::SendGenericMessageAllClients(ServerTCPConnection* sendingClient, Kargono::Network::Message& msg)
	{
		// Forward the message from the sending client to all other clients
		Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::GenericMessage_ServerMessage;
		newMessage << sendingClient->GetID();

		// Send message reliably to all other clients TCP
		SendUDPMessageAll(newMessage, sendingClient);
	}
	void Server::SendServerChatMessageAllClients(ServerTCPConnection* sendingClient, Kargono::Network::Message& msg)
	{
		// Forward to chat message to all other clients
		msg.m_Header.m_MessageType = MessageType::GenericMessage_ServerChat;
		msg << sendingClient->GetID();

		// Send message reliably to all other clients TCP
		SendUDPMessageAll(msg, sendingClient);
	}
	void Server::SendDenyClientJoinMessage(ServerTCPConnection* receivingClient)
	{
		// Create simple message notifying client of denial
		Message denyMessage;
		denyMessage.m_Header.m_MessageType = MessageType::ManageSession_DenyClientJoin;

		// Send message reliably with TCP
		receivingClient->SendUDPMessage(denyMessage);
	}
	void Server::SendApproveClientJoinMessage(ServerTCPConnection* receivingClient, uint16_t clientSlot)
	{
		// Create simple message nofiying the client that their join-session request was approved
		Message approveMessage;
		approveMessage.m_Header.m_MessageType = MessageType::ManageSession_ApproveClientJoin;
		approveMessage << clientSlot;

		// Send message reliably with TCP
		receivingClient->SendUDPMessage(approveMessage);
	}
	void Server::SendUpdateClientSlotMessage(ServerTCPConnection* receivingClient, uint16_t clientSlot)
	{
		// Create simple message notifying the client that a session-client slot has been changed
		Message updateSlotMessage;
		updateSlotMessage.m_Header.m_MessageType = MessageType::ManageSession_UpdateClientSlot;
		updateSlotMessage << clientSlot;

		// Send message reliably with TCP
		receivingClient->SendUDPMessage(updateSlotMessage);
	}
	void Server::SendReceiveClientCountMessage(ServerTCPConnection* receivingClient, uint32_t clientCount)
	{
		// Create simple return message to the client indicating the total count of clients on the server
		Message clientCountMessage;
		clientCountMessage.m_Header.m_MessageType = MessageType::ServerQuery_ReceiveClientCount;
		clientCountMessage << clientCount;

		// Send message reliably with TCP
		receivingClient->SendUDPMessage(clientCountMessage);
	}
	void Server::SendReceiveClientCountToAllMessage(ServerTCPConnection* ignoredClient, uint32_t clientCount)
	{
		// Send simple message to update the client count for all clients (except the ignored one)
		Message clientCountMessage;
		clientCountMessage.m_Header.m_MessageType = MessageType::ServerQuery_ReceiveClientCount;
		clientCountMessage << clientCount;

		// Send message reliably with TCP to *all clients
		SendUDPMessageAll(clientCountMessage, ignoredClient);
	}
	void Server::SendClientLeftMessage(ServerTCPConnection* receivingClient, uint16_t removedClientSlot)
	{
		// Create message notifying the indicated client that a client at the provided slot left the session
		Message clientLeftMessage;
		clientLeftMessage.m_Header.m_MessageType = MessageType::ManageSession_ClientLeft;
		clientLeftMessage << removedClientSlot;

		// Send message reliably with TCP
		receivingClient->SendUDPMessage(clientLeftMessage);
	}
	void Server::SendSyncPingMessage(ServerTCPConnection* receivingClient)
	{
		// Create message that sends a ping to synchronize clients inside a session
		Kargono::Network::Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::ManageSession_SyncPing;

		// Send message reliably with TCP
		receivingClient->SendUDPMessage(newMessage);
	}
	void Server::SendConfirmReadyCheckMessage(ServerTCPConnection* receivingClient, float waitTime)
	{
		// Create message that sends a notifies the client that the client that the ready check is done.
		// The client should wait the specified time and then handle the notification
		Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::ManageSession_ConfirmReadyCheck;
		newMessage << waitTime;

		// Send message reliably with TCP
		receivingClient->SendUDPMessage(newMessage);
	}
	void Server::SendUpdateLocationMessage(ServerTCPConnection* receivingClient, Message& msg)
	{
		// (Assuming the provided message already contains the location)

		// Forward the client's location to all other clients
		msg.m_Header.m_MessageType = MessageType::ManageSceneEntity_UpdateLocation;

		// Send message quickly using UDP
		SendUDPMessage(receivingClient, msg);
	}
	void Server::SendUpdatePhysicsMessage(ServerTCPConnection* receivingClient, Message& msg)
	{
		// (Assuming the provided message already contains the physics data)

		// Forward the client's physics data to all other clients
		msg.m_Header.m_MessageType = MessageType::ManageSceneEntity_UpdatePhysics;

		// Send message quickly using UDP
		SendUDPMessage(receivingClient, msg);
	}
	void Server::SendSignalMessage(ServerTCPConnection* receivingClient, Message& msg)
	{
		// (Assuming the provided message already contains the signal data)

		// Forward the signal to all other clients
		msg.m_Header.m_MessageType = MessageType::ScriptMessaging_ReceiveSignal;

		// Send message reliably using TCP
		SendUDPMessage(receivingClient, msg);
	}
	void Server::SendKeepAliveMessage(ServerTCPConnection* receivingClient)
	{
		// Return a keep alive message to... well.. keep the connection alive 
		Message keepAliveMessage;
		keepAliveMessage.m_Header.m_MessageType = MessageType::ManageConnection_KeepAlive;

		// Send message quickly using UDP
		SendUDPMessage(receivingClient, keepAliveMessage);
	}
	void Server::SendCheckUDPConnectionMessage(ServerTCPConnection* receivingClient)
	{
		// Return a check UDP message to the initial client to verify connection integrity
		Message checkConnection;
		checkConnection.m_Header.m_MessageType = MessageType::ManageConnection_CheckUDPConnection;

		// Send message quickly using UDP
		SendUDPMessage(receivingClient, checkConnection);
	}
	void Server::SendAcceptConnectionMessage(ServerTCPConnection* receivingClient, uint32_t clientCount)
	{
		// Return a message to the client indicating the connection was successful
		Kargono::Network::Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageConnection_AcceptConnection;
		msg << clientCount;

		// Send message reliably using TCP
		receivingClient->SendUDPMessage(msg);
	}
	void Server::SendSessionInitMessage(ServerTCPConnection* receivingClient)
	{
		// Create a message that indicates the start of the session. 
		// Server expects sync pings soon...
		Kargono::Network::Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::ManageSession_Init;

		// Send message reliably using TCP
		receivingClient->SendUDPMessage(newMessage);
	}
	void Server::StopServer()
	{
		// TODO: Close every TCP connection to all clients

		// TODO: Close the network/network-event threads and join its thread

		// TODO: Clean up all TCP connection objects ??????
		
	}
	
	void Server::SessionClock()
	{
		using namespace std::chrono_literals;

		// Set up the timer variables
		constexpr std::chrono::nanoseconds k_FrameTime { 1'000 * 1'000 * 1'000 / 60 }; // 1/60th of a second
		std::chrono::nanoseconds accumulator{ 0 };
		std::chrono::nanoseconds timeStep{ 0 };

		// Initialize the currentTime and lastCycleTime with now()
		std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::time_point<std::chrono::high_resolution_clock> lastCycleTime = currentTime;

		while (!m_StopTimingThread)
		{
			// Calculate timestep
			currentTime = std::chrono::high_resolution_clock::now();
			timeStep = currentTime - lastCycleTime;
			lastCycleTime = currentTime;

			// Update accumulator
			accumulator += timeStep;

			// Only proceed to an update when accumulator reaches the frametime
			if (accumulator < k_FrameTime)
			{
				continue;
			}

			// Handle adding an update
			accumulator -= k_FrameTime;
			m_UpdateCount++;
		}
	}
	void Server::StartSession()
	{
		// Set the starting frame for the session
		if (!m_TimingThread)
		{
			// Start the timing thread and open the session at frame 0
			m_OnlySession.SetSessionStartFrame(0);
			m_TimingThread = CreateScope<std::thread>(&Server::SessionClock, this);
		}
		else
		{
			// Use the current frame count
			m_OnlySession.SetSessionStartFrame(m_UpdateCount);
		}
	}
	void Server::OnClientValidated(Ref<ServerTCPConnection> client)
	{
		UNREFERENCED_PARAMETER(client);
	}
	bool Server::OnClientConnect(Ref<Kargono::Network::ServerTCPConnection> client)
	{
		KG_INFO("Client successfully connected [{}]", client->GetID());

		// TODO: Get the current size of the client array
		uint32_t clientCount = 0;

		// Let the new client know it has been connected
		SendAcceptConnectionMessage(client.get(), clientCount);

		// Update the client count for all other clients
		SendReceiveClientCountToAllMessage(client.get(), clientCount);

		return true;
	}
	void Server::OnClientDisconnect(Ref<Kargono::Network::ServerTCPConnection> client)
	{
		KG_INFO("Removing client [{}]", client->GetID());

		// TODO: Send a client count update indicating a client left
		SendReceiveClientCountToAllMessage(client.get(), 0 /*TODO: Supply current client count*/);

		if (m_OnlySession.GetAllClients().contains(client->GetID()))
		{
			// Remove the client from the session
			uint16_t removedClientSlot = m_OnlySession.RemoveClient(client->GetID());

			// Check if client removal failed
			if (removedClientSlot == k_InvalidSessionSlot)
			{
				KG_WARN("Client disconnect failed. Could not remove a client from a session.");
				return;
			}

			// Notify all other clients which client was removed
			SendClientLeftMessageToAll(removedClientSlot);
		}
	}

	bool ServerService::Init()
	{
		// Get the server location type from the network config
		bool isLocal = Projects::ProjectService::GetActiveServerLocation() == ServerLocation::LocalMachine;

		// Create the new server context and initialize the server
		s_Server = CreateRef<Network::Server>(Projects::ProjectService::GetActiveServerPort());
		if (!s_Server->StartServer(isLocal))
		{
			// Clean up network resources
			KG_WARN("Failed to start server");
			Terminate();
			return false;
		}

		KG_VERIFY(s_Server, "Server connection init");
		return true;
	}

	void ServerService::Terminate()
	{
		// Ensure the server context exists
		if (!s_Server)
		{
			KG_WARN("Attempt to terminate the active server context when none exists");
			return;
		}

		// Close the server connections and reset its context
		s_Server->StopServer();
		s_Server.reset();
		s_Server = nullptr;

		KG_VERIFY(!s_Server, "Closed server connection");
	}
	void ServerService::Run()
	{
		// TODO: Find method for stopping the server. Currently I just close the application abruptly.
		while (true)
		{
			// Poll the message queue
			s_Server->CheckForMessages();

			// Handle any events in the event queue
			ProcessEventQueue();
		}
	}
	Ref<Server> ServerService::GetActiveServer()
	{
		return s_Server;
	}
	void ServerService::SubmitToNetworkEventQueue(Ref<Events::Event> e)
	{
		// Obtain the event queue lock
		std::scoped_lock<std::mutex> lock(s_Server->m_EventQueueMutex);

		// Add the event
		s_Server->m_EventQueue.emplace_back(e);

		// TODO: Alert thread to wake up and process event
	}
	void ServerService::OnEvent(Events::Event* e)
	{
		if (e->GetEventType() == Events::EventType::StartSession)
		{
			OnStartSession(*(Events::StartSession*)e);
		}
	}
	bool ServerService::OnStartSession(Events::StartSession event)
	{
		KG_ASSERT(s_Server);

		// Handle starting the session runtime
		s_Server->StartSession();

		return true;
	}
	void ServerService::ProcessEventQueue()
	{
		KG_PROFILE_FUNCTION();

		std::vector<Ref<Events::Event>> cachedEvents;

		// Cache a copy of the event queue to prevent loop invalidation
		{
			// Obtain the event queue lock
			std::scoped_lock<std::mutex> lock(s_Server->m_EventQueueMutex);
			
			// Store the events in the cache
			cachedEvents = std::move(s_Server->m_EventQueue);
			s_Server->m_EventQueue.clear();
		}

		// Handle the event queue
		for (Ref<Events::Event> event : cachedEvents)
		{
			OnEvent(event.get());
		}
	}
}

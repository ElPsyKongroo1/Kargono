#include "kgpch.h"

#include "Kargono/Network/Server.h"

#include "Kargono/Projects/Project.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Utility/Operations.h"

#include <conio.h>
#include <queue>
#include <atomic>

namespace Kargono::Network
{
#if 0
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
	void Server::StopServer()
	{
		// TODO: Close every TCP connection to all clients

		// TODO: Close the network/network-event threads and join its thread

		// TODO: Clean up all TCP connection objects ??????

	}
#endif

	void ServerNetworkThread::OpenMessageFromClient(ClientIndex client, Kargono::Network::Message& incomingMessage)
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
	void ServerNetworkThread::OpenServerPingMessage(ClientIndex client, Kargono::Network::Message& msg)
	{
		SendServerPingMessage(client, msg);
	}
	void ServerNetworkThread::OpenMessageAllClientsMessage(ClientIndex client, Kargono::Network::Message& msg)
	{
		KG_INFO("[{}]: Message All", client);
		SendGenericMessageAllClients(client, msg);
	}
	void ServerNetworkThread::OpenMessageClientChatMessage(ClientIndex client, Kargono::Network::Message& msg)
	{
		KG_INFO("[{}]: Sent Chat", client);
		SendServerChatMessageAllClients(client, msg);
	}
	void ServerNetworkThread::OpenRequestClientJoinMessage(ClientIndex client, Kargono::Network::Message& msg)
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
			if (clientID == client)
			{
				continue;
			}

			// Send update message
			SendUpdateClientSlotMessage(connection, clientSlot);
		}

		// Updated new client with all other client data
		for (auto [slot, clientID] : m_OnlySession.GetAllSlots())
		{
			if (clientID == client)
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
	void ServerNetworkThread::OpenRequestClientCountMessage(ClientIndex client)
	{
#if 0
		KG_INFO("[{}]: User Count Request", Client);

		// Send return message to client
		SendReceiveClientCountMessage(client, (uint32_t)m_AllClientConnections.size());
#endif
	}
	void ServerNetworkThread::OpenNotifyAllLeaveMessage(ClientIndex client)
	{
		KG_INFO("[{}]: User Leaving Session", client);

		// Remove the client from the session
		uint16_t removedClient = m_OnlySession.RemoveClient(client);

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
	void ServerNetworkThread::OpenSyncPingMessage(ClientIndex client)
	{
		m_OnlySession.ReceiveSyncPing(client);
	}
	void ServerNetworkThread::OpenStartReadyCheckMessage(ClientIndex client)
	{
		m_OnlySession.StoreClientReadyCheck(client);
	}
	void ServerNetworkThread::OpenEnableReadyCheckMessage()
	{
		m_OnlySession.EnableReadyCheck();
	}
	void ServerNetworkThread::OpenSendAllClientsLocationMessage(ClientIndex client, Message& msg)
	{
		// Forward entity location to all other clients
		for (auto [clientID, connection] : m_OnlySession.GetAllClients())
		{
			// Do not forward the message back to the original client
			if (clientID == client)
			{
				continue;
			}

			SendUpdateLocationMessage(connection, msg);
		}
	}
	void ServerNetworkThread::OpenSendAllClientsPhysicsMessage(ClientIndex client, Message& msg)
	{

		// Forward entity Physics to all other clients
		for (auto [clientID, connection] : m_OnlySession.GetAllClients())
		{
			// Do not forward the message back to the original client
			if (clientID == client)
			{
				continue;
			}

			SendUpdatePhysicsMessage(connection, msg);
		}
	}
	void ServerNetworkThread::OpenSendAllClientsSignalMessage(ClientIndex client, Message& msg)
	{
		// Forward signal to all other session clients
		for (auto [clientID, connection] : m_OnlySession.GetAllClients())
		{
			// Do not forward the message back to the original client
			if (clientID == client)
			{
				continue;
			}

			SendSignalMessage(connection, msg);
		}
	}
	void ServerNetworkThread::OpenKeepAliveMessage(ClientIndex client)
	{
		SendKeepAliveMessage(client);
	}
	void ServerNetworkThread::OpenCheckUDPConnectionMessage(ClientIndex client)
	{
		SendCheckUDPConnectionMessage(client);
	}
	void ServerNetworkThread::CheckConnectionsValid()
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

	void ServerNetworkThread::HandleConnectionKeepAlive()
	{
		if (m_CongestionCounter % 3 == 0)
		{
			// Send keep-alive to all connections
			Message msg;
			msg.m_Header.m_MessageType = MessageType::ManageConnection_KeepAlive;
			SendToAllConnections(msg);
		}
		else
		{
			// Only send keep alive to non-congested connections
			ClientIndex index{ 0 };
			for (Connection& connection : m_AllConnections.GetAllConnections())
			{
				if (!m_AllConnections.IsConnectionActive(index))
				{
					continue;
				}

				if (!connection.m_ReliabilityContext.m_CongestionContext.IsCongested())
				{
					Message msg;
					msg.m_Header.m_MessageType = MessageType::ManageConnection_KeepAlive;
					SendToConnection(index, msg);
				}
				index++;
			}
		}
		m_CongestionCounter++;
	}

	void ServerNetworkThread::SendClientLeftMessageToAll(uint16_t removedClientSlot)
	{
		Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::ManageSession_ClientLeft;
		newMessage << removedClientSlot;

		// Notify all users in the same session that a client left
		for (auto& [clientID, connection] : m_OnlySession.GetAllClients())
		{
			SendToConnection(connection, newMessage);
		}
	}
	void ServerNetworkThread::SendServerPingMessage(ClientIndex client, Kargono::Network::Message& msg)
	{
		// Return the message back to the client to calculate the client's ping
		KG_INFO("[{}]: Server Ping", client);
		SendToConnection(client, msg);
	}
	void ServerNetworkThread::SendGenericMessageAllClients(ClientIndex sendingClient, Kargono::Network::Message& msg)
	{
		// Forward the message from the sending client to all other clients
		Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::GenericMessage_ServerMessage;
		newMessage << sendingClient;

		// Send message reliably to all other clients TCP
		SendToAllConnections(newMessage, sendingClient);
	}
	void ServerNetworkThread::SendServerChatMessageAllClients(ClientIndex sendingClient, Kargono::Network::Message& msg)
	{
		// Forward to chat message to all other clients
		msg.m_Header.m_MessageType = MessageType::GenericMessage_ServerChat;
		msg << sendingClient;

		// Send message reliably to all other clients TCP
		SendToAllConnections(msg, sendingClient);
	}
	void ServerNetworkThread::SendDenyClientJoinMessage(ClientIndex receivingClient)
	{
		// Create simple message notifying client of denial
		Message denyMessage;
		denyMessage.m_Header.m_MessageType = MessageType::ManageSession_DenyClientJoin;

		// Send message reliably with TCP
		SendToConnection(receivingClient, denyMessage);
	}
	void ServerNetworkThread::SendApproveClientJoinMessage(ClientIndex receivingClient, uint16_t clientSlot)
	{
		// Create simple message nofiying the client that their join-session request was approved
		Message approveMessage;
		approveMessage.m_Header.m_MessageType = MessageType::ManageSession_ApproveClientJoin;
		approveMessage << clientSlot;

		// Send message reliably with TCP
		SendToConnection(receivingClient, approveMessage);
	}
	void ServerNetworkThread::SendUpdateClientSlotMessage(ClientIndex receivingClient, uint16_t clientSlot)
	{
		// Create simple message notifying the client that a session-client slot has been changed
		Message updateSlotMessage;
		updateSlotMessage.m_Header.m_MessageType = MessageType::ManageSession_UpdateClientSlot;
		updateSlotMessage << clientSlot;

		// Send message reliably with TCP
		SendToConnection(receivingClient, updateSlotMessage);
	}
	void ServerNetworkThread::SendReceiveClientCountMessage(ClientIndex receivingClient, uint32_t clientCount)
	{
		// Create simple return message to the client indicating the total count of clients on the server
		Message clientCountMessage;
		clientCountMessage.m_Header.m_MessageType = MessageType::ServerQuery_ReceiveClientCount;
		clientCountMessage << clientCount;

		// Send message reliably with TCP
		SendToConnection(receivingClient, clientCountMessage);
	}
	void ServerNetworkThread::SendReceiveClientCountToAllMessage(ClientIndex ignoredClient, uint32_t clientCount)
	{
		// Send simple message to update the client count for all clients (except the ignored one)
		Message clientCountMessage;
		clientCountMessage.m_Header.m_MessageType = MessageType::ServerQuery_ReceiveClientCount;
		clientCountMessage << clientCount;

		// Send message reliably with TCP to *all clients
		SendToAllConnections(clientCountMessage, ignoredClient);
	}
	void ServerNetworkThread::SendClientLeftMessage(ClientIndex receivingClient, uint16_t removedClientSlot)
	{
		// Create message notifying the indicated client that a client at the provided slot left the session
		Message clientLeftMessage;
		clientLeftMessage.m_Header.m_MessageType = MessageType::ManageSession_ClientLeft;
		clientLeftMessage << removedClientSlot;

		// Send message reliably with TCP
		SendToConnection(receivingClient, clientLeftMessage);
	}
	void ServerNetworkThread::SendSyncPingMessage(ClientIndex receivingClient)
	{
		// Create message that sends a ping to synchronize clients inside a session
		Kargono::Network::Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::ManageSession_SyncPing;

		// Send message reliably with TCP
		SendToConnection(receivingClient, newMessage);
	}
	void ServerNetworkThread::SendConfirmReadyCheckMessage(ClientIndex receivingClient, float waitTime)
	{
		// Create message that sends a notifies the client that the client that the ready check is done.
		// The client should wait the specified time and then handle the notification
		Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::ManageSession_ConfirmReadyCheck;
		newMessage << waitTime;

		// Send message reliably with TCP
		SendToConnection(receivingClient, newMessage);
	}
	void ServerNetworkThread::SendUpdateLocationMessage(ClientIndex receivingClient, Message& msg)
	{
		// (Assuming the provided message already contains the location)

		// Forward the client's location to all other clients
		msg.m_Header.m_MessageType = MessageType::ManageSceneEntity_UpdateLocation;

		// Send message quickly using UDP
		SendToConnection(receivingClient, msg);
	}
	void ServerNetworkThread::SendUpdatePhysicsMessage(ClientIndex receivingClient, Message& msg)
	{
		// (Assuming the provided message already contains the physics data)

		// Forward the client's physics data to all other clients
		msg.m_Header.m_MessageType = MessageType::ManageSceneEntity_UpdatePhysics;

		// Send message quickly using UDP
		SendToConnection(receivingClient, msg);
	}
	void ServerNetworkThread::SendSignalMessage(ClientIndex receivingClient, Message& msg)
	{
		// (Assuming the provided message already contains the signal data)

		// Forward the signal to all other clients
		msg.m_Header.m_MessageType = MessageType::ScriptMessaging_ReceiveSignal;

		// Send message reliably using TCP
		SendToConnection(receivingClient, msg);
	}
	void ServerNetworkThread::SendKeepAliveMessage(ClientIndex receivingClient)
	{
		// Return a keep alive message to... well.. keep the connection alive 
		Message keepAliveMessage;
		keepAliveMessage.m_Header.m_MessageType = MessageType::ManageConnection_KeepAlive;

		// Send message quickly using UDP
		SendToConnection(receivingClient, keepAliveMessage);
	}
	void ServerNetworkThread::SendCheckUDPConnectionMessage(ClientIndex receivingClient)
	{
		// Return a check UDP message to the initial client to verify connection integrity
		Message checkConnection;
		checkConnection.m_Header.m_MessageType = MessageType::ManageConnection_CheckUDPConnection;

		// Send message quickly using UDP
		SendToConnection(receivingClient, checkConnection);
	}
	void ServerNetworkThread::SendAcceptConnectionMessage(ClientIndex receivingClient, uint32_t clientCount)
	{
		// Return a message to the client indicating the connection was successful
		Kargono::Network::Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageConnection_AcceptConnection;
		msg << clientCount;

		SendToConnection(receivingClient, msg);
	}
	void ServerNetworkThread::SendSessionInitMessage(ClientIndex receivingClient)
	{
		// Create a message that indicates the start of the session. 
		// Server expects sync pings soon...
		Kargono::Network::Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::ManageSession_Init;

		// Send message reliably using TCP
		SendToConnection(receivingClient, newMessage);
	}

	void ServerNetworkThread::SessionClock()
	{
		using namespace std::chrono_literals;

		// Set up the timer variables
		constexpr std::chrono::nanoseconds k_FrameTime{ 1'000 * 1'000 * 1'000 / 60 }; // 1/60th of a second
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
	void ServerNetworkThread::StartSession()
	{
		// Set the starting frame for the session
		if (!m_TimingThread)
		{
			// Start the timing thread and open the session at frame 0
			m_OnlySession.SetSessionStartFrame(0);
			m_TimingThread = CreateScope<std::thread>(&ServerNetworkThread::SessionClock, this);
		}
		else
		{
			// Use the current frame count
			m_OnlySession.SetSessionStartFrame(m_UpdateCount);
		}
	}
	ObserverIndex NetworkThreadNotifiers::AddSendPacketObserver(std::function<void(ClientIndex, PacketSequence)> func)
	{
		KG_ASSERT(!i_ServerActive || !*i_ServerActive);

		return m_SendPacketNotifier.AddObserver(func);
	}
	bool NetworkThreadNotifiers::RemoveSendPacketObserver(ObserverIndex index)
	{
		KG_ASSERT(!i_ServerActive || !*i_ServerActive);

		return m_SendPacketNotifier.RemoveObserver(index);
	}
	ObserverIndex NetworkThreadNotifiers::AddAckPacketObserver(std::function<void(ClientIndex, PacketSequence, float)> func)
	{
		KG_ASSERT(!i_ServerActive || !*i_ServerActive);

		return m_AckPacketNotifier.AddObserver(func);
	}
	bool NetworkThreadNotifiers::RemoveAckPacketObserver(ObserverIndex index)
	{
		KG_ASSERT(!i_ServerActive || !*i_ServerActive);

		return m_AckPacketNotifier.RemoveObserver(index);
	}
	void ServerNotifiers::Init(std::atomic<bool>* serverActive)
	{
		KG_ASSERT(serverActive);

		i_ServerActive = serverActive;
	}
	ObserverIndex ServerNotifiers::AddServerInitObserver(std::function<void()> func)
	{
		KG_ASSERT(!i_ServerActive || !*i_ServerActive);

		return m_ServerInitNotifier.AddObserver(func);
	}
	bool ServerNotifiers::RemoveServerInitObserver(ObserverIndex index)
	{
		KG_ASSERT(!i_ServerActive || !*i_ServerActive);

		return m_ServerInitNotifier.RemoveObserver(index);
	}
	ObserverIndex ServerNotifiers::AddServerTerminateObserver(std::function<void()> func)
	{
		KG_ASSERT(!i_ServerActive || !*i_ServerActive);

		return m_ServerTerminateNotifier.AddObserver(func);
	}
	bool ServerNotifiers::RemoveServerTerminateObserver(ObserverIndex index)
	{
		KG_ASSERT(!i_ServerActive || !*i_ServerActive);

		return m_ServerTerminateNotifier.RemoveObserver(index);
	}
	void NetworkThreadNotifiers::Init(std::atomic<bool>* serverActive)
	{
		KG_ASSERT(serverActive);

		i_ServerActive = serverActive;
	}
	ObserverIndex NetworkThreadNotifiers::AddClientConnectObserver(std::function<void(ClientIndex)> func)
	{
		KG_ASSERT(!i_ServerActive || !*i_ServerActive);

		return m_ClientConnectNotifier.AddObserver(func);
	}
	bool NetworkThreadNotifiers::RemoveClientConnectObserver(ObserverIndex index)
	{
		KG_ASSERT(!i_ServerActive || !*i_ServerActive);

		return m_ClientConnectNotifier.RemoveObserver(index);
	}
	ObserverIndex NetworkThreadNotifiers::AddClientDisconnectObserver(std::function<void(ClientIndex)> func)
	{
		KG_ASSERT(!i_ServerActive || !*i_ServerActive);

		return m_ClientDisconnectNotifier.AddObserver(func);
	}
	bool NetworkThreadNotifiers::RemoveClientDisconnectObserver(ObserverIndex index)
	{
		KG_ASSERT(!i_ServerActive || !*i_ServerActive);

		return m_ClientDisconnectNotifier.RemoveObserver(index);
	}
	
	void ServerNetworkThread::OnClientValidated(ClientIndex client)
	{
		KG_INFO("Client successfully validated [{}]", client);
	}
	bool ServerNetworkThread::OnClientConnect(ClientIndex client)
	{
		KG_INFO("Client successfully connected [{}]", client);

		// Send connection successful message
		Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageConnection_AcceptConnection;
		SendToConnection(client, msg);

		// Notify observers of client connection
		m_Notifiers.m_ClientConnectNotifier.Notify(client);

		// Run connection faster if necessary
		if (m_AllConnections.GetNumberOfClients() > 0)
		{
			// Set event thread to update faster
			i_EventThread->SetSyncPingFreq(i_ServerConfig->m_ServerActiveRefresh);

			// Set up connection(s) timer
			m_ManageConnectionTimer.InitializeTimer();
		}

#if 0 // TODO: Notify all other clients of a connection
		// TODO: Get the current size of the client array
		uint32_t clientCount = 0;

		// Let the new client know it has been connected
		SendAcceptConnectionMessage(client, clientCount);

		// Update the client count for all other clients
		SendReceiveClientCountToAllMessage(client, clientCount);
#endif

		return true;
	}
	void ServerNetworkThread::OnClientDisconnect(ClientIndex client)
	{
		KG_INFO("Removed client [{}]", client);

		// Notify observers of client disconnection
		m_Notifiers.m_ClientDisconnectNotifier.Notify(client);

		// Run connection faster if necessary
		if (m_AllConnections.GetNumberOfClients() == 0)
		{
			// Set event thread to update faster
			i_EventThread->SetSyncPingFreq(i_ServerConfig->m_ServerPassiveRefresh);

			// Reset timers
			m_ManageConnectionTimer.InitializeTimer();
		}

#if 0 // TODO: This section manages server sessions! Fix it later!
		// TODO: Send a client count update indicating a client left
		SendReceiveClientCountToAllMessage(client, 0 /*TODO: Supply current client count*/);

		if (m_OnlySession.GetAllClients().contains(client))
		{
			// Remove the client from the session
			uint16_t removedClientSlot = m_OnlySession.RemoveClient(client);

			// Check if client removal failed
			if (removedClientSlot == k_InvalidSessionSlot)
			{
				KG_WARN("Client disconnect failed. Could not remove a client from a session.");
				return;
			}

			// Notify all other clients which client was removed
			SendClientLeftMessageToAll(removedClientSlot);
		}

#endif 
	}

	bool Server::Init(const ServerConfig& initConfig)
	{
		// Set config
		m_Config = initConfig;

		// Initialize the OS specific socket context
		if (!SocketContext::InitializeSockets())
		{
			KG_WARN("Failed to initialize platform socket context");
			return false;
		}

		// Open the server socket
		if (!m_ServerSocket.Open(initConfig.m_ServerAddress.GetPort()))
		{
			KG_WARN("Failed to create socket!");
			SocketContext::ShutdownSockets();
			return false;
		}

		// Start event thread
		if (!m_EventThread.Init(&m_ServerSocket, &m_NetworkThread))
		{
			SocketContext::ShutdownSockets();
			m_ServerSocket.Close();
			return false;
		}

		// Start network thread
		if (!m_NetworkThread.Init(&m_Config, &m_ServerSocket, &m_ServerActive, &m_EventThread))
		{
			Terminate(false);
			return false;
		}

		// Init server notifiers
		m_Notifiers.Init(&m_ServerActive);

		// Set server active and notify observers
		m_ServerActive = true;
		m_Notifiers.m_ServerInitNotifier.Notify();

		return true;
	}

	bool Server::Terminate(bool withinNetworkThread)
	{
		// Stop threads
		m_NetworkThread.Terminate();
		m_EventThread.Terminate();

		// Clean up socket resources
		m_ServerSocket.Close();
		SocketContext::ShutdownSockets();

		// Set server in-active and notify observers
		m_ServerActive = false;
		m_Notifiers.m_ServerTerminateNotifier.Notify();

		return true;
	}

	void Server::WaitOnServerThreads()
	{
		m_NetworkThread.WaitOnThread();
		m_EventThread.WaitOnThread();
	}


	void ServerNetworkThread::ResumeThread(bool withinThread)
	{
		m_Thread.ResumeThread(withinThread);
	}

	void ServerNetworkThread::SuspendThread(bool withinThread)
	{
		m_Thread.SuspendThread(withinThread);
	}

	void ServerNetworkThread::RunThread()
	{
		if (m_ManageConnectionTimer.CheckForUpdate())
		{
			// Ensure accumulator does not overflow
			m_ManageConnectionTimer.ResetAccumulator();

			// Handle connection(s)
			HandleConnectionKeepAlive();
			HandleConnectionTimeouts(m_ManageConnectionTimer.GetConstantFrameTimeFloat());
		}
		
		// Process queues
		m_FunctionQueue.ProcessQueue();

		Address sender;
		unsigned char buffer[k_MaxPacketSize];
		int bytes_read{ 0 };

		do
		{
			bytes_read = i_ServerSocket->Receive(sender, buffer, sizeof(buffer));

			if (bytes_read >= (k_PacketHeaderSize))
			{
				// Check for a valid app ID
				if (*(AppID*)&buffer != i_ServerConfig->m_AppProtocolID)
				{
					KG_WARN("Failed to validate the app ID from packet");
					continue;
				}

				// Get the packet type
				MessageType type = (MessageType)buffer[sizeof(AppID)];

				ClientIndex index = (ClientIndex)buffer[sizeof(AppID) + sizeof(MessageType)];

				// Handle messages for already connected clients
				if (m_AllConnections.IsConnectionActive(index))
				{
					// Get the indicated connection
					Connection& connection = m_AllConnections.GetConnection(index);

					// Process packet reliability
					bool newAck = connection.m_ReliabilityContext.ProcessReliabilitySegmentFromPacket(&buffer[sizeof(AppID) + sizeof(MessageType) + sizeof(ClientIndex)]);

					if (newAck)
					{
						// Handle acknowleding each ack
						for (AckData& data : connection.m_ReliabilityContext.GetRecentAcks())
						{
							// TODO: Ack the packet

							// Notify acked packets
							m_Notifiers.m_AckPacketNotifier.Notify(index, data.m_Sequence, data.m_RTT);
						}

						// Notify an updated RTT

					}

					switch (type)
					{
					case MessageType::ManageConnection_KeepAlive:
					{
						Connection& clientConnection = m_AllConnections.GetConnection(index); 
						return;
					}
					case MessageType::ManageConnection_RequestConnection:
						continue;
					case MessageType::GenericMessage_ServerChat:
					{
						bool valid = Utility::Operations::IsValidCString((char*)buffer + k_PacketHeaderSize);
						if (!valid)
						{
							KG_WARN("Buffer could not be converted into a c-string");
							continue;
						}

						KG_WARN("[{}.{}.{}.{}:{}]: ", sender.GetA(), sender.GetB(),
							sender.GetC(), sender.GetD(), sender.GetPort());
						continue;
					}
					default:
						KG_WARN("Invalid packet ID obtained");
						continue;
					}
				}
				else
				{
					// Handle new connections
					if (type == MessageType::ManageConnection_RequestConnection)
					{
						ClientIndex connectionIndex = m_AllConnections.AddConnection(sender);

						// TODO: Handle rejection case better
						if (connectionIndex == k_InvalidClientIndex)
						{
							return;
						}

						// Update server about connection
						Connection& newConnection = m_AllConnections.GetConnection(connectionIndex);
						OnClientConnect(connectionIndex);
						
					}
				}
			}
		} while (bytes_read > 0);

		// Allow the thread to sleep if not managing connections
		m_Thread.SuspendThread(true);
	}

	bool ServerEventThread::Init(Socket* serverSocket, ServerNetworkThread* networkThread)
	{
		KG_ASSERT(serverSocket);
		KG_ASSERT(networkThread);

		// Store dependencies
		i_ServerSocket = serverSocket;
		i_NetworkThread = networkThread;

		// Create network event
		m_NetworkEventHandle = WSACreateEvent();
		if (WSAEventSelect(i_ServerSocket->GetHandle(), m_NetworkEventHandle, FD_READ) != 0)
		{
			KG_WARN("Failed to create the network event handle");
			return false;
		}

		// Start thread
		m_Thread.StartThread(KG_BIND_CLASS_FN(RunThread));
		return true;
	}

	void ServerEventThread::Terminate()
	{
		m_Thread.StopThread(false);
	}

	void ServerEventThread::WaitOnThread()
	{
		m_Thread.WaitOnThread();
	}

	void ServerEventThread::SetSyncPingFreq(size_t frequency)
	{
		m_WorkQueue.SubmitFunction([&, frequency]() 
		{
			m_ActiveSyncPingFreq = frequency;
		});
	}

	void ServerEventThread::RunThread()
	{
		m_WorkQueue.ProcessQueue();

		DWORD waitResult = WaitForMultipleObjects
		(
			1, &m_NetworkEventHandle, 
			FALSE, (DWORD)m_ActiveSyncPingFreq
		);

		if (waitResult == WAIT_OBJECT_0)
		{
			WSANETWORKEVENTS netEvents;
			WSAEnumNetworkEvents(i_ServerSocket->GetHandle(), m_NetworkEventHandle, &netEvents);

			if (netEvents.lNetworkEvents & FD_READ)
			{
				i_NetworkThread->ResumeThread(false);
			}
		}
		else
		{
			i_NetworkThread->ResumeThread(false);
		}
	}

	void ServerNetworkThread::HandleConnectionTimeouts(float deltaTime)
	{
		// Add delta-time to last-packet-received time for all connections
		std::vector<ClientIndex> clientsToRemove;
		ClientIndex index{ 0 };
		for (Connection& connection : m_AllConnections.GetAllConnections())
		{
			if (!m_AllConnections.IsConnectionActive(index))
			{
				continue;
			}

			connection.m_ReliabilityContext.OnUpdate(deltaTime);

			if (connection.m_ReliabilityContext.m_LastPacketReceived > i_ServerConfig->m_ConnectionTimeout)
			{
				clientsToRemove.push_back(index);
			}
			index++;
		}

		// Remove timed-out connections
		for (ClientIndex clientIndex : clientsToRemove)
		{
			// Remove client
			m_AllConnections.RemoveConnection(clientIndex);

			OnClientDisconnect(clientIndex);
		}
	}


	bool ServerNetworkThread::Init(ServerConfig* serverConfig, Socket* serverSocket, std::atomic<bool>* serverActive, ServerEventThread* eventThread)
	{
		KG_ASSERT(serverConfig);
		KG_ASSERT(serverSocket);
		KG_ASSERT(serverActive);
		KG_ASSERT(eventThread);

		// Store dependencies
		i_ServerConfig = serverConfig;
		i_ServerSocket = serverSocket;
		i_ServerActive = serverActive;
		i_EventThread = eventThread;

		// Init notifiers
		m_Notifiers.Init(i_ServerActive);

		// Init connections
		m_AllConnections = ConnectionList(64);

		// Init timers
		m_ManageConnectionTimer.InitializeTimer();
		m_ManageConnectionTimer.SetConstantFrameTime(
			std::chrono::nanoseconds(i_ServerConfig->m_ServerActiveRefresh * 1'000'000));

		// Start thread
		m_Thread.StartThread(KG_BIND_CLASS_FN(RunThread));

		return true;
	}

	void ServerNetworkThread::Terminate()
	{
		m_Thread.StopThread(false);
	}

	void ServerNetworkThread::WaitOnThread()
	{
		m_Thread.WaitOnThread();
	}

	void ServerNetworkThread::SubmitFunction(const std::function<void()>& workFunction)
	{
		m_FunctionQueue.SubmitFunction(workFunction);

		m_Thread.ResumeThread(false);
	}

	bool ServerNetworkThread::SendToConnection(ClientIndex clientIndex, Message& msg)
	{
		KG_ASSERT(m_AllConnections.IsConnectionActive(clientIndex));

		// Get the connection
		Connection& connection = m_AllConnections.GetConnection(clientIndex);

		if (msg.m_Header.m_PayloadSize >= k_MaxPayloadSize)
		{
			KG_WARN("Failed to send packet. Payload exceeds maximum size limit");
			return false;
		}

		// Prepare the final data buffer
		uint8_t buffer[k_MaxPacketSize];

		// Set the app ID
		AppID& appIDLocation = *(AppID*)&buffer[0];
		appIDLocation = i_ServerConfig->m_AppProtocolID;

		// Set the packet type
		MessageType& MessageTypeLocation = *(MessageType*)&buffer[sizeof(AppID)];
		MessageTypeLocation = msg.m_Header.m_MessageType;

		// Send the client connection Index
		ClientIndex& clientIndexLocation = *(ClientIndex*)&buffer[sizeof(AppID) + sizeof(MessageType)];
		clientIndexLocation = clientIndex;

		if (!IsConnectionManagementPacket(msg.m_Header.m_MessageType))
		{
			// Insert the sequence number + ack + ack_bitfield
			uint16_t sentPacketSeq = connection.m_ReliabilityContext.InsertReliabilitySegmentIntoPacket(&buffer[sizeof(AppID) + sizeof(MessageType)]);

			// Use send packet notifier
			m_Notifiers.m_SendPacketNotifier.Notify(clientIndex, sentPacketSeq);
		}

		// Optionally insert the payload
		if (msg.m_Header.m_PayloadSize > 0)
		{
			// Set the data
			memcpy(&buffer[k_PacketHeaderSize], msg.m_PayloadData.data(), msg.m_Header.m_PayloadSize);
		}

		i_ServerSocket->Send(connection.m_Address, buffer, msg.m_Header.m_PayloadSize + k_PacketHeaderSize);

		return true;
	}

	bool ServerNetworkThread::SendToAllConnections(Message& msg, ClientIndex ignoreClient)
	{
		// Check the payload size is valid
		if (msg.m_Header.m_PayloadSize >= k_MaxPayloadSize)
		{
			KG_WARN("Failed to send packet. Payload exceeds maximum size limit");
			return false;
		}

		// Loop through all of the connections
		ClientIndex currentIndex{ 0 };
		for (Connection& connection : m_AllConnections.GetAllConnections())
		{
			if (currentIndex == ignoreClient || !m_AllConnections.IsConnectionActive(currentIndex))
			{
				currentIndex++;
				continue;
			}

			// Prepare the final data buffer
			uint8_t buffer[k_MaxPacketSize];

			// Set the app ID
			AppID& appIDLocation = *(AppID*)&buffer[0];
			appIDLocation = i_ServerConfig->m_AppProtocolID;

			// Set the packet type
			MessageType& MessageTypeLocation = *(MessageType*)&buffer[sizeof(AppID)];
			MessageTypeLocation = msg.m_Header.m_MessageType;

			if (!IsConnectionManagementPacket(msg.m_Header.m_MessageType))
			{
				// Insert the sequence number + ack + ack_bitfield
				PacketSequence seq = connection.m_ReliabilityContext.InsertReliabilitySegmentIntoPacket(&buffer[sizeof(AppID) + sizeof(MessageType) + sizeof(ClientIndex)]);

				// Use send packet notifier
				m_Notifiers.m_SendPacketNotifier.Notify(currentIndex, seq);
			}

			if (msg.m_Header.m_PayloadSize > 0)
			{
				// Set the data
				memcpy(&buffer[k_PacketHeaderSize], msg.m_PayloadData.data(), msg.m_Header.m_PayloadSize);
			}

			i_ServerSocket->Send(connection.m_Address, buffer, k_PacketHeaderSize + msg.m_Header.m_PayloadSize);

			currentIndex++;
		}

		return true;
	}

	bool ServerService::Init()
	{
		if (s_Server.m_ServerActive)
		{
			KG_WARN("Failed server initialization. A server context already exists.");
			return false;
		}

		// Get the server location type from the network config
		bool isLocal = Projects::ProjectService::GetActiveServerLocation() == ServerLocation::LocalMachine;

		ServerConfig startConfig = Projects::ProjectService::GetServerConfig();

		// Begin the server
		if (!s_Server.Init(startConfig))
		{
			// Clean up network resources
			KG_WARN("Failed to start server");
			return false;
		}

		KG_VERIFY(s_Server.m_ServerActive, "Server connection init");
		return true;
	}

	bool ServerService::Terminate()
	{
		// Ensure the server context exists
		if (!s_Server.m_ServerActive)
		{
			KG_WARN("Failed to terminate server. No active server context exists.");
			return false;
		}

		// Close the server connections and reset its context
		s_Server.Terminate(false);

		KG_VERIFY(!s_Server.m_ServerActive, "Closed server connection");
		return true;
	}

	bool ServerService::IsServerActive()
	{
		return s_Server.m_ServerActive;
	}

	Server& ServerService::GetActiveServer()
	{
		return s_Server;
	}
	void ServerService::SubmitToNetworkEventQueue(Ref<Events::Event> e)
	{
		KG_ASSERT(s_Server.m_ServerActive);

		// TODO: Reimplement this please
		//s_Server.GetNetworkThread().SubmitEvent(e);
	}
	void ServerService::SubmitToNetworkFunctionQueue(const std::function<void()>& func)
	{
		KG_ASSERT(s_Server.m_ServerActive);

		s_Server.GetNetworkThread().SubmitFunction(func);	
	}
	void ServerService::OnEvent(Events::Event* e)
	{
#if 0 // TODO: Get sessions to work again
		if (e->GetEventType() == Events::EventType::StartSession)
		{
			OnStartSession(*(Events::StartSession*)e);
		}
#endif
	}
	bool ServerService::OnStartSession(Events::StartSession event)
	{
		KG_ASSERT(s_Server.m_ServerActive);

		// TODO: Get start session to work again
		// Handle starting the session runtime
		//s_Server.StartSession();

		return true;
	}
}

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
	void ServerNetworkThread::OpenMessageFromClient(ClientIndex client, Kargono::Network::Message& incomingMessage)
	{
		// Handle messages based on their type
		switch (incomingMessage.m_Header.m_MessageType)
		{
		case MessageType::ManageSession_RequestClientJoin:
			OpenRequestClientJoinMessage(client, incomingMessage);
			break;
		case MessageType::ServerQuery_RequestClientCount:
			OpenRequestClientCountMessage(client);
			break;
		case MessageType::ManageSession_NotifyAllLeave:
			OpenNotifyAllLeaveMessage(client);
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
			break;
		default:
			KG_ERROR("Invalid message type sent to server");
			break;
		}
	}
	void ServerNetworkThread::OpenRequestClientJoinMessage(ClientIndex newClient, Kargono::Network::Message& msg)
	{
		// Deny client join if session slots are full
		if (m_OnlySession.GetClientCount() >= k_MaxSessionClients)
		{
			SendDenyClientJoinMessage(newClient);
			return;
		}

		// Add client to session and ensure slot is valid
		SessionIndex clientSlot = m_OnlySession.AddClient(newClient);
		if (clientSlot == k_InvalidSessionIndex)
		{
			SendDenyClientJoinMessage(newClient);
			return;
		}

		// Send approval message to the new client
		SendApproveClientJoinMessage(newClient, clientSlot);

		// Notify all other session clients that new client has been added
		for (SessionIndex sessionIndex : m_OnlySession.GetSessionClients().GetActiveIndices())
		{
			ClientIndex sessionClient{ m_OnlySession.GetClient(sessionIndex)};

			// Skip the current client (it already knows from approval)
			if (newClient == sessionClient)
			{
				continue;
			}

			// Update existing client about new addition
			SendUpdateClientSlotMessage(sessionClient, clientSlot);

			// Update new client about other session clients
			SendUpdateClientSlotMessage(newClient, sessionIndex);
		}

		// If enough clients are connected, start the session
		if (m_OnlySession.GetClientCount() == k_MaxSessionClients)
		{
			// TODO: Probably should expose this to the scripts instead of automatically starting the session
			m_OnlySession.CreateSession();
		}
	}
	void ServerNetworkThread::OpenRequestClientCountMessage(ClientIndex client)
	{
		KG_INFO("[{}]: User Count Request", client);

		// Send return message to client
		SendReceiveClientCountMessage(client, m_AllConnections.GetNumberOfClients());
	}
	void ServerNetworkThread::OpenNotifyAllLeaveMessage(ClientIndex client)
	{
		KG_INFO("[{}]: User Leaving Session", client);

		// Remove the client from the session
		SessionIndex removedClient = m_OnlySession.RemoveClient(client);

		// Ensure the client removal was successful
		if (removedClient == k_InvalidSessionIndex)
		{
			KG_WARN("Failed to remove client from the active session");
			return;
		}

		// Notify all users in the same session that a client left
		for (ClientIndex sessionClient : m_OnlySession.GetSessionClients())
		{
			SendClientLeftMessage(sessionClient, removedClient);
		}

		// Notify the removed client it is removed 
		// (note this is necessary since the client no longer exists in the session list)
		SendClientLeftMessage(client, removedClient);
	}
	void ServerNetworkThread::OpenStartReadyCheckMessage(ClientIndex client)
	{
		m_OnlySession.StoreClientReady(client);
	}
	void ServerNetworkThread::OpenEnableReadyCheckMessage()
	{
		m_OnlySession.StartReadyCheck();
	}
	void ServerNetworkThread::OpenSendAllClientsLocationMessage(ClientIndex client, Message& msg)
	{
		// Forward entity location to all other clients
		for (ClientIndex sessionClient : m_OnlySession.GetSessionClients())
		{
			// Do not forward the message back to the original client
			if (client == sessionClient)
			{
				continue;
			}

			SendUpdateLocationMessage(sessionClient, msg);
		}
	}
	void ServerNetworkThread::OpenSendAllClientsPhysicsMessage(ClientIndex client, Message& msg)
	{

		// Forward entity Physics to all other clients
		for (ClientIndex sessionClient : m_OnlySession.GetSessionClients())
		{
			// Do not forward the message back to the original client
			if (client == sessionClient)
			{
				continue;
			}

			SendUpdatePhysicsMessage(sessionClient, msg);
		}

	}
	void ServerNetworkThread::OpenSendAllClientsSignalMessage(ClientIndex client, Message& msg)
	{
		// Forward signal to all other session clients
		for (ClientIndex sessionClient : m_OnlySession.GetSessionClients())
		{
			// Do not forward the message back to the original client
			if (client == sessionClient)
			{
				continue;
			}

			SendSignalMessage(sessionClient, msg);
		}
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

	void ServerNetworkThread::HandleNewConnectionPacket(MessageType type, Address address)
	{
		// Handle new connections
		if (type == MessageType::ManageConnection_RequestConnection)
		{
			if (m_AllConnections.IsAddressActive(address))
			{
				// Redundant accept request
				return;
			}

			ClientIndex connectionIndex = m_AllConnections.AddConnection(address);

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

	void ServerNetworkThread::SendClientLeftMessageToAll(SessionIndex removedClientSlot)
	{
		Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::ManageSession_ClientLeft;
		newMessage << removedClientSlot;

		// Notify all users in the same session that a client left
		for (ClientIndex clientIndex : m_OnlySession.GetSessionClients())
		{
			SendToConnection(clientIndex, newMessage);
		}

	}
	void ServerNetworkThread::SendServerPingMessage(ClientIndex client, Kargono::Network::Message& msg)
	{
		// Return the message back to the client to calculate the client's ping
		KG_INFO("[{}]: Server Ping", client);
		SendToConnection(client, msg);
	}
	void ServerNetworkThread::SendDenyClientJoinMessage(ClientIndex receivingClient)
	{
		// Create simple message notifying client of denial
		Message denyMessage;
		denyMessage.m_Header.m_MessageType = MessageType::ManageSession_DenyClientJoin;

		// Send message reliably with TCP
		SendToConnection(receivingClient, denyMessage);
	}
	void ServerNetworkThread::SendApproveClientJoinMessage(ClientIndex receivingClient, SessionIndex clientSlot)
	{
		// Create simple message nofiying the client that their join-session request was approved
		Message approveMessage;
		approveMessage.m_Header.m_MessageType = MessageType::ManageSession_ApproveClientJoin;
		approveMessage << clientSlot;

		// Send message reliably with TCP
		SendToConnection(receivingClient, approveMessage);
	}
	void ServerNetworkThread::SendUpdateClientSlotMessage(ClientIndex receivingClient, SessionIndex clientSlot)
	{
		// Create simple message notifying the client that a session-client slot has been changed
		Message updateSlotMessage;
		updateSlotMessage.m_Header.m_MessageType = MessageType::ManageSession_UpdateClientSlot;
		updateSlotMessage << clientSlot;

		// Send message reliably with TCP
		SendToConnection(receivingClient, updateSlotMessage);
	}
	void ServerNetworkThread::SendReceiveClientCountMessage(ClientIndex receivingClient, size_t clientCount)
	{
		// Create simple return message to the client indicating the total count of clients on the server
		Message clientCountMessage;
		clientCountMessage.m_Header.m_MessageType = MessageType::ServerQuery_ReceiveClientCount;
		clientCountMessage << clientCount;

		// Send message reliably with TCP
		SendToConnection(receivingClient, clientCountMessage);
	}
	void ServerNetworkThread::SendReceiveClientCountToAllMessage(ClientIndex ignoredClient, size_t clientCount)
	{
		// Send simple message to update the client count for all clients (except the ignored one)
		Message clientCountMessage;
		clientCountMessage.m_Header.m_MessageType = MessageType::ServerQuery_ReceiveClientCount;
		clientCountMessage << clientCount;

		// Send message reliably with TCP to *all clients
		SendToAllConnections(clientCountMessage, ignoredClient);
	}
	void ServerNetworkThread::SendClientLeftMessage(ClientIndex receivingClient, SessionIndex removedClientSlot)
	{
		// Create message notifying the indicated client that a client at the provided slot left the session
		Message clientLeftMessage;
		clientLeftMessage.m_Header.m_MessageType = MessageType::ManageSession_ClientLeft;
		clientLeftMessage << removedClientSlot;

		// Send message reliably with TCP
		SendToConnection(receivingClient, clientLeftMessage);
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
	void ServerNetworkThread::SendAcceptConnectionMessage(ClientIndex receivingClient, size_t clientCount)
	{
		// Return a message to the client indicating the connection was successful
		Kargono::Network::Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageConnection_AcceptConnection;
		msg << clientCount;

		// Send the message
		SendToConnection(receivingClient, msg);
	}
	void ServerNetworkThread::SendSessionInitMessage(ClientIndex receivingClient)
	{
		// Create a message that indicates the start of the session. 
		// Server expects sync pings soon...
		Kargono::Network::Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::ManageSession_Init;

		// Send the message
		SendToConnection(receivingClient, newMessage);
	}

	void ServerNetworkThread::SendStartSessionMessage(ClientIndex receivingClient, float waitTime)
	{
		// Create a message that indicates the start of the session. 
		// Server expects sync pings soon...
		Kargono::Network::Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::ManageSession_StartSession;

		newMessage << waitTime;

		// Send the message
		SendToConnection(receivingClient, newMessage);
	}

	void ServerNetworkThread::StartSession()
	{
		// Use the current frame count
		m_OnlySession.StartGameplay(m_ManageConnectionTimer.GetUpdateCount());
	}
	void ServerNotifiers::Init(std::atomic<bool>* serverActive)
	{
		KG_ASSERT(serverActive);

		i_ServerActive = serverActive;
	}
	ObserverIndex ServerNotifiers::AddServerActiveObserver(std::function<void(bool)> func)
	{
		KG_ASSERT(!i_ServerActive || !*i_ServerActive);

		return m_ServerActiveNotifier.AddObserver(func);
	}
	bool ServerNotifiers::RemoveServerActiveObserver(ObserverIndex index)
	{
		KG_ASSERT(!i_ServerActive || !*i_ServerActive);

		return m_ServerActiveNotifier.RemoveObserver(index);
	}
	void ServerNetworkNotifiers::Init(std::atomic<bool>* serverActive)
	{
		KG_ASSERT(serverActive);

		i_ServerActive = serverActive;
	}
	ObserverIndex ServerNetworkNotifiers::AddConnectObserver(std::function<void(ClientIndex)> func)
	{
		KG_ASSERT(!i_ServerActive || !*i_ServerActive);

		return m_ConnectNotifier.AddObserver(func);
	}
	bool ServerNetworkNotifiers::RemoveConnectObserver(ObserverIndex index)
	{
		KG_ASSERT(!i_ServerActive || !*i_ServerActive);

		return m_ConnectNotifier.RemoveObserver(index);
	}
	ObserverIndex ServerNetworkNotifiers::AddDisconnectObserver(std::function<void(ClientIndex)> func)
	{
		KG_ASSERT(!i_ServerActive || !*i_ServerActive);

		return m_DisconnectNotifier.AddObserver(func);
	}
	bool ServerNetworkNotifiers::RemoveDisconnectObserver(ObserverIndex index)
	{
		KG_ASSERT(!i_ServerActive || !*i_ServerActive);

		return m_DisconnectNotifier.RemoveObserver(index);
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
		m_Notifiers.m_ConnectNotifier.Notify(client);

		// Run connection faster if necessary
		if (m_AllConnections.GetNumberOfClients() > 0)
		{
			// Set event thread to update faster
			i_EventThread->SetSyncPingFreq(i_ServerConfig->m_ServerActiveRefresh);

			// Set up connection(s) timer
			m_ManageConnectionTimer.InitializeTimer();
			m_ManageConnectionTimer.SetConstantFrameTime(
				std::chrono::nanoseconds(i_ServerConfig->m_ServerActiveRefresh * 1'000'000));
		}

		ClientIndex clientCount = m_AllConnections.GetNumberOfClients();

		// Let the new client know it has been connected
		SendAcceptConnectionMessage(client, clientCount);

		// Update the client count for all other clients
		SendReceiveClientCountToAllMessage(client, clientCount);

		return true;
	}
	void ServerNetworkThread::OnClientDisconnect(ClientIndex client)
	{
		KG_INFO("Removed client [{}]", client);

		// Notify observers of client disconnection
		m_Notifiers.m_DisconnectNotifier.Notify(client);

		// Run connection faster if necessary
		if (m_AllConnections.GetNumberOfClients() == 0)
		{
			// Set event thread to update faster
			i_EventThread->SetSyncPingFreq(i_ServerConfig->m_ServerPassiveRefresh);

			// Reset timers
			m_ManageConnectionTimer.InitializeTimer();
			m_ManageConnectionTimer.SetConstantFrameTime(
				std::chrono::nanoseconds(i_ServerConfig->m_ServerPassiveRefresh * 1'000'000));
		}

		SendReceiveClientCountToAllMessage(client, m_AllConnections.GetNumberOfClients());

		for (ClientIndex sessionClient : m_OnlySession.GetSessionClients())
		{
			if (sessionClient == client)
			{
				// Remove the client from the session
				SessionIndex removedClientSlot = m_OnlySession.RemoveClient(client);

				// Check if client removal failed
				if (removedClientSlot == k_InvalidSessionIndex)
				{
					KG_WARN("Client disconnect failed. Could not remove a client from a session.");
					return;
				}

				// Notify all other clients which client was removed
				SendClientLeftMessageToAll(removedClientSlot);
				break;
			}
		}

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
		if (m_ServerSocket.Open(initConfig.m_ServerAddress.GetPort()) != SocketErrorCode::None)
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
		m_EventThread.SetSyncPingFreq(m_Config.m_ServerPassiveRefresh);

		// Set server active and notify observers
		m_ServerActive = true;
		m_Notifiers.m_ServerActiveNotifier.Notify(true);

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
		m_Notifiers.m_ServerActiveNotifier.Notify(false);

		return true;
	}

	void Server::WaitOnThreads()
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
		if (m_ManageConnectionTimer.CheckForMultipleUpdates() > 0)
		{
			// Handle connection(s)
			HandleConnectionKeepAlive();
			HandleConnectionTimeouts(m_ManageConnectionTimer.GetConstantFrameTimeFloat());
		}
		
		// Process queues
		m_FunctionQueue.ProcessQueue();
		m_EventQueue.ProcessQueue();

		Address sender;
		unsigned char buffer[k_MaxPacketSize];
		int bytesRead{ 0 };

		do
		{
			bytesRead = i_ServerSocket->Receive(sender, buffer, sizeof(buffer));
			if (bytesRead < k_PacketHeaderSize)
			{
				continue;
			}
			uint8_t* headerIterator{ buffer };

			// Check for a valid app ID
			AppID appID{ *(AppID*)headerIterator };
			if (appID != i_ServerConfig->m_AppProtocolID)
			{
				KG_WARN("Failed to validate the app ID from packet");
				continue;
			}
			headerIterator += sizeof(AppID);

			// Get the packet type
			MessageType type = *(MessageType*)headerIterator;
			headerIterator += sizeof(MessageType);

			ClientIndex clientIndex = *(ClientIndex*)headerIterator;
			headerIterator += sizeof(ClientIndex);

			// Handle messages for already connected clients
			if (m_AllConnections.IsConnectionActive(clientIndex))
			{
				if (IsConnectionManagementPacket(type))
				{
					continue;
				}

				// Get the indicated connection
				Connection& connection = m_AllConnections.GetConnection(clientIndex);

				// Process packet reliability
				bool packetAccepted = connection.m_ReliabilityContext.ProcessReliabilitySegmentFromPacket(headerIterator);

				if (!packetAccepted)
				{
					continue;
				}

				for (AckData data : connection.m_ReliabilityContext.GetRecentAcks())
				{
					m_ReliabilityNotifiers.m_AckPacketNotifier.Notify(clientIndex, data.m_Sequence, data.m_RTT);
				}


				// Set up message
				Message msg;
				msg.m_Header.m_MessageType = type;
				msg.m_Header.m_PayloadSize = (size_t)bytesRead - k_PacketHeaderSize;
				KG_ASSERT(msg.m_Header.m_PayloadSize >= 0);

				// Load in the payload
				if (msg.m_Header.m_PayloadSize > 0)
				{
					msg.m_PayloadData.resize(msg.m_Header.m_PayloadSize);
					memcpy(msg.m_PayloadData.data(), buffer + k_PacketHeaderSize, msg.m_Header.m_PayloadSize);
				}

				OpenMessageFromClient(clientIndex, msg);
			}
			else
			{
				HandleNewConnectionPacket(type, sender);
			}
			
		} while (bytesRead > 0);

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

			// Update reliability observers
			ReliabilityContext& relContext{ connection.m_ReliabilityContext };
			m_ReliabilityNotifiers.m_ReliabilityStateNotifier.Notify(
				index,
				relContext.m_CongestionContext.IsCongested(),
				relContext.m_RoundTripContext.GetAverageRoundTrip()
			);

			relContext.OnUpdate(deltaTime);

			if (relContext.m_LastPacketReceived > i_ServerConfig->m_ConnectionTimeout)
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
		m_ReliabilityNotifiers.Init(i_ServerActive);

		// Set up work queues
		m_EventQueue.Init(KG_BIND_CLASS_FN(OnEvent));

		// Init connections
		m_AllConnections = ConnectionList(64);

		// Init session
		m_OnlySession.Init(this, &m_AllConnections);

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

	void ServerNetworkThread::SubmitEvent(Ref<Events::Event> event)
	{
		m_EventQueue.SubmitEvent(event);

		m_Thread.ResumeThread(false);
	}

	void ServerNetworkThread::OnEvent(Events::Event* event)
	{
		if (event->GetEventType() == Events::EventType::StartSession)
		{
			StartSession();
		}
	}

	bool ServerNetworkThread::SendToConnection(ClientIndex clientIndex, Message& msg)
	{
		KG_ASSERT(m_AllConnections.IsConnectionActive(clientIndex));
		KG_ASSERT(msg.m_Header.m_PayloadSize < k_MaxPayloadSize);

		// Get the connection
		Connection& connection = m_AllConnections.GetConnection(clientIndex);

		// Prepare the final data buffer
		uint8_t buffer[k_MaxPacketSize];
		uint8_t* headerIterator{ buffer };

		// Set the app ID
		AppID& appIDLocation = *(AppID*)headerIterator;
		appIDLocation = i_ServerConfig->m_AppProtocolID;
		headerIterator += sizeof(AppID);

		// Set the packet type
		MessageType& MessageTypeLocation = *(MessageType*)headerIterator;
		MessageTypeLocation = msg.m_Header.m_MessageType;
		headerIterator += sizeof(MessageType);

		// Send the client connection Index
		ClientIndex& clientIndexLocation = *(ClientIndex*)headerIterator;
		clientIndexLocation = clientIndex;
		headerIterator += sizeof(ClientIndex);

		// Optionally insert reliability segment
		if (!IsConnectionManagementPacket(msg.m_Header.m_MessageType))
		{
			// Insert the sequence number + ack + ack_bitfield
			connection.m_ReliabilityContext.InsertReliabilitySegmentIntoPacket(headerIterator);
			PacketSequence sentPacketSeq{ *(PacketSequence*)headerIterator };

			// Use send packet notifier
			m_ReliabilityNotifiers.m_SendPacketNotifier.Notify(clientIndex, sentPacketSeq);
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
		// Loop through all of the connections
		ClientIndex currentIndex{ 0 };
		for (Connection& connection : m_AllConnections.GetAllConnections())
		{
			if (currentIndex == ignoreClient || !m_AllConnections.IsConnectionActive(currentIndex))
			{
				currentIndex++;
				continue;
			}

			SendToConnection(currentIndex, msg);
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
	void ServerService::SubmitToNetworkFunctionQueue(const std::function<void()>& func)
	{
		KG_ASSERT(s_Server.m_ServerActive);

		s_Server.GetNetworkThread().SubmitFunction(func);	
	}
	void ServerService::SubmitToNetworkEventQueue(Ref<Events::Event> event)
	{
		KG_ASSERT(s_Server.m_ServerActive);

		s_Server.GetNetworkThread().SubmitEvent(event);
	}
}

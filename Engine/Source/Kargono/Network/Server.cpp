#include "kgpch.h"

#include "Kargono/Network/Server.h"

#include "Kargono/Projects/Project.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Utility/Operations.h"

#include <conio.h>
#include <queue>
#include <atomic>


static HANDLE hNetworkEvent;
static HANDLE hInputEvent;
static HANDLE allEvents[2];
static std::string text;

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

	void Server::OpenMessageFromClient(ClientIndex client, Kargono::Network::Message& incomingMessage)
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
	void Server::OpenServerPingMessage(ClientIndex client, Kargono::Network::Message& msg)
	{
		SendServerPingMessage(client, msg);
	}
	void Server::OpenMessageAllClientsMessage(ClientIndex client, Kargono::Network::Message& msg)
	{
		KG_INFO("[{}]: Message All", client);
		SendGenericMessageAllClients(client, msg);
	}
	void Server::OpenMessageClientChatMessage(ClientIndex client, Kargono::Network::Message& msg)
	{
		KG_INFO("[{}]: Sent Chat", client);
		SendServerChatMessageAllClients(client, msg);
	}
	void Server::OpenRequestClientJoinMessage(ClientIndex client, Kargono::Network::Message& msg)
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
	void Server::OpenRequestClientCountMessage(ClientIndex client)
	{
#if 0
		KG_INFO("[{}]: User Count Request", Client);

		// Send return message to client
		SendReceiveClientCountMessage(client, (uint32_t)m_AllClientConnections.size());
#endif
	}
	void Server::OpenNotifyAllLeaveMessage(ClientIndex client)
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
	void Server::OpenSyncPingMessage(ClientIndex client)
	{
		m_OnlySession.ReceiveSyncPing(client);
	}
	void Server::OpenStartReadyCheckMessage(ClientIndex client)
	{
		m_OnlySession.StoreClientReadyCheck(client);
	}
	void Server::OpenEnableReadyCheckMessage()
	{
		m_OnlySession.EnableReadyCheck();
	}
	void Server::OpenSendAllClientsLocationMessage(ClientIndex client, Message& msg)
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
	void Server::OpenSendAllClientsPhysicsMessage(ClientIndex client, Message& msg)
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
	void Server::OpenSendAllClientsSignalMessage(ClientIndex client, Message& msg)
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
	void Server::OpenKeepAliveMessage(ClientIndex client)
	{
		SendKeepAliveMessage(client);
	}
	void Server::OpenCheckUDPConnectionMessage(ClientIndex client)
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

	void Server::SendClientLeftMessageToAll(uint16_t removedClientSlot)
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
	void Server::SendServerPingMessage(ClientIndex client, Kargono::Network::Message& msg)
	{
		// Return the message back to the client to calculate the client's ping
		KG_INFO("[{}]: Server Ping", client);
		SendToConnection(client, msg);
	}
	void Server::SendGenericMessageAllClients(ClientIndex sendingClient, Kargono::Network::Message& msg)
	{
		// Forward the message from the sending client to all other clients
		Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::GenericMessage_ServerMessage;
		newMessage << sendingClient;

		// Send message reliably to all other clients TCP
		SendToAllConnections(newMessage, sendingClient);
	}
	void Server::SendServerChatMessageAllClients(ClientIndex sendingClient, Kargono::Network::Message& msg)
	{
		// Forward to chat message to all other clients
		msg.m_Header.m_MessageType = MessageType::GenericMessage_ServerChat;
		msg << sendingClient;

		// Send message reliably to all other clients TCP
		SendToAllConnections(msg, sendingClient);
	}
	void Server::SendDenyClientJoinMessage(ClientIndex receivingClient)
	{
		// Create simple message notifying client of denial
		Message denyMessage;
		denyMessage.m_Header.m_MessageType = MessageType::ManageSession_DenyClientJoin;

		// Send message reliably with TCP
		SendToConnection(receivingClient, denyMessage);
	}
	void Server::SendApproveClientJoinMessage(ClientIndex receivingClient, uint16_t clientSlot)
	{
		// Create simple message nofiying the client that their join-session request was approved
		Message approveMessage;
		approveMessage.m_Header.m_MessageType = MessageType::ManageSession_ApproveClientJoin;
		approveMessage << clientSlot;

		// Send message reliably with TCP
		SendToConnection(receivingClient, approveMessage);
	}
	void Server::SendUpdateClientSlotMessage(ClientIndex receivingClient, uint16_t clientSlot)
	{
		// Create simple message notifying the client that a session-client slot has been changed
		Message updateSlotMessage;
		updateSlotMessage.m_Header.m_MessageType = MessageType::ManageSession_UpdateClientSlot;
		updateSlotMessage << clientSlot;

		// Send message reliably with TCP
		SendToConnection(receivingClient, updateSlotMessage);
	}
	void Server::SendReceiveClientCountMessage(ClientIndex receivingClient, uint32_t clientCount)
	{
		// Create simple return message to the client indicating the total count of clients on the server
		Message clientCountMessage;
		clientCountMessage.m_Header.m_MessageType = MessageType::ServerQuery_ReceiveClientCount;
		clientCountMessage << clientCount;

		// Send message reliably with TCP
		SendToConnection(receivingClient, clientCountMessage);
	}
	void Server::SendReceiveClientCountToAllMessage(ClientIndex ignoredClient, uint32_t clientCount)
	{
		// Send simple message to update the client count for all clients (except the ignored one)
		Message clientCountMessage;
		clientCountMessage.m_Header.m_MessageType = MessageType::ServerQuery_ReceiveClientCount;
		clientCountMessage << clientCount;

		// Send message reliably with TCP to *all clients
		SendToAllConnections(clientCountMessage, ignoredClient);
	}
	void Server::SendClientLeftMessage(ClientIndex receivingClient, uint16_t removedClientSlot)
	{
		// Create message notifying the indicated client that a client at the provided slot left the session
		Message clientLeftMessage;
		clientLeftMessage.m_Header.m_MessageType = MessageType::ManageSession_ClientLeft;
		clientLeftMessage << removedClientSlot;

		// Send message reliably with TCP
		SendToConnection(receivingClient, clientLeftMessage);
	}
	void Server::SendSyncPingMessage(ClientIndex receivingClient)
	{
		// Create message that sends a ping to synchronize clients inside a session
		Kargono::Network::Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::ManageSession_SyncPing;

		// Send message reliably with TCP
		SendToConnection(receivingClient, newMessage);
	}
	void Server::SendConfirmReadyCheckMessage(ClientIndex receivingClient, float waitTime)
	{
		// Create message that sends a notifies the client that the client that the ready check is done.
		// The client should wait the specified time and then handle the notification
		Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::ManageSession_ConfirmReadyCheck;
		newMessage << waitTime;

		// Send message reliably with TCP
		SendToConnection(receivingClient, newMessage);
	}
	void Server::SendUpdateLocationMessage(ClientIndex receivingClient, Message& msg)
	{
		// (Assuming the provided message already contains the location)

		// Forward the client's location to all other clients
		msg.m_Header.m_MessageType = MessageType::ManageSceneEntity_UpdateLocation;

		// Send message quickly using UDP
		SendToConnection(receivingClient, msg);
	}
	void Server::SendUpdatePhysicsMessage(ClientIndex receivingClient, Message& msg)
	{
		// (Assuming the provided message already contains the physics data)

		// Forward the client's physics data to all other clients
		msg.m_Header.m_MessageType = MessageType::ManageSceneEntity_UpdatePhysics;

		// Send message quickly using UDP
		SendToConnection(receivingClient, msg);
	}
	void Server::SendSignalMessage(ClientIndex receivingClient, Message& msg)
	{
		// (Assuming the provided message already contains the signal data)

		// Forward the signal to all other clients
		msg.m_Header.m_MessageType = MessageType::ScriptMessaging_ReceiveSignal;

		// Send message reliably using TCP
		SendToConnection(receivingClient, msg);
	}
	void Server::SendKeepAliveMessage(ClientIndex receivingClient)
	{
		// Return a keep alive message to... well.. keep the connection alive 
		Message keepAliveMessage;
		keepAliveMessage.m_Header.m_MessageType = MessageType::ManageConnection_KeepAlive;

		// Send message quickly using UDP
		SendToConnection(receivingClient, keepAliveMessage);
	}
	void Server::SendCheckUDPConnectionMessage(ClientIndex receivingClient)
	{
		// Return a check UDP message to the initial client to verify connection integrity
		Message checkConnection;
		checkConnection.m_Header.m_MessageType = MessageType::ManageConnection_CheckUDPConnection;

		// Send message quickly using UDP
		SendToConnection(receivingClient, checkConnection);
	}
	void Server::SendAcceptConnectionMessage(ClientIndex receivingClient, uint32_t clientCount)
	{
		// Return a message to the client indicating the connection was successful
		Kargono::Network::Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageConnection_AcceptConnection;
		msg << clientCount;

		// Send message reliably using TCP
		SendToConnection(receivingClient, msg);
	}
	void Server::SendSessionInitMessage(ClientIndex receivingClient)
	{
		// Create a message that indicates the start of the session. 
		// Server expects sync pings soon...
		Kargono::Network::Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::ManageSession_Init;

		// Send message reliably using TCP
		SendToConnection(receivingClient, newMessage);
	}

	void Server::SessionClock()
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
	void Server::OnClientValidated(ClientIndex client)
	{
		UNREFERENCED_PARAMETER(client);
	}
	bool Server::OnClientConnect(ClientIndex client)
	{
		KG_INFO("Client successfully connected [{}]", client);

		// TODO: Get the current size of the client array
		uint32_t clientCount = 0;

		// Let the new client know it has been connected
		SendAcceptConnectionMessage(client, clientCount);

		// Update the client count for all other clients
		SendReceiveClientCountToAllMessage(client, clientCount);

		return true;
	}
	void Server::OnClientDisconnect(ClientIndex client)
	{
		KG_INFO("Removing client [{}]", client);

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
	}


	bool Server::InitServer(const NetworkConfig& initConfig)
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

		m_NetworkEventQueue.Init(KG_BIND_CLASS_FN(OnEvent));

		// TODO: Move this please for the love of god
		// Create network event
		hNetworkEvent = WSACreateEvent();
		if (WSAEventSelect(m_ServerSocket.GetHandle(), hNetworkEvent, FD_READ) != 0)
		{
			KG_WARN("Failed to create the network event handle");
			return false;
		}

		// Get console input handle
		hInputEvent = GetStdHandle(STD_INPUT_HANDLE);
		SetConsoleMode(hInputEvent, 0);

		// Wait for both events
		allEvents[0] = hNetworkEvent;
		allEvents[1] = hInputEvent;

		m_AllConnections = ConnectionList(64);

		m_ManageConnections = false;

		// Start network thread
		m_ManageConnectionTimer.InitializeTimer();
		m_KeepAliveTimer.InitializeTimer(m_Config.m_SyncPingFrequency);
		m_NetworkThread.StartThread(KG_BIND_CLASS_FN(RunNetworkThread));
		m_NetworkEventThread.StartThread(KG_BIND_CLASS_FN(RunNetworkEventThread));

		return true;
	}

	bool Server::TerminateServer(bool withinNetworkThread)
	{
		// Join the network thread
		m_NetworkThread.StopThread(withinNetworkThread);
		m_NetworkEventThread.StopThread(withinNetworkThread);

		m_ManageConnections = false;

		// Clean up socket resources
		SocketContext::ShutdownSockets();

		return true;
	}

	void Server::WaitOnServerTerminate()
	{
		m_NetworkThread.WaitOnThread();
		m_NetworkEventThread.WaitOnThread();
	}


	void Server::RunNetworkThread()
	{
		// Run functions that manage the upkeep of active client connections
		if (m_ManageConnections && !ManageConnections())
		{
			return;
		}

		m_NetworkEventQueue.ProcessQueue();

		Address sender;
		unsigned char buffer[k_MaxPacketSize];
		int bytes_read{ 0 };

		do
		{
			bytes_read = m_ServerSocket.Receive(sender, buffer, sizeof(buffer));

			if (bytes_read >= (k_PacketHeaderSize))
			{
				// Check for a valid app ID
				if (*(AppID*)&buffer != m_Config.m_AppProtocolID)
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
					Connection* connection = m_AllConnections.GetConnection(index);
					KG_ASSERT(connection);

					// Process packet reliability
					connection->m_ReliabilityContext.ProcessReliabilitySegmentFromPacket(&buffer[sizeof(AppID) + sizeof(MessageType) + sizeof(ClientIndex)]);

					switch (type)
					{
					case MessageType::ManageConnection_KeepAlive:
					{
						Connection* clientConnection = m_AllConnections.GetConnection(index);
						if (!clientConnection)
						{
							KG_WARN("Failed to get connection object when receiving a keep alive packet");
							continue;
						}

						// Reset connection
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

						if (!m_ManageConnections && m_AllConnections.GetNumberOfClients() > 0)
						{
							m_ManageConnections = true;
							m_ManageConnectionTimer.InitializeTimer();
							m_KeepAliveTimer.InitializeTimer();
						}

						// Get the connection reference
						Connection* newConnection = m_AllConnections.GetConnection(connectionIndex);

						if (newConnection)
						{
							KG_WARN("New connection created");
							Message msg;
							msg.m_Header.m_MessageType = MessageType::ManageConnection_AcceptConnection;
							SendToConnection(connectionIndex, msg);
						}
					}
				}
			}
		} while (bytes_read > 0);

		// Allow the thread to sleep if not managing connections
		if (!m_ManageConnections)
		{
			m_NetworkThread.SuspendThread(true);
		}

	}

	void Server::RunNetworkEventThread()
	{
		DWORD waitResult = WaitForMultipleObjects(2, allEvents, FALSE, INFINITE);

		if (waitResult == WAIT_OBJECT_0)  // Network event
		{
			WSANETWORKEVENTS netEvents;
			WSAEnumNetworkEvents(m_ServerSocket.GetHandle(), hNetworkEvent, &netEvents);

			if (netEvents.lNetworkEvents & FD_READ)
			{
				m_NetworkThread.ResumeThread();
			}
		}
		else if (waitResult == WAIT_OBJECT_0 + 1)  // Console input event
		{
			INPUT_RECORD inputRecord;
			DWORD eventsRead;

			while (true)
			{
				DWORD numEvents;
				if (!GetNumberOfConsoleInputEvents(hInputEvent, &numEvents) || numEvents == 0)
					break;  // No more events, exit the loop

				if (ReadConsoleInput(hInputEvent, &inputRecord, 1, &eventsRead))
				{
					if (inputRecord.EventType == KEY_EVENT && inputRecord.Event.KeyEvent.bKeyDown)
					{
						char key = inputRecord.Event.KeyEvent.uChar.AsciiChar;

						m_NetworkEventQueue.SubmitEvent(CreateRef<Events::KeyPressedEvent>(key));
						m_NetworkThread.ResumeThread();
					}
				}
			}
		}
	}

	bool Server::ManageConnections()
	{
		if (!m_ManageConnectionTimer.CheckForUpdate())
		{
			return false;
		}

		if (m_KeepAliveTimer.CheckForUpdate(m_ManageConnectionTimer.GetConstantFrameTime()))
		{
			static uint32_t s_CongestionCounter{ 0 };

			if (s_CongestionCounter % 3 == 0)
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

			s_CongestionCounter++;
		}

		// Add delta-time to last-packet-received time for all connections
		std::vector<ClientIndex> clientsToRemove;
		ClientIndex index{ 0 };
		for (Connection& connection : m_AllConnections.GetAllConnections())
		{
			if (!m_AllConnections.IsConnectionActive(index))
			{
				continue;
			}

			connection.m_ReliabilityContext.OnUpdate(m_ManageConnectionTimer.GetConstantFrameTimeFloat());

			if (connection.m_ReliabilityContext.m_LastPacketReceived > m_Config.m_ConnectionTimeout)
			{
				clientsToRemove.push_back(index);
			}
			index++;
		}

		// Remove timed-out connections
		for (ClientIndex index : clientsToRemove)
		{
			KG_WARN("Removing client");
			m_AllConnections.RemoveConnection(index);
		}

		if (m_AllConnections.GetNumberOfClients() <= 0)
		{
			m_ManageConnections = false;
		}

		return true;
	}

	void Server::HandleConsoleInput(Events::KeyPressedEvent event)
	{
		char key = (char)event.GetKeyCode();
		if (key >= 32 && key < 127)
		{
			text += key;
			KG_WARN("{}", key);
		}
		if (key == 127 && text.size() > 0)
		{
			KG_WARN("\b \b");
			text.pop_back();
		}
		if (key == 27) // Escape key
		{
			TerminateServer(true);
			return;
		}
		if (key == 13)
		{
			Message msg;
			msg.m_Header.m_MessageType = MessageType::GenericMessage_ServerChat;
			msg.AppendPayload(text.data(), (int)strlen(text.data()) + 1);
			SendToAllConnections(msg);
			text.clear();
		}

		return;
	}

	void Server::OnEvent(Events::Event* event)
	{
		if (event->GetEventType() == Events::EventType::KeyPressed)
		{
			HandleConsoleInput(*(Events::KeyPressedEvent*)event);
		}
	}


	void Server::SubmitEvent(Ref<Events::Event> event)
	{
		m_NetworkEventQueue.SubmitEvent(event);

		m_NetworkEventThread.ResumeThread();
	}

	bool Server::SendToConnection(ClientIndex clientIndex, Message& msg)
	{
		// Get the connection
		Connection* connection = m_AllConnections.GetConnection(clientIndex);

		if (!connection)
		{
			KG_WARN("Failed to send message to connection. Invalid connection context provided");
			return false;
		}

		if (msg.m_Header.m_PayloadSize >= k_MaxPayloadSize)
		{
			KG_WARN("Failed to send packet. Payload exceeds maximum size limit");
			return false;
		}

		// Prepare the final data buffer
		uint8_t buffer[k_MaxPacketSize];

		// Set the app ID
		AppID& appIDLocation = *(AppID*)&buffer[0];
		appIDLocation = m_Config.m_AppProtocolID;

		// Set the packet type
		MessageType& MessageTypeLocation = *(MessageType*)&buffer[sizeof(AppID)];
		MessageTypeLocation = msg.m_Header.m_MessageType;

		// Send the client connection Index
		ClientIndex& clientIndexLocation = *(ClientIndex*)&buffer[sizeof(AppID) + sizeof(MessageType)];
		clientIndexLocation = clientIndex;

		if (!IsConnectionManagementPacket(msg.m_Header.m_MessageType))
		{
			// Insert the sequence number + ack + ack_bitfield
			connection->m_ReliabilityContext.InsertReliabilitySegmentIntoPacket(&buffer[sizeof(AppID) + sizeof(MessageType)]);
		}

		// Optionally insert the payload
		if (msg.m_Header.m_PayloadSize > 0)
		{
			// Set the data
			memcpy(&buffer[k_PacketHeaderSize], msg.m_PayloadData.data(), msg.m_Header.m_PayloadSize);
		}

		m_ServerSocket.Send(connection->m_Address, buffer, msg.m_Header.m_PayloadSize + k_PacketHeaderSize);

		return true;
	}

	bool Server::SendToAllConnections(Message& msg, ClientIndex ignoreClient)
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
			appIDLocation = m_Config.m_AppProtocolID;

			// Set the packet type
			MessageType& MessageTypeLocation = *(MessageType*)&buffer[sizeof(AppID)];
			MessageTypeLocation = msg.m_Header.m_MessageType;

			if (!IsConnectionManagementPacket(msg.m_Header.m_MessageType))
			{
				// Insert the sequence number + ack + ack_bitfield
				connection.m_ReliabilityContext.InsertReliabilitySegmentIntoPacket(&buffer[sizeof(AppID) + sizeof(MessageType) + sizeof(ClientIndex)]);
			}

			if (msg.m_Header.m_PayloadSize > 0)
			{
				// Set the data
				memcpy(&buffer[k_PacketHeaderSize], msg.m_PayloadData.data(), msg.m_Header.m_PayloadSize);
			}

			m_ServerSocket.Send(connection.m_Address, buffer, k_PacketHeaderSize + msg.m_Header.m_PayloadSize);

			currentIndex++;
		}

		return true;
	}

	bool ServerService::Init()
	{
		// Get the server location type from the network config
		bool isLocal = Projects::ProjectService::GetActiveServerLocation() == ServerLocation::LocalMachine;

		// Create the new server context and initialize the server
		s_Server = CreateRef<Network::Server>();

		// Begin the server
#if 0
		if (!s_Server->StartServer(isLocal))
		{
			// Clean up network resources
			KG_WARN("Failed to start server");
			Terminate();
			return false;
		}
#endif

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
		//s_Server->StopServer();
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
			//s_Server->CheckForMessages();

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

		s_Server->m_NetworkEventQueue.SubmitEvent(e);

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

		s_Server->m_NetworkEventQueue.ProcessQueue();
	}
}

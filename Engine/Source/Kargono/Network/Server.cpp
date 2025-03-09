#include "kgpch.h"

#include "Kargono/Network/Server.h"

#include "Kargono/Projects/Project.h"
#include "Kargono/Core/Engine.h"

namespace Kargono::Network
{

	void ServerUDPConnection::AddMessageToIncomingMessageQueue()
	{
		if (!m_IPAddressToConnection.contains(m_CurrentEndpoint))
		{
			// This will adjust the udp endpoint for an ip address in case it is changed
			// TODO: I KNOW THIS IS A HUGE SECURITY PROBLEM, but yea idk any other way
			//		to make this work currently
			bool foundMatchingAddress = false;
			asio::ip::udp::endpoint foundEndpoint;
			Ref<ServerTCPConnection> clientConnect { nullptr };
			for (auto& [endpoint, connection] : m_IPAddressToConnection)
			{
				if (endpoint.address() == m_CurrentEndpoint.address())
				{
					foundMatchingAddress = true;
					foundEndpoint = endpoint;
					clientConnect = connection;
					break;
				}
			}

			if (!foundMatchingAddress)
			{
				KG_WARN("Address not found in m_IPAddressToConnection()");
				return;
			}

			// Replace the old UDP endpoint with the newly found endpoint
			clientConnect->SetUDPRemoteReceiveEndpoint(m_CurrentEndpoint);
			clientConnect->SetUDPRemoteSendEndpoint(m_CurrentEndpoint);
			m_IPAddressToConnection.erase(foundEndpoint);
			m_IPAddressToConnection.insert_or_assign(m_CurrentEndpoint, clientConnect);

		}

		// Add UDP message to the queue
		m_NetworkContextPtr->m_IncomingMessageQueue.PushBack({ m_IPAddressToConnection.at(m_CurrentEndpoint).get(), m_MessageCache});

		// Wake up the network thread to handle the message
		m_NetworkContextPtr->NetworkThreadWakeUp();
	}
	void ServerUDPConnection::Disconnect(asio::ip::udp::endpoint key)
	{
		// Obtain the connection associated with the endpoint
		if (!m_IPAddressToConnection.contains(key))
		{
			KG_WARN("Address does not resolve to a connection pointer in UDPServer Disconnect()");
			return;
		}
		Ref<ServerTCPConnection> connection = m_IPAddressToConnection.at(key);
		if (!connection)
		{
			KG_WARN("Invalid connection in UDP Disconnect()");
			return;
		}

		// Disconnect the connection
		connection->Disconnect();
	}

	ServerTCPConnection::ServerTCPConnection(NetworkContext* networkContext, asio::ip::tcp::socket&& socket)
		: TCPConnection(networkContext, std::move(socket))
	{
		// Construct validation check data
		CalculateValidationData();
	}
	void ServerTCPConnection::CalculateValidationData()
	{
		// Connection is Server -> Client, construct random data for the client to transform and send back for validation
		m_ValidationOutput = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());

		// Pre-calculate the result for checking when the client responds
		m_ValidationCache = GenerateValidationToken(m_ValidationOutput);
	}
	void ServerTCPConnection::Connect(uint32_t uid)
	{
		// Initialize UDP information and start validation process
		if (m_TCPSocket.is_open())
		{
			m_ID = uid;

			// Create the UDP endpoint information from the active TCP socket
			m_UDPLocalEndpoint = asio::ip::udp::endpoint(m_TCPSocket.local_endpoint().address(),
				m_TCPSocket.local_endpoint().port());
			m_UDPRemoteSendEndpoint = asio::ip::udp::endpoint(m_TCPSocket.remote_endpoint().address(),
				m_TCPSocket.remote_endpoint().port());
			m_UDPRemoteReceiveEndpoint = asio::ip::udp::endpoint(m_TCPSocket.remote_endpoint().address(),
				m_TCPSocket.remote_endpoint().port());

			// A client has attempted to connect to the server, but we want
			// the client to first validate itself, so first write out the
			// handshake data to be validated
			WriteValidationAsync();

			// Next, issue a task to sit and wait asynchronously for precisely
			// the validation data sent back from the client
			ReadValidationAsync();

		}
		else
		{
			KG_WARN("Attempt to create a TCP connection, however, the socket is invalid");
		}
	}

	void ServerTCPConnection::Disconnect()
	{
		if (IsConnected())
		{
			// Post a blocking request to close the connection
			asio::post(m_NetworkContextPtr->m_AsioContext, [this]()
			{
				// Close the TCP socket and wait for a response
				m_TCPSocket.close();

				// Wake up the network thread to handle the response
				m_NetworkContextPtr->NetworkThreadWakeUp();
			});
		}
		KG_INFO("Connection to a client has been terminated");
	}

	void ServerTCPConnection::AddMessageToIncomingMessageQueue()
	{
		// Add the message to the queue
		m_NetworkContextPtr->m_IncomingMessageQueue.PushBack({ this, m_MessageCache });

		// Wake up the network thread to handle the message
		m_NetworkContextPtr->NetworkThreadWakeUp();
	}

	void ServerTCPConnection::WriteValidationAsync()
	{
		// Write a validation message to the client. The client should respond with an appropriately transformed
		// message to complete validation
		asio::async_write(m_TCPSocket, asio::buffer(&m_ValidationOutput, sizeof(uint64_t)),
		[this](std::error_code ec, std::size_t length)
		{
			UNREFERENCED_PARAMETER(length);

			// Simply close the connection if we could not provide the validation message to the client
			if (ec)
			{
				KG_WARN("Error occurred while attempting to write a TCP validation message. Error Code: [{}] Message: {}", ec.value(), ec.message());
				m_TCPSocket.close();
			}
		});
	}
	void ServerTCPConnection::ReadValidationAsync()
	{
		// Handle validating the client message and ensuring the server & client know the same secrets
		asio::async_read(m_TCPSocket, asio::buffer(&m_ValidationInput, sizeof(uint64_t)),
			[&](std::error_code ec, std::size_t length)
			{
				UNREFERENCED_PARAMETER(length);

				// Close connection if error occurs during validation
				if (ec)
				{
					KG_WARN("Error occurred while attempting to read a TCP validation message. Error Code: [{}] Message: {}", ec.value(), ec.message());
					Disconnect();
					return;
				}

				// Check if client validation is successful
				if (m_ValidationInput == m_ValidationCache)
				{
					// Client has provided valid solution, so allow it to connect properly
					KG_INFO("Client Validation Successful");
					// TODO: Add post client validation back
					//server->OnClientValidated(this->shared_from_this());

					// Sit waiting to receive data now
					ReadMessageHeaderAsync();
				}
				else
				{
					// Close the connection if validation fails
					KG_WARN("Client failed validation");
					Disconnect();
				}
			});
	}

	Server::Server(uint16_t nPort)
		: m_TCPAcceptor(m_NetworkContext.m_AsioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), nPort))
	{
	}

	bool Server::StartServer(bool isLocal)
	{
		// Start the UDP server
		if (!StartUDPServer(isLocal))
		{
			KG_WARN("Failed to start server. UDP service failed to start.");
			return false;
		}

		// Prime the TCP context to listen for connections
		WaitForClientConnectionAsync();

		// Prime the UDP context to listen for messages
		m_UDPServer->StartReadingMessages();

		// Start the network thread so it can wait for the above message types
		m_NetworkContext.m_AsioThread = std::thread([this]()
		{ 
			asio::error_code errorCode;

			// Start the server's asio context
			m_NetworkContext.m_AsioContext.run(errorCode);

			// Handle asio context starting failures
			if (errorCode)
			{
				KG_WARN("Failed to start the server's asio context. Error Code: [{}] Message: {}", errorCode.value(),
					errorCode.message());
				this->StopServer();
				return;
			}
		});

		// The network thread should be running

		KG_INFO("[SERVER] Started!");
		return true;
	}

	void Server::CheckForMessages(size_t maxMessages)
	{
		// Clean up dead connections
		CheckConnectionsValid();

		// Allow the server main thread to sleep when no work is provided
		if (m_NetworkContext.m_IncomingMessageQueue.IsEmpty())
		{ 
			m_NetworkContext.NetworkThreadWakeUp();
		}

		// Handle all messages until message queue is empty or maxMessages limit is reached
		size_t messageCount = 0;
		while (messageCount < maxMessages && !m_NetworkContext.m_IncomingMessageQueue.IsEmpty())
		{
			// Grab the front message
			OwnedMessage msg = m_NetworkContext.m_IncomingMessageQueue.PopFront();

			// Pass to message handler
			OpenMessageFromClient(msg.m_RemoteConnection, msg.m_Message);

			messageCount++;
		}
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
		KG_INFO("[{}]: User Count Request", client->GetID());

		// Send return message to client
		SendReceiveClientCountMessage(client, (uint32_t)m_AllClientConnections.size());
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
	}

	void Server::SendTCPMessage(ServerTCPConnection* client, const Message& msg)
	{
		// Ensure the client is valid
		if (client && client->IsConnected())
		{
			// Send the message
			client->SendTCPMessage(msg);
		}
	}

	void Server::SendUDPMessage(ServerTCPConnection* client, Message& msg)
	{
		// Ensure the client is valid
		if (client && client->IsConnected())
		{
			// Wrap the message and send it
			LabeledMessage labeled{ client->GetUDPRemoteSendEndpoint(), msg };
			m_UDPServer->SendUDPMessage(labeled);
		}
	}



	void Server::SendTCPMessageAll(const Message& msg, ServerTCPConnection* ignoreClient)
	{
		for (Ref<ServerTCPConnection> client : m_AllClientConnections)
		{
			// Check client is connected...
			if (client && client->IsConnected())
			{
				// ..it is!
				if (client.get() != ignoreClient) 
				{ 
					client->SendTCPMessage(msg); 
				}
			}

		}
	}
	void Server::SendClientLeftMessageToAll(uint16_t removedClientSlot)
	{
		Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::ManageSession_ClientLeft;
		newMessage << removedClientSlot;

		// Notify all users in the same session that a client left
		for (auto& [clientID, connection] : m_OnlySession.GetAllClients())
		{
			connection->SendTCPMessage(newMessage);
		}
	}
	void Server::SendServerPingMessage(ServerTCPConnection* client, Kargono::Network::Message& msg)
	{
		// Return the message back to the client to calculate the client's ping
		KG_INFO("[{}]: Server Ping", client->GetID());
		client->SendTCPMessage(msg);
	}
	void Server::SendGenericMessageAllClients(ServerTCPConnection* sendingClient, Kargono::Network::Message& msg)
	{
		// Forward the message from the sending client to all other clients
		Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::GenericMessage_ServerMessage;
		newMessage << sendingClient->GetID();

		// Send message reliably to all other clients TCP
		SendTCPMessageAll(newMessage, sendingClient);
	}
	void Server::SendServerChatMessageAllClients(ServerTCPConnection* sendingClient, Kargono::Network::Message& msg)
	{
		// Forward to chat message to all other clients
		msg.m_Header.m_MessageType = MessageType::GenericMessage_ServerChat;
		msg << sendingClient->GetID();

		// Send message reliably to all other clients TCP
		SendTCPMessageAll(msg, sendingClient);
	}
	void Server::SendDenyClientJoinMessage(ServerTCPConnection* receivingClient)
	{
		// Create simple message notifying client of denial
		Message denyMessage;
		denyMessage.m_Header.m_MessageType = MessageType::ManageSession_DenyClientJoin;

		// Send message reliably with TCP
		receivingClient->SendTCPMessage(denyMessage);
	}
	void Server::SendApproveClientJoinMessage(ServerTCPConnection* receivingClient, uint16_t clientSlot)
	{
		// Create simple message nofiying the client that their join-session request was approved
		Message approveMessage;
		approveMessage.m_Header.m_MessageType = MessageType::ManageSession_ApproveClientJoin;
		approveMessage << clientSlot;

		// Send message reliably with TCP
		receivingClient->SendTCPMessage(approveMessage);
	}
	void Server::SendUpdateClientSlotMessage(ServerTCPConnection* receivingClient, uint16_t clientSlot)
	{
		// Create simple message notifying the client that a session-client slot has been changed
		Message updateSlotMessage;
		updateSlotMessage.m_Header.m_MessageType = MessageType::ManageSession_UpdateClientSlot;
		updateSlotMessage << clientSlot;

		// Send message reliably with TCP
		receivingClient->SendTCPMessage(updateSlotMessage);
	}
	void Server::SendReceiveClientCountMessage(ServerTCPConnection* receivingClient, uint32_t clientCount)
	{
		// Create simple return message to the client indicating the total count of clients on the server
		Message clientCountMessage;
		clientCountMessage.m_Header.m_MessageType = MessageType::ServerQuery_ReceiveClientCount;
		clientCountMessage << clientCount;

		// Send message reliably with TCP
		receivingClient->SendTCPMessage(clientCountMessage);
	}
	void Server::SendReceiveClientCountToAllMessage(ServerTCPConnection* ignoredClient, uint32_t clientCount)
	{
		// Send simple message to update the client count for all clients (except the ignored one)
		Message clientCountMessage;
		clientCountMessage.m_Header.m_MessageType = MessageType::ServerQuery_ReceiveClientCount;
		clientCountMessage << clientCount;

		// Send message reliably with TCP to *all clients
		SendTCPMessageAll(clientCountMessage, ignoredClient);
	}
	void Server::SendClientLeftMessage(ServerTCPConnection* receivingClient, uint16_t removedClientSlot)
	{
		// Create message notifying the indicated client that a client at the provided slot left the session
		Message clientLeftMessage;
		clientLeftMessage.m_Header.m_MessageType = MessageType::ManageSession_ClientLeft;
		clientLeftMessage << removedClientSlot;

		// Send message reliably with TCP
		receivingClient->SendTCPMessage(clientLeftMessage);
	}
	void Server::SendSyncPingMessage(ServerTCPConnection* receivingClient)
	{
		// Create message that sends a ping to synchronize clients inside a session
		Kargono::Network::Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::ManageSession_SyncPing;

		// Send message reliably with TCP
		receivingClient->SendTCPMessage(newMessage);
	}
	void Server::SendConfirmReadyCheckMessage(ServerTCPConnection* receivingClient, float waitTime)
	{
		// Create message that sends a notifies the client that the client that the ready check is done.
		// The client should wait the specified time and then handle the notification
		Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::ManageSession_ConfirmReadyCheck;
		newMessage << waitTime;

		// Send message reliably with TCP
		receivingClient->SendTCPMessage(newMessage);
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
		SendTCPMessage(receivingClient, msg);
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
		receivingClient->SendTCPMessage(msg);
	}
	void Server::SendSessionInitMessage(ServerTCPConnection* receivingClient)
	{
		// Create a message that indicates the start of the session. 
		// Server expects sync pings soon...
		Kargono::Network::Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::ManageSession_Init;

		// Send message reliably using TCP
		receivingClient->SendTCPMessage(newMessage);
	}
	void Server::StopServer()
	{
		// Close the UDP "connection" context
		if (m_UDPServer)
		{
			m_UDPServer->Stop();
			m_UDPServer.reset();
		}

		// Close every TCP connection to all clients
		for (Ref<ServerTCPConnection> connection : m_AllClientConnections)
		{
			if (connection->IsConnected())
			{
				connection->Disconnect();
			}
		}

		// Close the asio context and join its thread
		m_NetworkContext.m_AsioContext.stop();
		if (m_NetworkContext.m_AsioThread.joinable())
		{
			m_NetworkContext.m_AsioThread.join();
		}

		// Clean up all TCP connection objects
		for (Ref<ServerTCPConnection> connection : m_AllClientConnections)
		{
			if (connection->IsConnected())
			{
				connection.reset();
			}
		}
	}
	bool Server::StartUDPServer(bool isLocal)
	{
		asio::error_code errorCode;

		// Create a resolver
		asio::ip::tcp::resolver resolver(m_NetworkContext.m_AsioContext);

		// Query for the local host entry
		asio::ip::tcp::resolver::query query(asio::ip::host_name(), "");

		// Perform the resolution and get the endpoint iterator
		asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);

		// Handle endpoint resolution errors
		if (errorCode)
		{
			KG_WARN("Failed to resolve local server endpoint. Error Code: [{}] Message: {}", errorCode.value(),
				errorCode.message());
			return false;
		}

		while (iter != asio::ip::tcp::resolver::iterator())
		{
			if (iter->endpoint().address().is_v4())
			{
				break;
			}
			++iter;
		}
		KG_ASSERT(iter != asio::ip::tcp::resolver::iterator(), "Could not get local host ip address!");

		// Local udp socket
		asio::ip::udp::socket serverUDPSocket{ m_NetworkContext.m_AsioContext };
		// Create Server UDP socket
		asio::ip::tcp::endpoint localTCPEndpoint = m_TCPAcceptor.local_endpoint();
		asio::ip::udp::endpoint localUDPEndpoint;
		if (isLocal)
		{
			localUDPEndpoint = asio::ip::udp::endpoint(asio::ip::udp::v4(), localTCPEndpoint.port());
		}
		else
		{
			localUDPEndpoint = asio::ip::udp::endpoint(iter->endpoint().address(), localTCPEndpoint.port());
		}

		serverUDPSocket.open(localUDPEndpoint.protocol());
		serverUDPSocket.bind(localUDPEndpoint);

		// Handle UDP socket errors
		if (errorCode)
		{
			KG_WARN("Failed to create UDP socket. Error Code: [{}] Message: {}", errorCode.value(),
				errorCode.message());
			return false;
		}

		// Create the UDP server
		m_UDPServer = CreateRef<ServerUDPConnection>(&m_NetworkContext, std::move(serverUDPSocket), m_IPAddressToConnection);

		return true;
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

		uint32_t clientCount = (uint32_t)(m_AllClientConnections.size() + 1);

		// Let the new client know it has been connected
		SendAcceptConnectionMessage(client.get(), clientCount);

		// Update the client count for all other clients
		SendReceiveClientCountToAllMessage(client.get(), clientCount);

		return true;
	}
	void Server::OnClientDisconnect(Ref<Kargono::Network::ServerTCPConnection> client)
	{
		KG_INFO("Removing client [{}]", client->GetID());

		// Send a client count update indicating a client left
		SendReceiveClientCountToAllMessage(client.get(), (uint32_t)(m_AllClientConnections.size() - 1));

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

	void Server::WaitForClientConnectionAsync()
	{
		m_TCPAcceptor.async_accept([this](std::error_code errorCode, asio::ip::tcp::socket socket)
		{
			if (errorCode)
			{
				KG_WARN("New connection error. Error Code: [{}] Message: {}", errorCode.value(),
					errorCode.message());
				WaitForClientConnectionAsync();
				return;
			}

			// Create the new TCP connection
			KG_INFO("New Connection: {}", socket.remote_endpoint());
			Ref<ServerTCPConnection> newConnection = CreateRef<ServerTCPConnection>(&m_NetworkContext, std::move(socket));

			// Ensure the socket is not malformed
			if (errorCode)
			{
				KG_WARN("New TCP connection creation error. Error Code: [{}] Message: {}", errorCode.value(),
					errorCode.message());
				WaitForClientConnectionAsync();
				return;
			}

			// Give the server a chance to deny connection
			if (!OnClientConnect(newConnection))
			{
				KG_WARN("Connection Denied");
				WaitForClientConnectionAsync();
				return;
			}

			// Add the connection to the server context and connect
			m_AllClientConnections.push_back(std::move(newConnection));
			m_AllClientConnections.back()->Connect(m_ClientIDCounter);
			m_ClientIDCounter++;

			// Connection allowed, so add to IP -> Client Map
			m_IPAddressToConnection.insert_or_assign(m_AllClientConnections.back()->GetUDPRemoteReceiveEndpoint(), m_AllClientConnections.back());

			KG_INFO("[{}]: Connection Approved", m_AllClientConnections.back()->GetID());

			// Prime the asio context with more work - again simply wait for
			//		another connection...
			WaitForClientConnectionAsync();
		});
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

		// Alert thread to wake up and process event
		s_Server->m_NetworkContext.NetworkThreadWakeUp();
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

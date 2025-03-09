#include "kgpch.h"

#include "Kargono/Network/Client.h"

#include "Kargono/Core/Engine.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/Timers.h"


namespace Kargono::Network
{
	void ClientUDPConnection::Disconnect(asio::ip::udp::endpoint key)
	{
		UNREFERENCED_PARAMETER(key);

		// Check if TCP connection is valid since it is the connection we are trying to stop
		if (!m_ActiveTCPConnection)
		{
			KG_ERROR("Invalid connection in UDP Disconnect()");
			return;
		}

		// Disconnect TCP connection
		m_ActiveTCPConnection->Disconnect();
	}
	void ClientUDPConnection::AddMessageToIncomingMessageQueue()
	{
		// Add message to NetworkContext's incoming queue, alert thread to wake up and continue reading messages asynchronously
		m_NetworkContextPtr->m_IncomingMessageQueue.PushBack({ nullptr, m_MessageCache });
		m_NetworkContextPtr->NetworkThreadWakeUp();
	}
		
	bool ClientTCPConnection::Connect(const asio::ip::tcp::resolver::results_type& endpoints)
	{
		std::atomic<bool> connectionSuccessful = false;
		asio::error_code errorCode;

		// Start async connection (Prime the network context/thread with work to do)
		asio::async_connect(m_TCPSocket, endpoints, [&](std::error_code ec, const asio::ip::tcp::endpoint& endpoint)
		{
			UNREFERENCED_PARAMETER(endpoint);

			if (ec)
			{
				// Handle async_connect failure
				KG_WARN("Error while attempting to connect to server. Error Code: [{}] Message: {}", ec.value(), ec.message());
				Disconnect();
			}
			else
			{
				// Start waiting for server to send the validation sequence
				ReadValidationAsync();

				// Assume once we return from the ReadValidationAsync function, a determination has been made
				connectionSuccessful = true;
				m_NetworkContextPtr->NetworkThreadWakeUp();
			}
		});
		
		// Start Context Thread (Start immediately handling the async_connect function from above)
		m_NetworkContextPtr->m_AsioThread = std::thread([this]() { m_NetworkContextPtr->m_AsioContext.run(); });

		// Block this thread until a validation determination is made
		while (!m_NetworkContextPtr->m_QuitNetworkThread && !connectionSuccessful)
		{
			m_NetworkContextPtr->NetworkThreadSleep();
		}

		// Check for failure to connect
		if (!connectionSuccessful || m_NetworkContextPtr->m_QuitNetworkThread)
		{
			KG_WARN("Failed to create ClientTCPConnection connection.");
			return false;
		}

		// Initialize UDP endpoints for new UDP connnection
		m_UDPLocalEndpoint = asio::ip::udp::endpoint(m_TCPSocket.local_endpoint(errorCode).address(),
			m_TCPSocket.local_endpoint(errorCode).port());
		m_UDPRemoteSendEndpoint = asio::ip::udp::endpoint(m_TCPSocket.remote_endpoint().address(),
			m_TCPSocket.remote_endpoint().port());
		m_UDPRemoteReceiveEndpoint = asio::ip::udp::endpoint(m_TCPSocket.remote_endpoint().address(),
			m_TCPSocket.remote_endpoint().port());

		// Handle endpoint creation failures
		if (errorCode)
		{
			KG_WARN("Failed to create ClientTCPConnection connection. Error Code: [{}] Message: {}", errorCode.value(),
				errorCode.message());
			return false;
		}

		// TCP Connection successful!
		return true;
	}
	void ClientTCPConnection::Disconnect()
	{
		// Close the connection
		if (IsConnected())
		{
			// Close the tcp connection TODO: Maybe do this inside TCPConnection
			asio::post(m_NetworkContextPtr->m_AsioContext, [this]()
			{
				m_TCPSocket.close();
			});
		}
		KG_INFO("Connection to server has been terminated");

		// Notify the main thread and the network thread of the disconnect
		EngineService::SubmitToEventQueue(CreateRef<Events::ConnectionTerminated>());
		ClientService::SubmitToNetworkEventQueue(CreateRef<Events::ConnectionTerminated>());

	}

	void ClientTCPConnection::AddMessageToIncomingMessageQueue()
	{
		// Add the new message (added to the message cache asynchronously) to the incoming message queue
		m_NetworkContextPtr->m_IncomingMessageQueue.PushBack({ nullptr, m_MessageCache });

		// Wake up the network thread to handle the new message
		m_NetworkContextPtr->NetworkThreadWakeUp();
	}

	void ClientTCPConnection::WriteValidationAsync()
	{
		asio::async_write(m_TCPSocket, asio::buffer(&m_ValidationOutput, sizeof(uint64_t)),
			[this](std::error_code ec, std::size_t length)
		{
			UNREFERENCED_PARAMETER(length);

			// Check for async_write error
			if (ec)
			{
				KG_WARN("Error occurred while attempting to write a TCP validation message. Error Code: [{}] Message: {}", ec.value(), ec.message());
				m_TCPSocket.close();
				return;
			}
			
			ReadMessageHeaderAsync();
		});
	}
	void ClientTCPConnection::ReadValidationAsync()
	{
		asio::async_read(m_TCPSocket, asio::buffer(&m_ValidationInput, sizeof(uint64_t)),
			[&](std::error_code ec, std::size_t length)
		{
			UNREFERENCED_PARAMETER(length);

			// Check for async_read error
			if (ec)
			{
				KG_WARN("Error occurred while attempting to read a TCP validation message. Error Code: [{}] Message: {}", ec.value(), ec.message());
				Disconnect();
				return;
			}
				
			// Connection is a client, so solve puzzle
			m_ValidationOutput = GenerateValidationToken(m_ValidationInput);
			// Write the result
			WriteValidationAsync();
		});
	}


	bool Client::ConnectToServer(const std::string& serverIP, uint16_t serverPort, bool remote)
	{
		// Use this error code a lot
		asio::error_code errorCode;

		// Create TCP Socket for this new connection
		asio::ip::tcp::socket localSocket{m_NetworkContext.m_AsioContext};

		// Create proper local endpoint if a remote connection is desired
		if (remote)
		{
			// Resolve this computer's TCP endpoint
			bool success = ResolveLocalTCPEndpoint(localSocket, serverPort);
			if (!success)
			{
				KG_WARN("Failed to connect to server. Local endpoint determination failed.");
				return false;
			}
		}

		// Now we are looking for the server...

		// Resolve provided host/port into a TCP endpoint list for the server
		asio::ip::tcp::resolver resolver(m_NetworkContext.m_AsioContext);
		asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(serverIP, std::to_string(serverPort));

		// Create a new TCP connection object
		m_ClientTCPConnection = CreateRef<ClientTCPConnection>(&m_NetworkContext, std::move(localSocket));

		// Initialize connection to remote, server endpoint
		KG_INFO("Attempting TCP connection to server");
		if (!m_ClientTCPConnection->Connect(endpoints)) 
		{ 
			KG_INFO("Failed to initiate TCP connection to server");
			return false; 
		}
		KG_INFO("Successful TCP connection to server");

		// Create Local UDP Socket
		KG_INFO("Attempting UDP connection to server");
		asio::ip::udp::socket serverUDPSocket {m_NetworkContext.m_AsioContext};

		// Get Local UDP Endpoint created from Connect Function
		//	and bind it to the socket
		asio::ip::udp::endpoint& localEndpoint = m_ClientTCPConnection->GetUDPLocalEndpoint();
		serverUDPSocket.open(localEndpoint.protocol());
		if (errorCode)
		{
			// Handle .open failure
			KG_WARN("Connection to server failed. Error Code: [{}] Message: {}", errorCode.value(),
				errorCode.message());
			return false;
		}
		serverUDPSocket.bind(localEndpoint);
		if (errorCode)
		{
			// Handle .bind failure
			KG_WARN("Connection to server failed. Error Code: [{}] Message: {}", errorCode.value(),
				errorCode.message());
			return false;
		}

		// Create the UDP connection object and initiate the connection
		m_ClientUDPConnection = CreateRef<ClientUDPConnection>(&m_NetworkContext, std::move(serverUDPSocket), m_ClientTCPConnection);
		m_ClientUDPConnection->StartReadingMessages();

		// Send multiple UDP connection checks to ensure UDP "connection" is valid
		// (BTW, this function call exits immediately. This is handled asynchronously.)
		// As long as the TCP connection is valid, the connection will start. The connection
		// is withdrawn if these UDP checks fail
		Utility::AsyncBusyTimer::CreateRecurringTimer(0.3f, 10, [&]()
		{
			// Submit a message check to the network thread
			ClientService::SubmitToNetworkFunctionQueue([&]()
			{
				SendCheckUDPConnectionMessage();
			});

			// Wake up the network thread so it can process the function queue
			m_NetworkContext.NetworkThreadWakeUp();
		}, [&]()
		{
			// Final check to ensure the UDP connection test was successful
			ClientService::SubmitToNetworkFunctionQueue([&]()
			{
				if (!m_UDPConnectionSuccessful)
				{
					KG_WARN("UDP Connection Unsuccessful!");
					DisconnectFromServer();
					return;
				}
				KG_INFO("UDP Connection Successful!");
			});

			// Wake up the network thread so it can process the function queue
			m_NetworkContext.NetworkThreadWakeUp();
		});

		return true;
	}

	void Client::DisconnectFromServer()
	{
		// Check if already connected
		if (IsConnectedToServer())
		{
			m_ClientTCPConnection->Disconnect();
		}

		// Close asio context, thread, and TCP connection
		m_NetworkContext.m_AsioContext.stop();
		if (m_NetworkContext.m_AsioThread.joinable())
		{
			m_NetworkContext.m_AsioThread.join();
		}
		m_ClientTCPConnection.reset();
	}

	bool Client::IsConnectedToServer()
	{
		// Check if TCP is valid. UDP is not necessary. (Not really a "connection" anyways)
		if (m_ClientTCPConnection) 
		{ 
			return m_ClientTCPConnection->IsConnected();
		}
		return false;
	}

	bool Client::ResolveLocalTCPEndpoint(asio::ip::tcp::socket& localSocket, uint16_t serverPort)
	{
		asio::error_code errorCode;

		// Get local TCP address and port from localhost
		asio::ip::tcp::resolver resolver(m_NetworkContext.m_AsioContext);
		asio::ip::tcp::resolver::query query(asio::ip::host_name(errorCode), "");
		asio::ip::tcp::resolver::iterator iter = resolver.resolve(query, errorCode);

		// Handle resolving failures
		if (errorCode)
		{
			// Handle .resolve failure
			KG_WARN("Failed to resolve local TCP endpoint. Error Code: [{}] Message: {}", errorCode.value(),
				errorCode.message());
			return false;
		}

		// Search for IPV4 address
		while (iter != asio::ip::tcp::resolver::iterator())
		{
			if (iter->endpoint().address().is_v4())
			{
				break;
			}
			iter++;
		}

		// Create local endpoint, bind IPV4 address to new socket,
		// and assign the local endpoint with (serverPort - 100)
		KG_ASSERT(serverPort > 100, "Port must be greater than 100! We need to use multiple ports below in indicated port.");

		asio::ip::tcp::endpoint localTCPEndpoint = asio::ip::tcp::endpoint
		(
			iter->endpoint().address(),
			serverPort - 100
		);

		// Bind the endpoint
		localSocket.open(localTCPEndpoint.protocol(), errorCode);
		localSocket.bind(localTCPEndpoint);
		// Handle binding failures
		if (errorCode)
		{
			KG_WARN("Failed to resolve local TCP endpoint. Error Code: [{}] Message: {}", errorCode.value(),
				errorCode.message());
			return false;
		}

		return true;
	}

	void Client::CheckMessagesFromServer(size_t maxMessages)
	{
		// Sleep the network thread if there is no messages to process
		if (m_NetworkContext.m_IncomingMessageQueue.IsEmpty())
		{ 
			m_NetworkContext.NetworkThreadSleep();
		}

		// Process messages until the message queue is empty or the maximum process amount is reached
		// (By-the-way, the max message count ensures the other network thread functions have a chance to be processed)
		size_t messageCount = 0;
		while (messageCount < maxMessages && !m_NetworkContext.m_IncomingMessageQueue.IsEmpty())
		{
			// Grab the front message
			Network::OwnedMessage msg = m_NetworkContext.m_IncomingMessageQueue.PopFront();

			// Pass to message handler
			OpenMessageFromServer(msg.m_Message);

			// Continue
			messageCount++;
		}
	}

	void Client::SendTCPToServer(const Message& msg)
	{
		if (IsConnectedToServer())
		{
			m_ClientTCPConnection->SendTCPMessage(msg);
		}
	}

	void Client::SendUDPToServer(Message& msg)
	{
		if (IsConnectedToServer())
		{
			LabeledMessage labeled{ m_ClientTCPConnection->GetUDPRemoteSendEndpoint(), msg };
			m_ClientUDPConnection->SendUDPMessage(labeled);
		}
	}

	void Client::SendChatToServer(const std::string& text)
	{
		Kargono::Network::Message msg;
		msg.m_Header.m_MessageType = MessageType::GenericMessage_ClientChat;
		msg.AppendPayload((void*)text.data(), text.size());
		SendTCPToServer(msg);
	}

	void Client::SendRequestUserCountMessage()
	{
		Kargono::Network::Message msg;
		msg.m_Header.m_MessageType = MessageType::ServerQuery_RequestClientCount;
		SendTCPToServer(msg);
	}

	void Client::SendAllEntityLocation(Events::SendAllEntityLocation& event)
	{
		Kargono::Network::Message msg;

		// Fill the message with the entity ID and location information
		msg.m_Header.m_MessageType = MessageType::ManageSceneEntity_SendAllClientsLocation;
		msg << event.GetEntityID();
		Math::vec3 translation = event.GetTranslation();
		msg << translation.x;
		msg << translation.y;
		msg << translation.z;

		// Send the message quickly with UDP
		SendUDPToServer(msg);
	}

	void Client::SendInitSyncPingMessage()
	{
		// Send a simple init sync ping message
		Kargono::Network::Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::ManageSession_SyncPing;
		// Send the message reliably with TCP
		SendTCPToServer(newMessage);
	}

	void Client::SendRequestJoinSessionMessage()
	{
		// Send a message that prompts the server to allow the client to join.
		// Should receive either an accept or deny message back.
		Kargono::Network::Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageSession_RequestClientJoin;

		// Send the message reliably with TCP
		SendTCPToServer(msg);
	}

	void Client::SendEnableReadyCheckMessage()
	{
		// Send the message to signify this client is ready
		Kargono::Network::Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageSession_EnableReadyCheck;

		// Send the message reliably with TCP
		SendTCPToServer(msg);
	}

	void Client::SendSessionReadyCheckMessage()
	{
		// Send a message to the server initiating a ready check
		Kargono::Network::Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageSession_StartReadyCheck;

		// Send the message reliably with TCP
		SendTCPToServer(msg);
	}

	void Client::SendAllClientsSignalMessage(Events::SignalAll& event)
	{
		// Send a scripting specific signal to all other clients
		Kargono::Network::Message msg;
		msg.m_Header.m_MessageType = MessageType::ScriptMessaging_SendAllClientsSignal;
		msg << event.GetSignal();

		// Send the message reliably with TCP
		SendTCPToServer(msg);
	}

	void Client::SendKeepAliveMessage()
	{
		// Send a simple message that keeps the UDP "connection" alive
		Kargono::Network::Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageConnection_KeepAlive;

		// Send the message quickly with UDP
		SendUDPToServer(msg);
	}

	void Client::SendAllEntityPhysicsMessage(Events::SendAllEntityPhysics& event)
	{
		// Create/send a message that updates all other clients with a particular entity's 
		// ID, location, and velocity.
		Kargono::Network::Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageSceneEntity_SendAllClientsPhysics;
		msg << event.GetEntityID();
		Math::vec3 translation = event.GetTranslation();
		Math::vec2 linearVelocity = event.GetLinearVelocity();
		msg << translation.x;
		msg << translation.y;
		msg << translation.z;
		msg << linearVelocity.x;
		msg << linearVelocity.y;

		// Send the message quickly with UDP
		SendUDPToServer(msg);
	}

	void Client::SendLeaveCurrentSessionMessage()
	{
		// Send a message notifying the server and all other clients that this client is leaving
		Kargono::Network::Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageSession_NotifyAllLeave;

		// Send the message reliably with TCP
		SendTCPToServer(msg);

		// Ensure current application/client is aware our session is invalid
		m_SessionSlot = std::numeric_limits<uint16_t>::max();
	}

	void Client::SendCheckUDPConnectionMessage()
	{
		// Create/send a message to the server that ensures the UDP connection is valid
		// Expecting a message back from the server
		Message newMessage{};
		newMessage.m_Header.m_MessageType = MessageType::ManageConnection_CheckUDPConnection;

		// Send the message using UDP
		SendUDPToServer(newMessage);
	}

	void Client::OpenMessageFromServer(Kargono::Network::Message& msg)
	{
		// Process message from server based on type
		switch (msg.m_Header.m_MessageType)
		{
		case MessageType::ManageConnection_AcceptConnection:
			OpenAcceptConnectionMessage(msg);
			break;
		case MessageType::ServerQuery_ReceiveClientCount:
			OpenReceiveUserCountMessage(msg);
			break;
		case MessageType::GenericMessage_ServerChat:
			OpenServerChatMessage(msg);
			break;
		case MessageType::ManageSession_ApproveClientJoin:
			OpenApproveJoinSessionMessage(msg);
			break;
		case MessageType::ManageSession_UpdateClientSlot:
			OpenUpdateSessionUserSlotMessage(msg);
			break;
		case MessageType::ManageSession_ClientLeft:
			OpenUserLeftSessionMessage(msg);
			break;
		case MessageType::ManageSession_DenyClientJoin:
			OpenDenyJoinSessionMessage(msg);
			break;
		case MessageType::ManageSession_Init:
			OpenCurrentSessionInitMessage(msg);
			break;
		case MessageType::ManageSession_SyncPing:
			OpenInitSyncPingMessage(msg);
			break;
		case MessageType::ManageSession_StartSession:
			OpenStartSessionMessage(msg);
			break;
		case MessageType::ManageSession_ConfirmReadyCheck:
			OpenSessionReadyCheckConfirmMessage(msg);
			break;
		case MessageType::ManageSceneEntity_UpdateLocation:
			OpenUpdateEntityLocationMessage(msg);
			break;
		case MessageType::ManageSceneEntity_UpdatePhysics:
			OpenUpdateEntityPhysicsMessage(msg);
			break;
		case MessageType::ScriptMessaging_ReceiveSignal:
			OpenReceiveSignalMessage(msg);
			break;
		case MessageType::ManageConnection_KeepAlive:
			OpenKeepAliveMessage(msg);
			break;
		case MessageType::ManageConnection_CheckUDPConnection:
			OpenUDPInitMessage(msg);
			break;
		default:
			KG_ERROR("Invalid message type received from the server!");
			break;
		}
	}

	void Client::OpenAcceptConnectionMessage(Kargono::Network::Message& msg)
	{
		KG_INFO("Connection to the server has been accepted!");

		// Pass the event along to the main thread
		uint32_t userCount{};
		msg >> userCount;
		EngineService::SubmitToEventQueue(CreateRef<Events::ReceiveOnlineUsers>(userCount));
	}

	void Client::OpenReceiveUserCountMessage(Message& msg)
	{
		// Pass the event along to the main thread
		uint32_t userCount{};
		msg >> userCount;
		EngineService::SubmitToEventQueue(CreateRef<Events::ReceiveOnlineUsers>(userCount));
	}

	void Client::OpenServerChatMessage(Message& msg)
	{
		// Parse the string from the message
		uint32_t clientID{};
		std::vector<uint8_t> data{};
		uint64_t dataSize{};
		msg >> clientID;
		msg >> dataSize;
		data = msg.GetPayloadCopy(dataSize);
		std::string text((char*)(data.data()), dataSize);

		// Simply log the message
		// TODO: Maybe there something else we can do here?
		KG_INFO("[{}]: {}", clientID, text);
	}

	void Client::OpenApproveJoinSessionMessage(Message& msg)
	{
		KG_INFO("Approved joining session");

		// Get the indicated user slot
		uint16_t userSlot{};
		msg >> userSlot;
		m_SessionSlot = userSlot;

		// Pass the event along to the main thread
		EngineService::SubmitToEventQueue(CreateRef<Events::ApproveJoinSession>(userSlot));
	}

	void Client::OpenUpdateSessionUserSlotMessage(Message& msg)
	{
		// Get the indicated user slot
		uint16_t userSlot{};
		msg >> userSlot;

		// Pass the event along to the main thread
		EngineService::SubmitToEventQueue(CreateRef<Events::UpdateSessionUserSlot>(userSlot));
	}

	void Client::OpenUserLeftSessionMessage(Message& msg)
	{
		KG_INFO("A User Left the Current Session");

		// Get the indicated user slot
		uint16_t userSlot{};
		msg >> userSlot;

		// Pass the event along to the main thread
		EngineService::SubmitToEventQueue(CreateRef<Events::UserLeftSession>(userSlot));
	}

	void Client::OpenDenyJoinSessionMessage(Message& msg)
	{
		KG_INFO("Denied joining session");
		// TODO: Maybe log this?
	}

	void Client::OpenCurrentSessionInitMessage(Message& msg)
	{
		KG_INFO("Active Session is initializing");

		// Pass the event along to the main thread
		EngineService::SubmitToEventQueue(CreateRef<Events::CurrentSessionInit>());
	}

	void Client::OpenInitSyncPingMessage(Message& msg)
	{
		// Continue to pass the sync ping back to the server until the server
		// determines the connection is stable
		SendInitSyncPingMessage();
	}

	void Client::OpenStartSessionMessage(Message& msg)
	{
		KG_INFO("Session Started");

		// Get the wait time indicated by the server
		float waitTime{};
		msg >> waitTime;

		// Wait for the time indicated by the server start the session.
		// This is meant to keep all clients in sync at the start of the session.
		Utility::AsyncBusyTimer::CreateTimer(waitTime, [&]()
		{
			// Ensure the active network context is aware of the session starting
			Network::ClientService::SubmitToNetworkEventQueue(CreateRef<Events::StartSession>());

			// Pass the event along to the main thread
			EngineService::SubmitToEventQueue(CreateRef<Events::StartSession>());
		});
	}

	void Client::OpenSessionReadyCheckConfirmMessage(Message& msg)
	{
		// Get the wait time indicated by the server
		float waitTime{};
		msg >> waitTime;

		// Wait for the time indicated by the server to handle the ready check.
		// This keeps the confirm events in sync between clients.
		Utility::AsyncBusyTimer::CreateTimer(waitTime, [&]()
		{
			// Pass the event along to the main thread
			EngineService::SubmitToEventQueue(CreateRef<Events::SessionReadyCheckConfirm>());
		});
	}

	void Client::OpenUpdateEntityLocationMessage(Message& msg)
	{
		// Get the entity ID and its location information
		uint64_t id;
		float x, y, z;
		msg >> z;
		msg >> y;
		msg >> x;
		msg >> id;
		Math::vec3 trans{ x, y, z };

		// Pass the event along to the main thread
		EngineService::SubmitToEventQueue(CreateRef<Events::UpdateEntityLocation>(id, trans));
	}

	void Client::OpenUpdateEntityPhysicsMessage(Message& msg)
	{
		// Get the entity ID, location information, and physics velocity
		uint64_t id;
		float x, y, z, linx, liny;
		msg >> liny;
		msg >> linx;
		msg >> z;
		msg >> y;
		msg >> x;
		msg >> id;
		Math::vec3 trans{ x, y, z };
		Math::vec2 linearV{ linx, liny };

		// Pass the event along to the main thread
		EngineService::SubmitToEventQueue(CreateRef<Events::UpdateEntityPhysics>(id, trans, linearV));
	}

	void Client::OpenReceiveSignalMessage(Message& msg)
	{
		// Get the indicated signal (depends on the application)
		uint16_t signal{};
		msg >> signal;

		// Pass the event along to the main thread
		EngineService::SubmitToEventQueue(CreateRef<Events::ReceiveSignal>(signal));
	}

	void Client::OpenKeepAliveMessage(Message& msg)
	{
		UNREFERENCED_PARAMETER(msg);

		// Nothing to do here...
	}

	void Client::OpenUDPInitMessage(Message& msg)
	{
		UNREFERENCED_PARAMETER(msg);

		// Receive final message from server indicated UDP connection was successful
		// The initialization context will check this value
		m_UDPConnectionSuccessful = true;
	}

	void ClientService::Init()
	{
		// Create new client
		s_Client = CreateRef<Network::Client>();

		// Start the network thread in the client and call the Run function
		s_Client->m_NetworkContext.m_NetworkThread = CreateRef<std::thread>(&Network::ClientService::Run);

		// Exit out immediately (the network thread is now running separately from the main thread)
		KG_VERIFY(s_Client, "Client connection init");
	}

	void ClientService::Terminate()
	{
		// Request for the current client's network thread to stop
		EndRun();

		// Wait for the network thread to stop and close the client
		s_Client->m_NetworkContext.m_NetworkThread->join();
		s_Client->m_NetworkContext.m_NetworkThread.reset();
		s_Client.reset();

		KG_VERIFY(!s_Client, "Closed client connection");
	}

	void ClientService::Run()
	{
		// Decide whether to connect using the local network or through the internet
		bool remoteConnection = false;
		if (Projects::ProjectService::GetActiveServerLocation() != ServerLocation::LocalMachine)
		{
			remoteConnection = true;
		}

		// Start connection to server
		bool connectionSuccess = s_Client->ConnectToServer
		(
			Utility::IPv4ToString(Projects::ProjectService::GetActiveServerIP()),
			Projects::ProjectService::GetActiveServerPort(), 
			remoteConnection
		);

		// Ensure the connection has started
		if (!connectionSuccess) 
		{ 
			KG_WARN("Failed to connect to the server");

			// Ensure the run loop never starts
			s_Client->m_NetworkContext.m_QuitNetworkThread = true; 
		}

		// Ensure UDP connection is valid
		if (s_Client->m_ClientUDPConnection && s_Client->IsConnectedToServer())
		{
			// Start timer for keep alive packets
			EngineService::SubmitToEventQueue(CreateRef<Events::AddTickGeneratorUsage>(k_KeepAliveDelay));
		}

		KG_INFO("Started client networking thread");

		// Main NetworkContext::NetworkThread loop
		while (!s_Client->m_NetworkContext.m_QuitNetworkThread)
		{
			// Validate the client is still connected
			if (s_Client->IsConnectedToServer())
			{
				// Process any new events/functions for the network thread
				ClientService::ProcessEventQueue();
				ClientService::ProcessFunctionQueue();

				// Check for a maximum of 5 messages and run another loop iteration if message queue is not empty
				s_Client->CheckMessagesFromServer(5);
			}
			else
			{
				// Exit the network loop
				s_Client->m_NetworkContext.m_QuitNetworkThread = true;
			}
		}

		// Close connection to server
		s_Client->DisconnectFromServer();

		// Ensure the UDP tick generator is removed
		if (s_Client->m_ClientUDPConnection)
		{
			KG_INFO("UDP connection tick generator has been removed.");
			EngineService::SubmitToEventQueue(CreateRef<Events::RemoveTickGeneratorUsage>(k_KeepAliveDelay));
		}
		KG_INFO("Closed client networking thread");
	}

	void ClientService::EndRun()
	{
		// TODO: This could be a location of an error. I am not sure though.
		// There may be a chance that the network thread could be notified while its running
		// and it goes to sleep without closing the thread. I will check this later.

		// Set run loop to close
		s_Client->m_NetworkContext.m_QuitNetworkThread = true;

		// Wake up the network thread, so it can close
		s_Client->m_NetworkContext.NetworkThreadWakeUp();
		KG_INFO("Notified client network thread to close");
	}

	uint16_t ClientService::GetActiveSessionSlot()
	{
		// Simply return the session slot if the client context lives
		if (s_Client)
		{
			return s_Client->m_SessionSlot;
		}

		// Client is unavailable so send error response
		return k_InvalidSessionSlot;
	}

	void ClientService::SendAllEntityLocation(UUID entityID, Math::vec3 location)
	{
		// Ensure network context is valid
		if (!s_Client)
		{
			KG_WARN("Attempt to interact with the client network context, but it is not valid");
			return;
		}

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::SendAllEntityLocation>(entityID, location));
	}

	void ClientService::SendAllEntityPhysics(UUID entityID, Math::vec3 translation, Math::vec2 linearVelocity)
	{
		// Ensure network context is valid
		if (!s_Client)
		{
			KG_WARN("Attempt to interact with the client network context, but it is not valid");
			return;
		}

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::SendAllEntityPhysics>(entityID, translation, linearVelocity));
	}

	void ClientService::EnableReadyCheck()
	{
		// Ensure network context is valid
		if (!s_Client)
		{
			KG_WARN("Attempt to interact with the client network context, but it is not valid");
			return;
		}

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::EnableReadyCheck>());
	}

	void ClientService::SessionReadyCheck()
	{
		// Ensure network context is valid
		if (!s_Client)
		{
			KG_WARN("Attempt to interact with the client network context, but it is not valid");
			return;
		}

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::SessionReadyCheck>());
	}

	void ClientService::RequestUserCount()
	{
		// Ensure network context is valid
		if (!s_Client)
		{
			KG_WARN("Attempt to interact with the client network context, but it is not valid");
			return;
		}

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::RequestUserCount>());
	}

	void ClientService::RequestJoinSession()
	{
		// Ensure network context is valid
		if (!s_Client)
		{
			KG_WARN("Attempt to interact with the client network context, but it is not valid");
			return;
		}

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::RequestJoinSession>());
	}

	void ClientService::LeaveCurrentSession()
	{
		// Ensure network context is valid
		if (!s_Client)
		{
			KG_WARN("Attempt to interact with the client network context, but it is not valid");
			return;
		}

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::LeaveCurrentSession>());
	}

	void ClientService::SignalAll(uint16_t signal)
	{
		// Ensure network context is valid
		if (!s_Client)
		{
			KG_WARN("Attempt to interact with the client network context, but it is not valid");
			return;
		}

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::SignalAll>(signal));
	}
	void ClientService::SubmitToNetworkFunctionQueue(const std::function<void()>& function)
	{
		// Ensure the network context is valid
		if (!s_Client)
		{
			KG_WARN("Attempt to interact with the client network context, but it is not valid");
			return;
		}
		// Obtain the function queue lock
		std::scoped_lock<std::mutex> lock(s_Client->m_FunctionQueueMutex);

		// Add the indicated function and alert the network thread to wake up
		s_Client->m_FunctionQueue.emplace_back(function);
		s_Client->m_NetworkContext.NetworkThreadWakeUp();
	}
	void ClientService::SubmitToNetworkEventQueue(Ref<Events::Event> e)
	{
		// Ensure the active client is valid
		if (!s_Client)
		{
			KG_WARN("Attempt to submit an event to the network thread, however, the client context is invalid");
			return;
		}

		// Obtain a lock on the network thread's event mutex
		std::scoped_lock<std::mutex> lock(s_Client->m_EventQueueMutex);

		// Add the indicated event and alert the network thread to wake up
		s_Client->m_EventQueue.emplace_back(e);
		s_Client->m_NetworkContext.NetworkThreadWakeUp();
	}
	void ClientService::OnEvent(Events::Event* e)
	{
		// Process events based on their type
		switch (e->GetEventType())
		{
		case Events::EventType::RequestJoinSession:
			OnRequestJoinSession(*(Events::RequestJoinSession*)e);
			break;
		case Events::EventType::RequestUserCount:
			OnRequestUserCount(*(Events::RequestUserCount*)e);
			break;
		case Events::EventType::LeaveCurrentSession:
			OnLeaveCurrentSession(*(Events::LeaveCurrentSession*)e);
			break;
		case Events::EventType::StartSession:
			OnStartSession(*(Events::StartSession*)e);
			break;
		case Events::EventType::ConnectionTerminated:
			OnConnectionTerminated(*(Events::ConnectionTerminated*)e);
			break;
		case Events::EventType::EnableReadyCheck:
			OnEnableReadyCheck(*(Events::EnableReadyCheck*)e);
			break;
		case Events::EventType::SendReadyCheck:
			OnSessionReadyCheck(*(Events::SessionReadyCheck*)e);
			break;
		case Events::EventType::SendAllEntityLocation:
			OnSendAllEntityLocation(*(Events::SendAllEntityLocation*)e);
			break;
		case Events::EventType::SendAllEntityPhysics:
			OnSendAllEntityPhysics(*(Events::SendAllEntityPhysics*)e);
			break;
		case Events::EventType::SignalAll:
			OnSignalAll(*(Events::SignalAll*)e);
			break;
		case Events::EventType::AppTick:
			OnAppTickEvent(*(Events::AppTickEvent*)e);
			break;
		}
	}
	bool ClientService::OnRequestUserCount(Events::RequestUserCount event)
	{
		s_Client->SendRequestUserCountMessage();
		return true;
	}
	bool ClientService::OnStartSession(Events::StartSession event)
	{
		s_Client->m_SessionStartFrame = EngineService::GetActiveEngine().GetUpdateCount();
		return true;
	}
	bool ClientService::OnConnectionTerminated(Events::ConnectionTerminated event)
	{
		// Essentially clear all session information from this client context
		s_Client->m_SessionSlot = k_InvalidSessionSlot;
		s_Client->m_SessionStartFrame = 0;
		return true;
	}
	bool ClientService::OnRequestJoinSession(Events::RequestJoinSession event)
	{
		s_Client->SendRequestJoinSessionMessage();
		return true;
	}
	bool ClientService::OnEnableReadyCheck(Events::EnableReadyCheck event)
	{
		s_Client->SendEnableReadyCheckMessage();
		return true;
	}
	bool ClientService::OnSessionReadyCheck(Events::SessionReadyCheck event)
	{
		s_Client->SendSessionReadyCheckMessage();
		return true;
	}
	bool ClientService::OnSendAllEntityLocation(Events::SendAllEntityLocation event)
	{
		s_Client->SendAllEntityLocation(event);
		return true;
	}
	bool ClientService::OnSignalAll(Events::SignalAll event)
	{
		s_Client->SendAllClientsSignalMessage(event);
		return true;
	}
	bool ClientService::OnAppTickEvent(Events::AppTickEvent event)
	{
		// Only send the keep alive message if it matches the delay time
		if (event.GetDelayMilliseconds() == k_KeepAliveDelay)
		{
			s_Client->SendKeepAliveMessage();
			return true;
		}
		return false;
	}
	bool ClientService::OnSendAllEntityPhysics(Events::SendAllEntityPhysics event)
	{
		s_Client->SendAllEntityPhysicsMessage(event);
		return true;
	}
	bool ClientService::OnLeaveCurrentSession(Events::LeaveCurrentSession event)
	{
		s_Client->SendLeaveCurrentSessionMessage();
		return true;
	}
	void ClientService::ProcessFunctionQueue()
	{
		KG_PROFILE_FUNCTION();

		std::vector<std::function<void()>> localFunctionCache;

		// Ensure the function queue doesn't become invalidated if one of the functions modifies the queue
		// inside the loop
		{
			// Obtain the function queue lock
			std::scoped_lock<std::mutex> lock(s_Client->m_FunctionQueueMutex);
			
			// Move the event queue into a local variable and reset the queue
			localFunctionCache = std::move(s_Client->m_FunctionQueue);

			// Reset the queue
			s_Client->m_FunctionQueue.clear();
		}


		// Call all of the functions in the queue
		for (std::function<void()>& func : localFunctionCache) 
		{ 
			func(); 
		}

		
	}
	void ClientService::ProcessEventQueue()
	{
		KG_PROFILE_FUNCTION();
		std::vector<Ref<Events::Event>> localEventCache;

		// Ensure the event queue doesn't become invalidated if one of the events modifies the queue
		// inside the loop
		{
			// Obtain the event queue lock for this small scope
			std::scoped_lock<std::mutex> lock(s_Client->m_EventQueueMutex);

			// Move the event queue into a local variable and reset the queue
			localEventCache = std::move(s_Client->m_EventQueue);
			s_Client->m_EventQueue.clear();
		}

		// Process all of the events in the event queue
		for (Ref<Kargono::Events::Event> event : localEventCache)
		{
			OnEvent(event.get());
		}
	}
}

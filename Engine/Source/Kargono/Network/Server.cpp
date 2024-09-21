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

			clientConnect->SetUDPRemoteReceiveEndpoint(m_CurrentEndpoint);
			clientConnect->SetUDPRemoteSendEndpoint(m_CurrentEndpoint);
			m_IPAddressToConnection.erase(foundEndpoint);
			m_IPAddressToConnection.insert_or_assign(m_CurrentEndpoint, clientConnect);

		}

		m_NetworkContextPtr->IncomingMessagesQueue.PushBack({ m_IPAddressToConnection.at(m_CurrentEndpoint).get(), m_MessageCache});
		NetworkThreadWakeUp();
		ReadMessageAsync();
	}
	void ServerUDPConnection::Disconnect(asio::ip::udp::endpoint key)
	{

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

		connection->Disconnect();
	}

	ServerTCPConnection::ServerTCPConnection(NetworkContext* networkContext, asio::ip::tcp::socket&& socket)
		: TCPConnection(networkContext, std::move(socket))
	{

		// Construct validation check data

		// Connection is Server -> Client, construct random data for the client to transform and send back for validation
		m_ValidationOutput = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());

		// Pre-calculate the result for checking when the client responds
		m_ValidationCache = GenerateValidationToken(m_ValidationOutput);


	}
	void ServerTCPConnection::Connect(uint32_t uid)
	{
		if (m_TCPSocket.is_open())
		{
			m_ID = uid;

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
	}

	void ServerTCPConnection::Disconnect()
	{
		if (IsConnected())
		{
			asio::post(m_NetworkContextPtr->AsioContext, [this]()
			{
				m_TCPSocket.close();
				NetworkThreadWakeUp();
			});
		}
		KG_INFO("Connection to a client has been terminated");
	}

	void ServerTCPConnection::AddMessageToIncomingMessageQueue()
	{
		m_NetworkContextPtr->IncomingMessagesQueue.PushBack({ this, m_MessageCache });
		NetworkThreadWakeUp();
		ReadMessageHeaderAsync();
	}

	void ServerTCPConnection::WriteValidationAsync()
	{
		asio::async_write(m_TCPSocket, asio::buffer(&m_ValidationOutput, sizeof(uint64_t)),
			[this](std::error_code ec, std::size_t length)
			{
				if (ec)
				{
					KG_WARN("Error occurred while attempting to write a TCP validation message. Error Code: [{}] Message: {}", ec.value(), ec.message());
					m_TCPSocket.close();
				}
			});
	}
	void ServerTCPConnection::ReadValidationAsync()
	{
		asio::async_read(m_TCPSocket, asio::buffer(&m_ValidationInput, sizeof(uint64_t)),
			[&](std::error_code ec, std::size_t length)
			{
				if (ec)
				{
					KG_WARN("Error occurred while attempting to read a TCP validation message. Error Code: [{}] Message: {}", ec.value(), ec.message());
					Disconnect();
					return;
				}

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
					KG_WARN("Client failed validation");
					Disconnect();
				}
			});
	}

	Server::Server(uint16_t nPort, bool isLocal)
		: m_TCPAcceptor(m_NetworkContext.AsioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), nPort))
	{
		// Create a resolver
		asio::ip::tcp::resolver resolver(m_NetworkContext.AsioContext);

		// Query for the local host entry
		asio::ip::tcp::resolver::query query(asio::ip::host_name(), "");

		// Perform the resolution and get the endpoint iterator
		asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
		while (iter != asio::ip::tcp::resolver::iterator())
		{
			if (iter->endpoint().address().is_v4()) { break; }
			++iter;
		}
		KG_ASSERT(iter != asio::ip::tcp::resolver::iterator(), "Could not get local host ip address!");

		// Local udp socket
		asio::ip::udp::socket serverUDPSocket {m_NetworkContext.AsioContext};
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
		m_UDPServer = CreateRef<ServerUDPConnection>(&m_NetworkContext, std::move(serverUDPSocket), m_IPAddressToConnection);
	}

	bool Server::StartServer()
	{
		try
		{
			WaitForClientConnectionAsync();
			m_UDPServer->Start();

			m_NetworkContext.AsioThread = std::thread([this]()
			{ 
				m_NetworkContext.AsioContext.run();
			});
		}
		catch (std::exception& e)
		{
			std::cerr << "[SERVER] Exception: " << e.what() << '\n';
			return false;
		}

		KG_INFO("[SERVER] Started!");
		return true;
	}

	void Server::CheckForMessages(size_t nMaxMessages)
	{
		CheckConnectionsValid();
		if (m_NetworkContext.IncomingMessagesQueue.IsEmpty())
		{ 
			NetworkThreadSleep(); 
		}

		size_t nMessageCount = 0;
		while (nMessageCount < nMaxMessages && !m_NetworkContext.IncomingMessagesQueue.IsEmpty())
		{
			// Grab the front message
			auto msg = m_NetworkContext.IncomingMessagesQueue.PopFront();

			// Pass to message handler
			OpenMessageFromClient(msg.remote, msg.msg);

			nMessageCount++;
		}
	}
	void Server::OpenMessageFromClient(ServerTCPConnection* client, Kargono::Network::Message& incomingMessage)
	{
		switch (static_cast<MessageType>(incomingMessage.Header.ID))
		{
		case MessageType::ServerPing:
		{
			KG_INFO("[{}]: Server Ping", client->GetID());
			client->SendTCPMessage(incomingMessage);
			break;
		}

		case MessageType::MessageAll:
		{
			KG_INFO("[{}]: Message All", client->GetID());
			Kargono::Network::Message newMessage;
			newMessage.Header.ID = static_cast<uint32_t>(MessageType::ServerMessage);
			newMessage << client->GetID();
			SendTCPMessageAll(newMessage, client);
			break;
		}
		case MessageType::ClientChat:
		{
			KG_INFO("[{}]: Sent Chat", client->GetID());
			incomingMessage.Header.ID = static_cast<uint32_t>(MessageType::ServerChat);
			incomingMessage << client->GetID();
			SendTCPMessageAll(incomingMessage, client);
			break;
		}
		case MessageType::RequestJoinSession:
		{
			Kargono::Network::Message newMessage;

			// Deny Client Join if session slots are full
			if (m_OnlySession.GetClientCount() >= k_MaxSessionClients)
			{
				newMessage.Header.ID = static_cast<uint32_t>(MessageType::DenyJoinSession);
				client->SendTCPMessage(newMessage);
				break;
			}

			// Add Client to session
			newMessage.Header.ID = static_cast<uint32_t>(MessageType::ApproveJoinSession);
			uint16_t clientSlot = m_OnlySession.AddClient(client);
			if (clientSlot == 0xFFFF)
			{
				return;
			}
			newMessage << clientSlot;

			// Send Approval Message to New Client
			client->SendTCPMessage(newMessage);

			// Notify all other session clients that new client has been added
			newMessage.Header.ID = static_cast<uint32_t>(MessageType::UpdateSessionUserSlot);
			for (auto& [clientID, connection] : m_OnlySession.GetAllClients())
			{
				if (clientID == client->GetID()) { continue; }
				connection->SendTCPMessage(newMessage);
			}

			// Updated new client with all other client data
			for (auto& [slot, clientID] : m_OnlySession.GetAllSlots())
			{
				if (clientID == client->GetID()) { continue; }

				Kargono::Network::Message otherClientMessage;
				otherClientMessage.Header.ID = static_cast<uint32_t>(MessageType::UpdateSessionUserSlot);
				otherClientMessage << slot;
				client->SendTCPMessage(otherClientMessage);
			}

			// If enough clients are connected, start the session
			if (m_OnlySession.GetClientCount() == k_MaxSessionClients)
			{
				m_OnlySession.InitSession();
			}

			break;
		}

		case MessageType::RequestUserCount:
		{
			KG_INFO("[{}]: User Count Request", client->GetID());
			Kargono::Network::Message newMessage;
			newMessage.Header.ID = static_cast<uint32_t>(MessageType::UpdateUserCount);
			uint32_t numberOfUsers = static_cast<uint32_t>(m_AllClientConnections.size());
			newMessage << numberOfUsers;
			client->SendTCPMessage(newMessage);
			break;
		}

		case MessageType::LeaveCurrentSession:
		{
			KG_INFO("[{}]: User Leaving Session", client->GetID());
			Kargono::Network::Message newMessage;
			newMessage.Header.ID = static_cast<uint32_t>(MessageType::UserLeftSession);
			newMessage << m_OnlySession.RemoveClient(client->GetID());

			// Notify all users in the same session that a client left
			for (auto& [clientID, connection] : m_OnlySession.GetAllClients())
			{
				connection->SendTCPMessage(newMessage);
			}
			client->SendTCPMessage(newMessage);

			break;
		}

		case MessageType::InitSyncPing:
		{
			m_OnlySession.ReceiveSyncPing(client->GetID());
			break;
		}

		case MessageType::SessionReadyCheck:
		{
			m_OnlySession.StoreClientReadyCheck(client->GetID());
			break;
		}

		case MessageType::EnableReadyCheck:
		{
			m_OnlySession.EnableReadyCheck();
			break;
		}

		case MessageType::SendAllEntityLocation:
		{
			incomingMessage.Header.ID = static_cast<uint32_t>(MessageType::UpdateEntityLocation);
			// Forward entity location to all other clients
			for (auto& [clientID, connection] : m_OnlySession.GetAllClients())
			{
				if (clientID == client->GetID()) { continue; }
				SendUDPMessage(connection, incomingMessage);
			}
			break;
		}

		case MessageType::SendAllEntityPhysics:
		{
			incomingMessage.Header.ID = static_cast<uint32_t>(MessageType::UpdateEntityPhysics);
			// Forward entity Physics to all other clients
			for (auto& [clientID, connection] : m_OnlySession.GetAllClients())
			{
				if (clientID == client->GetID()) { continue; }
				SendUDPMessage(connection, incomingMessage);
			}
			break;
		}

		case MessageType::SignalAll:
		{
			incomingMessage.Header.ID = static_cast<uint32_t>(MessageType::ReceiveSignal);
			// Forward signal to all other session clients
			for (auto& [clientID, connection] : m_OnlySession.GetAllClients())
			{
				if (clientID == client->GetID()) { continue; }
				connection->SendTCPMessage(incomingMessage);
			}
			break;
		}

		case MessageType::KeepAlive:
		{
			Kargono::Network::Message newMessage;
			newMessage.Header.ID = static_cast<uint32_t>(MessageType::KeepAlive);
			SendUDPMessage(client, newMessage);
			break;
		}

		case MessageType::UDPInit:
		{
			Kargono::Network::Message newMessage;
			newMessage.Header.ID = static_cast<uint32_t>(MessageType::UDPInit);
			SendUDPMessage(client, newMessage);
			break;
		}

		default:
		{
			KG_ERROR("Invalid message type sent to server in Server.cpp");
			break;
		}
		}
	}
	void Server::NetworkThreadSleep()
	{
		std::unique_lock<std::mutex> lock(m_NetworkContext.BlockThreadMutex);
		m_NetworkContext.BlockThreadCondVar.wait(lock);
	}
	void Server::NetworkThreadWakeUp()
	{
		std::unique_lock<std::mutex> lock(m_NetworkContext.BlockThreadMutex);
		m_NetworkContext.BlockThreadCondVar.notify_one();
	}
	void Server::CheckConnectionsValid()
	{
		bool bInvalidClientExists = false;
		for (auto& client : m_AllClientConnections)
		{
			// Check client is connected...
			if (!(client && client->IsConnected()))
			{
				// The client couldn't be contacted, so assume it has disconnected.
				OnClientDisconnect(client);
				m_IPAddressToConnection.erase(client->GetUDPRemoteReceiveEndpoint());
				client.reset();
				bInvalidClientExists = true;
			}
		}
		if (bInvalidClientExists)
		{
			m_AllClientConnections.erase(
				std::remove(m_AllClientConnections.begin(), m_AllClientConnections.end(), nullptr), m_AllClientConnections.end());
		}
	}

	void Server::SendTCPMessage(ServerTCPConnection* client, const Message& msg)
	{
		if (client && client->IsConnected())
		{
			client->SendTCPMessage(msg);
		}
	}

	void Server::SendUDPMessage(ServerTCPConnection* client, Message& msg)
	{
		if (client && client->IsConnected())
		{
			LabeledMessage labeled{ client->GetUDPRemoteSendEndpoint(), msg };

			m_UDPServer->SendUDPMessage(labeled);
		}
	}



	void Server::SendTCPMessageAll(const Message& msg, ServerTCPConnection* ignoreClient)
	{
		for (auto& client : m_AllClientConnections)
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
	void Server::StopServer()
	{
		if (m_UDPServer)
		{
			m_UDPServer->Stop();
			m_UDPServer.reset();
		}

		for (auto& connection : m_AllClientConnections)
		{
			if (connection->IsConnected())
			{
				connection->Disconnect();
			}
		}

		m_NetworkContext.AsioContext.stop();
		if (m_NetworkContext.AsioThread.joinable())
		{
			m_NetworkContext.AsioThread.join();
		}

		for (auto& connection : m_AllClientConnections)
		{
			if (connection->IsConnected())
			{
				connection.reset();
			}
		}
	}
	void Server::SessionClock()
	{
		using namespace std::chrono_literals;

		static const std::chrono::nanoseconds constantFrameTime { 1'000 * 1'000 * 1'000 / 60 };
		std::chrono::nanoseconds accumulator{ 0 };
		std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::time_point<std::chrono::high_resolution_clock> lastCycleTime = currentTime;
		std::chrono::nanoseconds timeStep{ 0 };

		while (!m_StopTimingThread)
		{
			currentTime = std::chrono::high_resolution_clock::now();
			timeStep = currentTime - lastCycleTime;
			lastCycleTime = currentTime;
			accumulator += timeStep;
			if (accumulator < constantFrameTime)
			{
				continue;
			}
			accumulator -= constantFrameTime;
			m_UpdateCount++;
		}
	}
	bool Server::OnClientConnect(Ref<Kargono::Network::ServerTCPConnection> client)
	{
		KG_INFO("Client successfully connected [{}]", client->GetID());
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(MessageType::AcceptConnection);
		uint32_t numberOfUsers = static_cast<uint32_t>(m_AllClientConnections.size() + 1);
		msg << numberOfUsers;
		client->SendTCPMessage(msg);
		msg.Header.ID = static_cast<uint32_t>(MessageType::UpdateUserCount);
		SendTCPMessageAll(msg, client.get());
		return true;
	}
	void Server::OnClientDisconnect(Ref<Kargono::Network::ServerTCPConnection> client)
	{
		KG_INFO("Removing client [{}]", client->GetID());
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(MessageType::UpdateUserCount);
		uint32_t numberOfUsers = static_cast<uint32_t>(m_AllClientConnections.size() - 1);
		msg << numberOfUsers;
		SendTCPMessageAll(msg, client.get());


		if (m_OnlySession.GetAllClients().contains(client->GetID()))
		{
			Kargono::Network::Message newMessage;
			newMessage.Header.ID = static_cast<uint32_t>(MessageType::UserLeftSession);
			newMessage << m_OnlySession.RemoveClient(client->GetID());

			// Notify all users in the same session that a client left
			for (auto& [clientID, connection] : m_OnlySession.GetAllClients())
			{
				connection->SendTCPMessage(newMessage);
			}
		}


	}

	void Server::WaitForClientConnectionAsync()
	{
		m_TCPAcceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket)
			{
				if (ec)
				{
					KG_INFO("New Connection Error: {}", ec.message());
					WaitForClientConnectionAsync();
					return;
				}

				KG_INFO("New Connection: {}", socket.remote_endpoint());



				Ref<ServerTCPConnection> newConnection = CreateRef<ServerTCPConnection>(&m_NetworkContext, std::move(socket));

				// Give the user server a chance to deny connection
				if (!OnClientConnect(newConnection))
				{
					KG_WARN("[-----] Connection Denied");
					WaitForClientConnectionAsync();
					return;
				}


				m_AllClientConnections.push_back(std::move(newConnection));

				m_AllClientConnections.back()->Connect(m_ClientIDCounter++);

				// Connection allowed, so add to container of new connections

				m_IPAddressToConnection.insert_or_assign(m_AllClientConnections.back()->GetUDPRemoteReceiveEndpoint(), m_AllClientConnections.back());

				KG_INFO("[{}]: Connection Approved", m_AllClientConnections.back()->GetID());



				// Prime the asio context with more work - again simply wait for
				//		another connection...
				WaitForClientConnectionAsync();
			});
	}

	Ref<Network::Server> ServerService::s_Server { nullptr };

	bool ServerService::Init()
	{
		bool isLocal = Projects::ProjectService::GetActiveServerLocation() == "LocalMachine";

		s_Server = CreateRef<Network::Server>(Projects::ProjectService::GetActiveServerPort(), isLocal);
		if (!s_Server->StartServer())
		{
			KG_WARN("Failed to start server");
			Terminate();
			return false;
		}

		KG_VERIFY(s_Server, "Server connection init");
		return true;
	}

	void ServerService::Terminate()
	{
		s_Server->StopServer();
		s_Server.reset();
		s_Server = nullptr;
		KG_VERIFY(!s_Server, "Closed server connection");
	}
	void ServerService::Run()
	{
		while (true)
		{
			s_Server->CheckForMessages();
			ProcessEventQueue();
		}
	}
	void ServerService::SubmitToEventQueue(Ref<Events::Event> e)
	{
		std::scoped_lock<std::mutex> lock(s_Server->m_EventQueueMutex);

		s_Server->m_EventQueue.emplace_back(e);

		// Alert thread to wake up and process event
		s_Server->NetworkThreadWakeUp();
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
		if (!s_Server->m_TimingThread)
		{
			s_Server->m_OnlySession.SetSessionStartFrame(0);
			s_Server->m_TimingThread = CreateScope<std::thread>(&Server::SessionClock, s_Server.get());
		}
		else
		{
			s_Server->m_OnlySession.SetSessionStartFrame(s_Server->m_UpdateCount);
		}

		return true;
	}
	void ServerService::ProcessEventQueue()
	{
		KG_PROFILE_FUNCTION();

		std::scoped_lock<std::mutex> lock(s_Server->m_EventQueueMutex);

		for (auto& event : s_Server->m_EventQueue)
		{
			OnEvent(event.get());
		}
		s_Server->m_EventQueue.clear();
	}
}

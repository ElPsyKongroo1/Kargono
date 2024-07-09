#include "kgpch.h"

#include "Kargono/Network/Server.h"

#include "Kargono/Core/Engine.h"

namespace Kargono::Network
{

	void UDPServerConnection::AddToIncomingMessageQueue()
	{
		if (!m_IPAddressToConnection.contains(m_CurrentEndpoint))
		{
			// This will adjust the udp endpoint for an ip address in case it is changed
			// TODO: I KNOW THIS IS A HUGE SECURITY PROBLEM, but yea idk any other way
			//		to make this work
			bool foundMatchingAddress = false;
			asio::ip::udp::endpoint foundEndpoint;
			Ref<TCPServerConnection> clientConnect { nullptr };
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
				KG_ERROR("Address not found in m_IPAddressToConnection()");
				return;
			}

			clientConnect->SetUDPRemoteReceiveEndpoint(m_CurrentEndpoint);
			clientConnect->SetUDPRemoteSendEndpoint(m_CurrentEndpoint);
			m_IPAddressToConnection.erase(foundEndpoint);
			m_IPAddressToConnection.insert_or_assign(m_CurrentEndpoint, clientConnect);

		}

		m_qMessagesIn.PushBack({ m_IPAddressToConnection.at(m_CurrentEndpoint), m_MsgTemporaryIn });
		WakeUpNetworkThread();
		ReadMessage();
	}
	void UDPServerConnection::Disconnect(asio::ip::udp::endpoint key)
	{

		if (!m_IPAddressToConnection.contains(key))
		{
			KG_WARN("Address does not resolve to a connection pointer in UDPServer Disconnect()");
			return;
		}
		Ref<TCPServerConnection> connection = m_IPAddressToConnection.at(key);
		if (!connection)
		{
			KG_WARN("Invalid connection in UDP Disconnect()");
			return;
		}

		connection->Disconnect();
	}

	TCPServerConnection::TCPServerConnection(asio::io_context& asioContext, asio::ip::tcp::socket&& socket, TSQueue<owned_message>& qIn, std::condition_variable& newCV,
		std::mutex& newMutex)
		: TCPConnection(asioContext, std::move(socket), qIn, newCV, newMutex)
	{

		// Construct validation check data

		// Connection is Server -> Client, construct random data for the client to transform and send back for validation
		m_nHandshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());

		// Pre-calculate the result for checking when the client responds
		m_nHandshakeCheck = Scramble(m_nHandshakeOut);


	}
	void TCPServerConnection::Connect(uint32_t uid)
	{
		if (m_TCPSocket.is_open())
		{
			id = uid;

			m_UDPLocalEndpoint = asio::ip::udp::endpoint(m_TCPSocket.local_endpoint().address(),
				m_TCPSocket.local_endpoint().port());
			m_UDPRemoteSendEndpoint = asio::ip::udp::endpoint(m_TCPSocket.remote_endpoint().address(),
				m_TCPSocket.remote_endpoint().port());
			m_UDPRemoteReceiveEndpoint = asio::ip::udp::endpoint(m_TCPSocket.remote_endpoint().address(),
				m_TCPSocket.remote_endpoint().port());

			// A client has attempted to connect to the server, but we want
			// the client to first validate itself, so first write out the
			// handshake data to be validated
			WriteValidation();

			// Next, issue a task to sit and wait asynchronously for precisely
			// the validation data sent back from the client
			ReadValidation();

		}
	}

	void TCPServerConnection::Disconnect()
	{
		if (IsConnected())
		{
			asio::post(m_asioContext, [this]()
				{
					m_TCPSocket.close();
					WakeUpNetworkThread();

				});
		}
		KG_INFO("[SERVER]: Connection has been terminated");
	}

	void TCPServerConnection::AddToIncomingMessageQueue()
	{
		m_qMessagesIn.PushBack({ this->shared_from_this(), m_msgTemporaryIn });
		WakeUpNetworkThread();
		ReadHeader();
	}

	void TCPServerConnection::WriteValidation()
	{
		asio::async_write(m_TCPSocket, asio::buffer(&m_nHandshakeOut, sizeof(uint64_t)),
			[this](std::error_code ec, std::size_t length)
			{
				if (!ec)
				{
				}
				else
				{
					m_TCPSocket.close();
				}
			});
	}
	void TCPServerConnection::ReadValidation()
	{
		asio::async_read(m_TCPSocket, asio::buffer(&m_nHandshakeIn, sizeof(uint64_t)),
			[&](std::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					if (m_nHandshakeIn == m_nHandshakeCheck)
					{
						// Client has provided valid solution, so allow it to connect properly
						KG_INFO("Client Validation Successful");
						// TODO: Add post client validation back
						//server->OnClientValidated(this->shared_from_this());

						// Sit waiting to receive data now
						ReadHeader();
					}
					else
					{
						KG_INFO("Client Disconnected (Failed Validation)");
						Disconnect();
					}
				}
				else
				{
					KG_INFO("Client Disconnected (ReadValidation)");
					Disconnect();
				}
			});
	}

	Ref<Network::Server> Server::s_Server { nullptr };

	Server::Server(uint16_t nPort, bool isLocal)
		: m_TCPAcceptor(m_Context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), nPort))
	{
		// Create a resolver
		asio::ip::tcp::resolver resolver(m_Context);

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
		asio::ip::udp::socket serverUDPSocket {m_Context};
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
		m_UDPServer = CreateRef<UDPServerConnection>(m_Context, std::move(serverUDPSocket), m_qMessagesIn,
			m_BlockThreadCV, m_BlockThreadMx, m_IPAddressToConnection);
	}

	bool Server::Start()
	{
		try
		{
			WaitForClientConnection();
			m_UDPServer->Start();

			m_threadContext = std::thread([this]() { m_Context.run(); });
		}
		catch (std::exception& e)
		{
			std::cerr << "[SERVER] Exception: " << e.what() << '\n';
			return false;
		}

		KG_INFO("[SERVER] Started!");
		return true;
	}
	void Server::Stop()
	{
		// Request the context to close
		m_Context.stop();

		// Tidy up the context thread
		if (m_threadContext.joinable()) { m_threadContext.join(); }
	}

	void Server::Update(size_t nMaxMessages)
	{
		CheckConnectionsValid();
		if (m_qMessagesIn.IsEmpty()) { Wait(); }

		size_t nMessageCount = 0;
		while (nMessageCount < nMaxMessages && !m_qMessagesIn.IsEmpty())
		{
			// Grab the front message
			auto msg = m_qMessagesIn.PopFront();

			// Pass to message handler
			OnMessage(msg.remote, msg.msg);

			nMessageCount++;
		}
	}
	void Server::Wait()
	{
		std::unique_lock<std::mutex> lock(m_BlockThreadMx);
		m_BlockThreadCV.wait(lock);
	}
	void Server::WakeUpNetworkThread()
	{
		std::unique_lock<std::mutex> lock(m_BlockThreadMx);
		m_BlockThreadCV.notify_one();
	}
	void Server::CheckConnectionsValid()
	{
		bool bInvalidClientExists = false;
		for (auto& client : m_Connections)
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
			m_Connections.erase(
				std::remove(m_Connections.begin(), m_Connections.end(), nullptr), m_Connections.end());
		}
	}

	void Server::MessageClient(Ref<TCPServerConnection> client, const Message& msg)
	{
		if (client && client->IsConnected())
		{
			client->Send(msg);
		}
	}

	void Server::MessageClientUDP(Ref<TCPServerConnection> client, Message& msg)
	{
		if (client && client->IsConnected())
		{
			LabeledMessage labeled{ client->GetUDPRemoteSendEndpoint(), msg };

			m_UDPServer->Send(labeled);
		}
	}



	void Server::MessageAllClients(const Message& msg, Ref<TCPServerConnection> pIgnoreClient)
	{
		for (auto& client : m_Connections)
		{
			// Check client is connected...
			if (client && client->IsConnected())
			{
				// ..it is!
				if (client != pIgnoreClient) { client->Send(msg); }
			}

		}
	}


	bool Server::StartServer()
	{
		return Start();
	}
	void Server::RunServer()
	{
		while (true)
		{
			Update();
			ProcessEventQueue();
		}
	}
	void Server::StopServer()
	{
		if (m_UDPServer)
		{
			m_UDPServer->Stop();
			m_UDPServer.reset();
		}

		for (auto& connection : m_Connections)
		{
			if (connection->IsConnected())
			{
				connection->Disconnect();
			}
		}

		m_Context.stop();
		if (m_threadContext.joinable())
			m_threadContext.join();

		for (auto& connection : m_Connections)
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

		while (!m_StopThread)
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
	bool Server::OnClientConnect(Ref<Kargono::Network::TCPServerConnection> client)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(MessageType::AcceptConnection);
		uint32_t numberOfUsers = static_cast<uint32_t>(m_Connections.size() + 1);
		msg << numberOfUsers;
		client->Send(msg);
		msg.Header.ID = static_cast<uint32_t>(MessageType::UpdateUserCount);
		MessageAllClients(msg, client);
		return true;
	}
	void Server::OnClientDisconnect(Ref<Kargono::Network::TCPServerConnection> client)
	{
		KG_INFO("Removing client [{}]", client->GetID());
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(MessageType::UpdateUserCount);
		uint32_t numberOfUsers = static_cast<uint32_t>(m_Connections.size() - 1);
		msg << numberOfUsers;
		MessageAllClients(msg, client);


		if (m_OnlySession.GetAllClients().contains(client->GetID()))
		{
			Kargono::Network::Message newMessage;
			newMessage.Header.ID = static_cast<uint32_t>(MessageType::UserLeftSession);
			newMessage << m_OnlySession.RemoveClient(client->GetID());

			// Notify all users in the same session that a client left
			for (auto& [clientID, connection] : m_OnlySession.GetAllClients())
			{
				connection->Send(newMessage);
			}
		}


	}
	void Server::OnMessage(Ref<Kargono::Network::TCPServerConnection> client, Kargono::Network::Message& incomingMessage)
	{
		switch (static_cast<MessageType>(incomingMessage.Header.ID))
		{
		case MessageType::ServerPing:
		{
			KG_INFO("[{}]: Server Ping", client->GetID());
			client->Send(incomingMessage);
			break;
		}

		case MessageType::MessageAll:
		{
			KG_INFO("[{}]: Message All", client->GetID());
			Kargono::Network::Message newMessage;
			newMessage.Header.ID = static_cast<uint32_t>(MessageType::ServerMessage);
			newMessage << client->GetID();
			MessageAllClients(newMessage, client);
			break;
		}
		case MessageType::ClientChat:
		{
			KG_INFO("[{}]: Sent Chat", client->GetID());
			incomingMessage.Header.ID = static_cast<uint32_t>(MessageType::ServerChat);
			incomingMessage << client->GetID();
			MessageAllClients(incomingMessage, client);
			break;
		}
		case MessageType::RequestJoinSession:
		{
			Kargono::Network::Message newMessage;

			// Deny Client Join if session slots are full
			if (m_OnlySession.GetClientCount() >= Session::k_MaxClients)
			{
				newMessage.Header.ID = static_cast<uint32_t>(MessageType::DenyJoinSession);
				client->Send(newMessage);
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
			client->Send(newMessage);

			// Notify all other session clients that new client has been added
			newMessage.Header.ID = static_cast<uint32_t>(MessageType::UpdateSessionUserSlot);
			for (auto& [clientID, connection] : m_OnlySession.GetAllClients())
			{
				if (clientID == client->GetID()) { continue; }
				connection->Send(newMessage);
			}

			// Updated new client with all other client data
			for (auto& [slot, clientID] : m_OnlySession.GetAllSlots())
			{
				if (clientID == client->GetID()) { continue; }

				Kargono::Network::Message otherClientMessage;
				otherClientMessage.Header.ID = static_cast<uint32_t>(MessageType::UpdateSessionUserSlot);
				otherClientMessage << slot;
				client->Send(otherClientMessage);
			}

			// If enough clients are connected, start the session
			if (m_OnlySession.GetClientCount() == Session::k_MaxClients)
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
			uint32_t numberOfUsers = static_cast<uint32_t>(m_Connections.size());
			newMessage << numberOfUsers;
			client->Send(newMessage);
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
				connection->Send(newMessage);
			}
			client->Send(newMessage);

			break;
		}

		case MessageType::InitSyncPing:
		{
			m_OnlySession.ReceiveSyncPing(client->GetID());
			break;
		}

		case MessageType::SessionReadyCheck:
		{
			m_OnlySession.ReadyCheck(client->GetID());
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
				MessageClientUDP(connection, incomingMessage);
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
				MessageClientUDP(connection, incomingMessage);
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
				connection->Send(incomingMessage);
			}
			break;
		}

		case MessageType::KeepAlive:
		{
			Kargono::Network::Message newMessage;
			newMessage.Header.ID = static_cast<uint32_t>(MessageType::KeepAlive);
			MessageClientUDP(client, newMessage);
			break;
		}

		case MessageType::UDPInit:
		{
			Kargono::Network::Message newMessage;
			newMessage.Header.ID = static_cast<uint32_t>(MessageType::UDPInit);
				MessageClientUDP(client, newMessage);
				break;
			}

			default:
			{
				KG_ERROR("Invalid message type sent to server in Server.cpp");
				break;
			}
		}
	}


	void Server::SubmitToEventQueue(Ref<Events::Event> e)
	{
		std::scoped_lock<std::mutex> lock(m_EventQueueMutex);

		m_EventQueue.emplace_back(e);

		// Alert thread to wake up and process event
		WakeUpNetworkThread();
	}

	void Server::ProcessEventQueue()
	{
		KG_PROFILE_FUNCTION();

		std::scoped_lock<std::mutex> lock(m_EventQueueMutex);

		for (auto& event : m_EventQueue)
		{
			OnEvent(*event);
		}
		m_EventQueue.clear();
	}

	void Server::OnEvent(Events::Event& e)
	{
		Events::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Events::StartSession>(KG_BIND_CLASS_FN(Server::OnStartSession));
		
	}

	bool Server::OnStartSession(Events::StartSession event)
	{
		if (!m_TimingThread)
		{
			m_OnlySession.SetSessionStartFrame(0);
			m_TimingThread = CreateScope<std::thread>(&Server::SessionClock, this);
		}
		else
		{
			m_OnlySession.SetSessionStartFrame(m_UpdateCount);
		}

		KG_TRACE_INFO("Starting frame is {}", m_OnlySession.GetSessionStartFrame());
		return true;
	}

	void Server::WaitForClientConnection()
	{
		m_TCPAcceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket)
			{
				if (ec)
				{
					KG_INFO("[SERVER] New Connection Error: {}", ec.message());
					WaitForClientConnection();
					return;
				}

				KG_INFO("[SERVER] New Connection: {}", socket.remote_endpoint());



				Ref<TCPServerConnection> newConnection = CreateRef<TCPServerConnection>(m_Context, std::move(socket),
					m_qMessagesIn, m_BlockThreadCV, m_BlockThreadMx);

				// Give the user server a chance to deny connection
				if (!OnClientConnect(newConnection))
				{
					KG_WARN("[-----] Connection Denied");
					WaitForClientConnection();
					return;
				}


				m_Connections.push_back(std::move(newConnection));

				m_Connections.back()->Connect(nIDCounter++);

				// Connection allowed, so add to container of new connections

				m_IPAddressToConnection.insert_or_assign(m_Connections.back()->GetUDPRemoteReceiveEndpoint(), m_Connections.back());

				KG_INFO("[{}]: Connection Approved", m_Connections.back()->GetID());



				// Prime the asio context with more work - again simply wait for
				//		another connection...
				WaitForClientConnection();
			});
	}
}

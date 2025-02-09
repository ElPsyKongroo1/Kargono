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
		m_NetworkContextPtr->IncomingMessagesQueue.PushBack({ nullptr, m_MessageCache });
		NetworkThreadWakeUp();
		ReadMessageAsync();
	}
		
	bool ClientTCPConnection::Connect(const asio::ip::tcp::resolver::results_type& endpoints)
	{
		std::atomic<bool> connectionSuccessful = false;

		// Start async connection
		asio::async_connect(m_TCPSocket, endpoints, [&](std::error_code ec, const asio::ip::tcp::endpoint& endpoint)
		{
			UNREFERENCED_PARAMETER(endpoint);

			if (ec)
			{
				KG_WARN("Error while attempting to connect to server. Error Code: [{}] Message: {}", ec.value(), ec.message());
				Disconnect();
			}
			else
			{
				// Start validation sequence
				ReadValidationAsync();
				// Wake up network thread
				connectionSuccessful = true;
				NetworkThreadWakeUp();
			}
		});
		
		// Start Context Thread
		m_NetworkContextPtr->AsioThread = std::thread([this]() { m_NetworkContextPtr->AsioContext.run(); });

		while (!m_NetworkContextPtr->Quit && !connectionSuccessful)
		{
			NetworkThreadSleep();
		}

		// Check for failure to connect
		if (!connectionSuccessful || m_NetworkContextPtr->Quit)
		{
			return false;
		}

		// Initialize UDP endpoints for new UDP connnection
		m_UDPLocalEndpoint = asio::ip::udp::endpoint(m_TCPSocket.local_endpoint().address(),
			m_TCPSocket.local_endpoint().port());
		m_UDPRemoteSendEndpoint = asio::ip::udp::endpoint(m_TCPSocket.remote_endpoint().address(),
			m_TCPSocket.remote_endpoint().port());
		m_UDPRemoteReceiveEndpoint = asio::ip::udp::endpoint(m_TCPSocket.remote_endpoint().address(),
			m_TCPSocket.remote_endpoint().port());

		// TCP Connection successful!
		return true;
	}
	void ClientTCPConnection::Disconnect()
	{
		if (IsConnected())
		{
			asio::post(m_NetworkContextPtr->AsioContext, [this]()
			{
				m_TCPSocket.close();
			});
		}
		KG_INFO("Connection to server has been terminated");

		EngineService::SubmitToEventQueue(CreateRef<Events::ConnectionTerminated>());
		ClientService::SubmitToEventQueue(CreateRef<Events::ConnectionTerminated>());

	}

	void ClientTCPConnection::AddMessageToIncomingMessageQueue()
	{
		m_NetworkContextPtr->IncomingMessagesQueue.PushBack({ nullptr, m_MessageCache });
		NetworkThreadWakeUp();
		ReadMessageHeaderAsync();
	}

	void ClientTCPConnection::WriteValidationAsync()
	{
		asio::async_write(m_TCPSocket, asio::buffer(&m_ValidationOutput, sizeof(uint64_t)),
			[this](std::error_code ec, std::size_t length)
		{
			UNREFERENCED_PARAMETER(length);

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


	bool Client::ConnectToServer(const std::string& serverIP, const uint16_t serverPort, bool remote)
	{
		try
		{
			// Create TCP Socket for this new connection
			asio::ip::tcp::socket localSocket{m_NetworkContext.AsioContext};

			if (remote)
			{
				// Get local TCP address and port from localhost
				asio::ip::tcp::resolver resolver(m_NetworkContext.AsioContext);
				asio::ip::tcp::resolver::query query(asio::ip::host_name(), "");
				asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);

				// Search for IPV4 address
				while (iter != asio::ip::tcp::resolver::iterator())
				{
					if (iter->endpoint().address().is_v4()) 
					{ 
						break; 
					}
					++iter;
				}

				// Create local endpoint, bind IPV4 address to new socket, and assign the local endpoint with the serverPort - 100
				KG_ASSERT(serverPort > 100, "Port must be greater than 100. If not, we have some issuessss!");
				asio::ip::tcp::endpoint localTCPEndpoint = asio::ip::tcp::endpoint(iter->endpoint().address(), serverPort - 100);
				localSocket.open(localTCPEndpoint.protocol());
				localSocket.bind(localTCPEndpoint);
			}

			// Resolve provided host/port into a TCP endpoint list
			asio::ip::tcp::resolver resolver(m_NetworkContext.AsioContext);
			asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(serverIP, std::to_string(serverPort));

			// Create a new connection object
			m_ClientTCPConnection = CreateRef<ClientTCPConnection>(&m_NetworkContext, std::move(localSocket));

			// Initialize connection to remote endpoint
			KG_INFO("Attempting TCP connection to server");
			if (!m_ClientTCPConnection->Connect(endpoints)) 
			{ 
				KG_INFO("Failed to initiate TCP connection to server");
				return false; 
			}
			KG_INFO("Successful TCP connection to server");


			/*KG_TRACE_INFO("The local endpoint is {} {}", m_ClientTCPConnection->GetUDPLocalEndpoint().address().to_string(),
				m_ClientTCPConnection->GetUDPLocalEndpoint().port());
			KG_TRACE_INFO("The remote send endpoint is {} {}", m_ClientTCPConnection->GetUDPRemoteSendEndpoint().address().to_string(),
				m_ClientTCPConnection->GetUDPRemoteSendEndpoint().port());*/

			// Create Local UDP Socket
			KG_INFO("Attempting UDP connection to server");
			asio::ip::udp::socket serverUDPSocket {m_NetworkContext.AsioContext};
			// Get Local UDP Endpoint created from Connect Function
			//		and bind it to the socket
			asio::ip::udp::endpoint& localEndpoint = m_ClientTCPConnection->GetUDPLocalEndpoint();
			serverUDPSocket.open(localEndpoint.protocol());
			serverUDPSocket.bind(localEndpoint);

			m_ClientUDPConnection = CreateRef<ClientUDPConnection>(&m_NetworkContext, std::move(serverUDPSocket), m_ClientTCPConnection);

			m_ClientUDPConnection->Start();

			

			Utility::AsyncBusyTimer::CreateRecurringTimer(0.3f, 10, [&]()
			{
				ClientService::SubmitToFunctionQueue([&]()
				{
					// Do checks for UDP connection
					Message newMessage{};
					newMessage.Header.ID = static_cast<uint32_t>(MessageType::UDPInit);
					SendUDP(newMessage);
				});
				NetworkThreadWakeUp();
			}, [&]()
			{
				ClientService::SubmitToFunctionQueue([&]()
				{
					if (!m_UDPConnectionSuccessful)
					{
						KG_WARN("UDP Connection Unsuccessful!");
						DisconnectFromServer();
						return;
					}
					KG_INFO("UDP Connection Successful!");
				});
				NetworkThreadWakeUp();
			});


		}
		catch (std::exception& e)
		{
			KG_ERROR("Client Exception: {}", e.what());
			return false;
		}

		return true;
	}

	void Client::DisconnectFromServer()
	{
		// Check if already connected
		if (IsConnected())
		{
			m_ClientTCPConnection->Disconnect();
		}

		// Close asio context, thread, and TCP connection
		m_NetworkContext.AsioContext.stop();
		if (m_NetworkContext.AsioThread.joinable())
		{
			m_NetworkContext.AsioThread.join();
		}

		m_ClientTCPConnection.reset();
	}

	bool Client::IsConnected()
	{
		if (m_ClientTCPConnection) 
		{ 
			return m_ClientTCPConnection->IsConnected();
		}
		return false;
	}

	void Client::CheckForMessages(size_t nMaxMessages)
	{
		if (m_NetworkContext.IncomingMessagesQueue.IsEmpty())
		{ 
			NetworkThreadSleep(); 
		}

		size_t messageCount = 0;
		while (messageCount < nMaxMessages && !m_NetworkContext.IncomingMessagesQueue.IsEmpty())
		{
			// Grab the front message
			auto msg = m_NetworkContext.IncomingMessagesQueue.PopFront();

			// Pass to message handler
			OpenMessageFromServer(msg.msg);
			messageCount++;
		}
	}

	void Client::SendTCP(const Message& msg)
	{
		if (IsConnected())
		{
			m_ClientTCPConnection->SendTCPMessage(msg);
		}
	}

	void Client::SendUDP(Message& msg)
	{
		if (IsConnected())
		{
			LabeledMessage labeled{ m_ClientTCPConnection->GetUDPRemoteSendEndpoint(), msg };
			m_ClientUDPConnection->SendUDPMessage(labeled);
		}
	}

	void Client::NetworkThreadSleep()
	{
		std::unique_lock<std::mutex> lock(m_NetworkContext.BlockThreadMutex);
		m_NetworkContext.BlockThreadCondVar.wait(lock);
	}

	void Client::NetworkThreadWakeUp()
	{
		std::unique_lock<std::mutex> lock(m_NetworkContext.BlockThreadMutex);
		m_NetworkContext.BlockThreadCondVar.notify_one();
	}

	void Client::SendChat(const std::string& text)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(MessageType::ClientChat);
		msg.StorePayload((void*)text.data(), text.size());
		SendTCP(msg);
	}

	void Client::OpenMessageFromServer(Kargono::Network::Message& msg)
	{
		// Process message from server based on type
		switch (static_cast<MessageType>(msg.Header.ID))
		{
		case MessageType::AcceptConnection:
		{
			KG_INFO("Connection to server has been accepted!");
			uint32_t userCount{};
			msg >> userCount;
			EngineService::SubmitToEventQueue(CreateRef<Events::UpdateOnlineUsers>(userCount));
			break;
		}

		case MessageType::UpdateUserCount:
		{
			uint32_t userCount{};
			msg >> userCount;
			EngineService::SubmitToEventQueue(CreateRef<Events::UpdateOnlineUsers>(userCount));
			break;
		}

		case MessageType::ServerChat:
		{
			uint32_t clientID{};
			std::vector<uint8_t> data{};
			uint64_t dataSize{};
			msg >> clientID;
			msg >> dataSize;
			data = msg.GetPayloadCopy(dataSize);
			std::string text((char*)(data.data()), dataSize);
			KG_INFO("[{}]: {}", clientID, text);
			break;
		}

		case MessageType::ApproveJoinSession:
		{
			KG_INFO("Approved joining session");
			uint16_t userSlot{};
			msg >> userSlot;
			m_SessionSlot = userSlot;
			EngineService::SubmitToEventQueue(CreateRef<Events::ApproveJoinSession>(userSlot));
			break;
		}

		case MessageType::UpdateSessionUserSlot:
		{
			uint16_t userSlot{};
			msg >> userSlot;
			EngineService::SubmitToEventQueue(CreateRef<Events::UpdateSessionUserSlot>(userSlot));
			break;
		}

		case MessageType::UserLeftSession:
		{
			KG_INFO("A User Left the Current Session");
			uint16_t userSlot{};
			msg >> userSlot;
			EngineService::SubmitToEventQueue(CreateRef<Events::UserLeftSession>(userSlot));
			break;
		}

		case MessageType::DenyJoinSession:
		{
			KG_INFO("Denied joining session");
			break;
		}

		case MessageType::CurrentSessionInit:
		{
			KG_INFO("Active Session is initializing");
			EngineService::SubmitToEventQueue(CreateRef<Events::CurrentSessionInit>());
			break;
		}

		case MessageType::InitSyncPing:
		{
			Kargono::Network::Message newMessage;
			newMessage.Header.ID = static_cast<uint32_t>(MessageType::InitSyncPing);
			SendTCP(newMessage);
			break;
		}

		case MessageType::StartSession:
		{
			KG_INFO("Session Started");
			float waitTime{};
			msg >> waitTime;
			Utility::AsyncBusyTimer::CreateTimer(waitTime, [&]()
			{
				// Note Starting Update
				Network::ClientService::SubmitToEventQueue(CreateRef<Events::StartSession>());

				// Open Gameplay
				EngineService::SubmitToEventQueue(CreateRef<Events::StartSession>());
			});
				
			break;
		}

		case MessageType::SessionReadyCheckConfirm:
		{
			float waitTime{};
			msg >> waitTime;
			Utility::AsyncBusyTimer::CreateTimer(waitTime, [&]()
			{
				// Open Gameplay
				EngineService::SubmitToEventQueue(CreateRef<Events::SessionReadyCheckConfirm>());
			});

			break;
		}

		case MessageType::UpdateEntityLocation:
		{
			uint64_t id;
			float x, y, z;
			msg >> z;
			msg >> y;
			msg >> x;
			msg >> id;
			Math::vec3 trans{x, y, z};
			EngineService::SubmitToEventQueue(CreateRef<Events::UpdateEntityLocation>(id, trans));

			break;
		}

		case MessageType::UpdateEntityPhysics:
		{
			uint64_t id;
			float x, y, z, linx, liny;

			msg >> liny;
			msg >> linx;
			msg >> z;
			msg >> y;
			msg >> x;
			msg >> id;
			Math::vec3 trans{x, y, z};
			Math::vec2 linearV{linx, liny};
			EngineService::SubmitToEventQueue(CreateRef<Events::UpdateEntityPhysics>(id, trans, linearV));

			break;
		}

		case MessageType::ReceiveSignal:
		{
			uint16_t signal{};
			msg >> signal;
			EngineService::SubmitToEventQueue(CreateRef<Events::ReceiveSignal>(signal));
			break;
		}

		case MessageType::KeepAlive:
		{
			break;
		}

		case MessageType::UDPInit:
		{
			m_UDPConnectionSuccessful = true;
			break;
		}

		default:
		{
			KG_ERROR("Invalid message type sent to client!");
			break;
		}
		}
	}

	void ClientService::Init()
	{
		// Create new client and start network 
		s_Client = CreateRef<Network::Client>();
		s_Client->m_NetworkContext.NetworkThread = CreateRef<std::thread>(&Network::ClientService::Run);
		KG_VERIFY(s_Client, "Client connection init");
	}

	void ClientService::Terminate()
	{
		EndRun();
		s_Client->m_NetworkContext.NetworkThread->join();
		s_Client->m_NetworkContext.NetworkThread.reset();
		s_Client.reset();
		KG_VERIFY(!s_Client, "Closed client connection");
	}

	void ClientService::Run()
	{
		// Decide whether to connect using the local network or through the internet
		bool remoteConnection = false;
		if (Projects::ProjectService::GetActiveServerLocation() != "LocalMachine")
		{
			remoteConnection = true;
		}

		// Start connection to server
		if (!s_Client->ConnectToServer(Projects::ProjectService::GetActiveServerIP(), Projects::ProjectService::GetActiveServerPort(), remoteConnection)) 
		{ 
			s_Client->m_NetworkContext.Quit = true; 
		}

		// Ensure UDP connection is valid
		if (s_Client->m_ClientUDPConnection && s_Client->IsConnected())
		{
			// Start timer for keep alive packets
			EngineService::SubmitToEventQueue(CreateRef<Events::AddTickGeneratorUsage>(k_KeepAliveDelay));
		}

		KG_INFO("Started client networking thread");

		// Main NetworkContext::NetworkThread loop
		while (!s_Client->m_NetworkContext.Quit)
		{
			if (s_Client->IsConnected())
			{
				ClientService::ProcessEventQueue();
				ClientService::ProcessFunctionQueue();
				s_Client->CheckForMessages(5);
			}
			else
			{
				// Client is disconnect
				s_Client->m_NetworkContext.Quit = true;
			}
		}

		// Close connection to server
		s_Client->DisconnectFromServer();
		if (s_Client->m_ClientUDPConnection)
		{
			EngineService::SubmitToEventQueue(CreateRef<Events::RemoveTickGeneratorUsage>(k_KeepAliveDelay));
		}
		KG_INFO("Closed client networking thread");
	}

	void ClientService::EndRun()
	{
		// Set run loop to close and alert network thread
		s_Client->m_NetworkContext.Quit = true;
		s_Client->NetworkThreadWakeUp();
		KG_INFO("Notified client network thread to close");
	}

	uint16_t ClientService::GetActiveSessionSlot()
	{
		if (s_Client)
		{
			return s_Client->m_SessionSlot;
		}
		// Client is unavailable so send error response
		return k_InvalidSessionSlot;
	}

	void ClientService::SendAllEntityLocation(UUID entityID, Math::vec3 location)
	{
		if (s_Client)
		{
			SubmitToEventQueue(CreateRef<Events::SendAllEntityLocation>(entityID, location));
		}
	}

	void ClientService::SendAllEntityPhysics(UUID entityID, Math::vec3 translation, Math::vec2 linearVelocity)
	{
		if (s_Client)
		{
			SubmitToEventQueue(CreateRef<Events::SendAllEntityPhysics>(entityID, translation, linearVelocity));
		}
	}

	void ClientService::EnableReadyCheck()
	{
		if (s_Client)
		{
			SubmitToEventQueue(CreateRef<Events::EnableReadyCheck>());
		}
	}

	void ClientService::SessionReadyCheck()
	{
		if (s_Client)
		{
			SubmitToEventQueue(CreateRef<Events::SessionReadyCheck>());
		}
	}

	void ClientService::RequestUserCount()
	{
		if (s_Client)
		{
			SubmitToEventQueue(CreateRef<Events::RequestUserCount>());
		}
	}

	void ClientService::RequestJoinSession()
	{
		if (s_Client)
		{
			SubmitToEventQueue(CreateRef<Events::RequestJoinSession>());
		}
	}

	void ClientService::LeaveCurrentSession()
	{
		if (s_Client)
		{
			SubmitToEventQueue(CreateRef<Events::LeaveCurrentSession>());
		}
	}

	void ClientService::SignalAll(uint16_t signal)
	{
		if (s_Client)
		{
			SubmitToEventQueue(CreateRef<Events::SignalAll>(signal));
		}
	}
	void ClientService::SubmitToFunctionQueue(const std::function<void()>& function)
	{
		if (!s_Client)
		{
			return;
		}
		std::scoped_lock<std::mutex> lock(s_Client->m_FunctionQueueMutex);

		s_Client->m_FunctionQueue.emplace_back(function);

		// Alert thread to wake up and process function
		s_Client->NetworkThreadWakeUp();
	}
	void ClientService::SubmitToEventQueue(Ref<Events::Event> e)
	{
		if (!s_Client)
		{
			return;
		}

		std::scoped_lock<std::mutex> lock(s_Client->m_EventQueueMutex);

		s_Client->m_EventQueue.emplace_back(e);

		// Alert thread to wake up and process event
		s_Client->NetworkThreadWakeUp();
	}
	void ClientService::OnEvent(Events::Event* e)
	{
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
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(MessageType::RequestUserCount);
		s_Client->SendTCP(msg);
		return true;
	}
	bool ClientService::OnStartSession(Events::StartSession event)
	{
		s_Client->m_SessionStartFrame = EngineService::GetActiveEngine().GetUpdateCount();
		return true;
	}
	bool ClientService::OnConnectionTerminated(Events::ConnectionTerminated event)
	{
		s_Client->m_SessionSlot = std::numeric_limits<uint16_t>::max();
		s_Client->m_SessionStartFrame = 0;
		return true;
	}
	bool ClientService::OnRequestJoinSession(Events::RequestJoinSession event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(MessageType::RequestJoinSession);
		s_Client->SendTCP(msg);
		return true;
	}
	bool ClientService::OnEnableReadyCheck(Events::EnableReadyCheck event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(MessageType::EnableReadyCheck);
		s_Client->SendTCP(msg);
		return true;
	}
	bool ClientService::OnSessionReadyCheck(Events::SessionReadyCheck event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(MessageType::SessionReadyCheck);
		s_Client->SendTCP(msg);
		return true;
	}
	bool ClientService::OnSendAllEntityLocation(Events::SendAllEntityLocation event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(MessageType::SendAllEntityLocation);
		msg << event.GetEntityID();
		Math::vec3 translation = event.GetTranslation();
		msg << translation.x;
		msg << translation.y;
		msg << translation.z;
		s_Client->SendUDP(msg);
		return true;
	}
	bool ClientService::OnSignalAll(Events::SignalAll event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(MessageType::SignalAll);
		msg << event.GetSignal();
		s_Client->SendTCP(msg);
		return true;
	}
	bool ClientService::OnAppTickEvent(Events::AppTickEvent event)
	{
		if (event.GetDelayMilliseconds() == k_KeepAliveDelay)
		{
			Kargono::Network::Message msg;
			msg.Header.ID = static_cast<uint32_t>(MessageType::KeepAlive);
			s_Client->SendUDP(msg);
			return true;
		}
		return false;
	}
	bool ClientService::OnSendAllEntityPhysics(Events::SendAllEntityPhysics event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(MessageType::SendAllEntityPhysics);
		msg << event.GetEntityID();
		Math::vec3 translation = event.GetTranslation();
		Math::vec2 linearVelocity = event.GetLinearVelocity();
		msg << translation.x;
		msg << translation.y;
		msg << translation.z;
		msg << linearVelocity.x;
		msg << linearVelocity.y;
		s_Client->SendUDP(msg);
		return true;
	}
	bool ClientService::OnLeaveCurrentSession(Events::LeaveCurrentSession event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(MessageType::LeaveCurrentSession);
		s_Client->SendTCP(msg);
		s_Client->m_SessionSlot = std::numeric_limits<uint16_t>::max();
		return true;
	}
	void ClientService::ProcessFunctionQueue()
	{
		KG_PROFILE_FUNCTION();
		std::scoped_lock<std::mutex> lock(s_Client->m_FunctionQueueMutex);

		for (auto& func : s_Client->m_FunctionQueue) { func(); }
		s_Client->m_FunctionQueue.clear();
	}
	void ClientService::ProcessEventQueue()
	{
		KG_PROFILE_FUNCTION();
		std::vector<Ref<Events::Event>> localEventCache;
		{
			std::scoped_lock<std::mutex> lock(s_Client->m_EventQueueMutex);
			localEventCache = std::move(s_Client->m_EventQueue);
			s_Client->m_EventQueue.clear();
		}

		for (auto& event : localEventCache)
		{
			OnEvent(event.get());
		}
	}

}



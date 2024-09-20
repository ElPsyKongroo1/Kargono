#include "kgpch.h"

#include "Kargono/Network/Client.h"

#include "Kargono/Core/Engine.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/Timers.h"


namespace Kargono::Network
{

	void ClientUDPConnection::Disconnect(asio::ip::udp::endpoint key)
	{
		if (!m_ActiveTCPConnection)
		{
			KG_ERROR("Invalid connection in UDP Disconnect()");
			return;
		}

		m_ActiveTCPConnection->Disconnect();
	}
	void ClientUDPConnection::AddToIncomingMessageQueue()
	{
		m_qMessagesIn.PushBack({ nullptr, m_MsgTemporaryIn });
		WakeUpNetworkThread();
		ReadMessage();
	}

	ClientTCPConnection::ClientTCPConnection(asio::io_context& asioContext, asio::ip::tcp::socket&& socket, TSQueue<owned_message>& qIn, std::condition_variable& newCV,
		std::mutex& newMutex)
		: TCPConnection(asioContext, std::move(socket), qIn, newCV, newMutex)
	{
		// Connection is Client -> Server, so we have nothing to define
		m_nHandshakeIn = 0;
		m_nHandshakeOut = 0;
	}
	bool ClientTCPConnection::Connect(const asio::ip::tcp::resolver::results_type& endpoints)
	{
		asio::error_code ec;
		//asio::connect(m_TCPSocket, endpoints, ec);

		asio::ip::tcp::resolver::iterator iter = endpoints.begin();
		asio::ip::tcp::resolver::iterator end; // End marker.

		ec.clear();
		bool connectionSuccessful = false;
		while (iter != end)
		{
			m_TCPSocket.connect(*iter, ec);
			if (!ec)
			{
				connectionSuccessful = true;
				break;
			}

			ec.clear();
			++iter;
		}
		if (!connectionSuccessful) { return false; }

		if (!ec)
		{
			ReadValidation();
			m_UDPLocalEndpoint = asio::ip::udp::endpoint(m_TCPSocket.local_endpoint().address(),
				m_TCPSocket.local_endpoint().port());
			m_UDPRemoteSendEndpoint = asio::ip::udp::endpoint(m_TCPSocket.remote_endpoint().address(),
				m_TCPSocket.remote_endpoint().port());
			m_UDPRemoteReceiveEndpoint = asio::ip::udp::endpoint(m_TCPSocket.remote_endpoint().address(),
				m_TCPSocket.remote_endpoint().port());
			// Connected Successfully
			return true;
		}

		// Failed to connect
		return false;

	}
	void ClientTCPConnection::Disconnect()
	{
		if (IsConnected())
		{
			asio::post(m_AsioContext, [this]()
			{
				m_TCPSocket.close();
			});
		}
		KG_INFO("[CLIENT]: Connection has been terminated");

		EngineService::SubmitToEventQueue(CreateRef<Events::ConnectionTerminated>());
		ClientService::SubmitToEventQueue(CreateRef<Events::ConnectionTerminated>());

	}

	void ClientTCPConnection::AddToIncomingMessageQueue()
	{
		m_IncomingMessageQueue.PushBack({ nullptr, m_MessageCache });
		WakeUpNetworkThread();
		ReadMessageHeader();
	}

	void ClientTCPConnection::WriteValidation()
	{
		asio::async_write(m_TCPSocket, asio::buffer(&m_nHandshakeOut, sizeof(uint64_t)),
			[this](std::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				ReadMessageHeader();
			}
			else
			{
				m_TCPSocket.close();
			}
		});
	}
	void ClientTCPConnection::ReadValidation()
	{
		asio::async_read(m_TCPSocket, asio::buffer(&m_nHandshakeIn, sizeof(uint64_t)),
			[&](std::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					// Connection is a client, so solve puzzle
					m_nHandshakeOut = Scramble(m_nHandshakeIn);

					// Write the result
					WriteValidation();
				}
				else
				{
					KG_INFO("Client Disconnected (ReadValidation)");
					Disconnect();
				}
			});
	}


	bool Client::ConnectToServer(const std::string& host, const uint16_t port, bool remote)
	{
		try
		{
			// Create TCP Socket for this new connection
			asio::ip::tcp::socket socket{m_AsioContext};

			if (remote)
			{
				// Get local TCP address and port
				asio::ip::tcp::resolver resolver(m_AsioContext);
				asio::ip::tcp::resolver::query query(asio::ip::host_name(), "");
				asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
				while (iter != asio::ip::tcp::resolver::iterator())
				{
					if (iter->endpoint().address().is_v4()) { break; }
					++iter;
				}

				// Create local endpoint and bind to new socket
				KG_ASSERT(port > 100, "Port must be greater than 100. If not, we have some issuessss!");
				asio::ip::tcp::endpoint localTCP = asio::ip::tcp::endpoint(iter->endpoint().address(), port - 100);
				socket.open(localTCP.protocol());
				socket.bind(localTCP);
			}

			// Resolve provided host/port into a TCP endpoint list
			asio::ip::tcp::resolver resolver(m_AsioContext);
			asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

			// Create a new connection object
			m_ClientTCPConnection = CreateRef<ClientTCPConnection>(
				m_AsioContext,
				std::move(socket),
				m_MessageInQueue,
				m_BlockThreadCV,
				m_BlockThreadMutex);

			// Initialize connection to remote endpoint
			KG_INFO("Attempting to TCP connection to server");
			if (!m_ClientTCPConnection->Connect(endpoints)) { return false; }
			KG_INFO("Successful TCP connection to server");

			KG_INFO("Attempting UDP connection to server");

			KG_TRACE_INFO("The local endpoint is {} {}", m_ClientTCPConnection->GetUDPLocalEndpoint().address().to_string(),
				m_ClientTCPConnection->GetUDPLocalEndpoint().port());
			KG_TRACE_INFO("The remote send endpoint is {} {}", m_ClientTCPConnection->GetUDPRemoteSendEndpoint().address().to_string(),
				m_ClientTCPConnection->GetUDPRemoteSendEndpoint().port());

			// Create Local UDP Socket
			asio::ip::udp::socket serverUDPSocket {m_AsioContext};
			// Get Local UDP Endpoint created from Connect Function
			//		and bind it to the socket
			asio::ip::udp::endpoint& localEndpoint = m_ClientTCPConnection->GetUDPLocalEndpoint();
			serverUDPSocket.open(localEndpoint.protocol());
			serverUDPSocket.bind(localEndpoint);

			m_ClientUDPConnection = CreateRef<ClientUDPConnection>(m_AsioContext, std::move(serverUDPSocket), m_MessageInQueue,
				m_BlockThreadCV, m_BlockThreadMutex, m_ClientTCPConnection);

			m_ClientUDPConnection->Start();

			// Start Context Thread
			m_AsioThread = std::thread([this]() { m_AsioContext.run(); });

			Utility::AsyncBusyTimer::CreateRecurringTimer(0.3f, 10, [&]()
			{
				ClientService::SubmitToFunctionQueue([&]()
					{
						KG_TRACE_INFO("Submitted a UDP Init");
						// Do checks for UDP connection
						Message newMessage{};
						newMessage.Header.ID = static_cast<uint32_t>(MessageType::UDPInit);
						SendUDP(newMessage);
					});
				WakeUpNetworkThread();
			}, [&]()
			{
				ClientService::SubmitToFunctionQueue([&]()
					{
						if (!m_UDPConnectionSuccessful)
						{
							DisconnectFromServer();
							return;
						}
						KG_INFO("UDP Connection Successful!");
					});
				WakeUpNetworkThread();
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
		if (IsConnected())
		{
			m_ClientTCPConnection->Disconnect();
		}

		m_AsioContext.stop();
		if (m_AsioThread.joinable())
			m_AsioThread.join();

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
		if (m_MessageInQueue.IsEmpty()) 
		{ 
			Wait(); 
		}

		size_t messageCount = 0;
		while (messageCount < nMaxMessages && !m_MessageInQueue.IsEmpty())
		{
			// Grab the front message
			auto msg = m_MessageInQueue.PopFront();

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

			m_ClientUDPConnection->Send(labeled);
		}
	}

	void Client::Wait()
	{
		std::unique_lock<std::mutex> lock(m_BlockThreadMutex);
		m_BlockThreadCV.wait(lock);
	}

	void Client::WakeUpNetworkThread()
	{
		std::unique_lock<std::mutex> lock(m_BlockThreadMutex);
		m_BlockThreadCV.notify_one();
	}

	void Client::SendChat(const std::string& text)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(MessageType::ClientChat);
		msg.PushBuffer((void*)text.data(), text.size());
		SendTCP(msg);
	}

	void Client::OpenMessageFromServer(Kargono::Network::Message& msg)
	{
		switch (static_cast<MessageType>(msg.Header.ID))
		{
		case MessageType::AcceptConnection:
		{
			KG_INFO("[SERVER]: Connection has been accepted!");
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
			data = msg.GetBuffer(dataSize);
			std::string text((char*)(data.data()), dataSize);
			KG_INFO("[{}]: {}", clientID, text);
			break;
		}

		case MessageType::ApproveJoinSession:
		{
			KG_INFO("[SERVER]: Approved joining session");
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
			KG_INFO("[SERVER]: Updated User Slot {}", userSlot);
			break;
		}

		case MessageType::UserLeftSession:
		{
			KG_INFO("[SERVER]: A User Left the Current Session");
			uint16_t userSlot{};
			msg >> userSlot;
			EngineService::SubmitToEventQueue(CreateRef<Events::UserLeftSession>(userSlot));
			break;
		}

		case MessageType::DenyJoinSession:
		{
			KG_INFO("[SERVER]: Denied joining session");
			break;
		}

		case MessageType::CurrentSessionInit:
		{
			KG_INFO("[SERVER]: Active Session is initializing...");
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
				KG_TRACE_INFO("Received UDP Init Packet");
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

	Ref<Network::Client> ClientService::s_Client { nullptr };

	void ClientService::Init()
	{
		// Create new client and start network thread
		s_Client = CreateRef<Network::Client>();
		s_Client->m_NetworkThread = CreateRef<std::thread>(&Network::ClientService::Run);
	}

	void ClientService::Terminate()
	{
		EndRun();
		s_Client->m_NetworkThread->join();
		s_Client->m_NetworkThread.reset();
		s_Client.reset();
	}

	void ClientService::Run()
	{
		bool remoteConnection = false;

		if (Projects::ProjectService::GetActiveServerLocation() != "LocalMachine")
		{
			remoteConnection = true;
		}

		if (!s_Client->ConnectToServer(Projects::ProjectService::GetActiveServerIP(), Projects::ProjectService::GetActiveServerPort(), remoteConnection)) { s_Client->m_Quit = true; }

		if (s_Client->m_ClientUDPConnection && s_Client->IsConnected())
		{
			// Start timer for keep alive packets
			EngineService::SubmitToEventQueue(CreateRef<Events::AddTickGeneratorUsage>(s_Client->m_ClientUDPConnection->GetKeepAliveDelay()));
		}

		while (!s_Client->m_Quit)
		{
			if (s_Client->IsConnected())
			{
				ClientService::ProcessEventQueue();
				ClientService::ProcessFunctionQueue();
				s_Client->CheckForMessages(5);
			}
			else
			{
				// Client is disconnected
				KG_WARN("Server Down");
				s_Client->m_Quit = true;
			}
		}
		s_Client->DisconnectFromServer();

		if (s_Client->m_ClientUDPConnection)
		{
			EngineService::SubmitToEventQueue(CreateRef<Events::RemoveTickGeneratorUsage>(s_Client->m_ClientUDPConnection->GetKeepAliveDelay()));
		}
	}

	void ClientService::EndRun()
	{
		s_Client->m_Quit = true;
		s_Client->WakeUpNetworkThread();
	}

	uint16_t ClientService::GetActiveSessionSlot()
	{
		if (s_Client)
		{
			return s_Client->m_SessionSlot;
		}
		// Client is unavailable so send error response
		return std::numeric_limits<uint16_t>::max();
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
		s_Client->WakeUpNetworkThread();
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
		if (event.GetDelayMilliseconds() == s_Client->m_ClientUDPConnection->GetKeepAliveDelay())
		{
			KG_TRACE_INFO("Send Keep Alive From Client");
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

		std::scoped_lock<std::mutex> lock(s_Client->m_EventQueueMutex);

		for (auto& event : s_Client->m_EventQueue)
		{
			OnEvent(event.get());
		}
		s_Client->m_EventQueue.clear();
	}

}



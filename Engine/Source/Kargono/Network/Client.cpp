#include "kgpch.h"

#include "Kargono/Network/Client.h"

#include "Kargono/Core/Engine.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/Timers.h"
#include "Kargono/Network/UDPClientConnection.h"


namespace Kargono::Network
{
	Ref<Network::Client> Client::s_Client { nullptr };
	Ref<std::thread> Client::s_NetworkThread { nullptr };

	Client::~Client()
	{
	}


	bool Client::Connect(const std::string& host, const uint16_t port, bool remote)
	{
		try
		{
			// Create TCP Socket for this new connection
			asio::ip::tcp::socket socket{m_context};

			if (remote)
			{
				// Get local TCP address and port
				asio::ip::tcp::resolver resolver(m_context);
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
			asio::ip::tcp::resolver resolver(m_context);
			asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

			// Create a new connection object
			m_connection = CreateRef<ConnectionToServer>(
				m_context,
				std::move(socket),
				m_qMessagesIn,
				m_BlockThreadCV,
				m_BlockThreadMx);

			// Initialize connection to remote endpoint
			KG_INFO("Attempting to TCP connection to server");
			if (!m_connection->Connect(endpoints)) { return false; }
			KG_INFO("Successful TCP connection to server");

			KG_INFO("Attempting UDP connection to server");

			KG_TRACE_INFO("The local endpoint is {} {}", m_connection->GetUDPLocalEndpoint().address().to_string(),
				m_connection->GetUDPLocalEndpoint().port());
			KG_TRACE_INFO("The remote send endpoint is {} {}", m_connection->GetUDPRemoteSendEndpoint().address().to_string(),
				m_connection->GetUDPRemoteSendEndpoint().port());

			// Create Local UDP Socket
			asio::ip::udp::socket serverUDPSocket {m_context};
			// Get Local UDP Endpoint created from Connect Function
			//		and bind it to the socket
			asio::ip::udp::endpoint& localEndpoint = m_connection->GetUDPLocalEndpoint();
			serverUDPSocket.open(localEndpoint.protocol());
			serverUDPSocket.bind(localEndpoint);

			m_UDPClient = CreateRef<UDPClientConnection>(m_context, std::move(serverUDPSocket), m_qMessagesIn,
				m_BlockThreadCV, m_BlockThreadMx, m_connection);

			m_UDPClient->Start();

			// Start Context Thread
			thrContext = std::thread([this]() { m_context.run(); });

			Utility::AsyncBusyTimer::CreateRecurringTimer(0.3f, 10, [&]()
				{
					Client::GetActiveClient()->SubmitToFunctionQueue([&]()
						{
							KG_TRACE_INFO("Submitted a UDP INIT BITCH");
							// Do checks for UDP connection
							Message newMessage{};
							newMessage.Header.ID = static_cast<uint32_t>(CustomMsgTypes::UDPInit);
							SendUDP(newMessage);
						});
					WakeUpNetworkThread();
				}, [&]()
				{
					Client::GetActiveClient()->SubmitToFunctionQueue([&]()
						{
							if (!m_UDPConnectionSuccessful)
							{
								Disconnect();
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

	void Client::Disconnect()
	{
		if (IsConnected())
		{
			m_connection->Disconnect();
		}

		m_context.stop();
		if (thrContext.joinable())
			thrContext.join();

		m_connection.reset();
	}

	bool Client::IsConnected()
	{
		if (m_connection) { return m_connection->IsConnected(); }
		return false;
	}

	void Client::Update(size_t nMaxMessages)
	{
		if (m_qMessagesIn.IsEmpty()) { Wait(); }

		size_t nMessageCount = 0;
		while (nMessageCount < nMaxMessages && !m_qMessagesIn.IsEmpty())
		{
			// Grab the front message
			auto msg = m_qMessagesIn.PopFront();

			// Pass to message handler
			OnMessage(msg.msg);

			nMessageCount++;
		}
	}

	void Client::Send(const Message& msg)
	{
		if (IsConnected())
			m_connection->Send(msg);
	}

	void Client::SendUDP(Message& msg)
	{
		if (IsConnected())
		{
			LabeledMessage labeled{ m_connection->GetUDPRemoteSendEndpoint(), msg };

			m_UDPClient->Send(labeled);
		}
	}

	void Client::Wait()
	{
		std::unique_lock<std::mutex> lock(m_BlockThreadMx);
		m_BlockThreadCV.wait(lock);
	}

	void Client::WakeUpNetworkThread()
	{
		std::unique_lock<std::mutex> lock(m_BlockThreadMx);
		m_BlockThreadCV.notify_one();
	}

	void Client::SendChat(const std::string& text)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(CustomMsgTypes::ClientChat);
		msg.PushBuffer((void*)text.data(), text.size());
		Send(msg);
	}

	void Client::OnEvent(Events::Event& e)
	{
		Events::EventDispatcher dispatcher(e);

		dispatcher.Dispatch<Events::RequestJoinSession>(KG_BIND_CLASS_FN(Client::OnRequestJoinSession));
		dispatcher.Dispatch<Events::RequestUserCount>(KG_BIND_CLASS_FN(Client::OnRequestUserCount));
		dispatcher.Dispatch<Events::LeaveCurrentSession>(KG_BIND_CLASS_FN(Client::OnLeaveCurrentSession));
		dispatcher.Dispatch<Events::StartSession>(KG_BIND_CLASS_FN(Client::OnStartSession));
		dispatcher.Dispatch<Events::ConnectionTerminated>(KG_BIND_CLASS_FN(Client::OnConnectionTerminated));
		dispatcher.Dispatch<Events::EnableReadyCheck>(KG_BIND_CLASS_FN(Client::OnEnableReadyCheck));
		dispatcher.Dispatch<Events::SessionReadyCheck>(KG_BIND_CLASS_FN(Client::OnSessionReadyCheck));
		dispatcher.Dispatch<Events::SendAllEntityLocation>(KG_BIND_CLASS_FN(Client::OnSendAllEntityLocation));
		dispatcher.Dispatch<Events::SendAllEntityPhysics>(KG_BIND_CLASS_FN(Client::OnSendAllEntityPhysics));
		dispatcher.Dispatch<Events::SignalAll>(KG_BIND_CLASS_FN(Client::OnSignalAll));
		dispatcher.Dispatch<Events::AppTickEvent>(KG_BIND_CLASS_FN(Client::OnAppTickEvent));

	}

	bool Client::OnRequestUserCount(Events::RequestUserCount event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(CustomMsgTypes::RequestUserCount);
		Send(msg);
		return true;
	}

	bool Client::OnStartSession(Events::StartSession event)
	{
		m_SessionStartFrame = EngineService::GetActiveEngine().GetUpdateCount();
		return true;
	}

	bool Client::OnConnectionTerminated(Events::ConnectionTerminated event)
	{
		m_SessionSlot = std::numeric_limits<uint16_t>::max();
		m_SessionStartFrame = 0;
		return true;
	}

	bool Client::OnRequestJoinSession(Events::RequestJoinSession event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(CustomMsgTypes::RequestJoinSession);
		Send(msg);
		return true;
	}

	bool Client::OnEnableReadyCheck(Events::EnableReadyCheck event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(CustomMsgTypes::EnableReadyCheck);
		Send(msg);
		return true;
	}

	bool Client::OnSessionReadyCheck(Events::SessionReadyCheck event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(CustomMsgTypes::SessionReadyCheck);
		Send(msg);
		return true;
	}

	bool Client::OnSendAllEntityLocation(Events::SendAllEntityLocation event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(CustomMsgTypes::SendAllEntityLocation);
		msg << event.GetEntityID();
		Math::vec3 translation = event.GetTranslation();
		msg << translation.x;
		msg << translation.y;
		msg << translation.z;
		SendUDP(msg);
		return true;
	}

	bool Client::OnSendAllEntityPhysics(Events::SendAllEntityPhysics event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(CustomMsgTypes::SendAllEntityPhysics);
		msg << event.GetEntityID();
		Math::vec3 translation = event.GetTranslation();
		Math::vec2 linearVelocity = event.GetLinearVelocity();
		msg << translation.x;
		msg << translation.y;
		msg << translation.z;
		msg << linearVelocity.x;
		msg << linearVelocity.y;
		SendUDP(msg);
		return true;
	}

	bool Client::OnSignalAll(Events::SignalAll event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(CustomMsgTypes::SignalAll);
		msg << event.GetSignal();
		Send(msg);
		return true;
	}

	bool Client::OnAppTickEvent(Events::AppTickEvent event)
	{
		if (event.GetDelayMilliseconds() == m_UDPClient->GetKeepAliveDelay())
		{
			KG_TRACE_INFO("Send Keep Alive From Client");
			Kargono::Network::Message msg;
			msg.Header.ID = static_cast<uint32_t>(CustomMsgTypes::KeepAlive);
			SendUDP(msg);
			return true;
		}
		return false;
	}

	bool Client::OnLeaveCurrentSession(Events::LeaveCurrentSession event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(CustomMsgTypes::LeaveCurrentSession);
		Send(msg);
		m_SessionSlot = std::numeric_limits<uint16_t>::max();
		return true;
	}

	void Client::RunClient()
	{
		auto currentProject = Projects::Project::GetActive();

		bool remoteConnection = false;

		if (currentProject->GetServerLocation() != "LocalMachine")
		{
			remoteConnection = true;
		}

		if (!Connect(currentProject->GetServerIP(), currentProject->GetServerPort(), remoteConnection)) { m_Quit = true; }

		if (m_UDPClient && IsConnected())
		{
			// Start timer for keep alive packets
			EngineService::SubmitToEventQueue(CreateRef<Events::AddTickGeneratorUsage>(m_UDPClient->GetKeepAliveDelay()));
		}

		while (!m_Quit)
		{
			if (IsConnected())
			{
				ProcessEventQueue();
				ExecuteFunctionQueue();
				Update(5);
			}
			else
			{
				// Client is disconnected
				KG_WARN("Server Down");
				m_Quit = true;
			}
		}
		Disconnect();

		if (m_UDPClient)
		{
			EngineService::SubmitToEventQueue(CreateRef<Events::RemoveTickGeneratorUsage>(m_UDPClient->GetKeepAliveDelay()));
		}
	}

	void Client::OnMessage(Kargono::Network::Message& msg)
	{
		switch (static_cast<CustomMsgTypes>(msg.Header.ID))
		{
		case CustomMsgTypes::AcceptConnection:
		{
			KG_INFO("[SERVER]: Connection has been accepted!");
			uint32_t userCount{};
			msg >> userCount;
			EngineService::SubmitToEventQueue(CreateRef<Events::UpdateOnlineUsers>(userCount));
			break;
		}

		case CustomMsgTypes::UpdateUserCount:
		{
			uint32_t userCount{};
			msg >> userCount;
			EngineService::SubmitToEventQueue(CreateRef<Events::UpdateOnlineUsers>(userCount));
			break;
		}

		case CustomMsgTypes::ServerChat:
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

		case CustomMsgTypes::ApproveJoinSession:
		{
			KG_INFO("[SERVER]: Approved joining session");
			uint16_t userSlot{};
			msg >> userSlot;
			m_SessionSlot = userSlot;
			EngineService::SubmitToEventQueue(CreateRef<Events::ApproveJoinSession>(userSlot));
			break;
		}

		case CustomMsgTypes::UpdateSessionUserSlot:
		{
			uint16_t userSlot{};
			msg >> userSlot;
			EngineService::SubmitToEventQueue(CreateRef<Events::UpdateSessionUserSlot>(userSlot));
			KG_INFO("[SERVER]: Updated User Slot {}", userSlot);
			break;
		}

		case CustomMsgTypes::UserLeftSession:
		{
			KG_INFO("[SERVER]: A User Left the Current Session");
			uint16_t userSlot{};
			msg >> userSlot;
			EngineService::SubmitToEventQueue(CreateRef<Events::UserLeftSession>(userSlot));
			break;
		}

		case CustomMsgTypes::DenyJoinSession:
		{
			KG_INFO("[SERVER]: Denied joining session");
			break;
		}

		case CustomMsgTypes::CurrentSessionInit:
		{
			KG_INFO("[SERVER]: Active Session is initializing...");
			EngineService::SubmitToEventQueue(CreateRef<Events::CurrentSessionInit>());
			break;
		}

		case CustomMsgTypes::InitSyncPing:
		{
			Kargono::Network::Message newMessage;
			newMessage.Header.ID = static_cast<uint32_t>(CustomMsgTypes::InitSyncPing);
				Send(newMessage);
				break;
			}

			case CustomMsgTypes::StartSession:
			{
				float waitTime{};
				msg >> waitTime;
				Utility::AsyncBusyTimer::CreateTimer(waitTime, [&]()
				{
					// Note Starting Update
					Network::Client::GetActiveClient()->SubmitToEventQueue(CreateRef<Events::StartSession>());

					// Open Gameplay
					EngineService::SubmitToEventQueue(CreateRef<Events::StartSession>());
				});
				
				break;
			}

			case CustomMsgTypes::SessionReadyCheckConfirm:
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

			case CustomMsgTypes::UpdateEntityLocation:
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

			case CustomMsgTypes::UpdateEntityPhysics:
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

			case CustomMsgTypes::ReceiveSignal:
			{
				uint16_t signal{};
				msg >> signal;
				EngineService::SubmitToEventQueue(CreateRef<Events::ReceiveSignal>(signal));
				break;
			}

			case CustomMsgTypes::KeepAlive:
			{
				break;
			}

			case CustomMsgTypes::UDPInit:
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

	uint16_t Client::GetActiveSessionSlot()
	{
		if (Network::Client::GetActiveClient())
		{
			return Network::Client::GetActiveClient()->GetSessionSlot();
		}
		// Client is unavailable so send error response
		return std::numeric_limits<uint16_t>::max();
	}

	void Client::SendAllEntityLocation(UUID entityID, Math::vec3 location)
	{
		if (Network::Client::GetActiveClient())
		{
			Network::Client::GetActiveClient()->SubmitToEventQueue(CreateRef<Events::SendAllEntityLocation>(entityID, location));
		}
	}

	void Client::SendAllEntityPhysics(UUID entityID, Math::vec3 translation, Math::vec2 linearVelocity)
	{
		if (GetActiveClient())
		{
			GetActiveClient()->SubmitToEventQueue(CreateRef<Events::SendAllEntityPhysics>(entityID, translation, linearVelocity));
		}
	}

	void Client::EnableReadyCheck()
	{
		if (GetActiveClient())
		{
			GetActiveClient()->SubmitToEventQueue(CreateRef<Events::EnableReadyCheck>());
		}
	}

	void Client::SessionReadyCheck()
	{
		if (GetActiveClient())
		{
			GetActiveClient()->SubmitToEventQueue(CreateRef<Events::SessionReadyCheck>());
		}
	}

	void Client::RequestUserCount()
	{
		if (GetActiveClient())
		{
			GetActiveClient()->SubmitToEventQueue(CreateRef<Events::RequestUserCount>());
		}
	}

	void Client::RequestJoinSession()
	{
		if (GetActiveClient())
		{
			GetActiveClient()->SubmitToEventQueue(CreateRef<Events::RequestJoinSession>());
		}
	}

	void Client::LeaveCurrentSession()
	{
		if (GetActiveClient())
		{
			GetActiveClient()->SubmitToEventQueue(CreateRef<Events::LeaveCurrentSession>());
		}
	}

	void Client::SignalAll(uint16_t signal)
	{
		if (GetActiveClient())
		{
			GetActiveClient()->SubmitToEventQueue(CreateRef<Events::SignalAll>(signal));
		}
	}

	void Client::StopClient()
	{
		m_Quit = true;
		WakeUpNetworkThread();
	}

	void Client::SubmitToFunctionQueue(const std::function<void()>& function)
	{
		std::scoped_lock<std::mutex> lock(m_FunctionQueueMutex);

		m_FunctionQueue.emplace_back(function);
	}

	void Client::ExecuteFunctionQueue()
	{
		KG_PROFILE_FUNCTION();
		std::scoped_lock<std::mutex> lock(m_FunctionQueueMutex);

		for (auto& func : m_FunctionQueue) { func(); }
		m_FunctionQueue.clear();
	}

	void Client::SubmitToEventQueue(Ref<Events::Event> e)
	{
		std::scoped_lock<std::mutex> lock(m_EventQueueMutex);

		m_EventQueue.emplace_back(e);

		// Alert thread to wake up and process event
		WakeUpNetworkThread();
	}

	void Client::ProcessEventQueue()
	{
		KG_PROFILE_FUNCTION();

		std::scoped_lock<std::mutex> lock(m_EventQueueMutex);

		for (auto& event : m_EventQueue)
		{
			OnEvent(*event);
		}
		m_EventQueue.clear();
	}
}



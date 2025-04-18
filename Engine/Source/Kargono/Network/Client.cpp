#include "kgpch.h"

#include "Kargono/Network/Client.h"

#include "Kargono/Core/Engine.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/Timers.h"

#include "Kargono/Utility/Operations.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Utility/Timers.h"

#include <conio.h>
#include <queue>

namespace Kargono::Network
{
	bool Client::Init(const ServerConfig& initConfig)
	{
		// Set config
		m_Config = initConfig;

		// Initialize the OS specific socket context
		if (!SocketContext::InitializeSockets())
		{
			KG_WARN("Failed to initialize platform socket context");
			return false;
		}

		// Open the Client socket
		if (!m_ClientSocket.Open(initConfig.m_ServerAddress.GetPort() + 1))
		{
			KG_WARN("Failed to create socket!");
			SocketContext::ShutdownSockets();
			return false;
		}
		// Initialize notifiers
		m_Notifiers.Init(&m_ClientActive);

		// Set client as active (inter-thread and network operations are now allowed)
		m_ClientActive = true;
		m_Notifiers.m_ClientStatusNotifier.Notify(true);

		// Initialize event thread
		if (!m_EventThread.Init(&m_ClientSocket, &m_NetworkThread))
		{
			KG_WARN("Failed to initialize the client's event thread!");
			Terminate(false);
			return false;
		}

		// Initialize network thread
		if (!m_NetworkThread.Init(&m_Config, &m_ClientSocket, &m_ClientActive, &m_EventThread, this))
		{
			KG_WARN("Failed to initialize the client's network thread!");
			Terminate(false);
			return false;
		}

		m_EventThread.SetSyncPingFreq(m_Config.m_ServerPassiveRefresh);

		return true;
	}

	bool ClientNetworkThread::Init(ServerConfig* serverConfig, Socket* clientSocket, std::atomic<bool>* clientActive, ClientEventThread* eventThread, Client* client)
	{
		KG_ASSERT(serverConfig);
		KG_ASSERT(clientSocket);
		KG_ASSERT(clientActive);
		KG_ASSERT(eventThread);
		KG_ASSERT(client);

		// Store dependencies
		i_ServerConfig = serverConfig;
		i_ClientSocket = clientSocket;
		i_ClientActive = clientActive;
		i_EventThread = eventThread;
		i_Client = client;

		// Initialize notifiers
		m_Notifiers.Init(clientActive);
		m_ReliabilityNotifiers.Init(clientActive);

		// Initialize server connection
		m_ServerConnection.Init(*i_ServerConfig);
		m_Notifiers.m_ConnectStatusNotifier.Notify(m_ServerConnection.m_Status, m_ServerConnection.m_ClientIndex);

		m_EventQueue.Init(KG_BIND_CLASS_FN(OnEvent));

		// Init request timers
		m_ManageConnectionTimer.InitializeTimer();
		m_ManageConnectionTimer.SetConstantFrameTime(
			std::chrono::nanoseconds(i_ServerConfig->m_ServerActiveRefresh * 1'000'000));
		m_RequestConnectionTimer.InitializeTimer(i_ServerConfig->m_RequestConnectionFrequency);

		// Send initial connection request
		m_ServerConnection.m_Status = ConnectionStatus::Connecting;
		m_Notifiers.m_ConnectStatusNotifier.Notify(m_ServerConnection.m_Status, m_ServerConnection.m_ClientIndex);

		SendRequestConnectionMessage();

		// Start request connection
		m_Thread.StartThread(KG_BIND_CLASS_FN(RequestConnection));
		return true;
	}

	void ClientNetworkThread::Terminate(bool withinNetworkThread)
	{
		m_Thread.StopThread(withinNetworkThread);
		m_ServerConnection.Terminate();
		m_Notifiers.m_ConnectStatusNotifier.Notify(m_ServerConnection.m_Status,
			m_ServerConnection.m_ClientIndex);
	}

	void ClientNetworkThread::WaitOnThread()
	{
		m_Thread.WaitOnThread();
	}

	bool ClientNetworkThread::StartConnection(ClientIndex index, bool withinNetworkThread)
	{
		KG_ASSERT(index != k_InvalidClientIndex);

		m_ServerConnection.m_Status = ConnectionStatus::Connected;
		m_ServerConnection.m_ClientIndex = index;
		m_Notifiers.m_ConnectStatusNotifier.Notify(m_ServerConnection.m_Status,
			m_ServerConnection.m_ClientIndex);

		// Ensure event thread is running quickly
		i_EventThread->SetSyncPingFreq(i_ServerConfig->m_ServerActiveRefresh);

		// Start network thread
		m_Thread.ChangeWorkFunction(KG_BIND_CLASS_FN(RunThread), withinNetworkThread);
		return true;
	}

	void ClientNetworkThread::HandleConnectionKeepAlive()
	{
		Connection& connection = m_ServerConnection.m_Connection;
		ReliabilityContext& reliableContext = connection.m_ReliabilityContext;

		// Handle sync pings
		if (connection.m_ReliabilityContext.m_CongestionContext.IsCongested())
		{
			if (m_CongestionCounter % 3 == 0)
			{
				// Send synchronization pings
				SendKeepAliveMessage();
			}
			m_CongestionCounter++;
		}
		else
		{
			SendKeepAliveMessage();
		}
	}

	bool ClientNetworkThread::HandleConnectionTimeout(float deltaTime)
	{
		Connection& connection = m_ServerConnection.m_Connection;
		ReliabilityContext& reliableContext = connection.m_ReliabilityContext;

		// Increment time since last sync ping for server connection
		reliableContext.OnUpdate(deltaTime);

		// Check for connection timeout
		if (reliableContext.m_LastPacketReceived > i_ServerConfig->m_ConnectionTimeout)
		{
			KG_WARN("Connection to server timed out");
			i_Client->Terminate(true);
			return false;
		}
		return true;
	}

	bool Client::Terminate(bool withinNetworkThread)
	{
		// Join the network thread
		m_NetworkThread.Terminate(withinNetworkThread);
		m_EventThread.Terminate(false);

		// Shutdown client socket
		m_ClientSocket.Close();
		SocketContext::ShutdownSockets();

		// Set client in-active
		m_ClientActive = false;
		m_Notifiers.m_ClientStatusNotifier.Notify(false);

		return true;
	}

	void Client::WaitOnThreads()
	{
		m_NetworkThread.WaitOnThread();
		m_EventThread.WaitOnThread();
	}

	void ClientNetworkThread::RunThread()
	{
		if (m_ManageConnectionTimer.CheckForUpdate())
		{
			// Update reliability observers
			ReliabilityContext& relContext{ m_ServerConnection.m_Connection.m_ReliabilityContext };
			m_ReliabilityNotifiers.m_ReliabilityStateNotifier.Notify(
				m_ServerConnection.m_ClientIndex,
				relContext.m_CongestionContext.IsCongested(), 
				relContext.m_RoundTripContext.GetAverageRoundTrip()
			);

			// Send keep-alive packets
			HandleConnectionKeepAlive();

			// Handle connection timeout
			if (!HandleConnectionTimeout(m_ManageConnectionTimer.GetConstantFrameTimeFloat()))
			{
				return;
			}
		}

		// Process the network event queue
		//m_EventQueue.ProcessQueue();
		m_WorkQueue.ProcessQueue();

		Address sender;
		unsigned char buffer[k_MaxPacketSize];
		int bytesRead{ 0 };

		do
		{
			bytesRead = i_ClientSocket->Receive(sender, buffer, sizeof(buffer));

			// Packet cannot be smaller than header
			if (bytesRead < k_PacketHeaderSize)
			{
				continue;
			}
			
			// Start forming incoming message
			Message msg;

			// Check for a valid app ID
			uint8_t* headerIterator{ buffer };
			AppID appID{ *(AppID*)buffer };
			if (appID != i_ServerConfig->m_AppProtocolID)
			{
				KG_WARN("Failed to validate the app ID from packet");
				continue;
			}
			headerIterator += sizeof(AppID);

			// Get the message type
			msg.m_Header.m_MessageType = *(MessageType*)headerIterator;
			if (IsConnectionManagementPacket(msg.m_Header.m_MessageType))
			{
				continue;
			}
			headerIterator += sizeof(MessageType);
			

			// Get the client index
			ClientIndex index = *(ClientIndex*)headerIterator;
			if (index != m_ServerConnection.m_ClientIndex)
			{
				KG_WARN("Failed to process packet. Invalid client index found {}. Active index is {}", 
					index, m_ServerConnection.m_ClientIndex);
				continue;
			}
			headerIterator += sizeof(ClientIndex);

			// Process reliability segment
			ReliabilityContext& relContext{ m_ServerConnection.m_Connection.m_ReliabilityContext };
			bool packetAccepted = relContext.ProcessReliabilitySegmentFromPacket(headerIterator);

			if (!packetAccepted)
			{
				continue;
			}

			// Process all ack's
			for (AckData data : relContext.GetRecentAcks())
			{
				m_ReliabilityNotifiers.m_AckPacketNotifier.Notify(index, data.m_Sequence, data.m_RTT);
			}

			// Load the packet payload into message
			if (bytesRead > k_PacketHeaderSize)
			{
				msg.m_Header.m_PayloadSize = bytesRead - k_PacketHeaderSize;
				msg.m_PayloadData.reserve(msg.m_Header.m_PayloadSize);
				memcpy(msg.m_PayloadData.data(), buffer + k_PacketHeaderSize, msg.m_Header.m_PayloadSize);
			}

			OpenMessageFromServer(msg);
			
		} while (bytesRead > 0);

		// Suspend the thread until an event occurs
		m_Thread.SuspendThread(true);
	}

	void ClientEventThread::RunThread()
	{
		m_WorkQueue.ProcessQueue();

		DWORD waitResult = WaitForMultipleObjects(1, &m_NetworkEventHandle, FALSE, (DWORD)m_ActiveSyncPingFreq);

		if (waitResult == WAIT_OBJECT_0)  // Network event
		{
			WSANETWORKEVENTS netEvents;
			WSAEnumNetworkEvents(i_ClientSocket->GetHandle(), m_NetworkEventHandle, &netEvents);

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

	void ClientNetworkThread::SubmitFunction(const std::function<void()> workFunction)
	{
		m_WorkQueue.SubmitFunction(workFunction);

		m_Thread.ResumeThread(false);
	}

	void ClientNetworkThread::SubmitEvent(Ref<Events::Event> event)
	{
		m_EventQueue.SubmitEvent(event);

		m_Thread.ResumeThread(false);
	}

	void ClientNetworkThread::OnEvent(Events::Event* event)
	{
		// Process events based on their type
		switch (event->GetEventType())
		{
		case Events::EventType::RequestJoinSession:
			SendRequestJoinSessionMessage();
			break;
		case Events::EventType::RequestUserCount:
			SendRequestUserCountMessage();
			break;
		case Events::EventType::LeaveCurrentSession:
			SendLeaveCurrentSessionMessage();
			break;
		case Events::EventType::StartSession:
			m_SessionStartFrame = EngineService::GetActiveEngine().GetUpdateCount();
			break;
		case Events::EventType::ConnectionTerminated:
			// Essentially clear all session information from this client context
			m_SessionSlot = k_InvalidSessionSlot;
			m_SessionStartFrame = 0;
			break;
		case Events::EventType::EnableReadyCheck:
			SendEnableReadyCheckMessage();
			break;
		case Events::EventType::SendReadyCheck:
			SendSessionReadyCheckMessage();
			break;
		case Events::EventType::SendAllEntityLocation:
			SendAllEntityLocation(*(Events::SendAllEntityLocation*)event);
			break;
		case Events::EventType::SendAllEntityPhysics:
			SendAllEntityPhysicsMessage(*(Events::SendAllEntityPhysics*)event);
			break;
		case Events::EventType::SignalAll:
			SendAllClientsSignalMessage(*(Events::SignalAll*)event);
			break;
		}
	}

	void ClientNetworkThread::RequestConnection()
	{
		// Check for a network update
		while (m_ManageConnectionTimer.CheckForUpdate())
		{
			// Handle sending connection requests
			if (m_RequestConnectionTimer.CheckForUpdate(m_ManageConnectionTimer.GetConstantFrameTime()))
			{
				// Send connection request
				SendRequestConnectionMessage();
			}

			// Handle the case of a timeout
			if (!HandleConnectionTimeout(m_ManageConnectionTimer.GetConstantFrameTimeFloat()))
			{
				return;
			}
		}

		Address sender;
		unsigned char buffer[k_MaxPacketSize];
		int bytes_read{ 0 };

		do
		{
			bytes_read = i_ClientSocket->Receive(sender, buffer, sizeof(buffer));

			if (bytes_read < k_PacketHeaderSize)
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

			// Get packet type
			MessageType type = *(MessageType*)headerIterator;
			headerIterator += sizeof(MessageType);

			ClientIndex index = *(ClientIndex*)headerIterator;
			headerIterator += sizeof(ClientIndex);

			if (!IsConnectionManagementPacket(type))
			{
				continue;
			}

			if (!OpenManagementPacket(type, index))
			{
				return;
			}
			
		} while (bytes_read > 0);

		// Suspend the thread until an event occurs
		m_Thread.SuspendThread(true);

	}

	void ClientNetworkThread::ResumeThread(bool withinThread)
	{
		m_Thread.ResumeThread(withinThread);
	}

	void ClientNetworkThread::SuspendThread(bool withinThread)
	{
		m_Thread.SuspendThread(withinThread);
	}

	bool ClientNetworkThread::SendToServer(Message& msg)
	{
		KG_ASSERT(msg.m_Header.m_PayloadSize < k_MaxPayloadSize);

		Connection& connection = m_ServerConnection.m_Connection;

		// Prepare the final data buffer
		uint8_t buffer[k_MaxPacketSize];
		uint8_t* headerIterator{ buffer };

		// Set the app ID
		AppID& appIDLocation = *(AppID*)headerIterator;
		appIDLocation = i_ServerConfig->m_AppProtocolID;
		headerIterator += sizeof(AppID);

		// Set the packet type
		MessageType& packetTypeLocation = *(MessageType*)headerIterator;
		packetTypeLocation = msg.m_Header.m_MessageType;
		headerIterator += sizeof(MessageType);

		// Send the client connection Index
		ClientIndex& clientIndexLocation = *(ClientIndex*)headerIterator;
		clientIndexLocation = m_ServerConnection.m_ClientIndex;
		headerIterator += sizeof(ClientIndex);

		if (!IsConnectionManagementPacket(msg.m_Header.m_MessageType))
		{
			// Set reliability segment
			connection.m_ReliabilityContext.InsertReliabilitySegmentIntoPacket(headerIterator);

			PacketSequence sentPacket{ *(PacketSequence*)headerIterator };
			m_ReliabilityNotifiers.m_SendPacketNotifier.Notify(m_ServerConnection.m_ClientIndex, sentPacket);
		}

		if (msg.m_Header.m_PayloadSize > 0)
		{
			// Set the payload data
			memcpy(&buffer[k_PacketHeaderSize], msg.m_PayloadData.data(), msg.m_Header.m_PayloadSize);
		}

		// Send the message
		bool sendSuccess{ false };
		sendSuccess = i_ClientSocket->Send(connection.m_Address, buffer, msg.m_Header.m_PayloadSize + k_PacketHeaderSize);

		return sendSuccess;
	}

	void ConnectionToServer::Init(const ServerConfig& config)
	{
		m_Connection.m_Address = config.m_ServerAddress;
		m_Connection.m_ReliabilityContext = ReliabilityContext();
		m_Status = ConnectionStatus::Disconnected;
		m_ClientIndex = k_InvalidClientIndex;
	}

	void ConnectionToServer::Terminate()
	{
		m_Connection.m_Address = Address();
		m_Connection.m_ReliabilityContext = ReliabilityContext();
		m_Status = ConnectionStatus::Disconnected;
		m_ClientIndex = k_InvalidClientIndex;
	}

	void ClientNetworkThread::SendRequestUserCountMessage()
	{
		Message msg;
		msg.m_Header.m_MessageType = MessageType::ServerQuery_RequestClientCount;
		SendToServer(msg);
	}

	void ClientNetworkThread::SendAllEntityLocation(Events::SendAllEntityLocation& event)
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
		SendToServer(msg);
	}

	void ClientNetworkThread::SendRequestJoinSessionMessage()
	{
		// Send a message that prompts the server to allow the client to join.
		// Should receive either an accept or deny message back.
		Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageSession_RequestClientJoin;

		// Send the message reliably with TCP
		SendToServer(msg);
	}

	void ClientNetworkThread::SendEnableReadyCheckMessage()
	{
		// Send the message to signify this client is ready
		Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageSession_EnableReadyCheck;

		// Send the message reliably with TCP
		SendToServer(msg);
	}

	void ClientNetworkThread::SendSessionReadyCheckMessage()
	{
		// Send a message to the server initiating a ready check
		Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageSession_StartReadyCheck;

		// Send the message reliably with TCP
		SendToServer(msg);
	}

	void ClientNetworkThread::SendAllClientsSignalMessage(Events::SignalAll& event)
	{
		// Send a scripting specific signal to all other clients
		Message msg;
		msg.m_Header.m_MessageType = MessageType::ScriptMessaging_SendAllClientsSignal;
		msg << event.GetSignal();

		// Send the message reliably with TCP
		SendToServer(msg);
	}

	void ClientNetworkThread::SendKeepAliveMessage()
	{
		// Send a simple message that keeps the UDP "connection" alive
		Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageConnection_KeepAlive;

		// Send the message quickly with UDP
		SendToServer(msg);
	}

	void ClientNetworkThread::SendAllEntityPhysicsMessage(Events::SendAllEntityPhysics& event)
	{
		// Create/send a message that updates all other clients with a particular entity's 
		// ID, location, and velocity.
		Message msg;
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
		SendToServer(msg);
	}

	void ClientNetworkThread::SendLeaveCurrentSessionMessage()
	{
		// Send a message notifying the server and all other clients that this client is leaving
		Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageSession_NotifyAllLeave;

		// Send the message reliably with TCP
		SendToServer(msg);

		// Ensure current application/client is aware our session is invalid
		m_SessionSlot = std::numeric_limits<uint16_t>::max();
	}

	void ClientNetworkThread::SendRequestConnectionMessage()
	{
		Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageConnection_RequestConnection;
		SendToServer(msg);
	}

	bool ClientNetworkThread::OpenManagementPacket(MessageType type, ClientIndex index)
	{
		if (type == MessageType::ManageConnection_AcceptConnection)
		{
			KG_WARN("Connection successful!");
			StartConnection(index, true);
			return false;
		}
		else if (type == MessageType::ManageConnection_DenyConnection)
		{
			KG_WARN("Connection to server denied");
			i_Client->Terminate(true);
			return false;
		}

		return true;
	}

	void ClientNetworkThread::OpenMessageFromServer(Kargono::Network::Message& msg)
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
			break;
		default:
			KG_ERROR("Invalid message type received from the server!");
			break;
		}
	}

	void ClientNetworkThread::OpenAcceptConnectionMessage(Kargono::Network::Message& msg)
	{
		KG_INFO("Connection to the server has been accepted!");

		// Pass the event along to the main thread
		size_t userCount{};
		msg >> userCount;
		EngineService::SubmitToEventQueue(CreateRef<Events::ReceiveOnlineUsers>(userCount));
	}

	void ClientNetworkThread::OpenReceiveUserCountMessage(Message& msg)
	{
		// Pass the event along to the main thread
		size_t userCount{};
		msg >> userCount;
		EngineService::SubmitToEventQueue(CreateRef<Events::ReceiveOnlineUsers>(userCount));
	}

	void ClientNetworkThread::OpenApproveJoinSessionMessage(Message& msg)
	{
		KG_INFO("Approved joining session");

		// Get the indicated user slot
		SessionIndex userSlot{};
		msg >> userSlot;
		m_SessionSlot = userSlot;

		// Pass the event along to the main thread
		EngineService::SubmitToEventQueue(CreateRef<Events::ApproveJoinSession>(userSlot));
	}

	void ClientNetworkThread::OpenUpdateSessionUserSlotMessage(Message& msg)
	{
		// Get the indicated user slot
		SessionIndex userSlot{};
		msg >> userSlot;

		// Pass the event along to the main thread
		EngineService::SubmitToEventQueue(CreateRef<Events::UpdateSessionUserSlot>(userSlot));
	}

	void ClientNetworkThread::OpenUserLeftSessionMessage(Message& msg)
	{
		KG_INFO("A User Left the Current Session");

		// Get the indicated user slot
		SessionIndex userSlot{};
		msg >> userSlot;

		// Pass the event along to the main thread
		EngineService::SubmitToEventQueue(CreateRef<Events::UserLeftSession>(userSlot));
	}

	void ClientNetworkThread::OpenDenyJoinSessionMessage(Message& msg)
	{
		KG_INFO("Denied joining session");
	}

	void ClientNetworkThread::OpenCurrentSessionInitMessage(Message& msg)
	{
		KG_INFO("Active Session is initializing");

		// Pass the event along to the main thread
		EngineService::SubmitToEventQueue(CreateRef<Events::CurrentSessionInit>());
	}

	void ClientNetworkThread::OpenStartSessionMessage(Message& msg)
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
			m_EventQueue.SubmitEvent(CreateRef<Events::StartSession>());

			// Pass the event along to the main thread
			EngineService::SubmitToEventQueue(CreateRef<Events::StartSession>());
		});
	}

	void ClientNetworkThread::OpenSessionReadyCheckConfirmMessage(Message& msg)
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

	void ClientNetworkThread::OpenUpdateEntityLocationMessage(Message& msg)
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

	void ClientNetworkThread::OpenUpdateEntityPhysicsMessage(Message& msg)
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

	void ClientNetworkThread::OpenReceiveSignalMessage(Message& msg)
	{
		// Get the indicated signal (depends on the application)
		uint16_t signal{};
		msg >> signal;

		// Pass the event along to the main thread
		EngineService::SubmitToEventQueue(CreateRef<Events::ReceiveSignal>(signal));
	}

	bool ClientService::Init()
	{
		if (s_Client.m_ClientActive)
		{
			KG_WARN("Failed to initialize client. A client context already exists.")
			return false;
		}

		if (!s_Client.Init(Projects::ProjectService::GetServerConfig()))
		{
			KG_WARN("Failed to initialize client. Client initialization failed.")
			return false;
		}

		KG_VERIFY(s_Client.m_ClientActive, "Client connection init");
		return true;
	}

	bool ClientService::Terminate()
	{
		if (!s_Client.m_ClientActive)
		{
			KG_WARN("Failed to terminate client. No client context is active.")
			return false;
		}

		if (!s_Client.Terminate(false))
		{
			KG_WARN("Failed to terminate client. Terminate function failed.");
			return false;
		}

		KG_VERIFY(!s_Client.m_ClientActive, "Closed client connection");
		return true;
	}

	bool ClientService::IsClientActive()
	{
		return s_Client.m_ClientActive;
	}

	uint16_t ClientService::GetActiveSessionSlot()
	{
		KG_ASSERT(s_Client.m_ClientActive);

		// Client is unavailable so send error response
		return k_InvalidSessionSlot;
	}

	void ClientService::SendAllEntityLocation(UUID entityID, Math::vec3 location)
	{
		KG_ASSERT(s_Client.m_ClientActive);

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::SendAllEntityLocation>(entityID, location));
	}

	void ClientService::SendAllEntityPhysics(UUID entityID, Math::vec3 translation, Math::vec2 linearVelocity)
	{
		KG_ASSERT(s_Client.m_ClientActive);

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::SendAllEntityPhysics>(entityID, translation, linearVelocity));
	}

	void ClientService::EnableReadyCheck()
	{
		KG_ASSERT(s_Client.m_ClientActive);

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::EnableReadyCheck>());
	}

	void ClientService::SessionReadyCheck()
	{
		KG_ASSERT(s_Client.m_ClientActive);

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::SessionReadyCheck>());
	}

	void ClientService::RequestUserCount()
	{
		KG_ASSERT(s_Client.m_ClientActive);

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::RequestUserCount>());
	}

	void ClientService::RequestJoinSession()
	{
		KG_ASSERT(s_Client.m_ClientActive);

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::RequestJoinSession>());
	}

	void ClientService::LeaveCurrentSession()
	{
		KG_ASSERT(s_Client.m_ClientActive);

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::LeaveCurrentSession>());
	}

	void ClientService::SignalAll(uint16_t signal)
	{
		KG_ASSERT(s_Client.m_ClientActive);

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::SignalAll>(signal));
	}
	void ClientService::SubmitToNetworkFunctionQueue(const std::function<void()>& function)
	{
		KG_ASSERT(s_Client.m_ClientActive);

		s_Client.GetNetworkThread().SubmitFunction(function);
	}
	void ClientService::SubmitToNetworkEventQueue(Ref<Events::Event> e)
	{
		KG_ASSERT(s_Client.m_ClientActive);

		s_Client.GetNetworkThread().SubmitEvent(e);
	}
	bool ClientEventThread::Init(Socket* clientSocket, ClientNetworkThread* networkThread)
	{
		KG_ASSERT(clientSocket);
		KG_ASSERT(networkThread);

		// Store dependencies
		i_ClientSocket = clientSocket;
		i_NetworkThread = networkThread;

		// Create network event
		m_NetworkEventHandle = WSACreateEvent();
		if (WSAEventSelect(i_ClientSocket->GetHandle(), m_NetworkEventHandle, FD_READ) != 0)
		{
			KG_WARN("Failed to create the network event handle");
			return false;
		}

		m_Thread.StartThread(KG_BIND_CLASS_FN(RunThread));

		return true;
	}
	void ClientEventThread::Terminate(bool withinEventThread)
	{
		m_Thread.StopThread(withinEventThread);
	}
	void ClientEventThread::WaitOnThread()
	{
		m_Thread.StopThread(false);
	}
	void ClientEventThread::SetSyncPingFreq(size_t frequency)
	{
		m_WorkQueue.SubmitFunction([&, frequency]()
		{
			m_ActiveSyncPingFreq = frequency;
		});
	}
	void ClientNotifiers::Init(std::atomic<bool>* clientActive)
	{
		KG_ASSERT(clientActive);

		i_ClientActive = clientActive;
	}
	ObserverIndex ClientNotifiers::AddClientActiveObserver(std::function<void(bool)> func)
	{
		KG_ASSERT(!i_ClientActive || !*i_ClientActive);

		return m_ClientStatusNotifier.AddObserver(func);
	}
	bool ClientNotifiers::RemoveClientActiveObserver(ObserverIndex index)
	{
		KG_ASSERT(!i_ClientActive || !*i_ClientActive);

		return m_ClientStatusNotifier.RemoveObserver(index);
	}
	void ClientNetworkNotifiers::Init(std::atomic<bool>* clientActive)
	{
		KG_ASSERT(clientActive);

		i_ClientActive = clientActive;
	}
	ObserverIndex ClientNetworkNotifiers::AddConnectStatusObserver(std::function<void(ConnectionStatus, ClientIndex)> func)
	{
		KG_ASSERT(!i_ClientActive || !*i_ClientActive);

		return m_ConnectStatusNotifier.AddObserver(func);
	}
}

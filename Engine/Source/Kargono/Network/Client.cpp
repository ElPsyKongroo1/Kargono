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

static HANDLE hNetworkEvent;
static HANDLE hInputEvent;
static HANDLE allEvents[2];
static std::string text;

namespace Kargono::Network
{
	bool Client::InitClient(const ServerConfig& initConfig)
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

		// Initialize server connection
		m_ServerConnection.Init(initConfig);

		m_NetworkEventQueue.Init(KG_BIND_CLASS_FN(OnEvent));

		// TODO: Move this please for the love of god
		// Create network event
		hNetworkEvent = WSACreateEvent();
		if (WSAEventSelect(m_ClientSocket.GetHandle(), hNetworkEvent, FD_READ) != 0)
		{
			KG_WARN("Failed to create the network event handle");
			return false;
		}

		// Set client as active (inter-thread and network operations are now allowed)
		s_ClientActive = true;

		// Get console input handle
		hInputEvent = GetStdHandle(STD_INPUT_HANDLE);
		SetConsoleMode(hInputEvent, 0);

		// Wait for both events
		allEvents[0] = hNetworkEvent;
		allEvents[1] = hInputEvent;

		// Initialize local timers
		m_NetworkThreadTimer.InitializeTimer();
		m_RequestConnectionTimer.InitializeTimer(m_Config.m_RequestConnectionFrequency);

		// Send initial connection request
		m_ServerConnection.m_Status = ConnectionStatus::Connecting;

		Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageConnection_RequestConnection;
		SendToServer(msg);

		// Start request connection
		m_NetworkThread.StartThread(KG_BIND_CLASS_FN(RequestConnection));
		m_NetworkEventThread.StartThread(KG_BIND_CLASS_FN(RunNetworkEventThread));
		return true;
	}

	bool Client::StartConnection(bool withinNetworkThread)
	{
		// Start network thread
		m_NetworkThreadTimer.InitializeTimer();
		m_KeepAliveTimer.InitializeTimer(m_Config.m_SyncPingFrequency);
		m_NetworkThread.ChangeWorkFunction(KG_BIND_CLASS_FN(RunNetworkThread), withinNetworkThread);
		return true;
	}

	bool Client::TerminateClient(bool withinNetworkThread)
	{
		// Join the network thread
		m_NetworkThread.StopThread(withinNetworkThread);
		m_NetworkEventThread.StopThread(false);

		// Shutdown client socket
		m_ClientSocket.Close();

		// Clean up socket resources
		SocketContext::ShutdownSockets();
		s_ClientActive = false;

		return true;
	}

	void Client::WaitOnClientTerminate()
	{
		m_NetworkThread.WaitOnThread();
		m_NetworkEventThread.WaitOnThread();
	}

	void Client::RunNetworkThread()
	{
		// Process the network event queue
		m_NetworkEventQueue.ProcessQueue();

		Address sender;
		unsigned char buffer[k_MaxPacketSize];
		int bytes_read{ 0 };

		do
		{
			bytes_read = m_ClientSocket.Receive(sender, buffer, sizeof(buffer));
			if (bytes_read >= k_PacketHeaderSize)
			{
				// Check for a valid app ID
				if (*(AppID*)&buffer != m_Config.m_AppProtocolID)
				{
					KG_WARN("Failed to validate the app ID from packet");
					continue;
				}

				MessageType type = (MessageType)buffer[sizeof(AppID)];

				if (IsConnectionManagementPacket(type))
				{
					continue;
				}

				// TODO: Verify this message is for the correct client
				ClientIndex index = (ClientIndex)buffer[sizeof(AppID) + sizeof(MessageType)];

				// Process reliability segment
				m_ServerConnection.m_Connection.m_ReliabilityContext.ProcessReliabilitySegmentFromPacket(&buffer[sizeof(AppID) + sizeof(MessageType) + sizeof(ClientIndex)]);


				switch (type)
				{
				case MessageType::ManageConnection_KeepAlive:
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
					KG_WARN("");
					break;
				}
				default:
					KG_WARN("Invalid packet ID obtained");
					continue;;
				}
			}
		} while (bytes_read > 0);

		// Suspend the thread until an event occurs
		m_NetworkThread.SuspendThread(true);

		// process packet
	}

	void Client::RunNetworkEventThread()
	{
		DWORD waitResult = WaitForMultipleObjects(2, allEvents, FALSE, m_Config.m_SyncPingFrequencyMs);

		if (waitResult == WAIT_OBJECT_0)  // Network event
		{
			WSANETWORKEVENTS netEvents;
			WSAEnumNetworkEvents(m_ClientSocket.GetHandle(), hNetworkEvent, &netEvents);

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
		else
		{
			//m_NetworkEventQueue.SubmitEvent(CreateRef<Events::Up>());
		}
	}

	bool Client::HandleConsoleInput(Events::KeyPressedEvent event)
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
			return false;
		}
		if (key == 13)
		{
			Message msg;
			msg.m_Header.m_MessageType = MessageType::GenericMessage_ServerChat;
			msg.AppendPayload(text.data(), (int)strlen(text.data()) + 1);
			SendToServer(msg);
			text.clear();
		}

		return true;
	}

	void Client::OnEvent(Events::Event* event)
	{
		Connection& connection = m_ServerConnection.m_Connection;
		ReliabilityContext& reliableContext = connection.m_ReliabilityContext;

		if (event->GetEventType() == Events::EventType::AppUpdate)
		{
			// Handle sync pings
			if (m_KeepAliveTimer.CheckForUpdate(m_NetworkThreadTimer.GetConstantFrameTime()))
			{
				if (connection.m_ReliabilityContext.m_CongestionContext.IsCongested())
				{
					static uint32_t s_CongestionCounter{ 0 };
					if (s_CongestionCounter % 3 == 0)
					{
						// Send synchronization pings
						Message msg;
						msg.m_Header.m_MessageType = MessageType::ManageConnection_KeepAlive;
						SendToServer(msg);
					}
					s_CongestionCounter++;
				}
				else
				{
					Message msg;
					msg.m_Header.m_MessageType = MessageType::ManageConnection_KeepAlive;
					SendToServer(msg);
				}
			}

			// Increment time since last sync ping for server connection
			reliableContext.OnUpdate(m_NetworkThreadTimer.GetConstantFrameTimeFloat());

			if (reliableContext.m_LastPacketReceived > m_Config.m_ConnectionTimeout)
			{
				m_ServerConnection.Terminate();
				m_NetworkThread.StopThread(true);
				KG_WARN("Connection closed");
				return;
			}
		}
		else if (event->GetEventType() == Events::EventType::KeyPressed)
		{
			// Handle any input events from the console
			HandleConsoleInput(*(Events::KeyPressedEvent*)event);
			return;
		}


	}

	void Client::SubmitEvent(Ref<Events::Event> event)
	{
		m_NetworkEventQueue.SubmitEvent(event);

		m_NetworkEventThread.ResumeThread();
	}

	void Client::RequestConnection()
	{
		ReliabilityContext& reliabilityContext = m_ServerConnection.m_Connection.m_ReliabilityContext;

		// Check for a network update
		if (!m_NetworkThreadTimer.CheckForUpdate())
		{
			return;
		}

		// Handle sending connection requests
		if (m_RequestConnectionTimer.CheckForUpdate(m_NetworkThreadTimer.GetConstantFrameTime()))
		{
			// Send connection request
			Message msg;
			msg.m_Header.m_MessageType = MessageType::ManageConnection_RequestConnection;
			SendToServer(msg);
		}

		// Increment time since start of connection attempt
		reliabilityContext.m_LastPacketReceived += m_NetworkThreadTimer.GetConstantFrameTimeFloat();
		if (reliabilityContext.m_LastPacketReceived > m_Config.m_ConnectionTimeout)
		{
			m_ServerConnection.m_Status = ConnectionStatus::Disconnected;
			TerminateClient(true);
			return;
		}

		Address sender;
		unsigned char buffer[k_MaxPacketSize];
		int bytes_read{ 0 };

		do
		{
			bytes_read = m_ClientSocket.Receive(sender, buffer, sizeof(buffer));
			if (bytes_read >= k_PacketHeaderSize)
			{
				// Check for a valid app ID
				if (*(AppID*)&buffer != m_Config.m_AppProtocolID)
				{
					KG_WARN("Failed to validate the app ID from packet");
					continue;
				}

				// Get packet type
				MessageType type = (MessageType)buffer[sizeof(AppID)];

				ClientIndex index = (ClientIndex)buffer[sizeof(AppID) + sizeof(MessageType)];

				if (!IsConnectionManagementPacket(type))
				{
					continue;
				}

				if (type == MessageType::ManageConnection_AcceptConnection)
				{
					m_ServerConnection.m_Status = ConnectionStatus::Connected;
					m_ServerConnection.m_ClientIndex = index;
					KG_WARN("Connection successful!");
					StartConnection(true);
					return;
				}
				else if (type == MessageType::ManageConnection_DenyConnection)
				{
					m_ServerConnection.m_Status = ConnectionStatus::Disconnected;
					KG_WARN("Connection denied!");
					TerminateClient(true);
					return;
				}
			}
		} while (bytes_read > 0);

	}

	bool Client::SendToServer(Message& msg)
	{
		Connection& connection = m_ServerConnection.m_Connection;

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
		MessageType& packetTypeLocation = *(MessageType*)&buffer[sizeof(AppID)];
		packetTypeLocation = msg.m_Header.m_MessageType;

		// Send the client connection Index
		ClientIndex& clientIndexLocation = *(ClientIndex*)&buffer[sizeof(AppID) + sizeof(MessageType)];
		clientIndexLocation = m_ServerConnection.m_ClientIndex;

		if (!IsConnectionManagementPacket(msg.m_Header.m_MessageType))
		{
			// Set reliability segment
			connection.m_ReliabilityContext.InsertReliabilitySegmentIntoPacket(&buffer[sizeof(AppID) + sizeof(MessageType) + sizeof(ClientIndex)]);
		}

		if (msg.m_Header.m_PayloadSize > 0)
		{
			// Set the payload data
			memcpy(&buffer[k_PacketHeaderSize], msg.m_PayloadData.data(), msg.m_Header.m_PayloadSize);
		}

		// Send the message
		bool sendSuccess{ false };
		sendSuccess = m_ClientSocket.Send(connection.m_Address, buffer, msg.m_Header.m_PayloadSize + k_PacketHeaderSize);

		return sendSuccess;
	}

	void ConnectionToServer::Init(const ServerConfig& config)
	{
		m_Connection.m_Address = config.m_ServerAddress;
		m_Connection.m_ReliabilityContext.m_LastPacketReceived = 0.0f;
		m_Status = ConnectionStatus::Disconnected;
		m_ClientIndex = k_InvalidClientIndex;
	}

	void ConnectionToServer::Terminate()
	{
		m_Connection.m_Address = Address();
		m_Connection.m_ReliabilityContext.m_LastPacketReceived = 0.0f;
		m_Status = ConnectionStatus::Disconnected;
		m_ClientIndex = k_InvalidClientIndex;
	}

	void Client::SendRequestUserCountMessage()
	{
		Message msg;
		msg.m_Header.m_MessageType = MessageType::ServerQuery_RequestClientCount;
		SendToServer(msg);
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
		SendToServer(msg);
	}

	void Client::SendInitSyncPingMessage()
	{
		// Send a simple init sync ping message
		Kargono::Network::Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::ManageSession_SyncPing;
		// Send the message reliably with TCP
		SendToServer(newMessage);
	}

	void Client::SendRequestJoinSessionMessage()
	{
		// Send a message that prompts the server to allow the client to join.
		// Should receive either an accept or deny message back.
		Kargono::Network::Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageSession_RequestClientJoin;

		// Send the message reliably with TCP
		SendToServer(msg);
	}

	void Client::SendEnableReadyCheckMessage()
	{
		// Send the message to signify this client is ready
		Kargono::Network::Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageSession_EnableReadyCheck;

		// Send the message reliably with TCP
		SendToServer(msg);
	}

	void Client::SendSessionReadyCheckMessage()
	{
		// Send a message to the server initiating a ready check
		Kargono::Network::Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageSession_StartReadyCheck;

		// Send the message reliably with TCP
		SendToServer(msg);
	}

	void Client::SendAllClientsSignalMessage(Events::SignalAll& event)
	{
		// Send a scripting specific signal to all other clients
		Kargono::Network::Message msg;
		msg.m_Header.m_MessageType = MessageType::ScriptMessaging_SendAllClientsSignal;
		msg << event.GetSignal();

		// Send the message reliably with TCP
		SendToServer(msg);
	}

	void Client::SendKeepAliveMessage()
	{
		// Send a simple message that keeps the UDP "connection" alive
		Kargono::Network::Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageConnection_KeepAlive;

		// Send the message quickly with UDP
		SendToServer(msg);
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
		SendToServer(msg);
	}

	void Client::SendLeaveCurrentSessionMessage()
	{
		// Send a message notifying the server and all other clients that this client is leaving
		Kargono::Network::Message msg;
		msg.m_Header.m_MessageType = MessageType::ManageSession_NotifyAllLeave;

		// Send the message reliably with TCP
		SendToServer(msg);

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
		SendToServer(newMessage);
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
				m_NetworkEventQueue.SubmitEvent(CreateRef<Events::StartSession>());

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

	void Client::OpenUDPInitMessage(Message& /*msg*/)
	{

		// Receive final message from server indicated UDP connection was successful
		// The initialization context will check this value
		// TODO: Indicate that UDP connection was successful
	}

#if 0
	bool Client::ConnectToServer(const std::string& serverIP, uint16_t serverPort, bool remote)
	{

		// TODO: Maybe resolve the local IP

		// TODO: Start UDP Connection

		// TODO: Send message if failure to connect

		return true;
	}

	void Client::DisconnectFromServer()
	{
		// TODO: Close server connection

		// TODO: Close network/event threads
	}

	bool Client::IsConnectedToServer()
	{
		// TODO: Query connected status
		return false;
	}
#endif

	bool ClientService::Init()
	{
		if (s_Client.s_ClientActive)
		{
			KG_WARN("Failed to initialize client. A client context already exists.")
			return false;
		}

		if (!s_Client.InitClient(Projects::ProjectService::GetServerConfig()))
		{
			KG_WARN("Failed to initialize client. Client initialization failed.")
			return false;
		}

		// TODO: Connect the event/function queues

		// TODO: Start the network thread in the client and call the Run function

		// TODO: Exit out immediately (the network thread is now running separately from the main thread)

		KG_VERIFY(s_Client.s_ClientActive, "Client connection init");
		return true;
	}

	bool ClientService::Terminate()
	{
		if (!s_Client.s_ClientActive)
		{
			KG_WARN("Failed to terminate client. No client context is active.")
			return false;
		}

		if (!s_Client.TerminateClient(false))
		{
			KG_WARN("Failed to terminate client. Terminate function failed.");
			return false;
		}

		KG_VERIFY(!s_Client.s_ClientActive, "Closed client connection");
		return true;
	}

	void ClientService::Run()
	{
		// Decide whether to connect using the local network or through the internet
		bool remoteConnection = false;
		if (Projects::ProjectService::GetActiveServerLocation() != ServerLocation::LocalMachine)
		{
			remoteConnection = true;
		}

#if 0
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

			// TODO: Ensure the run loop never starts
		}
#endif
		//s_Client->InitClient();

		KG_INFO("Started client networking thread");

		// TODO: Run network thread

#if 0
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
#endif

		// Close connection to server
		//s_Client->TerminateClient();

		KG_INFO("Closed client networking thread");
	}

	void ClientService::EndRun()
	{
		// TODO: This could be a location of an error. I am not sure though.
		// There may be a chance that the network thread could be notified while its running
		// and it goes to sleep without closing the thread. I will check this later.

		// Set run loop to close
		// TODO: Ensure the run loop never starts

		// TODO: Wake up the network thread, so it can close
		KG_INFO("Notified client network thread to close");
	}

	bool ClientService::IsClientActive()
	{
		return s_Client.s_ClientActive;
	}

	uint16_t ClientService::GetActiveSessionSlot()
	{
		KG_ASSERT(s_Client.s_ClientActive);

		// Client is unavailable so send error response
		return k_InvalidSessionSlot;
	}

	void ClientService::SendAllEntityLocation(UUID entityID, Math::vec3 location)
	{
		KG_ASSERT(s_Client.s_ClientActive);

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::SendAllEntityLocation>(entityID, location));
	}

	void ClientService::SendAllEntityPhysics(UUID entityID, Math::vec3 translation, Math::vec2 linearVelocity)
	{
		KG_ASSERT(s_Client.s_ClientActive);

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::SendAllEntityPhysics>(entityID, translation, linearVelocity));
	}

	void ClientService::EnableReadyCheck()
	{
		KG_ASSERT(s_Client.s_ClientActive);

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::EnableReadyCheck>());
	}

	void ClientService::SessionReadyCheck()
	{
		KG_ASSERT(s_Client.s_ClientActive);

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::SessionReadyCheck>());
	}

	void ClientService::RequestUserCount()
	{
		KG_ASSERT(s_Client.s_ClientActive);

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::RequestUserCount>());
	}

	void ClientService::RequestJoinSession()
	{
		KG_ASSERT(s_Client.s_ClientActive);

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::RequestJoinSession>());
	}

	void ClientService::LeaveCurrentSession()
	{
		KG_ASSERT(s_Client.s_ClientActive);

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::LeaveCurrentSession>());
	}

	void ClientService::SignalAll(uint16_t signal)
	{
		KG_ASSERT(s_Client.s_ClientActive);

		// Allow the network thread to handle this on its run loop
		SubmitToNetworkEventQueue(CreateRef<Events::SignalAll>(signal));
	}
	void ClientService::SubmitToNetworkFunctionQueue(const std::function<void()>& function)
	{
		KG_ASSERT(s_Client.s_ClientActive);

		s_Client.m_WorkQueue.SubmitFunction(function);

		// TODO: Add the indicated function and alert the network thread to wake up
	}
	void ClientService::SubmitToNetworkEventQueue(Ref<Events::Event> e)
	{
		KG_ASSERT(s_Client.s_ClientActive);

		s_Client.m_NetworkEventQueue.SubmitEvent(e);

		// TODO: Add the indicated event and alert the network thread to wake up
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
		}
	}
	bool ClientService::OnRequestUserCount(Events::RequestUserCount event)
	{
		s_Client.SendRequestUserCountMessage();
		return true;
	}
	bool ClientService::OnStartSession(Events::StartSession event)
	{
		s_Client.m_SessionStartFrame = EngineService::GetActiveEngine().GetUpdateCount();
		return true;
	}
	bool ClientService::OnConnectionTerminated(Events::ConnectionTerminated event)
	{
		// Essentially clear all session information from this client context
		s_Client.m_SessionSlot = k_InvalidSessionSlot;
		s_Client.m_SessionStartFrame = 0;
		return true;
	}
	bool ClientService::OnRequestJoinSession(Events::RequestJoinSession event)
	{
		s_Client.SendRequestJoinSessionMessage();
		return true;
	}
	bool ClientService::OnEnableReadyCheck(Events::EnableReadyCheck event)
	{
		s_Client.SendEnableReadyCheckMessage();
		return true;
	}
	bool ClientService::OnSessionReadyCheck(Events::SessionReadyCheck event)
	{
		s_Client.SendSessionReadyCheckMessage();
		return true;
	}
	bool ClientService::OnSendAllEntityLocation(Events::SendAllEntityLocation event)
	{
		s_Client.SendAllEntityLocation(event);
		return true;
	}
	bool ClientService::OnSignalAll(Events::SignalAll event)
	{
		s_Client.SendAllClientsSignalMessage(event);
		return true;
	}
	bool ClientService::OnSendAllEntityPhysics(Events::SendAllEntityPhysics event)
	{
		s_Client.SendAllEntityPhysicsMessage(event);
		return true;
	}
	bool ClientService::OnLeaveCurrentSession(Events::LeaveCurrentSession event)
	{
		s_Client.SendLeaveCurrentSessionMessage();
		return true;
	}
	void ClientService::ProcessFunctionQueue()
	{
		KG_PROFILE_FUNCTION();

		s_Client.m_WorkQueue.ProcessQueue();
	}
	void ClientService::ProcessEventQueue()
	{
		s_Client.m_NetworkEventQueue.ProcessQueue();
	}
}

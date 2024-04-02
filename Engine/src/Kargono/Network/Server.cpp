#include "kgpch.h"

#include "Kargono/Network/Server.h"

#include "Kargono/Core/Application.h"

namespace Kargono::Network
{
	Ref<Network::Server> Server::s_Server { nullptr };

	Server::Server(uint16_t nPort, bool isLocal)
		: Kargono::Network::ServerInterface(nPort, isLocal)
	{

	}
	void Server::RunServer()
	{
		Start();

		while (true)
		{
			Update();
			ProcessEventQueue();
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
	bool Server::OnClientConnect(Ref<Kargono::Network::ConnectionToClient> client)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(CustomMsgTypes::AcceptConnection);
		uint32_t numberOfUsers = static_cast<uint32_t>(m_Connections.size() + 1);
		msg << numberOfUsers;
		client->Send(msg);
		msg.Header.ID = static_cast<uint32_t>(CustomMsgTypes::UpdateUserCount);
		MessageAllClients(msg, client);
		return true;
	}
	void Server::OnClientDisconnect(Ref<Kargono::Network::ConnectionToClient> client)
	{
		KG_INFO("Removing client [{}]", client->GetID());
		Kargono::Network::Message msg;
		msg.Header.ID = static_cast<uint32_t>(CustomMsgTypes::UpdateUserCount);
		uint32_t numberOfUsers = static_cast<uint32_t>(m_Connections.size() - 1);
		msg << numberOfUsers;
		MessageAllClients(msg, client);


		if (m_OnlySession.GetAllClients().contains(client->GetID()))
		{
			Kargono::Network::Message newMessage;
			newMessage.Header.ID = static_cast<uint32_t>(CustomMsgTypes::UserLeftSession);
			newMessage << m_OnlySession.RemoveClient(client->GetID());

			// Notify all users in the same session that a client left
			for (auto& [clientID, connection] : m_OnlySession.GetAllClients())
			{
				connection->Send(newMessage);
			}
		}


	}
	void Server::OnMessage(Ref<Kargono::Network::ConnectionToClient> client, Kargono::Network::Message& incomingMessage)
	{
		switch (static_cast<CustomMsgTypes>(incomingMessage.Header.ID))
		{
		case CustomMsgTypes::ServerPing:
		{
			KG_INFO("[{}]: Server Ping", client->GetID());
			client->Send(incomingMessage);
			break;
		}

		case CustomMsgTypes::MessageAll:
		{
			KG_INFO("[{}]: Message All", client->GetID());
			Kargono::Network::Message newMessage;
			newMessage.Header.ID = static_cast<uint32_t>(CustomMsgTypes::ServerMessage);
			newMessage << client->GetID();
			MessageAllClients(newMessage, client);
			break;
		}
		case CustomMsgTypes::ClientChat:
		{
			KG_INFO("[{}]: Sent Chat", client->GetID());
			incomingMessage.Header.ID = static_cast<uint32_t>(CustomMsgTypes::ServerChat);
			incomingMessage << client->GetID();
			MessageAllClients(incomingMessage, client);
			break;
		}
		case CustomMsgTypes::RequestJoinSession:
		{
			Kargono::Network::Message newMessage;

			// Deny Client Join if session slots are full
			if (m_OnlySession.GetClientCount() >= Session::k_MaxClients)
			{
				newMessage.Header.ID = static_cast<uint32_t>(CustomMsgTypes::DenyJoinSession);
				client->Send(newMessage);
				break;
			}

			// Add Client to session
			newMessage.Header.ID = static_cast<uint32_t>(CustomMsgTypes::ApproveJoinSession);
			uint16_t clientSlot = m_OnlySession.AddClient(client);
			if (clientSlot == 0xFFFF)
			{
				return;
			}
			newMessage << clientSlot;

			// Send Approval Message to New Client
			client->Send(newMessage);

			// Notify all other session clients that new client has been added
			newMessage.Header.ID = static_cast<uint32_t>(CustomMsgTypes::UpdateSessionUserSlot);
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
				otherClientMessage.Header.ID = static_cast<uint32_t>(CustomMsgTypes::UpdateSessionUserSlot);
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

		case CustomMsgTypes::RequestUserCount:
		{
			KG_INFO("[{}]: User Count Request", client->GetID());
			Kargono::Network::Message newMessage;
			newMessage.Header.ID = static_cast<uint32_t>(CustomMsgTypes::UpdateUserCount);
			uint32_t numberOfUsers = static_cast<uint32_t>(m_Connections.size());
			newMessage << numberOfUsers;
			client->Send(newMessage);
			break;
		}

		case CustomMsgTypes::LeaveCurrentSession:
		{
			KG_INFO("[{}]: User Leaving Session", client->GetID());
			Kargono::Network::Message newMessage;
			newMessage.Header.ID = static_cast<uint32_t>(CustomMsgTypes::UserLeftSession);
			newMessage << m_OnlySession.RemoveClient(client->GetID());

			// Notify all users in the same session that a client left
			for (auto& [clientID, connection] : m_OnlySession.GetAllClients())
			{
				connection->Send(newMessage);
			}
			client->Send(newMessage);

			break;
		}

		case CustomMsgTypes::InitSyncPing:
		{
			m_OnlySession.ReceiveSyncPing(client->GetID());
			break;
		}

		case CustomMsgTypes::SessionReadyCheck:
		{
			m_OnlySession.ReadyCheck(client->GetID());
			break;
		}

		case CustomMsgTypes::EnableReadyCheck:
		{
			m_OnlySession.EnableReadyCheck();
			break;
		}

		case CustomMsgTypes::SendAllEntityLocation:
		{
			incomingMessage.Header.ID = static_cast<uint32_t>(CustomMsgTypes::UpdateEntityLocation);
			// Forward entity location to all other clients
			for (auto& [clientID, connection] : m_OnlySession.GetAllClients())
			{
				if (clientID == client->GetID()) { continue; }
				MessageClientUDP(connection, incomingMessage);
			}
			break;
		}

		case CustomMsgTypes::SendAllEntityPhysics:
		{
			incomingMessage.Header.ID = static_cast<uint32_t>(CustomMsgTypes::UpdateEntityPhysics);
			// Forward entity Physics to all other clients
			for (auto& [clientID, connection] : m_OnlySession.GetAllClients())
			{
				if (clientID == client->GetID()) { continue; }
				MessageClientUDP(connection, incomingMessage);
			}
			break;
		}

		case CustomMsgTypes::SignalAll:
		{
			incomingMessage.Header.ID = static_cast<uint32_t>(CustomMsgTypes::ReceiveSignal);
			// Forward signal to all other session clients
			for (auto& [clientID, connection] : m_OnlySession.GetAllClients())
			{
				if (clientID == client->GetID()) { continue; }
				connection->Send(incomingMessage);
			}
			break;
		}

		case CustomMsgTypes::KeepAlive:
		{
			Kargono::Network::Message newMessage;
			newMessage.Header.ID = static_cast<uint32_t>(CustomMsgTypes::KeepAlive);
			MessageClientUDP(client, newMessage);
			break;
		}

		case CustomMsgTypes::UDPInit:
		{
			Kargono::Network::Message newMessage;
			newMessage.Header.ID = static_cast<uint32_t>(CustomMsgTypes::UDPInit);
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
		dispatcher.Dispatch<Events::StartSession>(KG_BIND_EVENT_FN(Server::OnStartSession));
		
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

		KG_ERROR("Starting frame is {}", m_OnlySession.GetSessionStartFrame());
		return true;
	}
}

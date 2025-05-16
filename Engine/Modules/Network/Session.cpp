#include "kgpch.h"

#include "Modules/Network/Session.h"

#include "Modules/Network/Server.h"
#include "Kargono/Utility/Timers.h"

namespace Kargono::Network
{
	void Session::Init(ServerNetworkThread* networkThread, ConnectionList* parentConnectionList)
	{
		KG_ASSERT(parentConnectionList);
		KG_ASSERT(networkThread);

		// Set up dependencies
		i_ConnectionList = parentConnectionList;
		i_NetworkThread = networkThread;

		m_Active = true;
	}
	void Session::CreateSession()
	{
		KG_INFO("[SERVER]: Initializing Session...");

		// Notify clients that session initialization has started
		for (ClientIndex sessionClient : m_ActiveClients)
		{
			i_NetworkThread->SendSessionInitMessage(sessionClient);
		}

		KG_INFO("[SERVER]: Sending start-session messages");
		
		float longestRTT = GetLongestRTT();
		for (ClientIndex sessionClient : m_ActiveClients)
		{
			Connection& connection = i_ConnectionList->GetConnection(sessionClient);
			float waitTime{ longestRTT - connection.m_ReliabilityContext.
				m_RoundTripContext.GetAverageRoundTrip()};

			if (waitTime < 0.0f) { waitTime = 0.0f; }

			i_NetworkThread->SendStartSessionMessage(sessionClient, waitTime);
		}

		// Set up timer to start session gameplay
		Utility::AsyncBusyTimer::CreateTimer(longestRTT, [&]()
		{
			// Start Thread
			ServerService::GetActiveContext().GetNetworkThread().SubmitEvent(CreateRef<Events::StartSession>());
		});
	}

	void Session::Terminate()
	{
		m_Active = false;
	}
	float Session::GetLongestRTT()
	{
		KG_ASSERT(i_ConnectionList);

		float highestRTT{ 0.0f };
		for (ClientIndex sessionClient : m_ActiveClients)
		{
			Connection& clientConnection{ i_ConnectionList->GetConnection(sessionClient)};
			float clientRTT = clientConnection.m_ReliabilityContext.
				m_RoundTripContext.GetAverageRoundTrip();
			if (clientRTT > highestRTT)
			{
				highestRTT = clientRTT;
			}
		}

		return highestRTT;
	}
	void Session::StartGameplay(UpdateCount startFrame)
	{
		KG_ASSERT(m_Active);

		m_GameplayStartFrame = startFrame;
	}
	void Session::EndGameplay()
	{
		KG_ASSERT(m_Active);

		m_GameplayStartFrame = 0;
	}
	void Session::StoreClientReady(ClientIndex clientID)
	{
		KG_ASSERT(m_Active);
		KG_ASSERT(i_NetworkThread);

		// Ensure a ready check is active
		if (!m_ReadyCheckContext.IsActive())
		{
			return;
		}

		m_ReadyCheckContext.AddClient(clientID);

		// Check if all the clients have enabled their ready check
		if (m_ReadyCheckContext.IsReady())
		{
			// Handle completing the ready check
			float longestRTT = GetLongestRTT();

			// Send confirm ready check messages
			for (ClientIndex sessionClient : m_ActiveClients)
			{
				Connection& connection = i_ConnectionList->GetConnection(sessionClient);
				float waitTime{ longestRTT - connection.m_ReliabilityContext.
					m_RoundTripContext.GetAverageRoundTrip() };

				if (waitTime < 0.0f) { waitTime = 0.0f; }

				i_NetworkThread->SendConfirmReadyCheckMessage(sessionClient, waitTime);
			}

			// Reset ready check
			m_ReadyCheckContext.Clear();
		}
	}

	void Session::StartReadyCheck()
	{
		KG_ASSERT(m_Active);

		m_ReadyCheckContext.Init((size_t)m_ActiveClients.GetCount());
	}


	SessionIndex Session::AddClient(ClientIndex newClient)
	{
		KG_ASSERT(m_Active);

		// Ensure duplicate client does not exist
		for (SessionIndex sessionIndex : m_ActiveClients.GetActiveIndices())
		{
			if (m_ActiveClients[sessionIndex] == newClient)
			{
				KG_WARN("Failed to add client to session. Duplicate client found {}", newClient);
				return k_InvalidSessionIndex;
			}
		}

		// Add the new client
		Expected<EmplaceResult<ClientIndex, SessionIndex>> newActiveClient = m_ActiveClients.EmplaceLowest();

		// Handle failure to add client
		if (!newActiveClient)
		{
			return k_InvalidSessionIndex;
		}

		// Handle add client success
		newActiveClient->m_Value = newClient;
		return newActiveClient->m_ArrayIndex;
	}
	SessionIndex Session::RemoveClient(ClientIndex queryClient)
	{
		KG_ASSERT(m_Active);

		// Search through all session indices for the queryClient
		SessionIndex indexToRemove{ k_InvalidSessionIndex };
		for (SessionIndex sessionIndex : m_ActiveClients.GetActiveIndices())
		{
			if (m_ActiveClients[sessionIndex] == queryClient)
			{
				indexToRemove = sessionIndex;
				break;
			}
		}

		// Remove the client if it was found
		if (indexToRemove != k_InvalidSessionIndex)
		{
			m_ActiveClients.Remove(indexToRemove);
		}

		return indexToRemove;
	}
	void ReadyCheckContext::Init(size_t requiredCount)
	{
		KG_ASSERT(requiredCount > 0);

		m_Active = true;
		m_RequiredCount = requiredCount;
		m_ReadyClients.clear();
		m_ReadyClients.reserve(requiredCount);
	}
	void ReadyCheckContext::Clear()
	{
		m_Active = false;
		m_RequiredCount = 0;
		m_ReadyClients.clear();
	}
	bool ReadyCheckContext::AddClient(ClientIndex index)
	{
		KG_ASSERT(m_Active);

		auto [iter, wasInserted] = m_ReadyClients.insert(index);
		return wasInserted;
	}
}

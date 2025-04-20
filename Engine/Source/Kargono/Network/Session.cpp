#include "kgpch.h"

#include "Kargono/Network/Session.h"

#include "Kargono/Network/Server.h"
#include "Kargono/Utility/Timers.h"

namespace Kargono::Network
{
	void Session::InitSession()
	{

		KG_INFO("[SERVER]: Initializing Session...");

#if 0
		Ref<Server> activeServer = ServerService::GetActiveServer();
		KG_ASSERT(activeServer);

		// Notify clients that session initialization has started
		for (auto& [clientID, connection] : m_ConnectedClients)
		{
			activeServer->SendSessionInitMessage(connection);
		}

		// Clear session init state
		m_InitState = SessionInitState();
#endif
		KG_INFO("[SERVER]: Sending start-session messages");


#if 0
		// Send start message to each client
		float waitTime{ 0 };
		Kargono::Network::Message newMessage;
		newMessage.m_Header.m_MessageType = MessageType::ManageSession_StartSession;
		for (auto [clientID, connection] : m_ConnectedClients)
		{
			// Calculate wait time
			waitTime = longestLatency - connection->GetTCPLatency();
			if (waitTime < 0) { waitTime = 0; }

			// Send Message
			newMessage << waitTime;
			m_ConnectedClients.at(clientID)->SendTCPMessage(newMessage);
		}

		// TODO: Start Server Thread
		Utility::AsyncBusyTimer::CreateTimer(longestLatency, [&]()
		{
			// Start Thread
			ServerService::SubmitToNetworkEventQueue(CreateRef<Events::StartSession>());
		});
#endif

	}

	void Session::EndSession()
	{
	}
	void Session::StoreClientReadyCheck(ClientIndex clientID)
	{
#if 0
		// Ensure a ready check is active and this client is participating
		if (m_ReadyCheckData.m_ReadyClients.contains(clientID) || !m_ReadyCheckData.m_Active)
		{
			return;
		}

		// Set the client as having the ready check enabled
		m_ReadyCheckData.m_ReadyClients.insert(clientID);

		// Check if all the clients have enabled their ready check
		if (m_ReadyCheckData.m_ReadyClients.size() >= m_ConnectedClients.size())
		{
			// Handle completing the ready check
			float longestLatency = CalculateLongestLatency();

			// Send ready check confirm messages
			float waitTime{ 0 };
			for (auto [currentClientID, connection] : m_ConnectedClients)
			{
				// Calculate wait time for each client
				waitTime = longestLatency - connection->GetTCPLatency();
				if (waitTime < 0)
				{
					waitTime = 0;
				}

				// Ensure the server is active
				KG_ASSERT(ServerService::GetActiveServer());

				// Send the ready check confirm message
				ServerService::GetActiveServer()->SendConfirmReadyCheckMessage(
					m_ConnectedClients.at(currentClientID), waitTime);
			}

			// Reset the ready check context
			ResetReadyCheck();
		}
#endif
	}
	void Session::ResetReadyCheck()
	{
		m_ReadyCheckData.m_Active = false;
		m_ReadyCheckData.m_ReadyClients.clear();
	}
	SessionIndex Session::AddClient(ClientIndex newClient)
	{
		return 0;
#if 0
		// TODO: No insertion of the connection* actually occurs
		// Session already contains client, this is an error
		if (m_ConnectedClients.contains(newClient))
		{
			KG_WARN("Attempt to add client to session that already contains client ID!");
			return k_InvalidSessionSlot;
		}

		// Check for any gap slots
		if (!m_EmptySlots.empty())
		{
			// Get the last empty slot
			SessionIndex slotToFill = m_EmptySlots.back();

			// Insert the client at the last slot to fill
			auto [clientsIter, addClientSuccess] = m_ConnectedClients.insert({ newClient, nullptr });
			auto [sessionIter, addSlotSuccess] = m_SessionSlots.insert({ slotToFill , newClient });

			// Check if any insertion fails
			if (!addClientSuccess || !addSlotSuccess)
			{
				KG_WARN("Attempt to add client to session, however, failed to insert the client!");
				return k_InvalidSessionSlot;
			}

			// Remove the empty slot entry
			m_EmptySlots.pop_back();

			return slotToFill;
		}

		// If not gap slots are found, increase the client list size
		auto [clientsIter, addClientSuccess] = m_ConnectedClients.insert({ newClient, nullptr });
		auto [sessionIter, addSlotSuccess] = m_SessionSlots.insert({ m_SlotMax , newClient });

		// Check if any insertion fails
		if (!addClientSuccess || !addSlotSuccess)
		{
			KG_WARN("Attempt to add client to session, however, failed to insert the client!");
			return k_InvalidSessionSlot;
		}

		return m_SlotMax++;
#endif
	}
	SessionIndex Session::RemoveClient(ClientIndex queryID)
	{
		return 0;

#if 0
		// Session already contains client, this is an error
		if (!m_ConnectedClients.contains(queryID))
		{
			KG_WARN("Attempt to remove client to session that does not contain client ID!");
			return k_InvalidSessionSlot;
		}

		// Erase client from connected list
		size_t numClientsRemoved = m_ConnectedClients.erase(queryID);

		if (numClientsRemoved == 0)
		{
			KG_WARN("Failed to remove client from session slot. Erase operation failed for the client {}", queryID);
			return k_InvalidSessionSlot;
		}

		// Erase from slots list
		for (auto [currentSlot, currentClientID] : m_SessionSlots)
		{
			// Check if the slot index matches
			if (currentClientID == queryID)
			{
				// Add new entry to the vacant slots vector and sort it
				m_EmptySlots.emplace_back(currentSlot);
				std::ranges::sort(m_EmptySlots.begin(), m_EmptySlots.end(), std::greater <uint16_t>());

				// Erase the slot from the list
				size_t numElementsRemoved = m_SessionSlots.erase(currentSlot);

				// Handle the case of erase failure
				if (numElementsRemoved == 0)
				{
					KG_WARN("Failed to remove client from session slots. Erase operation failed for the client {}", queryID);
					return k_InvalidSessionSlot;
				}

				// Return the removed slot
				return currentSlot;
			}
		}

		KG_WARN("Failed to remove client from session slots. Could not locate client {}", queryID);
		return k_InvalidSessionSlot;
#endif
	}
}

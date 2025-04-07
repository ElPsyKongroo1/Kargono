#include "kgpch.h"

#include "Kargono/Network/Session.h"

#include "Kargono/Network/Server.h"
#include "Kargono/Utility/Timers.h"

namespace Kargono::Network
{
	void Session::InitSession()
	{
#if 0
		KG_INFO("[SERVER]: Initializing Session...");

		Ref<Server> activeServer = ServerService::GetActiveServer();
		KG_ASSERT(activeServer);

		// Notify clients that session initialization has started
		for (auto& [clientID, connection] : m_ConnectedClients)
		{
			activeServer->SendSessionInitMessage(connection);
		}

		// Clear session init state
		m_InitState = SessionInitState();

		KG_INFO("[SERVER]: Starting to determine connection latencies");

		// Record current time for each connection and send a ping to each client
		for (auto& [clientID, connection] : m_ConnectedClients)
		{
			m_InitState.m_LatencyCache.insert({ clientID, {} });
			m_InitState.m_LatencyCacheFilled.insert({ clientID, false });
			m_InitState.m_RecentTimePoints.insert_or_assign(clientID, std::chrono::high_resolution_clock::now());
			activeServer->SendSyncPingMessage(connection);
		}
#endif
	}

	void Session::ReceiveSyncPing(uint32_t clientID)
	{
#if 0
		// Ensure the client ID exists
		KG_ASSERT(m_InitState.m_RecentTimePoints.contains(clientID));
		KG_ASSERT(m_InitState.m_LatencyCacheFilled.contains(clientID));

		// Record Current TimePoint
		std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
		float durationSeconds = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - m_InitState.m_RecentTimePoints.at(clientID)).count() * 0.001f * 0.001f * 0.001f;
		float singleDirectionLatency = durationSeconds / 2.0f;

		// Check if cache is already filled
		if (m_InitState.m_LatencyCacheFilled.at(clientID))
		{
			KG_WARN("Received sync ping when cache has already been filled!");
			return;
		}

		// Insert duration into latency cache
		m_InitState.m_LatencyCache.at(clientID).push_back(singleDirectionLatency);

		// Check if latency cache for current client is full
		if (m_InitState.m_LatencyCache.at(clientID).size() >= k_MaxSyncPings)
		{
			m_InitState.m_LatencyCacheFilled.insert_or_assign(clientID, true);

			// Check if we are done with ping aggregation
			for (auto& [currentClientID, isCacheFilled] : m_InitState.m_LatencyCacheFilled)
			{
				UNREFERENCED_PARAMETER(currentClientID);
				if (isCacheFilled == false)
				{
					return;
				}
			}

			// If we have collected all of the pings, move on to next stage
			CompleteSessionInit();
			return;
		}

		// Store the current time to compare with round-trip time
		m_InitState.m_RecentTimePoints.insert_or_assign(clientID, std::chrono::high_resolution_clock::now());

		KG_ASSERT(ServerService::GetActiveServer());

		// If Cache is not filled yet, send another sync ping
		ServerService::GetActiveServer()->SendSyncPingMessage(m_ConnectedClients.at(clientID));

#endif
	}

	void Session::CompleteSessionInit()
	{
#if 0
		// Used for ending calculations
		std::vector<float> allLatencies{};

		for (auto& [clientID, latencies] : m_InitState.m_LatencyCache)
		{
			// Calculate current mean
			float meanLatency{ 0 };
			float latencySum{ 0 };
			for (float latency : latencies)
			{
				latencySum += latency;
			}

			meanLatency = latencySum / static_cast<float>(latencies.size());

			// Calculate Standard Deviation
			float varianceSum{ 0 };

			for (float latency : latencies)
			{
				varianceSum += static_cast<float>(std::pow(latency - meanLatency, 2));
			}

			float standardDeviation = sqrt(varianceSum / (float)latencies.size());

			// Remove outliers
			latencies.erase(std::remove_if(latencies.begin(), latencies.end(), [&](float latency)
				{
					// If the distance between the latency and meanLatency is greater than a standard
					//		deviation, remove the element.
					if (std::fabs(latency - meanLatency) > standardDeviation)
					{
						return true;
					}

					// Keep the latency if it is within one standard deviation of the mean
					return false;
				}));

			// Recalculate Mean Latency
			meanLatency = 0;
			latencySum = 0;
			for (float latency : latencies)
			{
				latencySum += latency;
			}
			meanLatency = latencySum / (float)latencies.size();

			// Store new Latency inside connection member
			m_ConnectedClients.at(clientID)->SetTCPLatency(meanLatency);
			allLatencies.push_back(meanLatency);
		}

		KG_INFO("[SERVER]: Completed Calculating Connection Latencies");

		// Find highest latency
		std::vector<float>::iterator maxLatencyIterator = std::max_element(allLatencies.begin(), allLatencies.end());
		KG_ASSERT(maxLatencyIterator != allLatencies.end(), "No largest element was found, probably an implementation error!");
		float longestLatency = *maxLatencyIterator;

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
	void Session::StoreClientReadyCheck(uint32_t clientID)
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
	float Session::CalculateLongestLatency()
	{
		return 0.0f;
#if 0
		// Find highest latency
		std::vector<float> allLatencies{};
		for (auto [currentClientID, connection] : m_ConnectedClients)
		{
			allLatencies.push_back(connection->GetTCPLatency());
		}
		std::vector<float>::iterator maxLatencyIterator = std::max_element(allLatencies.begin(), allLatencies.end());
		KG_ASSERT(maxLatencyIterator != allLatencies.end(), "No largest element was found, probably an implementation error!");
		return *maxLatencyIterator;
#endif
	}
	void Session::ResetReadyCheck()
	{
#if 0
		m_ReadyCheckData.m_Active = false;
		m_ReadyCheckData.m_ReadyClients.clear();
#endif
	}
	uint16_t Session::AddClient(ClientIndex newClient)
	{
		return k_InvalidSessionSlot;
#if 0
		// Session already contains client, this is an error
		if (m_ConnectedClients.contains(newClient->GetID()))
		{
			KG_WARN("Attempt to add client to session that already contains client ID!");
			return k_InvalidSessionSlot;
		}

		// Check for any gap slots
		if (!m_EmptySlots.empty())
		{
			// Get the last empty slot
			uint16_t slotToFill = m_EmptySlots.back();

			// Insert the client at the last slot to fill
			auto [clientsIter, addClientSuccess] = m_ConnectedClients.insert({ newClient->GetID(), newClient });
			auto [sessionIter, addSlotSuccess] = m_SessionSlots.insert({ slotToFill , newClient->GetID() });

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
		auto [clientsIter, addClientSuccess] = m_ConnectedClients.insert({ newClient->GetID(), newClient });
		auto [sessionIter, addSlotSuccess] = m_SessionSlots.insert({ m_SlotMax , newClient->GetID() });

		// Check if any insertion fails
		if (!addClientSuccess || !addSlotSuccess)
		{
			KG_WARN("Attempt to add client to session, however, failed to insert the client!");
			return k_InvalidSessionSlot;
		}

		return m_SlotMax++;

#endif
	}
	uint16_t Session::RemoveClient(uint32_t queryID)
	{
		return k_InvalidSessionSlot;
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
	}
#endif
	}
}

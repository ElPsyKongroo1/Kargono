#include "kgpch.h"

#include "Kargono/Network/Session.h"

#include "Kargono/Network/Server.h"
#include "Kargono/Utility/Timers.h"

namespace Kargono::Network
{
	void Session::InitSession()
	{
		KG_INFO("[SERVER]: Initializing Session...");

		// Notify clients that session initialization has started
		Kargono::Network::Message newMessage;
		newMessage.Header.ID = static_cast<uint32_t>(MessageType::CurrentSessionInit);
		for (auto& [clientID, connection] : m_ConnectedClients)
		{
			connection->SendTCPMessage(newMessage);
		}

		// Clear session init state
		m_InitState = SessionInitState();

		KG_INFO("[SERVER]: Starting to determine connection latencies");

		// Record current time for each connection and send a ping to each client
		newMessage.Header.ID = static_cast<uint32_t>(MessageType::InitSyncPing);
		for (auto& [clientID, connection] : m_ConnectedClients)
		{
			m_InitState.m_LatencyCache.insert({ clientID, {} });
			m_InitState.m_LatencyCacheFilled.insert({ clientID, false });
			m_InitState.m_RecentTimePoints.insert_or_assign(clientID, std::chrono::high_resolution_clock::now());
			connection->SendTCPMessage(newMessage);
		}
	}

	void Session::ReceiveSyncPing(uint32_t clientID)
	{
		// Record Current TimePoint
		std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
		float durationSeconds = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - m_InitState.m_RecentTimePoints.at(clientID)).count() * 0.001f * 0.001f * 0.001f;
		float singleDirectionLatency = durationSeconds / 2.0f;

		// Check if cache is already filled
		if (m_InitState.m_LatencyCacheFilled.at(clientID) == true)
		{
			KG_ERROR("Received sync ping when cache has already been filled!");
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

		// If Cache is not filled yet, send another sync ping
		Kargono::Network::Message newMessage;
		newMessage.Header.ID = static_cast<uint32_t>(MessageType::InitSyncPing);
		m_InitState.m_RecentTimePoints.insert_or_assign(clientID, std::chrono::high_resolution_clock::now());
		m_ConnectedClients.at(clientID)->SendTCPMessage(newMessage);
	}

	void Session::CompleteSessionInit()
	{
		// Used for ending calculations
		std::vector<float> allLatencies{};

		for (auto& [clientID, latencies] : m_InitState.m_LatencyCache)
		{
			// Calculate current mean
			float meanLatency{ 0 };
			float latencySum{ 0 };
			for (auto latency : latencies)
			{
				latencySum += latency;
			}

			meanLatency = latencySum / static_cast<float>(latencies.size());

			// Calculate Standard Deviation
			float varianceSum{ 0 };

			for (auto latency : latencies)
			{
				varianceSum += static_cast<float>(std::pow(latency - meanLatency, 2));
			}

			float standardDeviation = sqrt(varianceSum / static_cast<float>(latencies.size()));

			

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
			for (auto latency : latencies)
			{
				latencySum += latency;
			}
			meanLatency = latencySum / static_cast<float>(latencies.size());

			// Store new Latency inside connection member
			m_ConnectedClients.at(clientID)->SetTCPLatency(meanLatency);
			allLatencies.push_back(meanLatency);
		}

		KG_INFO("[SERVER]: Completed Calculating Connection Latencies");

		// Find highest latency
		auto maxLatencyIterator = std::max_element(allLatencies.begin(), allLatencies.end());
		KG_ASSERT(maxLatencyIterator != allLatencies.end(), "No largest element was found, probably an implementation error!");
		float longestLatency = *maxLatencyIterator;

		// Send start message to each client
		float waitTime{ 0 };
		Kargono::Network::Message newMessage;
		newMessage.Header.ID = static_cast<uint32_t>(MessageType::StartSession);
		for (auto& [clientID, connection] : m_ConnectedClients)
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
			ServerService::SubmitToEventQueue(CreateRef<Events::StartSession>());
		});

	}

	void Session::EndSession()
	{
	}
	void Session::StoreClientReadyCheck(uint32_t clientID)
	{
		if (m_ReadyCheck.contains(clientID) || !m_UseReadyCheck)
		{ 
			return; 
		}
		m_ReadyCheck.insert(clientID);

		if (m_ReadyCheck.size() >= m_ConnectedClients.size())
		{
			//KG_ERROR("Entered Ready Check Resolver");
			// Find highest latency
			std::vector<float> allLatencies{};
			for (auto& [currentClientID, connection] : m_ConnectedClients)
			{
				allLatencies.push_back(connection->GetTCPLatency());
			}
			auto maxLatencyIterator = std::max_element(allLatencies.begin(), allLatencies.end());
			KG_ASSERT(maxLatencyIterator != allLatencies.end(), "No largest element was found, probably an implementation error!");
			float longestLatency = *maxLatencyIterator;

			// Send start message to each client
			float waitTime{ 0 };
			Kargono::Network::Message newMessage;
			newMessage.Header.ID = static_cast<uint32_t>(MessageType::SessionReadyCheckConfirm);
			for (auto& [currentClientID, connection] : m_ConnectedClients)
			{
				// Calculate wait time
				waitTime = longestLatency - connection->GetTCPLatency();
				if (waitTime < 0) { waitTime = 0; }

				// Send Message
				newMessage << waitTime;
				m_ConnectedClients.at(currentClientID)->SendTCPMessage(newMessage);
			}

			// Reset Ready Check
			m_UseReadyCheck = false;
			m_ReadyCheck.clear();
		}
	}
	uint16_t Session::AddClient(ServerTCPConnection* newClient)
	{
		// Session already contains client, this is an error
		if (m_ConnectedClients.contains(newClient->GetID()))
		{
			KG_WARN("Attempt to add client to session that already contains client ID!");
			return 0xFFFF;
		}

		if (!m_EmptySlots.empty())
		{
			uint16_t slotToFill = m_EmptySlots.back();
			m_ConnectedClients.insert({ newClient->GetID(), newClient });
			m_SessionSlots.insert({ slotToFill , newClient->GetID() });

			m_EmptySlots.pop_back();

			return slotToFill;
		}

		m_ConnectedClients.insert({ newClient->GetID(), newClient });
		m_SessionSlots.insert({ m_SlotMax , newClient->GetID()});

		return m_SlotMax++;
	}
	uint16_t Session::RemoveClient(uint32_t clientID)
	{
		// Session already contains client, this is an error
		if (!m_ConnectedClients.contains(clientID))
		{
			KG_ERROR("Attempt to remove client to session that does not contain client ID!");
			return 0xFFFF;
		}

		// Erase client from connections list
		m_ConnectedClients.erase(clientID);

		// Erase from slots list
		for (auto it = m_SessionSlots.begin(); it != m_SessionSlots.end(); ++it)
		{
			if (it->second == clientID)
			{
				uint16_t slot = it->first;
				m_EmptySlots.push_back(slot);
				std::ranges::sort(m_EmptySlots.begin(), m_EmptySlots.end(), std::greater <uint16_t>());
				m_SessionSlots.erase(it);
				return slot;
			}
		}

		return 0xFFFF;
	}
}

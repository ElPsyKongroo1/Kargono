#pragma once

#include "Kargono/Network/ConnectionToClient.h"
#include "Kargono/Core/Base.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <thread>
#include <chrono>


namespace Kargono::Network
{
	class Server;

	struct SessionInitCache
	{
		uint16_t MaxSyncPings = 10;
		std::unordered_map<uint32_t, std::vector<float>> LatencyCache {};
		std::unordered_map<uint32_t, bool> LatencyCacheFilled {};
		std::unordered_map<uint32_t, std::chrono::time_point<std::chrono::high_resolution_clock>> RecentTimePoints {};
	};

	class Session
	{
	public:
		// TODO: VERY TEMPORARY. Only for pong!!!!
		static constexpr uint32_t k_MaxClients = 2;
	public:
		void InitSession();

		void ReceiveSyncPing(uint32_t clientID);

		void CompleteSessionInit();

		void EndSession();

		void ReadyCheck(uint32_t clientID);

		uint16_t AddClient(Ref<ConnectionToClient> newClient);
		uint16_t RemoveClient(uint32_t clientID);
		uint32_t GetClientCount() const { return static_cast<uint32_t>(m_ConnectedClients.size()); }
		void EnableReadyCheck() { m_UseReadyCheck = true; }
		void SetSessionStartFrame(uint64_t frame) { m_SessionStartFrame = frame; }
		uint64_t GetSessionStartFrame() const { return m_SessionStartFrame; }
		std::unordered_map<uint32_t, Ref<ConnectionToClient>>& GetAllClients() { return m_ConnectedClients; }
		std::unordered_map<uint16_t, uint32_t>& GetAllSlots() { return m_SessionSlots; }

	private:
		uint16_t m_SlotMax = 0;
		std::unordered_map<uint32_t, Ref<ConnectionToClient>> m_ConnectedClients {};
		std::unordered_map<uint16_t, uint32_t> m_SessionSlots{};
		std::unordered_set<uint32_t> m_ReadyCheck{};
		bool m_UseReadyCheck = false;
		std::vector<uint16_t> m_EmptySlots{};
		SessionInitCache m_InitCache {};
		uint64_t m_SessionStartFrame{ 0 };
	};
}

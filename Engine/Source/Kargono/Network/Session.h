#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Network/NetworkCommon.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <thread>
#include <chrono>


namespace Kargono::Network
{
	class Server;
	class ServerTCPConnection;

	struct SessionInitCache
	{
		std::unordered_map<uint32_t, std::vector<float>> LatencyCache {};
		std::unordered_map<uint32_t, bool> LatencyCacheFilled {};
		std::unordered_map<uint32_t, std::chrono::time_point<std::chrono::high_resolution_clock>> RecentTimePoints {};
	};

	//==============================
	// Session Class
	//==============================
	class Session
	{
	public:
		//==============================
		// LifeCycle Functions
		//==============================
		void InitSession();
		void CompleteSessionInit();
		void EndSession();

		//==============================
		// Client Synchronization
		//==============================
		void ReceiveSyncPing(uint32_t clientID);

		//==============================
		// Ready Check Process
		//==============================
		void StoreClientReadyCheck(uint32_t clientID);

		//==============================
		// Manage Clients
		//==============================
		uint16_t AddClient(ServerTCPConnection* newClient);
		uint16_t RemoveClient(uint32_t clientID);

		//==============================
		// Getter/Setters
		//==============================
		uint32_t GetClientCount() const { return static_cast<uint32_t>(m_ConnectedClients.size()); }
		void EnableReadyCheck() { m_UseReadyCheck = true; }
		void SetSessionStartFrame(uint64_t frame) { m_SessionStartFrame = frame; }
		uint64_t GetSessionStartFrame() const { return m_SessionStartFrame; }
		std::unordered_map<uint32_t, ServerTCPConnection*>& GetAllClients() { return m_ConnectedClients; }
		std::unordered_map<uint16_t, uint32_t>& GetAllSlots() { return m_SessionSlots; }

	private:
		uint16_t m_SlotMax{0};
		std::unordered_map<uint32_t, ServerTCPConnection*> m_ConnectedClients {};
		std::unordered_map<uint16_t, uint32_t> m_SessionSlots{};
		std::unordered_set<uint32_t> m_ReadyCheck{};
		bool m_UseReadyCheck{false};
		std::vector<uint16_t> m_EmptySlots{};
		SessionInitCache m_InitCache {};
		uint64_t m_SessionStartFrame{ 0 };
	};
}

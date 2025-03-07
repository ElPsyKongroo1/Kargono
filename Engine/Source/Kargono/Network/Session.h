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

	struct SessionInitState
	{
		std::unordered_map<uint32_t, std::vector<float>> m_LatencyCache {};
		std::unordered_map<uint32_t, bool> m_LatencyCacheFilled {};
		std::unordered_map<uint32_t, std::chrono::time_point<std::chrono::high_resolution_clock>> m_RecentTimePoints {};
	};

	struct ReadyCheckData
	{
		bool m_Active{ false };
		std::unordered_set<uint32_t> m_ReadyClients{};
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

		
	public:
		//==============================
		// Ready Check Process
		//==============================
		void StoreClientReadyCheck(uint32_t clientID);
		float CalculateLongestLatency();
	private:
		void ResetReadyCheck();
		
	public:
		//==============================
		// Manage Clients
		//==============================
		uint16_t AddClient(ServerTCPConnection* newClient);
		uint16_t RemoveClient(uint32_t clientID);

		//==============================
		// Getter/Setters
		//==============================
		uint32_t GetClientCount() const { return (uint32_t)m_ConnectedClients.size(); }
		void EnableReadyCheck() { m_ReadyCheckData.m_Active = true; }
		void SetSessionStartFrame(uint64_t frame) { m_SessionStartFrame = frame; }
		uint64_t GetSessionStartFrame() const { return m_SessionStartFrame; }
		std::unordered_map<uint32_t, ServerTCPConnection*>& GetAllClients() { return m_ConnectedClients; }
		std::unordered_map<uint16_t, uint32_t>& GetAllSlots() { return m_SessionSlots; }

	private:
		uint16_t m_SlotMax{0};
		std::unordered_map<uint32_t, ServerTCPConnection*> m_ConnectedClients {};
		std::unordered_map<uint16_t, uint32_t> m_SessionSlots{};
		std::vector<uint16_t> m_EmptySlots{};
		ReadyCheckData m_ReadyCheckData;
		SessionInitState m_InitState {};
		uint64_t m_SessionStartFrame{ 0 };
	};
}

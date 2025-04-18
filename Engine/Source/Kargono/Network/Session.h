#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Network/NetworkCommon.h"
#include "Kargono/Network/Connection.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <thread>
#include <chrono>


namespace Kargono::Network
{
	class Server;
	class ServerTCPConnection;

	struct ReadyCheckData
	{
		bool m_Active{ false };
		std::unordered_set<ClientIndex> m_ReadyClients{};
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
		void EndSession();

		
	public:
		//==============================
		// Ready Check Process
		//==============================
		void StoreClientReadyCheck(ClientIndex clientID);
	private:
		void ResetReadyCheck();
		
	public:
		//==============================
		// Manage Clients
		//==============================
		SessionIndex AddClient(ClientIndex newClient);
		SessionIndex RemoveClient(ClientIndex clientID);

		//==============================
		// Getter/Setters
		//==============================
		size_t GetClientCount() const { return m_ConnectedClients.size(); }
		void EnableReadyCheck() { m_ReadyCheckData.m_Active = true; }
		void SetSessionStartFrame(uint64_t frame) { m_SessionStartFrame = frame; }
		uint64_t GetSessionStartFrame() const { return m_SessionStartFrame; }
		std::unordered_map<ClientIndex, Connection*>& GetAllClients() { return m_ConnectedClients; }
		std::unordered_map<SessionIndex, ClientIndex>& GetAllSlots() { return m_SessionSlots; }

	private:
		SessionIndex m_SlotMax{0};
		std::unordered_map<ClientIndex, Connection*> m_ConnectedClients {};
		std::unordered_map<SessionIndex, ClientIndex> m_SessionSlots{};
		std::vector<SessionIndex> m_EmptySlots{};
		ReadyCheckData m_ReadyCheckData;
		uint64_t m_SessionStartFrame{ 0 };
	};
}

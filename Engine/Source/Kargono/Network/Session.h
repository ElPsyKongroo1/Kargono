#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Network/NetworkCommon.h"
#include "Kargono/Network/Connection.h"
#include "Kargono/Utility/Timers.h"
#include "Kargono/Core/DataStructures.h"

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
		size_t GetClientCount() const { return m_SessionClients.GetCount(); }
		void EnableReadyCheck() { m_ReadyCheckData.m_Active = true; }
		void SetSessionStartFrame(uint64_t frame) { m_SessionStartFrame = frame; }
		UpdateCount GetSessionStartFrame() const { return m_SessionStartFrame; }
		SparseArray<ClientIndex>& GetSessionClients()
		{
			return m_SessionClients;
		}

	private:
		SparseArray<ClientIndex> m_SessionClients{};
		ReadyCheckData m_ReadyCheckData;
		UpdateCount m_SessionStartFrame{ 0 };
	};

	class SessionList
	{

	};
}

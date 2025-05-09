#pragma once

#include "Kargono/Core/Base.h"
#include "Modules/Network/NetworkCommon.h"
#include "Modules/Network/Connection.h"
#include "Kargono/Utility/Timers.h"
#include "Kargono/Core/DataStructures.h"
#include "Modules/Network/Connection.h"

#include <unordered_set>


namespace Kargono::Network
{
	class ServerNetworkThread;

	class ReadyCheckContext
	{
	public:
		//==============================
		// LifeCycle Functions
		//==============================
		void Init(size_t requiredCount);
		void Clear();

		//==============================
		// Manage Ready Clients
		//==============================
		bool AddClient(ClientIndex index);

		//==============================
		// Query State
		//==============================
		bool IsActive() const
		{
			return m_Active;
		}
		bool IsReady() const
		{
			return m_ReadyClients.size() >= m_RequiredCount;
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		bool m_Active{ false };
		size_t m_RequiredCount{ 0 };
		std::unordered_set<ClientIndex> m_ReadyClients{};
	};

	class Session
	{
	public:
		//==============================
		// LifeCycle Functions
		//==============================
		// General session build-up/tear-down
		void CreateSession();
		void Init(ServerNetworkThread* networkThread, ConnectionList* parentConnectionList);
		void Terminate();
	private:
		// Helper functions
		float GetLongestRTT();
	public:

		// Gameplay set-up/tear-down
		void StartGameplay(UpdateCount startFrame);
		void EndGameplay();
	public:
		//==============================
		// Manage Ready Check
		//==============================
		void StoreClientReady(ClientIndex clientID);
		void StartReadyCheck();
		
	public:
		//==============================
		// Manage Clients
		//==============================
		SessionIndex AddClient(ClientIndex newClient);
		SessionIndex RemoveClient(ClientIndex clientID);

		//==============================
		// Getter/Setters
		//==============================
		SessionIndex GetClientCount() const 
		{ 
			return m_ActiveClients.GetCount(); 
		}
		ClientIndex GetClient(SessionIndex index)
		{
			return m_ActiveClients[index];
		}
		UpdateCount GetSessionStartFrame() const 
		{ 
			return m_GameplayStartFrame; 
		}
		SparseArray<ClientIndex, SessionIndex>& GetSessionClients()
		{
			return m_ActiveClients;
		}

	private:
		//==============================
		// Internal Fields
		//==============================
		// State data
		bool m_Active{ false };
		// Client data
		SparseArray<ClientIndex, SessionIndex> m_ActiveClients{k_MaxSessionClients};
		// Ready check data
		ReadyCheckContext m_ReadyCheckContext{};
		// Gameplay data
		UpdateCount m_GameplayStartFrame{ 0 };

		//==============================
		// Injected Dependencies
		//==============================
		ConnectionList* i_ConnectionList{ nullptr };
		ServerNetworkThread* i_NetworkThread{ nullptr };
	};
}

#pragma once

#include "Modules/Network/Address.h"
#include "Modules/Network/NetworkCommon.h"
#include "Modules/Network/ReliabilityContext.h"

#include <vector>

namespace Kargono::Network
{
	struct Connection
	{
		Address m_Address;
		ReliabilityContext m_ReliabilityContext{};
	};

	class ConnectionList
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ConnectionList() = default;
		ConnectionList(ClientIndex maxClients);
	public:
		//==============================
		// Manage Connections
		//==============================
		ClientIndex AddConnection(Address newAddress);
		bool RemoveConnection(ClientIndex clientIndex);

		//==============================
		// Query Context
		//==============================
		bool IsConnectionActive(ClientIndex clientIndex);
		bool IsAddressActive(Address clientAddress);

		//==============================
		// Getters/Setters
		//==============================
		Connection& GetConnection(ClientIndex clientIndex);
		ClientIndex GetNumberOfClients();
		std::vector<Connection>& GetAllConnections();
	private:
		//==============================
		// Internal Data
		//==============================
		ClientIndex m_MaxClients{ 0 };
		ClientIndex m_NumClients{ 0 };
		std::vector<Connection> m_AllConnections{};
		std::vector<bool> m_ClientsConnected{};
	};
}

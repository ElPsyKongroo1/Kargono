#include "kgpch.h"

#include "Kargono/Network/UDPServer.h"

#include "Kargono/Network/ConnectionToClient.h"

namespace Kargono::Network
{
	void UDPServer::AddToIncomingMessageQueue()
	{
		if (!m_IPAddressToConnection.contains(m_CurrentEndpoint))
		{
			// This will adjust the udp endpoint for an ip address in case it is changed
			// TODO: I KNOW THIS IS A HUGE SECURITY PROBLEM, but yea idk any other way
			//		to make this work
			bool foundMatchingAddress = false;
			asio::ip::udp::endpoint foundEndpoint;
			Ref<ConnectionToClient> clientConnect { nullptr };
			for (auto& [endpoint, connection] : m_IPAddressToConnection)
			{
				if (endpoint.address() == m_CurrentEndpoint.address())
				{
					foundMatchingAddress = true;
					foundEndpoint = endpoint;
					clientConnect = connection;
					break;
				}
			}

			if (!foundMatchingAddress)
			{
				KG_ERROR("Address not found in m_IPAddressToConnection()");
				return;
			}

			clientConnect->SetUDPRemoteReceiveEndpoint(m_CurrentEndpoint);
			clientConnect->SetUDPRemoteSendEndpoint(m_CurrentEndpoint);
			m_IPAddressToConnection.erase(foundEndpoint);
			m_IPAddressToConnection.insert_or_assign(m_CurrentEndpoint, clientConnect);

		}

		m_qMessagesIn.push_back({ m_IPAddressToConnection.at(m_CurrentEndpoint), m_MsgTemporaryIn });
		WakeUpNetworkThread();
		ReadMessage();
	}
	void UDPServer::Disconnect(asio::ip::udp::endpoint key)
	{

		if (!m_IPAddressToConnection.contains(key))
		{
			KG_ERROR("Address does not resolve to a connection pointer in UDPServer Disconnect()");
			return;
		}
		Ref<ConnectionToClient> connection = m_IPAddressToConnection.at(key);
		if (!connection)
		{
			KG_ERROR("Invalid connection in UDP Disconnect()");
			return;
		}

		connection->Disconnect();
	}
}

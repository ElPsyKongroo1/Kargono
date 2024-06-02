#include "kgpch.h"

#include "Kargono/Network/UDPClient.h"

#include "Kargono/Network/ConnectionToServer.h"

namespace Kargono::Network
{
	void Kargono::Network::UDPClient::Disconnect(asio::ip::udp::endpoint key)
	{
		if (!m_ActiveConnection)
		{
			KG_ERROR("Invalid connection in UDP Disconnect()");
			return;
		}

		m_ActiveConnection->Disconnect();
	}
	void UDPClient::AddToIncomingMessageQueue()
	{
		m_qMessagesIn.push_back({ nullptr, m_MsgTemporaryIn });
		WakeUpNetworkThread();
		ReadMessage();
	}
}

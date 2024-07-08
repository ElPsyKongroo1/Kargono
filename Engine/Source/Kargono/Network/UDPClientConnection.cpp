#include "kgpch.h"

#include "Kargono/Network/UDPClientConnection.h"

#include "Kargono/Network/ConnectionToServer.h"

namespace Kargono::Network
{
	void Kargono::Network::UDPClientConnection::Disconnect(asio::ip::udp::endpoint key)
	{
		if (!m_ActiveConnection)
		{
			KG_ERROR("Invalid connection in UDP Disconnect()");
			return;
		}

		m_ActiveConnection->Disconnect();
	}
	void UDPClientConnection::AddToIncomingMessageQueue()
	{
		m_qMessagesIn.PushBack({ nullptr, m_MsgTemporaryIn });
		WakeUpNetworkThread();
		ReadMessage();
	}
}

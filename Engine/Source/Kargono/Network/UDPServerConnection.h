#pragma once
#include "Kargono/Network/UDPConnection.h"

#include <unordered_map>
#include <utility>

namespace Kargono::Network
{
	class ConnectionToClient;

	class UDPServerConnection : public std::enable_shared_from_this<UDPServerConnection>, public UDPConnection
	{
	public:
		UDPServerConnection(asio::io_context& asioContext, asio::ip::udp::socket&& socket, TSQueue<owned_message>& qIn,
			std::condition_variable& newCV, std::mutex& newMutex, 
			std::unordered_map<asio::ip::udp::endpoint, Ref<ConnectionToClient>>& ipMap)
			: UDPConnection(asioContext, std::move(socket), qIn, newCV, newMutex), m_IPAddressToConnection(ipMap)
		{
		}
		virtual ~UDPServerConnection() override = default;
	public:
		virtual void AddToIncomingMessageQueue() override;

		virtual void Disconnect(asio::ip::udp::endpoint key) override;

	protected:
		std::unordered_map<asio::ip::udp::endpoint, Ref<ConnectionToClient>>& m_IPAddressToConnection;
	};
}

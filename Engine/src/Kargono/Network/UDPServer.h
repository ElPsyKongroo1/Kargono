#pragma once
#include "Kargono/Network/UDPService.h"

#include <unordered_map>
#include <utility>

namespace Kargono::Network
{
	class ConnectionToClient;

	class UDPServer : public std::enable_shared_from_this<UDPServer>, public UDPService
	{
	public:
		UDPServer(asio::io_context& asioContext, asio::ip::udp::socket&& socket, tsqueue<owned_message>& qIn,
			std::condition_variable& newCV, std::mutex& newMutex, 
			std::unordered_map<asio::ip::udp::endpoint, Ref<ConnectionToClient>>& ipMap)
			: UDPService(asioContext, std::move(socket), qIn, newCV, newMutex), m_IPAddressToConnection(ipMap)
		{
		}
		virtual ~UDPServer() override = default;
	public:
		virtual void AddToIncomingMessageQueue() override;

		virtual void Disconnect(asio::ip::udp::endpoint key) override;

	protected:
		std::unordered_map<asio::ip::udp::endpoint, Ref<ConnectionToClient>>& m_IPAddressToConnection;
	};
}

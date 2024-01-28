#pragma once
#include "Kargono/Network/UDPService.h"

namespace Kargono::Network
{
	class ConnectionToServer;

	class UDPClient : public std::enable_shared_from_this<UDPClient>, public UDPService
	{
	public:
		UDPClient(asio::io_context& asioContext, asio::ip::udp::socket&& socket, tsqueue<owned_message>& qIn,
			std::condition_variable& newCV, std::mutex& newMutex, Ref<ConnectionToServer> connection)
			: UDPService(asioContext, std::move(socket), qIn, newCV, newMutex), m_ActiveConnection(connection)
		{

		}
		virtual ~UDPClient() override = default;
	public:

		virtual void Disconnect(asio::ip::udp::endpoint key) override;

		virtual void AddToIncomingMessageQueue() override;
	private:
		Ref<ConnectionToServer> m_ActiveConnection{ nullptr };
	};
}

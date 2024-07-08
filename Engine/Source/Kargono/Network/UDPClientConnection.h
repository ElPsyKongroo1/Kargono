#pragma once
#include "Kargono/Network/UDPConnection.h"

namespace Kargono::Network
{
	class ConnectionToServer;

	class UDPClientConnection : public std::enable_shared_from_this<UDPClientConnection>, public UDPConnection
	{
	public:
		UDPClientConnection(asio::io_context& asioContext, asio::ip::udp::socket&& socket, TSQueue<owned_message>& qIn,
			std::condition_variable& newCV, std::mutex& newMutex, Ref<ConnectionToServer> connection)
			: UDPConnection(asioContext, std::move(socket), qIn, newCV, newMutex), m_ActiveConnection(connection)
		{

		}
		virtual ~UDPClientConnection() override = default;
	public:

		virtual void Disconnect(asio::ip::udp::endpoint key) override;

		virtual void AddToIncomingMessageQueue() override;
	private:
		Ref<ConnectionToServer> m_ActiveConnection{ nullptr };
	};
}

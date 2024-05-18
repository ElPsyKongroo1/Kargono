#include "kgpch.h"

#include "Kargono/Network/ConnectionToClient.h"

#include "Client.h"
#include "Kargono/Core/Core.h"
#include "Kargono/Events/NetworkingEvent.h"
#include "Kargono/Network/ClientInterface.h"
#include "Kargono/Network/ServerInterface.h"

#include "Kargono/Projects/Project.h"


namespace Kargono::Network
{
	ConnectionToClient::ConnectionToClient(asio::io_context& asioContext, asio::ip::tcp::socket&& socket, tsqueue<owned_message>& qIn, std::condition_variable& newCV,
		std::mutex& newMutex)
		: Connection(asioContext, std::move(socket), qIn, newCV, newMutex)
	{

		// Construct validation check data
		
		// Connection is Server -> Client, construct random data for the client to transform and send back for validation
		m_nHandshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());

		// Pre-calculate the result for checking when the client responds
		m_nHandshakeCheck = Scramble(m_nHandshakeOut);
		
		
	}
	void ConnectionToClient::Connect(uint32_t uid)
	{
		if (m_TCPSocket.is_open())
		{
			id = uid;

			m_UDPLocalEndpoint = asio::ip::udp::endpoint(m_TCPSocket.local_endpoint().address(),
				m_TCPSocket.local_endpoint().port());
			m_UDPRemoteSendEndpoint = asio::ip::udp::endpoint(m_TCPSocket.remote_endpoint().address(),
				m_TCPSocket.remote_endpoint().port());
			m_UDPRemoteReceiveEndpoint = asio::ip::udp::endpoint(m_TCPSocket.remote_endpoint().address(),
				m_TCPSocket.remote_endpoint().port());

			// A client has attempted to connect to the server, but we want
			// the client to first validate itself, so first write out the
			// handshake data to be validated
			WriteValidation();

			// Next, issue a task to sit and wait asynchronously for precisely
			// the validation data sent back from the client
			ReadValidation();

		}
	}

	void ConnectionToClient::Disconnect()
	{
		if (IsConnected())
		{
			asio::post(m_asioContext, [this]()
				{
					m_TCPSocket.close();
					WakeUpNetworkThread();
					
				});
		}
		KG_INFO("[SERVER]: Connection has been terminated");
	}

	void ConnectionToClient::AddToIncomingMessageQueue()
	{
		m_qMessagesIn.push_back({ this->shared_from_this(), m_msgTemporaryIn });
		WakeUpNetworkThread();
		ReadHeader();
	}

	void ConnectionToClient::WriteValidation()
	{
		asio::async_write(m_TCPSocket, asio::buffer(&m_nHandshakeOut, sizeof(uint64_t)),
			[this](std::error_code ec, std::size_t length)
			{
				if (!ec)
				{
				}
				else
				{
					m_TCPSocket.close();
				}
			});
	}
	void ConnectionToClient::ReadValidation()
	{
		asio::async_read(m_TCPSocket, asio::buffer(&m_nHandshakeIn, sizeof(uint64_t)),
			[&](std::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					if (m_nHandshakeIn == m_nHandshakeCheck)
					{
						// Client has provided valid solution, so allow it to connect properly
						KG_INFO("Client Validation Successful");
						// TODO: Add post client validation back
						//server->OnClientValidated(this->shared_from_this());

						// Sit waiting to receive data now
						ReadHeader();
					}
					else
					{
						KG_INFO("Client Disconnected (Failed Validation)");
						Disconnect();
					}
				}
				else
				{
					KG_INFO("Client Disconnected (ReadValidation)");
					Disconnect();
				}
			});
	}
}



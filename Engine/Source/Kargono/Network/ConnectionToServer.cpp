#include "kgpch.h"

#include "Kargono/Network/ConnectionToServer.h"

#include "Kargono/Network/Client.h"
#include "Kargono/Core/EngineCore.h"
#include "Kargono/Events/NetworkingEvent.h"
#include "Kargono/Network/ClientInterface.h"
#include "Kargono/Network/ServerInterface.h"
#include "Kargono/Network/UDPClient.h"
#include "Kargono/Projects/Project.h"


namespace Kargono::Network
{
	ConnectionToServer::ConnectionToServer(asio::io_context& asioContext, asio::ip::tcp::socket&& socket, tsqueue<owned_message>& qIn, std::condition_variable& newCV,
		std::mutex& newMutex)
		: Connection(asioContext, std::move(socket), qIn, newCV, newMutex)
	{
		// Connection is Client -> Server, so we have nothing to define
		m_nHandshakeIn = 0;
		m_nHandshakeOut = 0;
	}
	bool ConnectionToServer::Connect(const asio::ip::tcp::resolver::results_type& endpoints)
	{
		asio::error_code ec;
		//asio::connect(m_TCPSocket, endpoints, ec);

		asio::ip::tcp::resolver::iterator iter = endpoints.begin();
		asio::ip::tcp::resolver::iterator end; // End marker.

		ec.clear();
		bool connectionSuccessful = false;
		while (iter != end)
		{
			m_TCPSocket.connect(*iter, ec);
			if (!ec)
			{
				connectionSuccessful = true;
				break;
			}
			
			ec.clear();
			++iter;
		}
		if (!connectionSuccessful) { return false; }

		if (!ec)
		{
			ReadValidation();
			m_UDPLocalEndpoint = asio::ip::udp::endpoint(m_TCPSocket.local_endpoint().address(),
			                                        m_TCPSocket.local_endpoint().port());
			m_UDPRemoteSendEndpoint = asio::ip::udp::endpoint(m_TCPSocket.remote_endpoint().address(),
				m_TCPSocket.remote_endpoint().port());
			m_UDPRemoteReceiveEndpoint = asio::ip::udp::endpoint(m_TCPSocket.remote_endpoint().address(),
				m_TCPSocket.remote_endpoint().port());
			// Connected Successfully
			return true;
		}

		// Failed to connect
		return false;

	}
	void ConnectionToServer::Disconnect()
	{
		if (IsConnected())
		{
			asio::post(m_asioContext, [this]()
				{
					m_TCPSocket.close();
				});
		}
		KG_INFO("[CLIENT]: Connection has been terminated");
		
		EngineCore::GetCurrentEngineCore().SubmitToEventQueue(CreateRef<Events::ConnectionTerminated>());
		Client::GetActiveClient()->SubmitToEventQueue(CreateRef<Events::ConnectionTerminated>());
		
	}

	void ConnectionToServer::AddToIncomingMessageQueue()
	{
		m_qMessagesIn.push_back({ nullptr, m_msgTemporaryIn });
		WakeUpNetworkThread();
		ReadHeader();
	}

	void ConnectionToServer::WriteValidation()
	{
		asio::async_write(m_TCPSocket, asio::buffer(&m_nHandshakeOut, sizeof(uint64_t)),
			[this](std::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					ReadHeader();
				}
				else
				{
					m_TCPSocket.close();
				}
			});
	}
	void ConnectionToServer::ReadValidation()
	{
		asio::async_read(m_TCPSocket, asio::buffer(&m_nHandshakeIn, sizeof(uint64_t)),
			[&](std::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					// Connection is a client, so solve puzzle
					m_nHandshakeOut = Scramble(m_nHandshakeIn);

					// Write the result
					WriteValidation();
				}
				else
				{
					KG_INFO("Client Disconnected (ReadValidation)");
					Disconnect();
				}
			});
	}
}



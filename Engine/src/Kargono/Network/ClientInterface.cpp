#include "kgpch.h"

#include "Kargono/Network/ClientInterface.h"
#include "Kargono/Network/Client.h"

#include "Kargono/Network/Common.h"
#include "Kargono/Core/Application.h"
#include "Kargono/Core/Timers.h"

namespace Kargono::Network
{
	ClientInterface::ClientInterface()
	{
		// Initialize the socket with the io context, so it can work!
	}
	ClientInterface::~ClientInterface()
	{
		// If the client is destroyed, always attempt to disconnect from server
		Disconnect();
	}
	bool ClientInterface::Connect(const std::string& host, const uint16_t port, bool remote)
	{
		try
		{
			// Create TCP Socket for this new connection
			asio::ip::tcp::socket socket{m_context};

			if (remote)
			{
				// Get local TCP address and port
				asio::ip::tcp::resolver resolver(m_context);
				asio::ip::tcp::resolver::query query(asio::ip::host_name(), "");
				asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
				while (iter != asio::ip::tcp::resolver::iterator()) 
				{
					if (iter->endpoint().address().is_v4()) { break; }
					++iter;
				}

				// Create local endpoint and bind to new socket
				KG_ASSERT(port > 100, "Port must be greater than 100. If not, we have some issuessss!");
				asio::ip::tcp::endpoint localTCP = asio::ip::tcp::endpoint(iter->endpoint().address(), port - 100);
				socket.open(localTCP.protocol());
				socket.bind(localTCP);
			}

			// Resolve provided host/port into a TCP endpoint list
			asio::ip::tcp::resolver resolver(m_context);
			asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

			// Create a new connection object
			m_connection = CreateRef<ConnectionToServer>(
				m_context,
				std::move(socket),
				m_qMessagesIn,
				m_BlockThreadCV,
				m_BlockThreadMx);

			// Initialize connection to remote endpoint
			KG_INFO("Attempting to TCP connection to server");
			if (!m_connection->Connect(endpoints)) { return false; }
			KG_INFO("Successful TCP connection to server");

			KG_INFO("Attempting UDP connection to server");

			KG_TRACE("The local endpoint is {} {}", m_connection->GetUDPLocalEndpoint().address().to_string(), 
				m_connection->GetUDPLocalEndpoint().port());
			KG_TRACE("The remote send endpoint is {} {}", m_connection->GetUDPRemoteSendEndpoint().address().to_string(),
				m_connection->GetUDPRemoteSendEndpoint().port());

			// Create Local UDP Socket
			asio::ip::udp::socket serverUDPSocket {m_context};
			// Get Local UDP Endpoint created from Connect Function
			//		and bind it to the socket
			asio::ip::udp::endpoint& localEndpoint = m_connection->GetUDPLocalEndpoint();
			serverUDPSocket.open(localEndpoint.protocol());
			serverUDPSocket.bind(localEndpoint);

			m_UDPClient = CreateRef<UDPClient>(m_context, std::move(serverUDPSocket), m_qMessagesIn,
				m_BlockThreadCV, m_BlockThreadMx, m_connection);

			m_UDPClient->Start();

			// Start Context Thread
			thrContext = std::thread([this]() { m_context.run(); });

			Timers::AsyncBusyTimer::CreateRecurringTimer(0.3f, 10, [&]()
			{
				Client::GetActiveClient()->SubmitToFunctionQueue([&]()
				{
					KG_TRACE("Submitted a UDP INIT BITCH");
					// Do checks for UDP connection
					Message newMessage{};
					newMessage.Header.ID = static_cast<uint32_t>(CustomMsgTypes::UDPInit);
					SendUDP(newMessage);
				});
				WakeUpNetworkThread();
			}, [&]()
			{
				Client::GetActiveClient()->SubmitToFunctionQueue([&]()
				{
					if (!m_UDPConnectionSuccessful)
					{
						Disconnect();
						return;
					}
					KG_INFO("UDP Connection Successful!");
				});
				WakeUpNetworkThread();
			});


		}
		catch (std::exception& e)
		{
			KG_ERROR("Client Exception: {}", e.what());
			return false;
		}

		return true;
	}

	void ClientInterface::Disconnect()
	{
		if (IsConnected())
		{
			m_connection->Disconnect();
		}

		m_context.stop();
		if (thrContext.joinable())
			thrContext.join();

		m_connection.reset();
	}

	bool ClientInterface::IsConnected()
	{
		if (m_connection) { return m_connection->IsConnected(); }
		return false;
	}

	void ClientInterface::Update(size_t nMaxMessages)
	{
		if (m_qMessagesIn.empty()) { Wait(); }

		size_t nMessageCount = 0;
		while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty())
		{
			// Grab the front message
			auto msg = m_qMessagesIn.pop_front();

			// Pass to message handler
			OnMessage(msg.msg);

			nMessageCount++;
		}
	}

	void ClientInterface::Send(const Message& msg)
	{
		if (IsConnected())
			m_connection->Send(msg);
	}

	void ClientInterface::SendUDP(Message& msg)
	{
		if (IsConnected())
		{
			LabeledMessage labeled{ m_connection->GetUDPRemoteSendEndpoint(), msg };

			m_UDPClient->Send(labeled);
		}
	}

	void ClientInterface::Wait()
	{
		std::unique_lock<std::mutex> lock(m_BlockThreadMx);
		m_BlockThreadCV.wait(lock);
	}

	void ClientInterface::WakeUpNetworkThread()
	{
		std::unique_lock<std::mutex> lock(m_BlockThreadMx);
		m_BlockThreadCV.notify_one();
	}

}

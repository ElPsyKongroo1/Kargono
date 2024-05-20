#include "kgpch.h"

#include "Kargono/Network/ServerInterface.h"

#include "Kargono/Network/Common.h"

namespace Kargono::Network
{
	ServerInterface::ServerInterface(uint16_t port, bool isLocal)
		: m_TCPAcceptor(m_Context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
	{

		// Create a resolver
		asio::ip::tcp::resolver resolver(m_Context);

		// Query for the local host entry
		asio::ip::tcp::resolver::query query(asio::ip::host_name(), "");

		// Perform the resolution and get the endpoint iterator
		asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
		while (iter != asio::ip::tcp::resolver::iterator())
		{
			if (iter->endpoint().address().is_v4()) { break; }
			++iter;
		}
		KG_ASSERT(iter != asio::ip::tcp::resolver::iterator(), "Could not get local host ip address!");

		// Local udp socket
		asio::ip::udp::socket serverUDPSocket {m_Context};
		// Create Server UDP socket
		asio::ip::tcp::endpoint localTCPEndpoint = m_TCPAcceptor.local_endpoint();
		asio::ip::udp::endpoint localUDPEndpoint;
		if (isLocal)
		{
			localUDPEndpoint = asio::ip::udp::endpoint(asio::ip::udp::v4(), localTCPEndpoint.port());
		}
		else
		{
			localUDPEndpoint = asio::ip::udp::endpoint( iter->endpoint().address(), localTCPEndpoint.port() );
		}

		serverUDPSocket.open(localUDPEndpoint.protocol());
		serverUDPSocket.bind(localUDPEndpoint);
		m_UDPServer = CreateRef<UDPServer>(m_Context, std::move(serverUDPSocket), m_qMessagesIn,
			m_BlockThreadCV, m_BlockThreadMx, m_IPAddressToConnection);
	}
	ServerInterface::~ServerInterface()
	{
		Stop();
	}
	bool ServerInterface::Start()
	{
		try
		{
			WaitForClientConnection();
			m_UDPServer->Start();

			m_threadContext = std::thread([this]() { m_Context.run(); });
		}
		catch (std::exception& e)
		{
			std::cerr << "[SERVER] Exception: " << e.what() << '\n';
			return false;
		}

		KG_INFO("[SERVER] Started!");
		return true;
	}
	void ServerInterface::Stop()
	{
		// Request the context to close
		m_Context.stop();

		// Tidy up the context thread
		if (m_threadContext.joinable()) { m_threadContext.join(); }

		KG_INFO("[SERVER] Stopped!");
	}
	void ServerInterface::WaitForClientConnection()
	{
		m_TCPAcceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket)
		{
			if (ec)
			{
				KG_INFO("[SERVER] New Connection Error: {}", ec.message());
				WaitForClientConnection();
				return;
			}

			KG_INFO("[SERVER] New Connection: {}", socket.remote_endpoint());

			

			Ref<ConnectionToClient> newConnection = CreateRef<ConnectionToClient>(m_Context, std::move(socket),
				m_qMessagesIn, m_BlockThreadCV, m_BlockThreadMx);

			// Give the user server a chance to deny connection
			if (!OnClientConnect(newConnection))
			{
				KG_WARN("[-----] Connection Denied");
				WaitForClientConnection();
				return;
			}
			

			m_Connections.push_back(std::move(newConnection));

			m_Connections.back()->Connect(nIDCounter++);

			// Connection allowed, so add to container of new connections
			
			m_IPAddressToConnection.insert_or_assign(m_Connections.back()->GetUDPRemoteReceiveEndpoint(), m_Connections.back());

			KG_INFO("[{}]: Connection Approved", m_Connections.back()->GetID());



			// Prime the asio context with more work - again simply wait for
			//		another connection...
			WaitForClientConnection();
		});
	}
	void ServerInterface::MessageClient(Ref<ConnectionToClient> client, const Message& msg)
	{
		if (client && client->IsConnected())
		{
			client->Send(msg);
		}
	}

	void ServerInterface::MessageClientUDP(Ref<ConnectionToClient> client, Message& msg)
	{
		if (client && client->IsConnected())
		{
			LabeledMessage labeled{ client->GetUDPRemoteSendEndpoint(), msg };

			m_UDPServer->Send(labeled);
		}
	}



	void ServerInterface::MessageAllClients(const Message& msg, Ref<ConnectionToClient> pIgnoreClient)
	{
		for (auto& client : m_Connections)
		{
			// Check client is connected...
			if (client && client->IsConnected())
			{
				// ..it is!
				if (client != pIgnoreClient) { client->Send(msg); }
			}
			
		}
	}
	void ServerInterface::Update(size_t nMaxMessages)
	{
		CheckConnectionsValid();
		if (m_qMessagesIn.empty()){ Wait(); }

		size_t nMessageCount = 0;
		while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty())
		{
			// Grab the front message
			auto msg = m_qMessagesIn.pop_front();

			// Pass to message handler
			OnMessage(msg.remote, msg.msg);

			nMessageCount++;
		}
	}
	void ServerInterface::Wait()
	{
		std::unique_lock<std::mutex> lock(m_BlockThreadMx);
		m_BlockThreadCV.wait(lock);
	}
	void ServerInterface::WakeUpNetworkThread()
	{
		std::unique_lock<std::mutex> lock(m_BlockThreadMx);
		m_BlockThreadCV.notify_one();
	}
	void ServerInterface::CheckConnectionsValid()
	{
		bool bInvalidClientExists = false;
		for (auto& client : m_Connections)
		{
			// Check client is connected...
			if (!(client && client->IsConnected()))
			{
				// The client couldn't be contacted, so assume it has disconnected.
				OnClientDisconnect(client);
				m_IPAddressToConnection.erase(client->GetUDPRemoteReceiveEndpoint());
				client.reset();
				bInvalidClientExists = true;
			}
		}
		if (bInvalidClientExists)
		{
			m_Connections.erase(
				std::remove(m_Connections.begin(), m_Connections.end(), nullptr), m_Connections.end());
		}
	}
}

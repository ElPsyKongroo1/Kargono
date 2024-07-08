#pragma once

#include "Kargono/Core/DataStructures.h"
#include "Kargono/Network/Message.h"
#include "Kargono/Network/Connection.h"

#include "API/Network/AsioAPI.h"

#include <memory>
#include <cstdint>
#include <mutex>
#include <condition_variable>

namespace Kargono::Network
{
	class Connection
	{
	public:
		Connection(asio::io_context& asioContext, asio::ip::tcp::socket&& socket, TSQueue<owned_message>& qIn,
		    std::condition_variable& newCV, std::mutex& newMutex);
		virtual ~Connection() = default;

	public:
		void WakeUpNetworkThread();
		void Send(const Message& msg);
		void ReadHeader();
		void ReadBody();
		void WriteHeader();
		void WriteBody();
		virtual void Disconnect() = 0;
		virtual void AddToIncomingMessageQueue() = 0;
		uint64_t Scramble(uint64_t nInput);

	public:
		asio::ip::udp::endpoint& GetUDPLocalEndpoint() { return m_UDPLocalEndpoint; }
		asio::ip::udp::endpoint& GetUDPRemoteSendEndpoint() { return m_UDPRemoteSendEndpoint; }
		asio::ip::udp::endpoint& GetUDPRemoteReceiveEndpoint() { return m_UDPRemoteReceiveEndpoint; }

		void SetUDPLocalEndpoint(asio::ip::udp::endpoint& endpoint) { m_UDPLocalEndpoint = endpoint; }
		void SetUDPRemoteSendEndpoint(asio::ip::udp::endpoint& endpoint) { m_UDPRemoteSendEndpoint = endpoint; }
		void SetUDPRemoteReceiveEndpoint(asio::ip::udp::endpoint& endpoint) { m_UDPRemoteReceiveEndpoint = endpoint; }
	protected:
		// Each connection has a unique socket to a remote
		asio::ip::tcp::socket m_TCPSocket;

		// This context is shared with the whole asio instance
		asio::io_context& m_asioContext;

		// This queue holds all message to be sent to the remote
		// side of this connection
		TSQueue<Message> m_qMessagesOut;

		// This queue holds all messages that have been received from
		// the remote side of this connection. Note it is a reference
		// as the owner of this connection is expected to provide a queue
		TSQueue<owned_message>& m_qMessagesIn;

		asio::ip::udp::endpoint m_UDPLocalEndpoint;
		asio::ip::udp::endpoint m_UDPRemoteSendEndpoint;
		asio::ip::udp::endpoint m_UDPRemoteReceiveEndpoint;

		Message m_msgTemporaryIn;

		// Handshake Validation
		uint64_t m_nHandshakeOut = 0;
		uint64_t m_nHandshakeIn = 0;

		// These Variables reference the main client or server mutex/condition_variables
		std::condition_variable& m_BlockThreadCV;
		std::mutex& m_BlockThreadMx;
	};
}

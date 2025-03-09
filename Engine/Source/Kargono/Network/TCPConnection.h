#pragma once
#include "Kargono/Network/NetworkCommon.h"

#include <memory>

namespace Kargono::Network
{
	class TCPConnection
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		TCPConnection(NetworkContext* networkContext, asio::ip::tcp::socket&& socket);
		virtual ~TCPConnection() = default;

	public:

		//==============================
		// Receive Messages
		//==============================
		void ReadMessageHeaderAsync();
		void ReadMessagePayloadAsync();

		//==============================
		// Send Messages
		//==============================
		void SendTCPMessage(const Message& msg);
		void WriteMessageHeaderAsync();
		void WriteMessagePayloadAsync();

		//==============================
		// Getter/Setters
		//==============================
		asio::ip::udp::endpoint& GetUDPLocalEndpoint() { return m_UDPLocalEndpoint; }
		asio::ip::udp::endpoint& GetUDPRemoteSendEndpoint() { return m_UDPRemoteSendEndpoint; }
		asio::ip::udp::endpoint& GetUDPRemoteReceiveEndpoint() { return m_UDPRemoteReceiveEndpoint; }
		void SetUDPLocalEndpoint(asio::ip::udp::endpoint& endpoint) { m_UDPLocalEndpoint = endpoint; }
		void SetUDPRemoteSendEndpoint(asio::ip::udp::endpoint& endpoint) { m_UDPRemoteSendEndpoint = endpoint; }
		void SetUDPRemoteReceiveEndpoint(asio::ip::udp::endpoint& endpoint) { m_UDPRemoteReceiveEndpoint = endpoint; }

	protected:
		//==============================
		// Client/Server Specific Functionality
		//==============================
		virtual void Disconnect() = 0;
		virtual void AddMessageToIncomingMessageQueue() = 0;

		//==============================
		// Internal Functionality
		//==============================
		uint64_t GenerateValidationToken(uint64_t nInput);

	protected:

		// Pointer to active network context
		NetworkContext* m_NetworkContextPtr { nullptr };

		// Each connection has a unique socket to a remote
		asio::ip::tcp::socket m_TCPSocket;

		// This queue holds all message to be sent to the remote
		// side of this connection
		TSQueue<Message> m_OutgoingMessageQueue;
		
		// UDP endpoint used in processes
		asio::ip::udp::endpoint m_UDPLocalEndpoint;
		asio::ip::udp::endpoint m_UDPRemoteSendEndpoint;
		asio::ip::udp::endpoint m_UDPRemoteReceiveEndpoint;

		// Simple cache when processing messages
		Message m_MessageCache;

		// Validation Cache
		uint64_t m_ValidationOutput {0};
		uint64_t m_ValidationInput {0};
	};
}

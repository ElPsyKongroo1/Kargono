#pragma once

#include "Kargono/Network/NetworkCommon.h"

#include <memory>
#include <utility>

namespace Kargono::Network
{
	class UDPConnection
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		UDPConnection(NetworkContext* networkContext, asio::ip::udp::socket socket);
		virtual ~UDPConnection() = default;
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		void Start();
		void Stop();

		//==============================
		// Receive Messages
		//==============================
		void ReadMessageAsync();

		//==============================
		// Send Messages
		//==============================
		void SendUDPMessage(const LabeledMessage& msg);
		void WriteMessageAsync();

		//==============================
		// Manage Global Network Thread
		//==============================
		void NetworkThreadWakeUp();
		void NetworkThreadSleep();

		//==============================
		// Client/Server Specific Functionality
		//==============================
		virtual void Disconnect(asio::ip::udp::endpoint key) = 0;
		virtual void AddMessageToIncomingMessageQueue() = 0;

	protected:

		// Pointer to active network context
		NetworkContext* m_NetworkContextPtr{ nullptr };

		// Unique udp socket for client or server
		asio::ip::udp::socket m_Socket;
		asio::ip::udp::endpoint m_CurrentEndpoint;

		Message m_MessageCache{};

		// This queue holds all message to be sent to the remote
		// side of this connection
		TSQueue<LabeledMessage> m_OutgoingMessagesQueue;
	};
}

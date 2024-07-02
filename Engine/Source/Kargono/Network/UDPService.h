#pragma once

#include "Kargono/Network/DataStructures.h"
#include "Kargono/Network/Message.h"
#include "Kargono/Network/Connection.h"

#include "API/Network/AsioAPI.h"

#include <memory>
#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <utility>

namespace Kargono::Network
{
	class UDPService
	{
	public:
		UDPService(asio::io_context& asioContext, asio::ip::udp::socket socket, tsqueue<owned_message>& qIn,
			std::condition_variable& newCV, std::mutex& newMutex);
		virtual ~UDPService() = default;
	public:
		void Start();
		void Stop();
		virtual void Disconnect(asio::ip::udp::endpoint key) = 0;
		void ReadMessage();
		void WriteMessage();
		void WakeUpNetworkThread();
		void Send(const LabeledMessage& msg);
		virtual void AddToIncomingMessageQueue() = 0;

		uint64_t GetKeepAliveDelay() const { return m_KeepAliveDelay; }
	protected:
		// Unique udp socket for client or server
		asio::ip::udp::socket m_Socket;

		asio::ip::udp::endpoint m_CurrentEndpoint;
		// This context is shared with the whole asio instance
		asio::io_context& m_AsioContext;

		Message m_MsgTemporaryIn{};

		uint64_t m_KeepAliveDelay{ 10'000 };

		// This queue holds all message to be sent to the remote
		// side of this connection
		tsqueue<LabeledMessage> m_qMessagesOut;

		tsqueue<owned_message>& m_qMessagesIn;

		std::condition_variable& m_BlockThreadCV;
		std::mutex& m_BlockThreadMx;
	};
}

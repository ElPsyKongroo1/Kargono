#pragma once

#include "Kargono/Network/DataStructures.h"
#include "Kargono/Network/Message.h"
#include "Kargono/Network/Connection.h"

#include "API/Network/AsioAPI.h"

#include <memory>
#include <cstdint>
#include <mutex>
#include <condition_variable>


namespace Kargono::Network
{
	//============================================================
	// Connection Class
	//============================================================

	class ConnectionToClient : public std::enable_shared_from_this<ConnectionToClient>, public Connection
	{
	public:

		//==============================
		// Constructors/Destructors
		//==============================

		ConnectionToClient(asio::io_context& asioContext, asio::ip::tcp::socket&& socket,
			tsqueue<owned_message>& qIn, std::condition_variable& newCV,
			std::mutex& newMutex);

		virtual ~ConnectionToClient() override {}

		//==============================
		// Getter/Setters
		//==============================

		float GetTCPLatency() const { return m_TCPLatency; }
		void SetTCPLatency(float newLatency) { m_TCPLatency = newLatency; }

		uint32_t GetID() const { return id; }

		//==============================
		// LifeCycle Functions
		//==============================

		void Connect(uint32_t uid = 0);

		virtual void Disconnect() override;

		bool IsConnected() const { return m_TCPSocket.is_open(); }

		//==============================
		// Extra Internal Functionality
		//==============================

		virtual void AddToIncomingMessageQueue() override;

		//==============================
		// ASIO ASYNC Functions
		//==============================

	private:
		// ASYNC - Used by both client and server to write validation packet
		void WriteValidation();

		void ReadValidation();

	protected:
		uint32_t id = 0;

		// Handshake Validation
		uint64_t m_nHandshakeCheck = 0;

		// Latency Variables
		float m_TCPLatency{};

	};
}

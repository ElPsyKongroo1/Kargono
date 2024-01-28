#pragma once

#include "Kargono/Network/NetworkCommon.h"
#include "Kargono/Network/DataStructures.h"
#include "Kargono/Network/Message.h"
#include "Kargono/Network/Connection.h"

#include <memory>
#include <cstdint>
#include <mutex>
#include <condition_variable>


namespace Kargono::Network
{
	class UDPClient;

	//============================================================
	// Connection Class
	//============================================================

	class ConnectionToServer : public std::enable_shared_from_this<ConnectionToServer>, public Connection
	{
	public:

		//==============================
		// Constructors/Destructors
		//==============================

		ConnectionToServer(asio::io_context& asioContext, asio::ip::tcp::socket&& socket,
			tsqueue<owned_message>& qIn, std::condition_variable& newCV,
			std::mutex& newMutex);
		virtual ~ConnectionToServer() override {}

		//==============================
		// LifeCycle Functions
		//==============================

		bool Connect(const asio::ip::tcp::resolver::results_type& endpoints);

		virtual void Disconnect() override;

		bool IsConnected() const { return m_TCPSocket.is_open(); }

	private:

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

	};
}

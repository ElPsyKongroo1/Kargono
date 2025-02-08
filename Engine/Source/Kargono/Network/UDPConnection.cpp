#include "kgpch.h"

#include "Kargono/Network/UDPConnection.h"

#include "Kargono/Utility/FileSystem.h"


namespace Kargono::Network
{
	constexpr uint64_t k_MaxBufferSize{ sizeof(uint8_t) * 1024 * 1 }; // 1KB
	std::vector<uint8_t> s_SocketReceiveBuffer {};
	std::vector<uint8_t> s_SocketSendBuffer {};

	UDPConnection::UDPConnection(NetworkContext* networkContext, asio::ip::udp::socket socket)
		: m_NetworkContextPtr(networkContext), m_Socket(std::move(socket))
	{
		s_SocketReceiveBuffer.resize(k_MaxBufferSize);
		s_SocketSendBuffer.resize(k_MaxBufferSize);
	}

	void UDPConnection::NetworkThreadWakeUp()
	{
		std::unique_lock<std::mutex> lock(m_NetworkContextPtr->BlockThreadMutex);
		m_NetworkContextPtr->BlockThreadCondVar.notify_one();
	}

	void UDPConnection::NetworkThreadSleep()
	{
	}

	void UDPConnection::SendUDPMessage(const LabeledMessage& msg)
	{
		asio::post(m_NetworkContextPtr->AsioContext, [this, msg]()
		{
			bool bWritingMessage = !m_OutgoingMessagesQueue.IsEmpty();
			m_OutgoingMessagesQueue.PushBack(msg);
			if (!bWritingMessage)
			{
				WriteMessageAsync();
			}
		});
	}

	void UDPConnection::Start()
	{
		ReadMessageAsync();
	}

	void UDPConnection::Stop()
	{
	}


	void UDPConnection::ReadMessageAsync()
	{
		// Asynchronous lambda that is handled by the active asio thread to read a message
		m_Socket.async_receive_from(asio::buffer(s_SocketReceiveBuffer.data(), k_MaxBufferSize), m_CurrentEndpoint,
			[this](std::error_code ec, std::size_t length)
			{
				// Check if an error present in the asio context
				if (ec)
				{
					KG_WARN("Error occurred while attempting read a UDP Message. Error Code: [{}] Message: {}", ec.value(), ec.message());
					if (ec.value() == 995) // Application requests termination
					{
						return;
					}
					//Disconnect(m_CurrentEndpoint);
					ReadMessageAsync();
					return;
				}

				// Make sure CRC and Header can be read. Packet might have been broken apart.
				if (length < (sizeof(uint32_t) + sizeof(MessageHeader)))
				{
					ReadMessageAsync();
					return;
				}

				// Load in Message header from buffer
				memcpy(&m_MessageCache.Header, s_SocketReceiveBuffer.data() + sizeof(uint32_t), sizeof(MessageHeader));

				// Get Payload Size
				uint64_t payloadSize = m_MessageCache.Header.PayloadSize;

				// Calculate Cyclic Redundency Check and compare with received value
				uint32_t receivedCRC{};

				memcpy(&receivedCRC, s_SocketReceiveBuffer.data(), sizeof(uint32_t));

				uint32_t currentCRC = Utility::FileSystem::CRCFromBuffer(s_SocketReceiveBuffer.data() + sizeof(uint32_t),
					sizeof(MessageHeader) + payloadSize);

				// If cyclic redundency check fails, move on! It happens...
				if (currentCRC != receivedCRC)
				{
					ReadMessageAsync();
					return;
				}

				// If CRC and Header are valid and payload is too big, remove connection.
				if (payloadSize > k_MaxBufferSize)
				{
					KG_WARN("Payload size of received UDP message is larger than buffer!");
					Disconnect(m_CurrentEndpoint);
					return;
				}

				// Load in Payload into outgoing message!
				if (payloadSize > 0)
				{
					m_MessageCache.Payload.resize(payloadSize);
					memcpy(m_MessageCache.Payload.data(),
						s_SocketReceiveBuffer.data() + sizeof(MessageHeader) + sizeof(uint32_t),
						payloadSize);
				}
				
				AddMessageToIncomingMessageQueue();
				
			});
	}

	void UDPConnection::WriteMessageAsync()
	{
		uint64_t payloadSize = m_OutgoingMessagesQueue.GetFront().msg.Header.PayloadSize;

		if (m_OutgoingMessagesQueue.GetFront().msg.GetEntireMessageSize() > k_MaxBufferSize)
		{
			KG_WARN("Attempt to write UDP message that is larger than the max buffer size!");
			Disconnect(m_CurrentEndpoint);
			return;
		}

		// Load Header after CRC location
		memcpy(s_SocketSendBuffer.data() + sizeof(uint32_t),
			&m_OutgoingMessagesQueue.GetFront().msg.Header,
			sizeof(MessageHeader));

		// Load Buffer after Header
		if (payloadSize > 0)
		{
			memcpy(s_SocketSendBuffer.data() + sizeof(uint32_t) + sizeof(MessageHeader),
				m_OutgoingMessagesQueue.GetFront().msg.Payload.data(),
				payloadSize);
		}

		// Generate CRC hash from completed buffer
		uint32_t hash = Utility::FileSystem::CRCFromBuffer(s_SocketSendBuffer.data() + sizeof(uint32_t),
			sizeof(MessageHeader) + payloadSize);

		// Fill CRC location
		memcpy(s_SocketSendBuffer.data(), &hash, sizeof(uint32_t));

		/*KG_TRACE("The local endpoint address/port are {} {} and remote is {} {}", m_Socket.local_endpoint().address().to_string(), m_Socket.local_endpoint().port(),
			m_qMessagesOut.front().endpoint.address().to_string(), m_qMessagesOut.front().endpoint.port());*/

		m_Socket.async_send_to(asio::buffer(s_SocketSendBuffer.data(), sizeof(uint32_t) + sizeof(MessageHeader) + payloadSize), m_OutgoingMessagesQueue.GetFront().endpoint, [this](std::error_code ec, std::size_t length)
		{
			if (ec)
			{
				KG_WARN("Error occurred while attempting write a UDP Message. Error Code: [{}] Message: {}", ec.value(), ec.message());
				Disconnect(m_CurrentEndpoint);
				return;
			}

			m_OutgoingMessagesQueue.PopFront();
			if (!m_OutgoingMessagesQueue.IsEmpty())
			{
				WriteMessageAsync();
			}
		});
	}
}

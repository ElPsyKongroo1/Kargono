#include "kgpch.h"

#include "Kargono/Network/UDPService.h"

#include "Kargono/Network/Common.h"
#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Network/ConnectionToClient.h"


namespace Kargono::Network
{
	constexpr uint64_t k_MaxBufferSize{ sizeof(uint8_t) * 1024 * 1 }; // 1KB
	std::vector<uint8_t> s_SocketReceiveBuffer {};
	std::vector<uint8_t> s_SocketSendBuffer {};

	UDPService::UDPService(asio::io_context& asioContext, asio::ip::udp::socket socket, 
		tsqueue<owned_message>& qIn, std::condition_variable& newCV, std::mutex& newMutex)
		: m_AsioContext(asioContext), m_Socket(std::move(socket)), m_qMessagesIn(qIn),
		m_BlockThreadCV(newCV), m_BlockThreadMx(newMutex)
	{
		s_SocketReceiveBuffer.resize(k_MaxBufferSize);
		s_SocketSendBuffer.resize(k_MaxBufferSize);
	}

	void UDPService::WakeUpNetworkThread()
	{
		std::unique_lock<std::mutex> lock(m_BlockThreadMx);
		m_BlockThreadCV.notify_one();
	}

	void UDPService::Send(const LabeledMessage& msg)
	{
		asio::post(m_AsioContext, [this, msg]()
		{
			bool bWritingMessage = !m_qMessagesOut.empty();
			m_qMessagesOut.push_back(msg);
			if (!bWritingMessage)
			{
				WriteMessage();
			}
		});
	}

	void UDPService::Start()
	{
		ReadMessage();
	}

	void UDPService::Stop()
	{
	}


	void UDPService::ReadMessage()
	{
		m_Socket.async_receive_from(asio::buffer(s_SocketReceiveBuffer.data(), k_MaxBufferSize), m_CurrentEndpoint,
			[this](std::error_code ec, std::size_t length)
			{
				if (ec)
				{
					KG_ERROR("Failure to read UDP Message! {}", ec.message());
					//Disconnect(m_CurrentEndpoint);
					ReadMessage();
					return;
				}

				// Make sure CRC and Header can be read. Packet might have been broken apart.
				if (length < (sizeof(uint32_t) + sizeof(MessageHeader)))
				{
					ReadMessage();
					return;
				}

				// Load in Message header from buffer
				memcpy_s(&m_MsgTemporaryIn.Header,
					sizeof(MessageHeader),
					s_SocketReceiveBuffer.data() + sizeof(uint32_t),
					sizeof(MessageHeader));

				// Get Payload Size
				uint64_t payloadSize = m_MsgTemporaryIn.Header.PayloadSize;

				// Calculate Cyclic Redundency Check and compare with received value
				uint32_t receivedCRC{};

				memcpy_s(&receivedCRC, sizeof(uint32_t), s_SocketReceiveBuffer.data(), sizeof(uint32_t));

				uint32_t currentCRC = Utility::FileSystem::ChecksumCRCFromBuffer(s_SocketReceiveBuffer.data() + sizeof(uint32_t),
					sizeof(MessageHeader) + payloadSize);

				// If cyclic redundency check fails, move on! It happens...
				if (currentCRC != receivedCRC)
				{
					ReadMessage();
					return;
				}

				// If CRC and Header are valid and payload is too big, remove connection.
				if (payloadSize > k_MaxBufferSize)
				{
					KG_ERROR("Payload size of received UDP message is larger than buffer!");
					Disconnect(m_CurrentEndpoint);
					return;
				}

				// Load in Payload into outgoing message!
				if (payloadSize > 0)
				{
					m_MsgTemporaryIn.Body.resize(payloadSize);
					memcpy_s(m_MsgTemporaryIn.Body.data(),
						payloadSize,
						s_SocketReceiveBuffer.data() + sizeof(MessageHeader) + sizeof(uint32_t),
						payloadSize);
				}
				
				AddToIncomingMessageQueue();
				
			});
	}

	void UDPService::WriteMessage()
	{
		uint64_t payloadSize = m_qMessagesOut.front().msg.Header.PayloadSize;

		if (m_qMessagesOut.front().msg.Size() > k_MaxBufferSize)
		{
			KG_ERROR("Attempt to write UDP message that is large that max buffer size!");
			Disconnect(m_CurrentEndpoint);
			return;
		}

		// Load Header after CRC location
		memcpy_s(s_SocketSendBuffer.data() + sizeof(uint32_t),
			sizeof(MessageHeader),
			&m_qMessagesOut.front().msg.Header,
			sizeof(MessageHeader));

		// Load Buffer after Header
		if (payloadSize > 0)
		{
			memcpy_s(s_SocketSendBuffer.data() + sizeof(uint32_t) + sizeof(MessageHeader),
				payloadSize,
				m_qMessagesOut.front().msg.Body.data(),
				payloadSize);
		}

		uint32_t hash = Utility::FileSystem::ChecksumCRCFromBuffer(s_SocketSendBuffer.data() + sizeof(uint32_t),
			sizeof(MessageHeader) + payloadSize);

		// Fill CRC location
		memcpy_s(s_SocketSendBuffer.data(), sizeof(uint32_t), &hash, sizeof(uint32_t));

		/*KG_TRACE("The local endpoint address/port are {} {} and remote is {} {}", m_Socket.local_endpoint().address().to_string(), m_Socket.local_endpoint().port(),
			m_qMessagesOut.front().endpoint.address().to_string(), m_qMessagesOut.front().endpoint.port());*/

		m_Socket.async_send_to(asio::buffer(s_SocketSendBuffer.data(), sizeof(uint32_t) + sizeof(MessageHeader) + payloadSize), m_qMessagesOut.front().endpoint, [this](std::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				m_qMessagesOut.pop_front();

				if (!m_qMessagesOut.empty())
				{
					WriteMessage();
				}
			}
			else
			{
				KG_ERROR("Failure to write Header!");
				Disconnect(m_CurrentEndpoint);
			}
		});
	}
}

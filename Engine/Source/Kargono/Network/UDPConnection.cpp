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
		// Initalize data buffers
		InitalizeAsyncDataBuffers();
	}

	void UDPConnection::InitalizeAsyncDataBuffers()
	{
		// Initalize read and write buffers
		s_SocketReceiveBuffer.resize(k_MaxBufferSize);
		s_SocketSendBuffer.resize(k_MaxBufferSize);
	}

	void UDPConnection::SendUDPMessage(const LabeledMessage& msg)
	{
		// Invoke an asynchronous function to send the message in pieces
		asio::post(m_NetworkContextPtr->m_AsioContext, [this, msg]()
		{
			// Check if the queue is already writing messages
			bool writingMessage = !m_OutgoingMessagesQueue.IsEmpty();

			// Add the new message to the outgoing queue
			m_OutgoingMessagesQueue.PushBack(msg);

			// Start writing messages if we are not already
			if (!writingMessage)
			{
				WriteMessageAsync();
			}
		});
	}

	void UDPConnection::StartReadingMessages()
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
				// This lambda is called upon completion of the read operation

				// Check if error occurred after attempting a read operation
				if (ec)
				{
					KG_WARN("Error occurred while attempting read a UDP Message. Error Code: [{}] Message: {}", ec.value(), ec.message());
					if (ec.value() == 995) // Application requests termination
					{
						return;
					}
					// TODO: why?
					//Disconnect(m_CurrentEndpoint);
					ReadMessageAsync();
					return;
				}

				// Make sure CRC (validation hash) and Header can be read. Packet might have been broken apart.
				if (length < (sizeof(uint32_t) + sizeof(MessageHeader)))
				{
					ReadMessageAsync();
					return;
				}

				// Load in message header from buffer
				memcpy(&m_MessageCache.m_Header, s_SocketReceiveBuffer.data() + sizeof(uint32_t), sizeof(MessageHeader));

				// Get Payload Size
				uint64_t payloadSize = m_MessageCache.m_Header.m_PayloadSize;

				// Calculate Cyclic Redundency Check and compare with received value
				uint32_t receivedCRC{};

				memcpy(&receivedCRC, s_SocketReceiveBuffer.data(), sizeof(uint32_t));

				uint32_t currentCRC = Utility::FileSystem::CRCFromBuffer(s_SocketReceiveBuffer.data() + sizeof(uint32_t),
					sizeof(MessageHeader) + payloadSize);

				// If cyclic redundency check fails, move on! It happens...
				if (currentCRC != receivedCRC)
				{
					// TODO: Could be a good thing to log!
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
					m_MessageCache.m_PayloadData.resize(payloadSize);
					memcpy(m_MessageCache.m_PayloadData.data(),
						s_SocketReceiveBuffer.data() + sizeof(MessageHeader) + sizeof(uint32_t),
						payloadSize);
				}
				
				// Store the message based on derived class (Server/Client)
				AddMessageToIncomingMessageQueue();

				// Prime the UDP context to accept more messages
				ReadMessageAsync();
				
			});
	}

	void UDPConnection::WriteMessageAsync()
	{
		uint64_t payloadSize = m_OutgoingMessagesQueue.GetFront().m_Message.m_Header.m_PayloadSize;

		// Ensure the new message to write is within the max buffer size
		if (m_OutgoingMessagesQueue.GetFront().m_Message.GetEntireMessageSize() > k_MaxBufferSize)
		{
			// Disconnect the client if message if oversized
			KG_WARN("Attempt to write UDP message that is larger than the max buffer size!");
			Disconnect(m_CurrentEndpoint);
			return;
		}

		// Load Header after CRC location
		memcpy(s_SocketSendBuffer.data() + sizeof(uint32_t),
			&m_OutgoingMessagesQueue.GetFront().m_Message.m_Header,
			sizeof(MessageHeader));

		// Load Buffer after Header
		if (payloadSize > 0)
		{
			memcpy(s_SocketSendBuffer.data() + sizeof(uint32_t) + sizeof(MessageHeader),
				m_OutgoingMessagesQueue.GetFront().m_Message.m_PayloadData.data(),
				payloadSize);
		}

		// Generate CRC hash from completed buffer
		uint32_t hash = Utility::FileSystem::CRCFromBuffer(s_SocketSendBuffer.data() + sizeof(uint32_t),
			sizeof(MessageHeader) + payloadSize);

		// Fill CRC location
		memcpy(s_SocketSendBuffer.data(), &hash, sizeof(uint32_t));

		// Write out the entire UDP packet
		m_Socket.async_send_to(asio::buffer(s_SocketSendBuffer.data(), sizeof(uint32_t) + sizeof(MessageHeader) + payloadSize), m_OutgoingMessagesQueue.GetFront().m_OutgoingEndpoint, [this](std::error_code ec, std::size_t length)
		{
			// This lambda is called upon completion of the write operation

			UNREFERENCED_PARAMETER(length);

			// Handle the case where the write operation fails
			if (ec)
			{
				// Close the TCP & UDP connecetion
				KG_WARN("Error occurred while attempting write a UDP Message. Error Code: [{}] Message: {}", ec.value(), ec.message());
				Disconnect(m_CurrentEndpoint);
				return;
			}

			// Assuming the message has been written successfully

			// Remove the message that has already been processed
			m_OutgoingMessagesQueue.PopFront();

			// Determine if another message should be processed
			if (!m_OutgoingMessagesQueue.IsEmpty())
			{
				WriteMessageAsync();
			}
		});
	}
}

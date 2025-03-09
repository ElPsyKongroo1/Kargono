#include "kgpch.h"

#include "Kargono/Network/TCPConnection.h"

#include "Kargono/Network/Client.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Projects/Project.h"

namespace Kargono::Network
{
	constexpr uint64_t k_MaxBufferSize{ sizeof(uint8_t) * 1024 * 1024 * 10 }; // 10MB

	TCPConnection::TCPConnection(NetworkContext* networkContext, asio::ip::tcp::socket&& socket)
		: m_NetworkContextPtr(networkContext), m_TCPSocket(std::move(socket))
	{
	}
	void TCPConnection::SendTCPMessage(const Message& msg)
	{
		// Invoke an asynchronous function to send the message in pieces
		asio::post(m_NetworkContextPtr->m_AsioContext, [this, msg]()
		{
			// Check if the queue is already writing messages
			bool writingMessage = !m_OutgoingMessageQueue.IsEmpty();

			// Add the new message to the outgoing queue
			m_OutgoingMessageQueue.PushBack(msg);

			// Start writing messages if we are not already
			if (!writingMessage)
			{
				WriteMessageHeaderAsync();
			}
		});
	}
	void TCPConnection::ReadMessageHeaderAsync()
	{
		asio::async_read(m_TCPSocket, asio::buffer(&m_MessageCache.m_Header, sizeof(MessageHeader)),
			[this](std::error_code ec, std::size_t length)
			{
				// Lambda is called upon completion of a read operation

				UNREFERENCED_PARAMETER(length);

				// Handle the case where the read operation fails
				if (ec)
				{
					KG_WARN("Error occurred while attempting to read TCP Header. Error Code: [{}] Message: {}", ec.value(), ec.message());
					Disconnect();
					return;
				}

				// Decide if a payload needs to be processed
				if (m_MessageCache.m_Header.m_PayloadSize > 0)
				{
					// Payload needs to be processed

					// Close the connection when the payload size exceeds buffer
					if (m_MessageCache.m_Header.m_PayloadSize > k_MaxBufferSize)
					{
						KG_WARN("Payload sent that is too large. Malformed message!");
						Disconnect();
						return;
					}

					// Expand the message size to match the payload
					m_MessageCache.m_PayloadData.resize(m_MessageCache.m_Header.m_PayloadSize);

					// Retrieve the payload using another read operation
					ReadMessagePayloadAsync();
				}
				else
				{
					// Handle storing the incoming message (based on server/client)
					AddMessageToIncomingMessageQueue();

					// Prompt the asio context to continue reading messages
					ReadMessageHeaderAsync();
				}
			});
	}
	void TCPConnection::ReadMessagePayloadAsync()
	{
		// This async_read operation stores the received data directly into the m_MessageCache
		asio::async_read(m_TCPSocket, asio::buffer(m_MessageCache.m_PayloadData.data(), m_MessageCache.m_PayloadData.size()),
			[this](std::error_code ec, std::size_t length)
			{
				// Lambda is called upon completion of a read operation

				UNREFERENCED_PARAMETER(length);

				// Handle the case where the read operation fails
				if (ec)
				{
					KG_WARN("Error occurred while attempting to read TCP Payload. Error Code: [{}] Message: {}", ec.value(), ec.message());
					Disconnect();
					return;
				}

				// Handle storing the incoming message (based on server/client)
				AddMessageToIncomingMessageQueue();

				// Prompt the asio context to continue reading messages
				ReadMessageHeaderAsync();
			});
	}
	void TCPConnection::WriteMessageHeaderAsync()
	{
		asio::async_write(m_TCPSocket, asio::buffer(&m_OutgoingMessageQueue.GetFront().m_Header, sizeof(MessageHeader)),
			[this](std::error_code ec, std::size_t length)
			{
				// This lambda is called upon completion of a write operation

				UNREFERENCED_PARAMETER(length);

				// Handle a failure to write to the remote endpoint
				if (ec)
				{
					// Disconnect from the remote endpoint
					KG_WARN("Error occurred while attempting to write a TCP Header. Error Code: [{}] Message: {}", ec.value(), ec.message());
					Disconnect();
					return;
				}

				// Check if a payload needs to be written
				if (m_OutgoingMessageQueue.GetFront().m_PayloadData.size() > 0)
				{
					// Provided payload is too big (might be an attack)
					if (m_OutgoingMessageQueue.GetFront().m_PayloadData.size() > k_MaxBufferSize)
					{
						KG_WARN("Attempt to send message that is larger than maximum buffer size!");
						Disconnect();
						return;
					}

					// Continue to writing out the payload
					WriteMessagePayloadAsync();
				}
				else
				{
					// Since there is no payload, the message has been sent

					// Remove the message from the queue since it has been processed
					m_OutgoingMessageQueue.PopFront();

					// Decide if we should continue writing messages
					if (!m_OutgoingMessageQueue.IsEmpty())
					{
						WriteMessageHeaderAsync();
					}
				}
			});
	}
	void TCPConnection::WriteMessagePayloadAsync()
	{
		asio::async_write(m_TCPSocket, asio::buffer(m_OutgoingMessageQueue.GetFront().m_PayloadData.data(), m_OutgoingMessageQueue.GetFront().m_PayloadData.size()),
			[this](std::error_code ec, std::size_t length)
			{
				// This function is called once the write operation is complete

				UNREFERENCED_PARAMETER(length);

				// Handle the failure to write the payload
				if (ec)
				{
					// Disconnect from remote endpoint upon failure
					KG_WARN("Error occurred while attempting to write a TCP Payload. Error Code: [{}] Message: {}", ec.value(), ec.message());
					Disconnect();
					return;
				}

				// Since the payload of the message has been processed, remove the processed message
				m_OutgoingMessageQueue.PopFront();

				// Decide if more messages should be processed
				if (!m_OutgoingMessageQueue.IsEmpty())
				{
					WriteMessageHeaderAsync();
				}
			});
	}
	uint64_t TCPConnection::GenerateValidationToken(uint64_t input)
	{
		// Transform the input using bit operations with the provided secrets
		uint64_t out = input ^ Projects::ProjectService::GetActiveSecretOne();
		out = (out & Projects::ProjectService::GetActiveSecretTwo()) >> 4 | (out & Projects::ProjectService::GetActiveSecretThree()) << 4;

		// Output the transformed data
		return out ^ Projects::ProjectService::GetActiveSecretFour();
	}
}

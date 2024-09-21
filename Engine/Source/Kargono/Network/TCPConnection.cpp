#include "kgpch.h"

#include "Kargono/Network/TCPConnection.h"

#include "Kargono/Network/Client.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Projects/Project.h"

namespace Kargono::Network
{
	constexpr uint64_t k_MaxBufferSize{ sizeof(uint8_t) * 1024 * 1024 * 10 }; // 5MB


	TCPConnection::TCPConnection(NetworkContext* networkContext, asio::ip::tcp::socket&& socket)
		: m_NetworkContextPtr(networkContext), m_TCPSocket(std::move(socket))
	{
	}
	void TCPConnection::NetworkThreadWakeUp()
	{
		std::unique_lock<std::mutex> lock(m_NetworkContextPtr->BlockThreadMutex);
		m_NetworkContextPtr->BlockThreadCondVar.notify_one();
	}
	void TCPConnection::NetworkThreadSleep()
	{
		std::unique_lock<std::mutex> lock(m_NetworkContextPtr->BlockThreadMutex);
		m_NetworkContextPtr->BlockThreadCondVar.wait(lock);
	}
	void TCPConnection::SendTCPMessage(const Message& msg)
	{
		asio::post(m_NetworkContextPtr->AsioContext, [this, msg]()
		{
			bool bWritingMessage = !m_OutgoingMessageQueue.IsEmpty();
			m_OutgoingMessageQueue.PushBack(msg);
			if (!bWritingMessage)
			{
				WriteMessageHeaderAsync();
			}
		});
	}
	void TCPConnection::ReadMessageHeaderAsync()
	{
		asio::async_read(m_TCPSocket, asio::buffer(&m_MessageCache.Header, sizeof(MessageHeader)),
			[this](std::error_code ec, std::size_t length)
			{
				if (ec)
				{
					KG_WARN("Error occurred while attempting to read TCP Header. Error Code: [{}] Message: {}", ec.value(), ec.message());
					Disconnect();
					return;
				}

				if (m_MessageCache.Header.PayloadSize > 0)
				{
					if (m_MessageCache.Header.PayloadSize > k_MaxBufferSize)
					{
						KG_WARN("Payload sent that is too large. Malformed message!");
						Disconnect();
						return;
					}
					m_MessageCache.Payload.resize(m_MessageCache.Header.PayloadSize);
					ReadMessagePayloadAsync();
				}
				else
				{
					AddMessageToIncomingMessageQueue();
				}
			});
	}
	void TCPConnection::ReadMessagePayloadAsync()
	{
		asio::async_read(m_TCPSocket, asio::buffer(m_MessageCache.Payload.data(), m_MessageCache.Payload.size()),
			[this](std::error_code ec, std::size_t length)
			{
				if (ec)
				{
					KG_WARN("Error occurred while attempting to read TCP Payload. Error Code: [{}] Message: {}", ec.value(), ec.message());
					Disconnect();
					return;
				}

				AddMessageToIncomingMessageQueue();
			});
	}
	void TCPConnection::WriteMessageHeaderAsync()
	{
		asio::async_write(m_TCPSocket, asio::buffer(&m_OutgoingMessageQueue.GetFront().Header, sizeof(MessageHeader)),
			[this](std::error_code ec, std::size_t length)
			{
				if (ec)
				{
					KG_WARN("Error occurred while attempting to write a TCP Header. Error Code: [{}] Message: {}", ec.value(), ec.message());
					Disconnect();
					return;
				}

				if (m_OutgoingMessageQueue.GetFront().Payload.size() > 0)
				{
					if (m_OutgoingMessageQueue.GetFront().Payload.size() > k_MaxBufferSize)
					{
						KG_WARN("Attempt to send message that is larger than maximum buffer size!");
						Disconnect();
						return;
					}

					WriteMessagePayloadAsync();
				}
				else
				{
					m_OutgoingMessageQueue.PopFront();

					if (!m_OutgoingMessageQueue.IsEmpty())
					{
						WriteMessageHeaderAsync();
					}
				}
			});
	}
	void TCPConnection::WriteMessagePayloadAsync()
	{
		asio::async_write(m_TCPSocket, asio::buffer(m_OutgoingMessageQueue.GetFront().Payload.data(), m_OutgoingMessageQueue.GetFront().Payload.size()),
			[this](std::error_code ec, std::size_t length)
			{
				if (ec)
				{
					KG_WARN("Error occurred while attempting to write a TCP Payload. Error Code: [{}] Message: {}", ec.value(), ec.message());
					Disconnect();
					return;
				}

				m_OutgoingMessageQueue.PopFront();
				if (!m_OutgoingMessageQueue.IsEmpty())
				{
					WriteMessageHeaderAsync();
				}
			});
	}
	uint64_t TCPConnection::GenerateValidationToken(uint64_t nInput)
	{
		uint64_t out = nInput ^ Projects::ProjectService::GetActiveSecretOne();
		out = (out & Projects::ProjectService::GetActiveSecretTwo()) >> 4 | (out & Projects::ProjectService::GetActiveSecretThree()) << 4;
		return out ^ Projects::ProjectService::GetActiveSecretFour();
	}
}

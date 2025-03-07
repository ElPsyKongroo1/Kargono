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
	void TCPConnection::NetworkThreadWakeUp()
	{
		std::unique_lock<std::mutex> lock(m_NetworkContextPtr->m_BlockNetworkThreadMutex);
		m_NetworkContextPtr->m_BlockNetworkThreadCondVar.notify_one();
	}
	void TCPConnection::NetworkThreadSleep()
	{
		std::unique_lock<std::mutex> lock(m_NetworkContextPtr->m_BlockNetworkThreadMutex);
		m_NetworkContextPtr->m_BlockNetworkThreadCondVar.wait(lock);
	}
	void TCPConnection::SendTCPMessage(const Message& msg)
	{
		asio::post(m_NetworkContextPtr->m_AsioContext, [this, msg]()
		{
			bool writingMessage = !m_OutgoingMessageQueue.IsEmpty();
			m_OutgoingMessageQueue.PushBack(msg);
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
				UNREFERENCED_PARAMETER(length);
				if (ec)
				{
					KG_WARN("Error occurred while attempting to read TCP Header. Error Code: [{}] Message: {}", ec.value(), ec.message());
					Disconnect();
					return;
				}

				if (m_MessageCache.m_Header.m_PayloadSize > 0)
				{
					if (m_MessageCache.m_Header.m_PayloadSize > k_MaxBufferSize)
					{
						KG_WARN("Payload sent that is too large. Malformed message!");
						Disconnect();
						return;
					}
					m_MessageCache.m_PayloadData.resize(m_MessageCache.m_Header.m_PayloadSize);
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
		asio::async_read(m_TCPSocket, asio::buffer(m_MessageCache.m_PayloadData.data(), m_MessageCache.m_PayloadData.size()),
			[this](std::error_code ec, std::size_t length)
			{
				UNREFERENCED_PARAMETER(length);
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
		asio::async_write(m_TCPSocket, asio::buffer(&m_OutgoingMessageQueue.GetFront().m_Header, sizeof(MessageHeader)),
			[this](std::error_code ec, std::size_t length)
			{
				UNREFERENCED_PARAMETER(length);
				if (ec)
				{
					KG_WARN("Error occurred while attempting to write a TCP Header. Error Code: [{}] Message: {}", ec.value(), ec.message());
					Disconnect();
					return;
				}

				if (m_OutgoingMessageQueue.GetFront().m_PayloadData.size() > 0)
				{
					if (m_OutgoingMessageQueue.GetFront().m_PayloadData.size() > k_MaxBufferSize)
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
		asio::async_write(m_TCPSocket, asio::buffer(m_OutgoingMessageQueue.GetFront().m_PayloadData.data(), m_OutgoingMessageQueue.GetFront().m_PayloadData.size()),
			[this](std::error_code ec, std::size_t length)
			{
				UNREFERENCED_PARAMETER(length);

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

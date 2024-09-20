#include "kgpch.h"

#include "Kargono/Network/TCPConnection.h"

#include "Kargono/Network/Client.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Projects/Project.h"

namespace Kargono::Network
{
	constexpr uint64_t k_MaxBufferSize{ sizeof(uint8_t) * 1024 * 1024 * 10 }; // 5MB


	TCPConnection::TCPConnection(asio::io_context& asioContext, asio::ip::tcp::socket&& socket, TSQueue<owned_message>& qIn, std::condition_variable& newCV, std::mutex& newMutex)
		: m_AsioContext(asioContext), m_TCPSocket(std::move(socket)), m_IncomingMessageQueue(qIn), m_BlockThreadCV(newCV), m_BlockThreadMutex(newMutex)
	{
	}
	void TCPConnection::WakeUpNetworkThread()
	{
		std::unique_lock<std::mutex> lock(m_BlockThreadMutex);
		m_BlockThreadCV.notify_one();
	}
	void TCPConnection::SendTCPMessage(const Message& msg)
	{
		asio::post(m_AsioContext, [this, msg]()
			{
				bool bWritingMessage = !m_OutgoingMessageQueue.IsEmpty();
				m_OutgoingMessageQueue.PushBack(msg);
				if (!bWritingMessage)
				{
					WriteMessageHeader();
				}
			});
	}
	void TCPConnection::ReadMessageHeader()
	{
		asio::async_read(m_TCPSocket, asio::buffer(&m_MessageCache.Header, sizeof(MessageHeader)),
			[this](std::error_code ec, std::size_t length)
			{
				if (ec)
				{
					KG_WARN("Failure to read Header!");
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
					m_MessageCache.Body.resize(m_MessageCache.Header.PayloadSize);
					ReadMessageBody();
				}
				else
				{
					AddToIncomingMessageQueue();
				}
			});
	}
	void TCPConnection::ReadMessageBody()
	{
		asio::async_read(m_TCPSocket, asio::buffer(m_MessageCache.Body.data(), m_MessageCache.Body.size()),
			[this](std::error_code ec, std::size_t length)
			{
				if (ec)
				{
					KG_WARN("Failure to read body!");
					Disconnect();
					return;
				}

				AddToIncomingMessageQueue();
			});
	}
	void TCPConnection::WriteMessageHeader()
	{
		asio::async_write(m_TCPSocket, asio::buffer(&m_OutgoingMessageQueue.GetFront().Header, sizeof(MessageHeader)),
			[this](std::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					if (m_OutgoingMessageQueue.GetFront().Body.size() > 0)
					{
						if (m_OutgoingMessageQueue.GetFront().Body.size() > k_MaxBufferSize)
						{
							KG_ERROR("Attempt to send message that is larger than maximum buffer size!");
							Disconnect();
							return;
						}

						WriteMessageBody();
					}
					else
					{
						m_OutgoingMessageQueue.PopFront();

						if (!m_OutgoingMessageQueue.IsEmpty())
						{
							WriteMessageHeader();
						}
					}
				}
				else
				{
					KG_ERROR("Failure to write Header!");
					Disconnect();
				}
			});
	}
	void TCPConnection::WriteMessageBody()
	{
		asio::async_write(m_TCPSocket, asio::buffer(m_OutgoingMessageQueue.GetFront().Body.data(), m_OutgoingMessageQueue.GetFront().Body.size()),
			[this](std::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					m_OutgoingMessageQueue.PopFront();

					if (!m_OutgoingMessageQueue.IsEmpty())
					{
						WriteMessageHeader();
					}
				}
				else
				{
					KG_WARN("Failure to write Body!");
					Disconnect();
				}
			});
	}
	uint64_t TCPConnection::Scramble(uint64_t nInput)
	{
		uint64_t out = nInput ^ Projects::ProjectService::GetActiveSecretOne();
		out = (out & Projects::ProjectService::GetActiveSecretTwo()) >> 4 | (out & Projects::ProjectService::GetActiveSecretThree()) << 4;
		return out ^ Projects::ProjectService::GetActiveSecretFour();
	}
}

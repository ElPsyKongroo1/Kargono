#include "kgpch.h"

#include "Kargono/Network/TCPConnection.h"

#include "Kargono/Network/Client.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Projects/Project.h"

namespace Kargono::Network
{
	constexpr uint64_t k_MaxBufferSize{ sizeof(uint8_t) * 1024 * 1024 * 10 }; // 5MB


	TCPConnection::TCPConnection(asio::io_context& asioContext, asio::ip::tcp::socket&& socket, TSQueue<owned_message>& qIn, std::condition_variable& newCV, std::mutex& newMutex)
		: m_asioContext(asioContext), m_TCPSocket(std::move(socket)), m_qMessagesIn(qIn), m_BlockThreadCV(newCV), m_BlockThreadMx(newMutex)
	{
	}
	void TCPConnection::WakeUpNetworkThread()
	{
		std::unique_lock<std::mutex> lock(m_BlockThreadMx);
		m_BlockThreadCV.notify_one();
	}
	void TCPConnection::Send(const Message& msg)
	{
		asio::post(m_asioContext, [this, msg]()
			{
				bool bWritingMessage = !m_qMessagesOut.IsEmpty();
				m_qMessagesOut.PushBack(msg);
				if (!bWritingMessage)
				{
					WriteHeader();
				}
			});
	}
	void TCPConnection::ReadHeader()
	{
		asio::async_read(m_TCPSocket, asio::buffer(&m_msgTemporaryIn.Header, sizeof(MessageHeader)),
			[this](std::error_code ec, std::size_t length)
			{
				if (ec)
				{
					KG_WARN("Failure to read Header!");
					Disconnect();
					return;
				}

				if (m_msgTemporaryIn.Header.PayloadSize > 0)
				{
					if (m_msgTemporaryIn.Header.PayloadSize > k_MaxBufferSize)
					{
						KG_WARN("Payload sent that is too large. Malformed message!");
						Disconnect();
						return;
					}
					m_msgTemporaryIn.Body.resize(m_msgTemporaryIn.Header.PayloadSize);
					ReadBody();
				}
				else
				{
					AddToIncomingMessageQueue();
				}
			});
	}
	void TCPConnection::ReadBody()
	{
		asio::async_read(m_TCPSocket, asio::buffer(m_msgTemporaryIn.Body.data(), m_msgTemporaryIn.Body.size()),
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
	void TCPConnection::WriteHeader()
	{
		asio::async_write(m_TCPSocket, asio::buffer(&m_qMessagesOut.GetFront().Header, sizeof(MessageHeader)),
			[this](std::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					if (m_qMessagesOut.GetFront().Body.size() > 0)
					{
						if (m_qMessagesOut.GetFront().Body.size() > k_MaxBufferSize)
						{
							KG_ERROR("Attempt to send message that is larger than maximum buffer size!");
							Disconnect();
							return;
						}

						WriteBody();
					}
					else
					{
						m_qMessagesOut.PopFront();

						if (!m_qMessagesOut.IsEmpty())
						{
							WriteHeader();
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
	void TCPConnection::WriteBody()
	{
		asio::async_write(m_TCPSocket, asio::buffer(m_qMessagesOut.GetFront().Body.data(), m_qMessagesOut.GetFront().Body.size()),
			[this](std::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					m_qMessagesOut.PopFront();

					if (!m_qMessagesOut.IsEmpty())
					{
						WriteHeader();
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

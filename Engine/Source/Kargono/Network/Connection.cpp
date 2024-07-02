#include "kgpch.h"

#include "Kargono/Network/Connection.h"

#include "Kargono/Network/Client.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Network/ClientInterface.h"
#include "Kargono/Projects/Project.h"

namespace Kargono::Network
{
	constexpr uint64_t k_MaxBufferSize{ sizeof(uint8_t) * 1024 * 1024 * 10 }; // 5MB


	Connection::Connection(asio::io_context& asioContext, asio::ip::tcp::socket&& socket, tsqueue<owned_message>& qIn, std::condition_variable& newCV, std::mutex& newMutex)
		: m_asioContext(asioContext), m_TCPSocket(std::move(socket)), m_qMessagesIn(qIn), m_BlockThreadCV(newCV), m_BlockThreadMx(newMutex)
	{
	}
	void Connection::WakeUpNetworkThread()
	{
		std::unique_lock<std::mutex> lock(m_BlockThreadMx);
		m_BlockThreadCV.notify_one();
	}
	void Connection::Send(const Message& msg)
	{
		asio::post(m_asioContext, [this, msg]()
			{
				bool bWritingMessage = !m_qMessagesOut.empty();
				m_qMessagesOut.push_back(msg);
				if (!bWritingMessage)
				{
					WriteHeader();
				}
			});
	}
	void Connection::ReadHeader()
	{
		asio::async_read(m_TCPSocket, asio::buffer(&m_msgTemporaryIn.Header, sizeof(MessageHeader)),
			[this](std::error_code ec, std::size_t length)
			{
				if (ec)
				{
					KG_ERROR("Failure to read Header!");
					Disconnect();
					return;
				}

				if (m_msgTemporaryIn.Header.PayloadSize > 0)
				{
					if (m_msgTemporaryIn.Header.PayloadSize > k_MaxBufferSize)
					{
						KG_ERROR("Payload sent that is too large. Malformed message!");
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
	void Connection::ReadBody()
	{
		asio::async_read(m_TCPSocket, asio::buffer(m_msgTemporaryIn.Body.data(), m_msgTemporaryIn.Body.size()),
			[this](std::error_code ec, std::size_t length)
			{
				if (ec)
				{
					KG_ERROR("Failure to read body!");
					Disconnect();
					return;
				}

				AddToIncomingMessageQueue();
			});
	}
	void Connection::WriteHeader()
	{
		asio::async_write(m_TCPSocket, asio::buffer(&m_qMessagesOut.front().Header, sizeof(MessageHeader)),
			[this](std::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					if (m_qMessagesOut.front().Body.size() > 0)
					{
						if (m_qMessagesOut.front().Body.size() > k_MaxBufferSize)
						{
							KG_ERROR("Attempt to send message that is larger than maximum buffer size!");
							Disconnect();
							return;
						}

						WriteBody();
					}
					else
					{
						m_qMessagesOut.pop_front();

						if (!m_qMessagesOut.empty())
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
	void Connection::WriteBody()
	{
		asio::async_write(m_TCPSocket, asio::buffer(m_qMessagesOut.front().Body.data(), m_qMessagesOut.front().Body.size()),
			[this](std::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					m_qMessagesOut.pop_front();

					if (!m_qMessagesOut.empty())
					{
						WriteHeader();
					}
				}
				else
				{
					KG_ERROR("Failure to write Body!");
					Disconnect();
				}
			});
	}
	uint64_t Connection::Scramble(uint64_t nInput)
	{
		auto currentProject = Projects::Project::GetActive();

		uint64_t out = nInput ^ currentProject->GetSecretOne();
		out = (out & currentProject->GetSecretTwo()) >> 4 | (out & currentProject->GetSecretThree()) << 4;
		return out ^ currentProject->GetSecretFour();
	}
}

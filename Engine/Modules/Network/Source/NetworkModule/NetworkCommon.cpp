#include "kgpch.h"

#include "NetworkModule/NetworkCommon.h"

namespace Kargono::Network
{
	size_t Message::GetEntireMessageSize() const
	{
		return sizeof(MessageHeader) + m_PayloadData.size();
	}
	void Message::AppendPayload(void* buffer, uint64_t size)
	{
		// Cache current size of vector, as this will be the point we insert the data
		size_t cachedBufferSize = m_PayloadData.size();

		// Resize the vector to contain the old-data + new-data + size-of-payload
		m_PayloadData.resize(m_PayloadData.size() + size + sizeof(uint64_t));

		// Copy the data into the end of the current buffer
		std::memcpy(m_PayloadData.data() + cachedBufferSize, buffer, size);

		// Append the new size of the payload to the end
		std::memcpy(m_PayloadData.data() + cachedBufferSize + size, &size, sizeof(uint64_t));

		// Update message header's payload size
		m_Header.m_PayloadSize = m_PayloadData.size();
	}
	std::vector<uint8_t> Message::GetPayloadCopy(uint64_t size)
	{
		std::vector<uint8_t> newBuffer{};
		newBuffer.resize(size);

		// Cache current size of vector, as this will be the point we insert the data
		size_t bufferSizeAfterRemoval = m_PayloadData.size() - size;

		// Copy the data over
		std::memcpy((void*)newBuffer.data(), m_PayloadData.data() + bufferSizeAfterRemoval, size);

		// Shrink the vector to remove the read bytes
		m_PayloadData.resize(bufferSizeAfterRemoval);

		// Update message header size
		m_Header.m_PayloadSize = m_PayloadData.size();

		return newBuffer;
	}
	std::ostream& operator<<(std::ostream& os, const Message& msg)
	{
		os << "ID:" << static_cast<int32_t>(msg.m_Header.m_MessageType) << " Size:" << msg.m_Header.m_PayloadSize;
		return os;
	}
}

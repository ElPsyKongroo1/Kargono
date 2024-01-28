#include "kgpch.h"

#include "Kargono/Network/Message.h"

namespace Kargono::Network
{
	size_t Message::Size() const
	{
		return sizeof(MessageHeader) + Body.size();
	}
	void Message::PushBuffer(void* buffer, uint64_t size)
	{
		// Cache current size of vector, as this will be the point we insert the data
		size_t currentBufferSize = Body.size();

		// Resize the vector to contain the new data being added
		Body.resize(Body.size() + size + sizeof(uint64_t));

		// Copy the data over
		std::memcpy(Body.data() + currentBufferSize, buffer, size);

		// Copy the data size over
		std::memcpy(Body.data() + currentBufferSize + size, &size, sizeof(uint64_t));

		// Update message header size
		Header.PayloadSize = size;

		// Update message header size
		Header.PayloadSize = Body.size();

	}
	std::vector<uint8_t> Message::GetBuffer(uint64_t size)
	{
		std::vector<uint8_t> newBuffer{};
		newBuffer.resize(size);

		// Cache current size of vector, as this will be the point we insert the data
		size_t bufferSizeAfterRemoval = Body.size() - size;

		// Copy the data over
		std::memcpy((void*)newBuffer.data(), Body.data() + bufferSizeAfterRemoval, size);

		// Shrink the vector to remove the read bytes
		Body.resize(bufferSizeAfterRemoval);

		// Update message header size
		Header.PayloadSize = Body.size();

		return newBuffer;
	}
	std::ostream& operator<<(std::ostream& os, const Message& msg)
	{
		os << "ID:" << static_cast<int32_t>(msg.Header.ID) << " Size:" << msg.Header.PayloadSize;
		return os;
	}
	std::ostream& operator<<(std::ostream& os, const owned_message& msg)
	{
		os << msg.msg;
		return os;
	}
}

#include "kgpch.h"

#include "Kargono/Network/NetworkCommon.h"

namespace Kargono::Network
{
	size_t Message::GetEntireMessageSize() const
	{
		return sizeof(MessageHeader) + Payload.size();
	}
	void Message::StorePayload(void* buffer, uint64_t size)
	{
		// Cache current size of vector, as this will be the point we insert the data
		size_t currentBufferSize = Payload.size();

		// Resize the vector to contain the new data being added
		Payload.resize(Payload.size() + size + sizeof(uint64_t));

		// Copy the data over
		std::memcpy(Payload.data() + currentBufferSize, buffer, size);

		// Copy the data size over
		std::memcpy(Payload.data() + currentBufferSize + size, &size, sizeof(uint64_t));

		// Update message header size
		Header.PayloadSize = size;

		// Update message header size
		Header.PayloadSize = Payload.size();

	}
	std::vector<uint8_t> Message::GetPayloadCopy(uint64_t size)
	{
		std::vector<uint8_t> newBuffer{};
		newBuffer.resize(size);

		// Cache current size of vector, as this will be the point we insert the data
		size_t bufferSizeAfterRemoval = Payload.size() - size;

		// Copy the data over
		std::memcpy((void*)newBuffer.data(), Payload.data() + bufferSizeAfterRemoval, size);

		// Shrink the vector to remove the read bytes
		Payload.resize(bufferSizeAfterRemoval);

		// Update message header size
		Header.PayloadSize = Payload.size();

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

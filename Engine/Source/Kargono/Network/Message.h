#pragma once


#include "Kargono/Core/Base.h"

#include "API/Network/AsioAPI.h"

#include <cstdint>
#include <vector>


namespace Kargono::Network
{
	//class Connection;
	class ConnectionToClient;

	//============================================================
	// Message Struct
	//============================================================

	struct MessageHeader
	{
		uint32_t ID{};
		uint64_t PayloadSize = 0;
	};

	//============================================================
	// Message Struct
	//============================================================
	struct Message
	{
		MessageHeader Header {};
		std::vector<uint8_t> Body;

		//==============================
		// Getters/Setters
		//==============================

		// Return size of Payload + Header
		size_t Size() const;

		// Add Variable Length Buffer
		void PushBuffer(void* buffer, uint64_t size);

		std::vector<uint8_t> GetBuffer(uint64_t size);

		// Returns pointer to start of payload
		void* GetBodyData() { return (void*)Body.data(); }

		// Returns size of payload in bytes
		uint64_t GetBodySize() { return Header.PayloadSize; }

		//==============================
		// Operator Overloads
		//==============================

		// Overload for printing messages to the standard console
		friend std::ostream& operator << (std::ostream& os, const Message& msg);

		// Push trivial data types into the message buffer
		template <typename DataType>
		friend Message& operator << (Message& msg, const DataType& data);

		// Push trivial data types into the message buffer
		template <typename DataType>
		friend Message& operator >> (Message& msg, const DataType& data);
	};

	//============================================================
	// Owned Message Struct
	//============================================================

	struct owned_message
	{
		Ref<ConnectionToClient> remote = nullptr;
		Message msg;

		//==============================
		// Operator Overloads
		//==============================
		friend std::ostream& operator<<(std::ostream& os, const owned_message& msg);
	};

	struct LabeledMessage
	{
		asio::ip::udp::endpoint& endpoint;
		Message msg;
	};

	//==============================
	// Operator Overload Definitions
	//==============================

	template<typename DataType>
	Message& operator<<(Message& msg, const DataType& data)
	{
		// Check that the type of the data being pushed is trivially copyable
		static_assert(std::is_standard_layout_v<DataType>, "Data is too complex");

		// Cache current size of vector, as this will be the point we insert the data
		size_t currentBufferSize = msg.Body.size();

		// Resize the vector to contain the new data being added
		msg.Body.resize(msg.Body.size() + sizeof(DataType));

		// Copy the data over
		std::memcpy(msg.Body.data() + currentBufferSize, &data, sizeof(DataType));

		// Update message header size
		msg.Header.PayloadSize = static_cast<uint32_t>(msg.Body.size());

		return msg;
	}

	template<typename DataType>
	Message& operator>>(Message& msg, const DataType& data)
	{
		// Check that the type of the data being pushed is trivially copyable
		static_assert(std::is_standard_layout<DataType>::value, "Data is too complex");

		// Cache current size of vector, as this will be the point we insert the data
		size_t bufferSizeAfterRemoval = msg.Body.size() - sizeof(DataType);

		// Copy the data over
		std::memcpy((void*)&data, msg.Body.data() + bufferSizeAfterRemoval, sizeof(DataType));

		// Shrink the vector to remove the read bytes
		msg.Body.resize(bufferSizeAfterRemoval);

		// Update message header size
		msg.Header.PayloadSize = static_cast<uint32_t>(msg.Size());

		return msg;
	}
}

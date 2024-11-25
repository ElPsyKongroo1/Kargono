#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/DataStructures.h"

#include "API/Network/AsioAPI.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <cstdint>
#include <vector>

namespace Kargono::Network
{
	//==============================
	// Message Type
	//==============================
	enum class MessageType : uint32_t
	{
		AcceptConnection = 0,
		DenyConnection,
		ServerPing,
		MessageAll,
		ServerMessage,
		ClientChat,
		ServerChat,
		KeepAlive,
		UDPInit,

		RequestUserCount,
		UpdateUserCount,
		UpdateSessionUserSlot,
		LeaveCurrentSession,
		UserLeftSession,

		// Session Messages
		RequestJoinSession,
		ApproveJoinSession,
		DenyJoinSession,
		CurrentSessionInit,
		StartSession,
		InitSyncPing,
		SessionReadyCheck,
		SessionReadyCheckConfirm,
		EnableReadyCheck,

		// Entity Updates
		SendAllEntityLocation,
		UpdateEntityLocation,
		SendAllEntityPhysics,
		UpdateEntityPhysics,
		SignalAll,
		ReceiveSignal
	};

	class ServerTCPConnection;

	//==============================
	// Message Header Struct
	//==============================
	struct MessageHeader
	{
		uint32_t ID{};
		uint64_t PayloadSize{ 0 };
	};

	//==============================
	// Message Struct
	//==============================
	struct Message
	{
		MessageHeader Header{};
		std::vector<uint8_t> Payload;

		//==============================
		// Modify Message Data
		//==============================
		// Replace payload data with provided buffer data
		void StorePayload(void* buffer, uint64_t size);
		// Push provided data onto the end of the message's payload
		template <typename DataType>
		friend Message& operator << (Message& msg, const DataType& data);
		// Remove provided data type from the end of the message and place the data into the indicated location
		template <typename DataType>
		friend Message& operator >> (Message& msg, const DataType& data);

		//==============================
		// Getters/Setters
		//==============================
		// Return size of Payload + Header
		size_t GetEntireMessageSize() const;
		// Returns pointer to start of payload
		void* GetPayloadPointer() { return (void*)Payload.data(); }
		// Returns size of payload in bytes
		uint64_t GetPayloadSize() { return Header.PayloadSize; }
		// Return copy internal buffer
		std::vector<uint8_t> GetPayloadCopy(uint64_t size);

	};

	//============================================================
	// Owned Message Struct
	//============================================================
	struct owned_message
	{
		ServerTCPConnection* remote = nullptr;
		Message msg;
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
		size_t currentBufferSize = msg.Payload.size();

		// Resize the vector to contain the new data being added
		msg.Payload.resize(msg.Payload.size() + sizeof(DataType));

		// Copy the data over
		std::memcpy(msg.Payload.data() + currentBufferSize, &data, sizeof(DataType));

		// Update message header size
		msg.Header.PayloadSize = static_cast<uint32_t>(msg.Payload.size());

		return msg;
	}

	template<typename DataType>
	Message& operator>>(Message& msg, const DataType& data)
	{
		// Check that the type of the data being pushed is trivially copyable
		static_assert(std::is_standard_layout<DataType>::value, "Data is too complex");

		// Cache current size of vector, as this will be the point we insert the data
		size_t bufferSizeAfterRemoval = msg.Payload.size() - sizeof(DataType);

		// Copy the data over
		std::memcpy((void*)&data, msg.Payload.data() + bufferSizeAfterRemoval, sizeof(DataType));

		// Shrink the vector to remove the read bytes
		msg.Payload.resize(bufferSizeAfterRemoval);

		// Update message header size
		msg.Header.PayloadSize = static_cast<uint32_t>(msg.GetEntireMessageSize());

		return msg;
	}

	struct NetworkContext
	{
		// Asio context and accompanying thread
		asio::io_context AsioContext;
		std::thread AsioThread;
		// Network thread and supporting mutex/condition_variable
		Ref<std::thread> NetworkThread { nullptr };
		std::condition_variable BlockThreadCondVar {};
		std::mutex BlockThreadMutex {};
		std::atomic<bool> Quit { false };
		// Only incoming message queue
		TSQueue<owned_message> IncomingMessagesQueue;
	};

	static inline constexpr uint64_t k_KeepAliveDelay{ 10'000 };
	inline static constexpr uint16_t k_MaxSyncPings = 10;
	inline static constexpr uint16_t k_InvalidSessionSlot = std::numeric_limits<uint16_t>::max();
	// TODO: VERY TEMPORARY. Only for pong!!!!
	inline static constexpr uint32_t k_MaxSessionClients {2};
}



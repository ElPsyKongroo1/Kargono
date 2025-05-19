#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/DataStructures.h"
#include "Kargono/Math/MathAliases.h"

#include <limits>
#include <thread>
#include <atomic>
#include <mutex>
#include <cstdint>
#include <vector>
#include <string>

namespace Kargono::Network
{
	//==============================
	// Message Type
	//==============================
	enum class MessageType : uint8_t
	{
		// Manage client <-> server connnection
		ManageConnection_AcceptConnection = 0,
		ManageConnection_DenyConnection,
		ManageConnection_RequestConnection,
		ManageConnection_KeepAlive,

		// Query the active server state
		ServerQuery_RequestClientCount,
		ServerQuery_ReceiveClientCount,

		// Session messages
		ManageSession_UpdateClientSlot,
		ManageSession_NotifyAllLeave,
		ManageSession_ClientLeft,
		ManageSession_RequestClientJoin,
		ManageSession_ApproveClientJoin,
		ManageSession_DenyClientJoin,
		ManageSession_StartSession,
		ManageSession_Init,
		ManageSession_StartReadyCheck,
		ManageSession_ConfirmReadyCheck,
		ManageSession_EnableReadyCheck,

		// Entity updates
		ManageSceneEntity_SendAllClientsLocation,
		ManageSceneEntity_UpdateLocation,
		ManageSceneEntity_SendAllClientsPhysics,
		ManageSceneEntity_UpdatePhysics,

		// Script communication
		ScriptMessaging_SendAllClientsSignal,
		ScriptMessaging_ReceiveSignal
	};

	// Packet Header Types
	using AppID = uint8_t;
	using ClientIndex = uint8_t;

	// Reliability Types
	using PacketSequence = uint16_t;
	using AckBitField = uint32_t;
	constexpr PacketSequence k_AckBitFieldSize{ (PacketSequence)sizeof(AckBitField) * (PacketSequence)8 };

	constexpr ClientIndex k_InvalidClientIndex{ std::numeric_limits<ClientIndex>::max() };

	constexpr size_t k_ReliabilitySegmentSize
	{
		sizeof(PacketSequence) /*packetSequenceNum*/ +
		sizeof(PacketSequence) /*ackSequenceNum*/ +
		sizeof(AckBitField) /*ackBitfield*/
	};
	constexpr size_t k_PacketHeaderSize
	{
		sizeof(AppID) /*appID*/ +
		sizeof(MessageType) /*messageType*/ +
		sizeof(ClientIndex) + /*clientIndex*/
		k_ReliabilitySegmentSize /*packetAckSegment*/
	};
	constexpr size_t k_MaxPacketSize{ 256 };
	constexpr size_t k_MaxPayloadSize{ k_MaxPacketSize - k_PacketHeaderSize };

	inline bool IsConnectionManagementPacket(MessageType type)
	{
		switch (type)
		{
		case MessageType::ManageConnection_AcceptConnection:
		case MessageType::ManageConnection_DenyConnection:
		case MessageType::ManageConnection_RequestConnection:
			return true;
		default:
			return false;
		}
	}

	//==============================
	// Message Header Struct
	//==============================
	struct MessageHeader
	{
		MessageType m_MessageType{};
		uint64_t m_PayloadSize{ 0 };
	};

	//==============================
	// Message Struct
	//==============================
	struct Message
	{
		//==============================
		// Fields
		//==============================
		MessageHeader m_Header{};
		std::vector<uint8_t> m_PayloadData;

		//==============================
		// Modify Message Data
		//==============================
		// Replace payload data with provided buffer data
		void AppendPayload(void* buffer, uint64_t size);
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
		void* GetPayloadPointer() { return (void*)m_PayloadData.data(); }
		// Returns size of payload in bytes
		uint64_t GetPayloadSize() { return m_Header.m_PayloadSize; }
		// Return copy internal buffer
		std::vector<uint8_t> GetPayloadCopy(uint64_t size);

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
		size_t currentBufferSize = msg.m_PayloadData.size();

		// Resize the vector to contain the new data being added
		msg.m_PayloadData.resize(msg.m_PayloadData.size() + sizeof(DataType));

		// Copy the data over
		std::memcpy(msg.m_PayloadData.data() + currentBufferSize, &data, sizeof(DataType));

		// Update message header size
		msg.m_Header.m_PayloadSize = (uint32_t)msg.m_PayloadData.size();

		return msg;
	}

	template<typename DataType>
	Message& operator>>(Message& msg, const DataType& data)
	{
		// Check that the type of the data being pushed is trivially copyable
		static_assert(std::is_standard_layout<DataType>::value, "Data is too complex");
		KG_ASSERT((int)msg.m_PayloadData.size() - (int)sizeof(DataType) >= 0);

		// Cache current size of vector, as this will be the point we insert the data
		size_t bufferSizeAfterRemoval = msg.m_PayloadData.size() - sizeof(DataType);

		// Copy the data over
		std::memcpy((void*)&data, msg.m_PayloadData.data() + bufferSizeAfterRemoval, sizeof(DataType));

		// Shrink the vector to remove the read bytes
		msg.m_PayloadData.resize(bufferSizeAfterRemoval);

		// Update message header size
		msg.m_Header.m_PayloadSize = (uint32_t)msg.GetEntireMessageSize();

		return msg;
	}
	using SessionIndex = uint8_t;
	constexpr SessionIndex k_InvalidSessionIndex = std::numeric_limits<SessionIndex>::max();
	// TODO: VERY TEMPORARY. Only for pong!!!!
	constexpr size_t k_MaxSessionClients{ 2 };

}

#pragma once
#include <cstdint>
#include <limits>

namespace Kargono::Network
{
	using AppID = uint8_t;
	using ClientIndex = uint8_t;

	constexpr ClientIndex k_InvalidClientIndex{ std::numeric_limits<ClientIndex>::max() };

	enum class PacketType : uint8_t
	{
		KeepAlive,
		Message,
		ConnectionRequest,
		ConnectionSuccess,
		ConnectionDenied
	};

	constexpr size_t k_ReliabilitySegmentSize
	{
		sizeof(uint16_t) /*packetSequenceNum*/ +
		sizeof(uint16_t) /*ackSequenceNum*/ +
		sizeof(uint32_t) /*ackBitfield*/
	};
	constexpr size_t k_PacketHeaderSize
	{
		sizeof(AppID) /*appID*/ +
		sizeof(PacketType) /*packetType*/ +
		sizeof(ClientIndex) + /*clientIndex*/
		k_ReliabilitySegmentSize /*packetAckSegment*/
	};
	constexpr size_t k_MaxPacketSize{ 256 };
	constexpr size_t k_MaxPayloadSize{ k_MaxPacketSize - k_PacketHeaderSize };

	inline bool IsConnectionManagementPacket(PacketType type)
	{
		switch (type)
		{
		case PacketType::ConnectionDenied:
		case PacketType::ConnectionRequest:
		case PacketType::ConnectionSuccess:
			return true;
		default:
			return false;
		}
	}
}

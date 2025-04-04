#include "kgpch.h"

#include "ReliabilityContext.h"

#include <cmath>
#include <chrono>
#include <bit>
#include <iostream>
#include <format>

namespace Kargono::Network
{
	static float GetTime()
	{
		using namespace std::chrono;
		return duration<float>(steady_clock::now().time_since_epoch()).count();
	}

	static bool SequenceGreaterThan(uint16_t sequence1, uint16_t sequence2)
	{
		constexpr uint16_t k_HalfShort{ 32768 };

		return ((sequence1 > sequence2) && (sequence1 - sequence2 <= k_HalfShort)) ||
			((sequence1 < sequence2) && (sequence2 - sequence1 > k_HalfShort));
	}

	void ReliabilityContext::OnUpdate(float deltaTime)
	{
		m_LastPacketReceived += deltaTime;

		m_CongestionContext.OnUpdate(deltaTime, m_RoundTripContext.GetAverageRoundTrip());
	}

	void ReliabilityContext::InsertReliabilitySegmentIntoPacket(uint8_t* segmentLocation)
	{
		// Initialize the locations of the seq, ack, and ack-bitfield
		uint16_t& sequenceLocation = *(uint16_t*)segmentLocation;
		uint16_t& ackLocation = *(uint16_t*)(segmentLocation + sizeof(sequenceLocation));
		uint32_t& bitFieldLocation = *(uint32_t*)(segmentLocation +
			sizeof(sequenceLocation) + sizeof(ackLocation));

		//KG_WARN("Sent the packet %d. It has the ack %d. It also has the bit field: ", m_LocalSequence, m_RemoteSequence);
		//std::cout << std::format("{:032b}\n", m_RemoteAckField.GetRawBitfield());

		// Insert the sequence number (appID|[sequenceNum]|ackNum|ackBitField|...)
		InsertLocalSequenceNumber(sequenceLocation);

		// Insert the recent ack number (appID|sequenceNum|[ackNum]|ackBitField|...) 
		InsertRemoteSequenceNumber(ackLocation);

		// Insert the recent ack's bitfield (appID|sequenceNum|ackNum|[ackBitField]|...)
		InsertRemoteSequenceBitField(bitFieldLocation);
	}

	void ReliabilityContext::ProcessReliabilitySegmentFromPacket(uint8_t* segmentLocation)
	{
		// Initialize the locations of the seq, ack, and ack-bitfield
		uint16_t packetSequence = *(uint16_t*)segmentLocation;
		uint16_t packetAck = *(uint16_t*)(segmentLocation + sizeof(packetSequence));
		uint32_t packetAckBitfield = *(uint32_t*)(segmentLocation +
			sizeof(packetSequence) + sizeof(packetAck));

		// Update the remote data based on the received sequence number
		if (!ProcessReceivedSequenceNumber(packetSequence))
		{
			return;
		}

		// Check the ack context
		if (!ProcessReceivedAck(packetAck, packetAckBitfield))
		{
			return;
		}

		// Packet received successfully
		m_LastPacketReceived = 0.0f;
	}

	void ReliabilityContext::InsertLocalSequenceNumber(uint16_t& sequenceLocation)
	{
		// Insert the current local sequence number the into packet location
		sequenceLocation = m_LocalSequence;

		// Check for drop packet at the 32 bit boundary of the bitfield!
		if (!m_LocalAckField.IsFlagSet(31))
		{
			uint16_t droppedPacketSeq = sequenceLocation - 31;
			float packetRTT = GetTime() - m_RoundTripContext.GetTimePoint(droppedPacketSeq);
			ProcessRoundTrip(packetRTT);
		}

		// Add new packet creation time to round trip calculator
		m_RoundTripContext.AddTimePoint(sequenceLocation);

		// Move sequence number to next packet number
		m_LocalSequence++;

		// Update the local bitfield to make space for the new packet
		m_LocalAckField.SetRawBitfield(m_LocalAckField.GetRawBitfield() << 1); // I want it to overflow!
	}

	void ReliabilityContext::InsertRemoteSequenceNumber(uint16_t& ackLocation)
	{
		// Insert the current remote sequence number into the packet
		ackLocation = m_RemoteSequence;
	}

	void ReliabilityContext::InsertRemoteSequenceBitField(uint32_t& bitFieldLocation)
	{
		// Insert the bitfield
		bitFieldLocation = (uint32_t)m_RemoteAckField.GetRawBitfield(); // Truncate the bitfield
	}

	bool ReliabilityContext::ProcessReceivedSequenceNumber(uint16_t receivedSequenceNumber)
	{
		uint16_t distance = std::abs((int16_t)receivedSequenceNumber - (int16_t)m_RemoteSequence);
		bool excessiveDistance = distance > 31;

		// Check if the current sequence number is newer
		if (SequenceGreaterThan(receivedSequenceNumber, m_RemoteSequence))
		{
			// The received packet is 'newer' than the current sequence number's packet

			// Clear the bit set if we are shifting too much
			if (excessiveDistance)
			{
				m_RemoteAckField.ClearAllFlags();
				return true;
			}

			m_RemoteAckField.SetRawBitfield(m_RemoteAckField.GetRawBitfield() << (distance));
			m_RemoteAckField.SetFlag(0);


			// Update to the new sequence number
			m_RemoteSequence = receivedSequenceNumber;
		}
		else
		{

			// The received packet is 'older' than the current sequence number's packet

			// Early out if we are shifting too much or packet is already ack'd
			if (excessiveDistance || m_RemoteAckField.IsFlagSet((uint8_t)distance))
			{
				return false;
			}

			// Update the bit of the packet received
			m_RemoteAckField.SetFlag((uint8_t)distance);
		}

		return true;
	}

	bool ReliabilityContext::ProcessReceivedAck(uint16_t ackNumber, uint32_t ackBitField)
	{
		uint16_t distance = std::abs((int16_t)m_LocalSequence - (int16_t)ackNumber);
		bool excessiveDistance = distance > 32;

		// The ack number should never be greater than the local sequence value
		// (If so, likely a corrupted packet or a bad actor)
		if (SequenceGreaterThan(ackNumber, m_LocalSequence) ||
			ackNumber == m_LocalSequence ||
			excessiveDistance)
		{
			return false;
		}

		// Modify the received bit field to align with the local bitfield
		ackBitField = (ackBitField << (distance - 1));

		// Use logical implication to reveal modified packets
		uint32_t newlyAcknowledgedField = (~m_LocalAckField.GetRawBitfield()) & ackBitField;

		// Scan the newly-acknowledged-field and acknowledge the packets
		// TODO: Add meaningful acknowledgement method here
		for (uint16_t iteration{ 0 }; iteration < 32; iteration++)
		{
			// Check this specific bit
			bool isSet = (newlyAcknowledgedField >> iteration) & 1;

			// Acknowledged a packet
			if (isSet)
			{
				uint16_t ackPacketSeq = m_LocalSequence - 1 - iteration;
				float packetRTT = GetTime() - m_RoundTripContext.GetTimePoint(ackPacketSeq);
				ProcessRoundTrip(packetRTT);
			}
		}

		// Finally, update the local bitfield with new acknowledgements
		m_LocalAckField.SetRawBitfield(m_LocalAckField.GetRawBitfield() | ackBitField);

		return newlyAcknowledgedField;
	}

	void ReliabilityContext::ProcessRoundTrip(float packetRoundTrip)
	{
		m_RoundTripContext.UpdateAverageRoundTrip(packetRoundTrip);

		m_CongestionContext.OnRoundTripChange(m_RoundTripContext.GetAverageRoundTrip());
	}


	void RoundTripContext::AddTimePoint(uint16_t sequenceNumber)
	{
		m_SendTimepoints[sequenceNumber % 32] = GetTime();
	}

	float RoundTripContext::GetTimePoint(uint16_t sequenceNumber)
	{
		return m_SendTimepoints[sequenceNumber % 32];
	}

	void RoundTripContext::UpdateAverageRoundTrip(float roundTrip)
	{
		constexpr float k_ShiftFactor{ 0.1f };

		m_AverageRoundTrip = (1.0f - k_ShiftFactor) * m_AverageRoundTrip + k_ShiftFactor * roundTrip;
	}

	float RoundTripContext::GetAverageRoundTrip()
	{
		return m_AverageRoundTrip;
	}

	void CongestionContext::OnUpdate(float deltaTime, float averageRoundTrip)
	{
		if (averageRoundTrip < m_CongestionConfig.m_CongestedRTTThreshold)
		{
			m_TimeNotCongested += deltaTime;
		}

		if (m_IsCongested && m_TimeNotCongested > m_ResetCongestedTime)
		{
			KG_WARN("Connection is no longer congested");
			m_IsCongested = false;
		}
	}

	void CongestionContext::OnRoundTripChange(float averageRoundTrip)
	{
		// Update congestion values
		if (averageRoundTrip > m_CongestionConfig.m_CongestedRTTThreshold)
		{
			m_TimeNotCongested = 0.0f;

			if (!m_IsCongested)
			{
				KG_WARN("Connection is now congested");
				m_IsCongested = true;
			}
		}
	}

	bool CongestionContext::IsCongested()
	{
		return m_IsCongested;
	}
}

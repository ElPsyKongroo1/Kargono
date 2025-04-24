#include "kgpch.h"

#include "NetworkPlugin/ReliabilityContext.h"

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

	static bool SequenceGreaterThan(PacketSequence sequence1, PacketSequence sequence2)
	{
		constexpr PacketSequence k_HalfSequence{ std::numeric_limits<PacketSequence>::max() / 2 };

		return ((sequence1 > sequence2) && (sequence1 - sequence2 <= k_HalfSequence)) ||
			((sequence1 < sequence2) && (sequence2 - sequence1 > k_HalfSequence));
	}

	void ReliabilityContext::OnUpdate(float deltaTime)
	{
		m_LastPacketReceived += deltaTime;

		m_CongestionContext.OnUpdate(deltaTime, m_RoundTripContext.GetAverageRoundTrip());
	}

	void ReliabilityContext::InsertReliabilitySegmentIntoPacket(uint8_t* segmentLocation)
	{
		// Initialize the locations of the seq, ack, and ack-bitfield
		PacketSequence& sequenceLocation = *(PacketSequence*)segmentLocation;
		PacketSequence& ackLocation = *(PacketSequence*)(segmentLocation + sizeof(sequenceLocation));
		AckBitField& bitFieldLocation = *(AckBitField*)(segmentLocation +
			sizeof(sequenceLocation) + sizeof(ackLocation));

		// Get this packet's sequence number
		PacketSequence returnSequence{ m_LocalSequence };

		// Insert the sequence number (appID...|[sequenceNum]|ackNum|ackBitField|...)
		InsertLocalSequenceNumber(sequenceLocation);

		// Insert the recent ack number (appID...|sequenceNum|[ackNum]|ackBitField|...) 
		InsertRemoteSequenceNumber(ackLocation);

		// Insert the recent ack's bitfield (appID...|sequenceNum|ackNum|[ackBitField]|...)
		InsertRemoteSequenceBitField(bitFieldLocation);
	}

	bool ReliabilityContext::ProcessReliabilitySegmentFromPacket(uint8_t* segmentLocation)
	{

		// Initialize the locations of the seq, ack, and ack-bitfield
		PacketSequence packetSequence = *(PacketSequence*)segmentLocation;
		PacketSequence packetAck = *(PacketSequence*)(segmentLocation + sizeof(packetSequence));
		AckBitField packetAckBitfield = *(AckBitField*)(segmentLocation +
			sizeof(packetSequence) + sizeof(packetAck));

		// Update the remote data based on the received sequence number
		if (!ProcessReceivedSequenceNumber(packetSequence))
		{
			return false;
		}

		// Packet received successfully
		m_LastPacketReceived = 0.0f;

		// Check the ack context
		ProcessReceivedAck(packetAck, packetAckBitfield);

		return true;
	}

	void ReliabilityContext::InsertLocalSequenceNumber(PacketSequence& sequenceLocation)
	{
		// Insert the current local sequence number the into packet location
		sequenceLocation = m_LocalSequence;

		constexpr uint8_t k_AckFieldFinalElement{ sizeof(AckBitField) * 8 - 1 };

		// Check for drop packet at the 32 bit boundary of the bitfield!
		if (!m_LocalAckField.IsFlagSet(k_AckFieldFinalElement))
		{
			PacketSequence droppedPacketSeq = sequenceLocation - k_AckFieldFinalElement;
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

	void ReliabilityContext::InsertRemoteSequenceNumber(PacketSequence& ackLocation)
	{
		// Insert the current remote sequence number into the packet
		ackLocation = m_RemoteSequence;
	}

	void ReliabilityContext::InsertRemoteSequenceBitField(AckBitField& bitFieldLocation)
	{
		// Insert the bitfield
		bitFieldLocation = (AckBitField)m_RemoteAckField.GetRawBitfield(); // Truncate the bitfield
	}

	bool ReliabilityContext::ProcessReceivedSequenceNumber(PacketSequence receivedSequenceNumber)
	{
		PacketSequence distance = (PacketSequence)std::abs((int)receivedSequenceNumber - (int)m_RemoteSequence);

		constexpr PacketSequence k_EndAckFieldDistance{ sizeof(AckBitField) * 8 - 1 };

		bool excessiveDistance = distance > k_EndAckFieldDistance;

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

	bool ReliabilityContext::ProcessReceivedAck(PacketSequence ackNumber, AckBitField ackBitField)
	{
		PacketSequence distance = std::abs((int32_t)m_LocalSequence - (int32_t)ackNumber);
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
		AckBitField newlyAcknowledgedField = (~m_LocalAckField.GetRawBitfield()) & ackBitField;

		// Scan the newly-acknowledged-field and acknowledge the packets
		size_t i{ 0 };
		for (AckBitField field = newlyAcknowledgedField; field != 0; field &= field - 1)
		{
			// Get the next index
			AckBitField index = std::countr_zero(field);

			// Update round trip time
			PacketSequence ackPacketSeq = m_LocalSequence - 1 - index;
			float packetRTT = GetTime() - m_RoundTripContext.GetTimePoint(ackPacketSeq);
			ProcessRoundTrip(packetRTT);

			// Store recent ack
			m_RecentAcks[i].m_Sequence = ackPacketSeq;
			m_RecentAcks[i].m_RTT = packetRTT;

			i++;
		}

		m_RecentAckCount = i;

		// Finally, update the local bitfield with new acknowledgements
		m_LocalAckField.SetRawBitfield(m_LocalAckField.GetRawBitfield() | ackBitField);

		return newlyAcknowledgedField;
	}

	void ReliabilityContext::ProcessRoundTrip(float packetRoundTrip)
	{
		m_RoundTripContext.UpdateAverageRoundTrip(packetRoundTrip);

		m_CongestionContext.OnRoundTripChange(m_RoundTripContext.GetAverageRoundTrip());
	}


	void RoundTripContext::AddTimePoint(PacketSequence sequenceNumber)
	{
		m_SendTimepoints[sequenceNumber % k_AckBitFieldSize] = GetTime();
	}

	float RoundTripContext::GetTimePoint(PacketSequence sequenceNumber)
	{
		return m_SendTimepoints[sequenceNumber % k_AckBitFieldSize];
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

	void ReliabilityContextNotifiers::Init(std::atomic<bool>* contextActive)
	{
		KG_ASSERT(contextActive);

		i_ContextActive = contextActive;
	}

	ObserverIndex ReliabilityContextNotifiers::AddReliabilityStateObserver(std::function<void(ClientIndex, bool, float)> func)
	{
		KG_ASSERT(!i_ContextActive || !*i_ContextActive);

		return m_ReliabilityStateNotifier.AddObserver(func);
	}

	ObserverIndex ReliabilityContextNotifiers::AddSendPacketObserver(std::function<void(ClientIndex, PacketSequence)> func)
	{
		KG_ASSERT(!i_ContextActive || !*i_ContextActive);

		return m_SendPacketNotifier.AddObserver(func);
	}
	ObserverIndex ReliabilityContextNotifiers::AddAckPacketObserver(std::function<void(ClientIndex, PacketSequence, float)> func)
	{
		KG_ASSERT(!i_ContextActive || !*i_ContextActive);

		return m_AckPacketNotifier.AddObserver(func);
	}
}

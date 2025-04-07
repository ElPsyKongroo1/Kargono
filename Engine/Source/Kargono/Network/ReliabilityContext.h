#pragma once

#include "Kargono/Core/BitField.h"

#include <cstdint>
#include <array>

namespace Kargono::Network
{
	struct CongestionConfig
	{
		float m_CongestedRTTThreshold{ 250.0f / 1000.0f }; // Quarter of a second round-trip
		float m_DefaultResetCongestedTimeSec{ 10.0f }; // 10 seconds
		float m_MaxResetCongestedTimeSec{ 60.0f }; // 60 seconds
		float m_MinResetCongestedTimeSec{ 1.0f }; // 1 second
	};

	class CongestionContext
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		CongestionContext() = default;
		~CongestionContext() = default;
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		void OnUpdate(float deltaTime, float averageRoundTrip);
		void OnRoundTripChange(float averageRoundTrip);

		//==============================
		// Getters/Setters
		//==============================
		bool IsCongested();
	private:
		//==============================
		// Internal Fields
		//==============================
		CongestionConfig m_CongestionConfig{};
		bool m_IsCongested{ false };
		float m_TimeNotCongested{ 0.0f };
		float m_ResetCongestedTime{ 10.0f };
	};

	class RoundTripContext
	{
	public:
		//==============================
		// Interact with Timepoints
		//==============================
		void AddTimePoint(uint16_t sequenceNumber);
		float GetTimePoint(uint16_t sequenceNumber);

		//==============================
		// Interact with Average Round Trip
		//==============================
		void UpdateAverageRoundTrip(float roundTrip);
		float GetAverageRoundTrip();

	private:
		//==============================
		// Internal Fields
		//==============================
		float m_AverageRoundTrip{ 0.0f };
		std::array<float, 32> m_SendTimepoints;
	};

	class ReliabilityContext
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ReliabilityContext() = default;
		~ReliabilityContext() = default;

		//==============================
		// Lifecycle Functions
		//==============================
		void OnUpdate(float deltaTime);

		//==============================
		// Interact with Packet
		//==============================
		void InsertReliabilitySegmentIntoPacket(uint8_t* segmentLocation);
		void ProcessReliabilitySegmentFromPacket(uint8_t* segmentLocation);

	private:
		// Insert-segment helpers
		void InsertLocalSequenceNumber(uint16_t& sequenceLocation);
		void InsertRemoteSequenceNumber(uint16_t& ackLocation);
		void InsertRemoteSequenceBitField(uint32_t& bitFieldLocation);
		// Process-segment helpers
		bool ProcessReceivedSequenceNumber(uint16_t receivedSequenceNumber);
		bool ProcessReceivedAck(uint16_t ackNumber, uint32_t ackBitField);
	private:
		// Update state based on new round trip entry
		void ProcessRoundTrip(float packetRoundTrip);

	public:

		//==============================
		// External Fields
		//==============================
		float m_LastPacketReceived{ 0.0f };
		// Round trip calculation data
		RoundTripContext m_RoundTripContext{};
		// Congestion avoidance data
		CongestionContext m_CongestionContext{};

	private:
		//==============================
		// Internal Fields
		//==============================
		// Sequencing data
		uint16_t m_LocalSequence{ 0 };
		uint16_t m_RemoteSequence{ 0 };
		BitField<uint32_t> m_LocalAckField{ 0b1111'1111'1111'1111'1111'1111'1111'1111 };
		BitField<uint32_t> m_RemoteAckField{ 0b1111'1111'1111'1111'1111'1111'1111'1110 };
	};
}

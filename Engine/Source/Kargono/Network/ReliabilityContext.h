#pragma once

#include "Kargono/Core/BitField.h"
#include "Kargono/Network/NetworkCommon.h"
#include "Kargono/Core/Notifier.h"

#include <cstdint>
#include <array>
#include <span>

namespace Kargono::Network
{
	class ServerNetworkThread;
	class ClientNetworkThread;

	class ReliabilityContextNotifiers
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ReliabilityContextNotifiers() = default;
		~ReliabilityContextNotifiers() = default;

		//==============================
		// Lifecycle Functions
		//==============================
		void Init(std::atomic<bool>* contextActive);

	public:
		// Reliability state
		ObserverIndex AddReliabilityStateObserver(std::function<void(ClientIndex, bool, float)> func);
		// Packet sent observers
		ObserverIndex AddSendPacketObserver(std::function<void(ClientIndex, PacketSequence)> func);
		ObserverIndex AddAckPacketObserver(std::function<void(ClientIndex, PacketSequence, float)> func);
	private:
		//==============================
		// Internal Fields
		//==============================
		// General reliability state
		Notifier<ClientIndex, bool, float> m_ReliabilityStateNotifier{};
		// Packet notifiers
		Notifier<ClientIndex, PacketSequence> m_SendPacketNotifier{};
		Notifier<ClientIndex, PacketSequence, float> m_AckPacketNotifier{};

		//==============================
		// Injected Dependencies
		//==============================
		std::atomic<bool>* i_ContextActive{ nullptr };

	private:
		friend class ServerNetworkThread;
		friend class ClientNetworkThread;
	};


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
		void AddTimePoint(PacketSequence sequenceNumber);
		float GetTimePoint(PacketSequence sequenceNumber);

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
		std::array<float, (size_t)k_AckBitFieldSize> m_SendTimepoints;
	};

	struct AckData
	{
		PacketSequence m_Sequence{ 0 };
		float m_RTT{ 0.0f };
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
		bool ProcessReliabilitySegmentFromPacket(uint8_t* segmentLocation);

	private:
		// Insert-segment helpers
		void InsertLocalSequenceNumber(PacketSequence& sequenceLocation);
		void InsertRemoteSequenceNumber(PacketSequence& ackLocation);
		void InsertRemoteSequenceBitField(AckBitField& bitFieldLocation);
		// Process-segment helpers
		bool ProcessReceivedSequenceNumber(PacketSequence receivedSequenceNumber);
		bool ProcessReceivedAck(PacketSequence ackNumber, AckBitField ackBitField);
	private:
		// Update state based on new round trip entry
		void ProcessRoundTrip(float packetRoundTrip);

	public:
		//==============================
		// Getters/Setters
		//==============================
		std::span<AckData> GetRecentAcks()
		{
			KG_ASSERT(m_RecentAckCount <= m_RecentAcks.size());

			return { m_RecentAcks.data(), m_RecentAckCount };
		}

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
		PacketSequence m_LocalSequence{ 0 };
		PacketSequence m_RemoteSequence{ 0 };
		BitField<AckBitField> m_LocalAckField{ 0b1111'1111'1111'1111'1111'1111'1111'1111 };
		BitField<AckBitField> m_RemoteAckField{ 0b1111'1111'1111'1111'1111'1111'1111'1110 };
		std::array<AckData, k_AckBitFieldSize> m_RecentAcks{};
		size_t m_RecentAckCount{ 0 };
	};
}

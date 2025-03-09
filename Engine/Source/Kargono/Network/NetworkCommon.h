#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/DataStructures.h"
#include "Kargono/Math/MathAliases.h"

#include "API/Network/AsioAPI.h"

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
	enum class MessageType : uint32_t
	{
		// Manage client <-> server connnection
		ManageConnection_AcceptConnection = 0,
		ManageConnection_DenyConnection,
		ManageConnection_CheckUDPConnection,
		ManageConnection_KeepAlive,
		ManageConnection_ServerPing,

		// Generic messaging API
		GenericMessage_MessageAllClients,
		GenericMessage_ServerMessage,
		GenericMessage_ClientChat,
		GenericMessage_ServerChat,

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
		ManageSession_SyncPing,
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

	class ServerTCPConnection;

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

	//============================================================
	// Owned Message Struct
	//============================================================
	struct OwnedMessage
	{
		ServerTCPConnection* m_RemoteConnection{ nullptr };
		Message m_Message;
	};

	struct LabeledMessage
	{
		asio::ip::udp::endpoint& m_OutgoingEndpoint;
		Message m_Message;
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
		msg.m_Header.m_PayloadSize = static_cast<uint32_t>(msg.m_PayloadData.size());

		return msg;
	}

	template<typename DataType>
	Message& operator>>(Message& msg, const DataType& data)
	{
		// Check that the type of the data being pushed is trivially copyable
		static_assert(std::is_standard_layout<DataType>::value, "Data is too complex");

		// Cache current size of vector, as this will be the point we insert the data
		size_t bufferSizeAfterRemoval = msg.m_PayloadData.size() - sizeof(DataType);

		// Copy the data over
		std::memcpy((void*)&data, msg.m_PayloadData.data() + bufferSizeAfterRemoval, sizeof(DataType));

		// Shrink the vector to remove the read bytes
		msg.m_PayloadData.resize(bufferSizeAfterRemoval);

		// Update message header size
		msg.m_Header.m_PayloadSize = static_cast<uint32_t>(msg.GetEntireMessageSize());

		return msg;
	}

	struct NetworkContext
	{
		//==============================
		// Fields
		//==============================
		// Asio context and accompanying thread
		asio::io_context m_AsioContext;
		std::thread m_AsioThread;
		// Network thread and supporting mutex/condition_variable
		Ref<std::thread> m_NetworkThread { nullptr };
		std::condition_variable m_BlockNetworkThreadCondVar {};
		std::mutex m_BlockNetworkThreadMutex {};
		std::atomic<bool> m_QuitNetworkThread { false };
		// Singular incoming message queue
		TSQueue<OwnedMessage> m_IncomingMessageQueue;

		//==============================
		// Manage Network Thread
		//==============================
		void NetworkThreadSleep();
		void NetworkThreadWakeUp();
	};

	constexpr uint64_t k_KeepAliveDelay{ 10'000 };
	constexpr uint16_t k_MaxSyncPings = 10;
	constexpr uint16_t k_InvalidSessionSlot = std::numeric_limits<uint16_t>::max();
	constexpr size_t k_MaxMessageCount = std::numeric_limits<size_t>::max();
	// TODO: VERY TEMPORARY. Only for pong!!!!
	constexpr uint32_t k_MaxSessionClients {2};

	enum class ServerLocation
	{
		None = 0,
		LocalMachine,
		LocalNetwork,
		Remote
	};

	struct ServerConfig
	{
		Math::u8vec4 m_IPv4{0};
		uint16_t m_Port{ 101 };
		ServerLocation m_ServerLocation{ServerLocation::LocalMachine};
		Math::u64vec4 m_ValidationSecrets{0};
	};
}

namespace Kargono::Utility
{
	inline const char* ServerLocationToString(Network::ServerLocation type)
	{
		switch (type)
		{
		case Network::ServerLocation::LocalMachine: return "LocalMachine";
		case Network::ServerLocation::LocalNetwork: return "LocalNetwork";
		case Network::ServerLocation::Remote: return "Remote";
		case Network::ServerLocation::None: return "None";
		}
		KG_ERROR("Unknown type of server-location enum");
		return "";
	}

	inline Network::ServerLocation StringToServerLocation(std::string_view type)
	{
		if (type == "LocalMachine") { return Network::ServerLocation::LocalMachine; }
		if (type == "LocalNetwork") { return Network::ServerLocation::LocalNetwork; }
		if (type == "Remote") { return Network::ServerLocation::Remote; }
		if (type == "None") { return Network::ServerLocation::None; }

		KG_ERROR("Unknown type of server-location string");
		return Network::ServerLocation::None;
	}

	inline std::string IPv4ToString(Math::u8vec4 ip)
	{
		std::string returnString(16, '\0'); 
		int length = std::snprintf(returnString.data(), returnString.capacity(), "%u.%u.%u.%u",
			ip.x, ip.y, ip.z, ip.w);

		returnString.resize(length);
		return returnString;
	}
}



#pragma once
#include "Kargono/Network/NetworkCommon.h"
#include "Kargono/Network/Address.h"

namespace Kargono::Network
{
	enum class ServerLocation
	{
		None = 0,
		LocalMachine,
		LocalNetwork,
		Remote
	};

	struct ServerConfig
	{
		// Identification
		AppID m_AppProtocolID{ 0 };
		Address m_ServerAddress{};
		ServerLocation m_ServerLocation{ ServerLocation::LocalMachine };
		// Operation
		float m_ConnectionTimeout{ 10.0f };
		float m_SyncPingFrequency{ 0.05f };
		size_t m_SyncPingFrequencyMs{ 50 }; // TODO: Please remove this
		float m_RequestConnectionFrequency{ 1.0f };
		// Validation
		Math::u64vec4 m_ValidationSecrets{ 0 };
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

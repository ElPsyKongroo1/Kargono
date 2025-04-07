#pragma once
#include "Kargono/Network/NetworkCommon.h"
#include "Kargono/Network/Address.h"

namespace Kargono::Network
{
	struct NetworkConfig
	{
		AppID m_AppProtocolID{ 0 };
		Address m_ServerAddress{};
		float m_ConnectionTimeout{ 10.0f };
		float m_SyncPingFrequency{ 0.05f };
		float m_RequestConnectionFrequency{ 1.0f };
	};
}

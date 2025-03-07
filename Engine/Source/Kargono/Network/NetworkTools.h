#pragma once

#include "Kargono/Network/NetworkCommon.h"
#include "Kargono/Math/MathAliases.h"

#include <string>
#include <cstdint>

namespace Kargono::Network
{
	class NetworkTools
	{
	public:
		//==============================
		// Manage Config File
		//==============================
		static std::string CreateServerVariablesConfigFile(const Network::ServerConfig& config);
	};
}

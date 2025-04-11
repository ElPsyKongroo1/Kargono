#pragma once

#include "Kargono/Math/MathAliases.h"
#include "Kargono/Network/ServerConfig.h"

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

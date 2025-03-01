#pragma once

#include "Kargono/Math/MathAliases.h"

#include <string>
#include <cstdint>

namespace Kargono::Network
{
	class NetworkTools
	{
		static std::string CreateServerVariablesConfigFile
		(
			Math::u8vec4 serverIP, 
			uint16_t portNumber, 
			const char* serverLocation, 
			const Math::u64vec4& secrets
		);
	};
}

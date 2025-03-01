#include "kgpch.h"
#include "NetworkTools.h"

namespace Kargono::Network
{
	std::string Kargono::Network::NetworkTools::CreateServerVariablesConfigFile(const Network::ServerConfig& config)
	{
		// Intialize string stream
		std::stringstream outputStream;

		// Save the original formatting flags
		std::ios::fmtflags originalFlags = outputStream.flags();

		// Add general tips/description of the file
		const char* header =
			R"(# Creating your own server requires port forwarding.This includes
# disabling any firewall rules on the port that you select and
# routing a port on your router to your machine's local IP address.
# If you are on Windows and you are not sure what firewalls you have
# check your program and features tab.You likely only need to modify
# Windows Defender.I'll leave some links that should help:
# Port Forwarding : https://www.lifewire.com/how-to-port-forward-4163829
# Windows Defender : https://optics.ansys.com/hc/en-us/articles/7144748040467-Adding-inbound-rules-to-Windows-defender-firewall
# Also, make sure to open inbound and outbound rules for the port on TCP / UDP(Both).)";
		outputStream << header << '\n';
		outputStream << "ServerVariables:" << '\n';

		// Server ip
		outputStream << "  # This is your public gateway ip address. This can be found in the public" << '\n';
		outputStream << "  # info section on your router's portal. This should be a IPV4 address." << '\n';
		outputStream << "  # ##.##.##.##" << '\n';
		outputStream << "  ServerIP: " << '[' <<
			std::to_string(config.m_IPv4.x) << ',' <<
			std::to_string(config.m_IPv4.y) << ',' <<
			std::to_string(config.m_IPv4.z) << ',' <<
			std::to_string(config.m_IPv4.w) << ']' <<
			'\n';

		// Server port number
		outputStream << "  # Choose any port.I recommend something larger than 10, 000 but must be less than 63655" << '\n';
		outputStream << "  ServerPort: " << std::to_string(config.m_Port) << '\n';

		// Server location
		outputStream << "  # This variable decides whether to use your local machine or the ServerIP you specified earlier." << '\n';
		outputStream << "  # \"LocalMachine\" only allows clients to connect on the same computer. \"Internet\" allows online connections" << '\n';
		outputStream << "  ServerLocation: " << Utility::ServerLocationToString(config.m_ServerLocation) << '\n';

		// Client validation secrets
		outputStream << "  # The secrets can be any number you want that is a 64 bit unsigned integer.These are used for basic security." << '\n';

		// Set hex and uppercase
		outputStream.setf(std::ios::hex, std::ios::basefield);
		outputStream.setf(std::ios::uppercase);

		// Write out client validation secrets
		outputStream << "  SecretOne: " << "0x" << config.m_ValidationSecrets.x << '\n';
		outputStream << "  SecretTwo: " << "0x" << config.m_ValidationSecrets.y << '\n';
		outputStream << "  SecretThree: " << "0x" << config.m_ValidationSecrets.z << '\n';
		outputStream << "  SecretFour: " << "0x" << config.m_ValidationSecrets.w << '\n';

		// Revert to original formatting
		outputStream.flags(originalFlags);

		// Return the file
		return outputStream.str();
	}
}

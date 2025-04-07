#include "kgpch.h"

#include "Kargono/Network/Socket.h"

#include "stdio.h"

namespace Kargono::Network
{
	bool Socket::Open(unsigned short m_Port)
	{
		// Create the UDP socket
		m_Handle = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		// Ensure the socket is created
		if (m_Handle == -1)
		{
			// TODO: This call is windows specific
			KG_WARN("Failed to create a socket: {}", WSAGetLastError());
			return false;
		}

		// Create the socket's address
		sockaddr_in m_Address;
		m_Address.sin_family = AF_INET;
		m_Address.sin_addr.s_addr = INADDR_ANY;
		m_Address.sin_port = htons((unsigned short)m_Port);

		// Bind the address to the socket
		if (bind(m_Handle, (const sockaddr*)&m_Address, sizeof(sockaddr_in)) < 0)
		{
			KG_WARN("Failed to bind socket");
			return false;
		}

		// Set socket to non-blocking mode
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
		int nonBlocking = 1;
		if (fcntl(m_Handle, F_SETFL, O_NONBLOCK, nonBlocking) == -1)
		{
			KG_WARN("Failed to set non-blocking");
			return false;
		}
#elif PLATFORM == PLATFORM_WINDOWS
		DWORD nonBlocking = 1;
		if (ioctlsocket(m_Handle, FIONBIO, &nonBlocking) != 0)
		{
			KG_WARN("Failed to set non-blocking");
			return false;
		}
#endif

		return true;
	}

	void Socket::Close()
	{
		// Destroy a socket
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
		close(m_Handle);
#elif PLATFORM == PLATFORM_WINDOWS
		closesocket(m_Handle);
#endif
	}

	bool Socket::IsOpen() const
	{
		KG_WARN("IsOpen function is unimplemented");
		return false;
	}

	int Socket::GetHandle() const
	{
		return m_Handle;
	}

	bool Socket::Send(const Address& destination, const void* data, int size)
	{
		// Creating destination address
		sockaddr_in destAddress;
		destAddress.sin_family = AF_INET;
		destAddress.sin_addr.s_addr = htonl(destination.GetAddress());
		destAddress.sin_port = htons(destination.GetPort());

		// Sending a packet to a specific address
		int sent_bytes = sendto(m_Handle, (const char*)data, size, 0, (sockaddr*)&destAddress, sizeof(sockaddr_in));

		// Note that the return value only indicated whether the packet was sent successfully (not necessarily received)
		if (sent_bytes != size)
		{
			KG_WARN("Failed to send packet");
			return false;
		}
		return true;
	}

	int Socket::Receive(Address& sender, void* data, int size)
	{

#if PLATFORM == PLATFORM_WINDOWS
		typedef int socklen_t;
#endif

		sockaddr_in from;
		socklen_t fromLength = sizeof(from);
		// Note that any packets larger than the max size are silently discarded!
		int bytes = recvfrom(m_Handle, (char*)data, size, 0, (sockaddr*)&from, &fromLength);

		if (bytes <= 0)
		{
			return 0;
		}

		// Modify the sender's address and port
		sender.SetAddress(ntohl(from.sin_addr.s_addr));
		sender.SetNewPort(ntohs(from.sin_port));
		return bytes;
	}

	bool SocketContext::InitializeSockets()
	{
#if PLATFORM == PLATFORM_WINDOWS
		WSADATA WsaData;
		return WSAStartup(MAKEWORD(2, 2), &WsaData) == NO_ERROR;
#else
		return true;
#endif
	}

	void SocketContext::ShutdownSockets()
	{
#if PLATFORM == PLATFORM_WINDOWS
		WSACleanup();
#endif
	}
}

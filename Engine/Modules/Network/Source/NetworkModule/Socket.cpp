#include "kgpch.h"

#include "NetworkModule/Socket.h"

#include "stdio.h"

namespace Kargono::Network
{
	static SocketErrorCode GetSocketError(int windowsError)
	{
		switch (windowsError)
		{
		case 0:
			return SocketErrorCode::None;
		case 10'048:
			return SocketErrorCode::AddressInUse;
		default:
			return SocketErrorCode::OtherFailure;
		}
	}

	SocketErrorCode Socket::Open(unsigned short m_Port)
	{
		// Create the UDP socket
		m_Handle = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		// Ensure the socket is created
		if (m_Handle == -1)
		{
			// TODO: This call is windows specific
			KG_WARN("Failed to create a socket: {}", WSAGetLastError());
			return GetSocketError(WSAGetLastError());
		}

		// Create the socket's address
		sockaddr_in m_Address;
		m_Address.sin_family = AF_INET;
		m_Address.sin_addr.s_addr = INADDR_ANY;
		m_Address.sin_port = htons((unsigned short)m_Port);

		// Bind the address to the socket
		if (bind(m_Handle, (const sockaddr*)&m_Address, sizeof(sockaddr_in)) < 0)
		{
			KG_WARN("Failed to bind socket {}", WSAGetLastError());
			return GetSocketError(WSAGetLastError());
		}

		// Set socket to non-blocking mode
#if defined(KG_PLATFORM_LINUX) || defined(KG_PLATFORM_MAC)
		int nonBlocking = 1;
		if (fcntl(m_Handle, F_SETFL, O_NONBLOCK, nonBlocking) == -1)
		{
			KG_WARN("Failed to set non-blocking {}", WSAGetLastError());
			return GetSocketError(WSAGetLastError());
		}
#elif defined(KG_PLATFORM_WINDOWS)
		DWORD nonBlocking = 1;
		if (ioctlsocket(m_Handle, FIONBIO, &nonBlocking) != 0)
		{
			KG_WARN("Failed to set non-blocking {}", WSAGetLastError());
			return GetSocketError(WSAGetLastError());
		}
#endif

		return SocketErrorCode::None;
	}

	void Socket::Close()
	{
		// Destroy a socket
#if defined(KG_PLATFORM_LINUX) || defined(KG_PLATFORM_MAC)
		close(m_Handle);
#elif defined(KG_PLATFORM_WINDOWS)
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

#if defined(KG_PLATFORM_WINDOWS)
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
		if (s_SocketsUsageCount > 0)
		{
			s_SocketsUsageCount++;
			return true;
		}

		bool success{ false };
#if defined(KG_PLATFORM_WINDOWS)
		WSADATA WsaData;
		success = WSAStartup(MAKEWORD(2, 2), &WsaData) == NO_ERROR;
#else
		success = true;
#endif
		if (success)
		{
			s_SocketsUsageCount++;
		}
		return success;
	}

	void SocketContext::ShutdownSockets()
	{
		if (s_SocketsUsageCount > 0)
		{
			s_SocketsUsageCount--;
		}

		if (s_SocketsUsageCount > 0)
		{
			return;
		}

		#if defined(KG_PLATFORM_WINDOWS)
		WSACleanup();
		#endif
	}
}

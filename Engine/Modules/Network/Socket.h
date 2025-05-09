#pragma once
#include "Modules/Network/Platform/PosixSocket.h"
#include "Modules/Network/Address.h"

namespace Kargono::Network
{
	enum class SocketErrorCode
	{
		None = 0,
		AddressInUse,
		OtherFailure,
	};

	class Socket
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Socket() = default;
		~Socket() = default;

		//==============================
		// Lifecycle Functions
		//==============================
		SocketErrorCode Open(unsigned short m_Port);
		void Close();

		//==============================
		// Send/Receive Messages
		//==============================
		bool Send(const Address& destination, const void* data, int size);
		int Receive(Address& sender, void* data, int size);

		//==============================
		// Query Socket State
		//==============================
		bool IsOpen() const;

		//==============================
		// Getters/Setters
		//==============================
		int GetHandle() const;
	private:
		//==============================
		// Internal Fields
		//==============================
		int m_Handle{0};
	};

	//===========================
	// Socket Context
	//===========================

	class SocketContext
	{
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		static bool InitializeSockets();
		static void ShutdownSockets();

	private:
		inline static size_t s_SocketsUsageCount{ 0 };
	};
}

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

	using SocketsReferenceCount = size_t;

	class SocketContext
	{
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		[[nodiscard]] bool AddUsage();
		[[nodiscard]] bool RemoveUsage();

	private:
		SocketsReferenceCount m_SocketsReferenceCount{ 0 };
	};

	class SocketService
	{
	public:
		static SocketContext& GetActiveContext()
		{
			return s_SocketContext;
		}
	private:
		inline static SocketContext s_SocketContext{};
	};
}

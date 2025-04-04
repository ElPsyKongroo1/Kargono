#pragma once

#include "Socket.h"
#include "NetworkConfig.h"
#include "Connection.h"

#include "Kargono/Utility/LoopTimer.h"
#include "Kargono/Utility/PassiveLoopTimer.h"
#include "Kargono/Core/Thread.h"
#include "Kargono/Events/EventQueue.h"
#include "Kargono/Events/KeyEvent.h"

namespace Kargono::Network
{
	enum ConnectionStatus : uint8_t
	{
		Disconnected,
		Connecting,
		Connected
	};

	class ConnectionToServer
	{
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		void Init(const NetworkConfig& config);
		void Terminate();
	public:
		//==============================
		// Public Fields
		//==============================
		Connection m_Connection{};
		ClientIndex m_ClientIndex{};
		ConnectionStatus m_Status{ Disconnected };
	};

	class Client2
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Client2() = default;
		~Client2() = default;

		//==============================
		// Lifecycle Functions
		//==============================
		bool InitClient(const NetworkConfig& initConfig);
		bool TerminateClient();

		// Allows other threads to wait on the client to close
		void WaitOnClientTerminate();

		//==============================
		// On Event
		//==============================
		void OnEvent(Events::Event* event);

		//==============================
		// Manage Events
		//==============================
		void SubmitEvent(Ref<Events::Event> event);
	private:
		// Manage the server connection
		void RequestConnection();
	public:
		//==============================
		// Run Threads
		//==============================
		// Run socket/packet handling thread
		void RunNetworkThread();
		void RunNetworkEventThread();

	private:
		// Helper functions
		bool HandleConsoleInput(Events::KeyPressedEvent event);
	public:
		//==============================
		// Send Packets
		//==============================
		bool SendToServer(PacketType type, const void* payload, int payloadSize);
	private:
		//==============================
		// Internal Data
		//==============================
		Socket m_ClientSocket;
		KGThread m_NetworkThread;
		KGThread m_NetworkEventThread;
		NetworkConfig m_Config;
		Utility::LoopTimer m_NetworkThreadTimer;
		Utility::PassiveLoopTimer m_RequestConnectionTimer;
		Utility::PassiveLoopTimer m_KeepAliveTimer;
		Events::EventQueue m_NetworkEventQueue;

		// Server connection
		ConnectionToServer m_ServerConnection;

	};
}

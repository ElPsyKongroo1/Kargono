#pragma once

#include "Socket.h"
#include "Connection.h"
#include "NetworkConfig.h"

#include "Kargono/Utility/PassiveLoopTimer.h"
#include "Kargono/Utility/LoopTimer.h"
#include "Kargono/Core/Thread.h"
#include "Kargono/Events/EventQueue.h"
#include "Kargono/Events/KeyEvent.h"


namespace Kargono::Network
{
	class Server2
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Server2() = default;
		~Server2() = default;

		//==============================
		// Lifecycle Functions
		//==============================
		bool InitServer(const NetworkConfig& initConfig);
		bool TerminateServer(bool withinNetworkThread = false);

		// Allows other threads to wait on the server to close
		void WaitOnServerTerminate();
	public:
		//==============================
		// Run Threads
		//==============================
		// Run socket/packet handling thread
		void RunNetworkThread();
		void RunNetworkEventThread();

	private:
		// Helper functions
		bool ManageConnections();
		void HandleConsoleInput(Events::KeyPressedEvent event);

	public:
		//==============================
		// On Event
		//==============================
		void OnEvent(Events::Event* event);

		//==============================
		// Manage Events
		//==============================
		void SubmitEvent(Ref<Events::Event> event);

		//==============================
		// Send Packets
		//==============================
		bool SendToConnection(ClientIndex clientIndex, PacketType type, const void* data, int size);
		bool SendToAllConnections(PacketType type, const void* data, int size);
	private:
		//==============================
		// Internal Data
		//==============================
		bool m_ManageConnections{ false };
		Socket m_ServerSocket;
		NetworkConfig m_Config;
		KGThread m_NetworkThread;
		KGThread m_NetworkEventThread;
		Utility::LoopTimer m_ManageConnectionTimer;
		Utility::PassiveLoopTimer m_KeepAliveTimer;
		ConnectionList m_AllConnections;
		Events::EventQueue m_NetworkEventQueue;
	};
}

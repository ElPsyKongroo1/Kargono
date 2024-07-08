#pragma once

#include "Kargono/Network/ConnectionToServer.h"
#include "Kargono/Events/NetworkingEvent.h"
#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Core/Base.h"

#include "API/Network/AsioAPI.h"

#include <string>
#include <atomic>
#include <thread>
#include <limits>
#include <cstdint>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace Kargono::Network
{
	class Client
	{
	public:
		Client() = default;
		~Client();
	public:
		//==============================
		// LifeCycle Functions
		//==============================

		// Connect to server with hostname/ip-address and port
		bool Connect(const std::string& host, const uint16_t port, bool remote = false);

		// Disconnect from the server
		void Disconnect();

		// Check if client is actually connected to a server
		bool IsConnected();

		void Update(size_t nMaxMessages = -1);

		// Send message to server
		void Send(const Message& msg);

		void SendUDP(Message& msg);

		void Wait();

		void WakeUpNetworkThread();
	public:
		//==============================
		// Getters
		//==============================

		TSQueue<owned_message>& Incoming() { return m_qMessagesIn; }
	public:
		void SendChat(const std::string& text);

		void OnEvent(Events::Event& e);

		bool OnRequestUserCount(Events::RequestUserCount event);

		bool OnStartSession(Events::StartSession event);

		bool OnConnectionTerminated(Events::ConnectionTerminated event);

		bool OnRequestJoinSession(Events::RequestJoinSession event);

		bool OnEnableReadyCheck(Events::EnableReadyCheck event);

		bool OnSessionReadyCheck(Events::SessionReadyCheck event);

		bool OnSendAllEntityLocation(Events::SendAllEntityLocation event);

		bool OnSignalAll(Events::SignalAll event);

		bool OnAppTickEvent(Events::AppTickEvent event);

		bool OnSendAllEntityPhysics(Events::SendAllEntityPhysics event);

		bool OnLeaveCurrentSession(Events::LeaveCurrentSession event);

		void RunClient();

		void StopClient();

		void SubmitToFunctionQueue(const std::function<void()>& function);

		void ExecuteFunctionQueue();

		void SubmitToEventQueue(Ref<Events::Event> e);

		void ProcessEventQueue();

		void OnMessage(Kargono::Network::Message& msg);

	public:
		uint16_t GetSessionSlot() const { return m_SessionSlot; }
		uint64_t GetSessionStartFrame() const { return m_SessionStartFrame; }
		void SetSessionStartFrame(uint64_t startFrame) { m_SessionStartFrame = startFrame; }

		static uint16_t GetActiveSessionSlot();
		static void SendAllEntityLocation(UUID entityID, Math::vec3 location);
		static void SendAllEntityPhysics(UUID entityID, Math::vec3 translation, Math::vec2 linearVelocity);

		static void EnableReadyCheck();
		static void SessionReadyCheck();
		static void RequestUserCount();
		static void RequestJoinSession();
		static void LeaveCurrentSession();
		static void SignalAll(uint16_t signal);
		static Ref<Network::Client> GetActiveClient() { return s_Client; }
		static void SetActiveClient(Ref<Network::Client> newClient) { s_Client = newClient; }

		static Ref<std::thread> GetActiveNetworkThread() { return s_NetworkThread; }
		static void SetActiveNetworkThread(Ref<std::thread> newThread) { s_NetworkThread = newThread; }

	private:
		std::atomic<bool> m_Quit = false;

		std::vector<std::function<void()>> m_FunctionQueue;
		std::mutex m_FunctionQueueMutex;

		std::vector<Ref<Events::Event>> m_EventQueue {};
		std::mutex m_EventQueueMutex {};
		uint64_t m_SessionStartFrame{ 0 };
		std::atomic<uint16_t> m_SessionSlot{std::numeric_limits<uint16_t>::max()};
	private:
		static Ref<Network::Client> s_Client;
		static Ref<std::thread> s_NetworkThread;

	private:
		// asio context handles the data transfer...
		asio::io_context m_context;
		// ... but needs a thread of its own to execute its word commands
		std::thread thrContext;

		std::atomic<bool> m_UDPConnectionSuccessful;

		// The client has a single instance of a "connection" object, which handles data transfer
		Ref<ConnectionToServer> m_connection;

		Ref<UDPClientConnection> m_UDPClient {nullptr};
	private:
		// This is the thread safe queue of incoming messages from the server
		TSQueue<owned_message> m_qMessagesIn;

		// Variables for sleeping thread until a notify command is received
		std::condition_variable m_BlockThreadCV {};
		std::mutex m_BlockThreadMx {};
	};

	
}



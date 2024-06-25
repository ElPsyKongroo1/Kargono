#pragma once

#include "Kargono/Network/Network.h"
#include "Kargono/Network/Common.h"
#include "Kargono/Events/NetworkingEvent.h"
#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Core/Base.h"

#include <string>
#include <atomic>
#include <thread>
#include <limits>


namespace Kargono::Network
{
	class Client : public Kargono::Network::ClientInterface
	{
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

		virtual void OnMessage(Kargono::Network::Message& msg) override;

	public:
		uint16_t GetSessionSlot() const { return m_SessionSlot; }
		uint64_t GetSessionStartFrame() const { return m_SessionStartFrame; }
		void SetSessionStartFrame(uint64_t startFrame) { m_SessionStartFrame = startFrame; }

		static uint16_t GetActiveSessionSlot();
		static void SendAllEntityLocation(UUID entityID, Math::vec3 location);
		static void EnableReadyCheck();
		static void SessionReadyCheck();
		static void RequestUserCount();
		static Ref<Network::Client> GetActiveClient() { return s_Client; }
		static void SetActiveClient(Ref<Network::Client> newClient) { s_Client = newClient; }

		static Ref<std::thread> GetActiveNetworkThread() { return s_NetworkThread; }
		static void SetActiveNetworkThread(Ref<std::thread> newThread) { s_NetworkThread = newThread; }

	private:
		std::atomic<bool> m_Quit = false;
		//std::atomic<uint32_t> m_UserCount = 0;

		std::vector<std::function<void()>> m_FunctionQueue;
		std::mutex m_FunctionQueueMutex;

		std::vector<Ref<Events::Event>> m_EventQueue {};
		std::mutex m_EventQueueMutex {};
		uint64_t m_SessionStartFrame{ 0 };
		std::atomic<uint16_t> m_SessionSlot{std::numeric_limits<uint16_t>::max()};
	private:
		static Ref<Network::Client> s_Client;
		static Ref<std::thread> s_NetworkThread;
	};

	
}



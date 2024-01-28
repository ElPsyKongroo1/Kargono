#pragma once
#include "Kargono/Network/Common.h"
#include "Kargono/Network/ServerInterface.h"
#include "Kargono/Network/Session.h"
#include "Kargono/Events/Event.h"
#include "Kargono/Events/NetworkingEvent.h"

#include "Kargono/Core/Base.h"

#include <thread>
#include <atomic>

namespace Kargono::Network
{
	class Server : public Kargono::Network::ServerInterface
	{
	public:
		Server(uint16_t nPort, bool isLocal = false);

		void RunServer();

		void SessionClock();

		void SubmitToEventQueue(Ref<Events::Event> e);


	protected:
		virtual bool OnClientConnect(std::shared_ptr<Kargono::Network::ConnectionToClient> client) override;

		virtual void OnClientDisconnect(std::shared_ptr<Kargono::Network::ConnectionToClient> client) override;

		virtual void OnMessage(std::shared_ptr<Kargono::Network::ConnectionToClient> client, Kargono::Network::Message& incomingMessage) override;
	private:
		void ProcessEventQueue();
		void OnEvent(Events::Event& e);
		bool OnStartSession(Events::StartSession event);
	public:
		static Ref<Network::Server> GetActiveServer() { return s_Server; }
		static void SetActiveServer(Ref<Network::Server> newServer) { s_Server = newServer; }
	private:
		// TODO: This is very temporary. Only support one session currently!
		Session m_OnlySession{};
		Scope<std::thread> m_TimingThread { nullptr };
		bool m_StopThread = false;
		std::atomic<uint64_t> m_UpdateCount{ 0 };

		std::vector<Ref<Events::Event>> m_EventQueue {};
		std::mutex m_EventQueueMutex {};

	private:
		static Ref<Network::Server> s_Server;
	};
}

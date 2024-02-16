#include "kgpch.h"

#include "Kargono/Core/Application.h"
#include "Kargono/Network/Client.h"
#include "Kargono/Projects/Project.h"

#include <string>
#include <stdlib.h>

#include "Kargono/Core/Timers.h"

namespace Kargono::Network
{
	Ref<Network::Client> Client::s_Client { nullptr };
	Ref<std::thread> Client::s_NetworkThread { nullptr };

	void Client::SendChat(const std::string& text)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = CustomMsgTypes::ClientChat;
		msg.PushBuffer((void*)text.data(), text.size());
		Send(msg);
	}

	void Client::OnEvent(Events::Event& e)
	{
		Events::EventDispatcher dispatcher(e);

		dispatcher.Dispatch<Events::RequestJoinSession>(KG_BIND_EVENT_FN(Client::OnRequestJoinSession));
		dispatcher.Dispatch<Events::RequestUserCount>(KG_BIND_EVENT_FN(Client::OnRequestUserCount));
		dispatcher.Dispatch<Events::LeaveCurrentSession>(KG_BIND_EVENT_FN(Client::OnLeaveCurrentSession));
		dispatcher.Dispatch<Events::StartSession>(KG_BIND_EVENT_FN(Client::OnStartSession));
		dispatcher.Dispatch<Events::ConnectionTerminated>(KG_BIND_EVENT_FN(Client::OnConnectionTerminated));
		dispatcher.Dispatch<Events::EnableReadyCheck>(KG_BIND_EVENT_FN(Client::OnEnableReadyCheck));
		dispatcher.Dispatch<Events::SessionReadyCheck>(KG_BIND_EVENT_FN(Client::OnSessionReadyCheck));
		dispatcher.Dispatch<Events::SendAllEntityLocation>(KG_BIND_EVENT_FN(Client::OnSendAllEntityLocation));
		dispatcher.Dispatch<Events::SendAllEntityPhysics>(KG_BIND_EVENT_FN(Client::OnSendAllEntityPhysics));
		dispatcher.Dispatch<Events::SignalAll>(KG_BIND_EVENT_FN(Client::OnSignalAll));
		dispatcher.Dispatch<Events::AppTickEvent>(KG_BIND_EVENT_FN(Client::OnAppTickEvent));

	}

	bool Client::OnRequestUserCount(Events::RequestUserCount event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = CustomMsgTypes::RequestUserCount;
		Send(msg);
		return true;
	}

	bool Client::OnStartSession(Events::StartSession event)
	{
		m_SessionStartFrame = Application::GetCurrentApp().GetUpdateCount();
		return true;
	}

	bool Client::OnConnectionTerminated(Events::ConnectionTerminated event)
	{
		m_SessionSlot = std::numeric_limits<uint16_t>::max();
		m_SessionStartFrame = 0;
		return true;
	}

	bool Client::OnRequestJoinSession(Events::RequestJoinSession event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = CustomMsgTypes::RequestJoinSession;
		Send(msg);
		return true;
	}

	bool Client::OnEnableReadyCheck(Events::EnableReadyCheck event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = CustomMsgTypes::EnableReadyCheck;
		Send(msg);
		return true;
	}

	bool Client::OnSessionReadyCheck(Events::SessionReadyCheck event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = CustomMsgTypes::SessionReadyCheck;
		Send(msg);
		return true;
	}

	bool Client::OnSendAllEntityLocation(Events::SendAllEntityLocation event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = CustomMsgTypes::SendAllEntityLocation;
		msg << event.GetEntityID();
		Math::vec3 translation = event.GetTranslation();
		msg << translation.x;
		msg << translation.y;
		msg << translation.z;
		SendUDP(msg);
		return true;
	}

	bool Client::OnSendAllEntityPhysics(Events::SendAllEntityPhysics event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = CustomMsgTypes::SendAllEntityPhysics;
		msg << event.GetEntityID();
		Math::vec3 translation = event.GetTranslation();
		Math::vec2 linearVelocity = event.GetLinearVelocity();
		msg << translation.x;
		msg << translation.y;
		msg << translation.z;
		msg << linearVelocity.x;
		msg << linearVelocity.y;
		SendUDP(msg);
		return true;
	}

	bool Client::OnSignalAll(Events::SignalAll event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = CustomMsgTypes::SignalAll;
		msg << event.GetSignal();
		Send(msg);
		return true;
	}

	bool Client::OnAppTickEvent(Events::AppTickEvent event)
	{
		if (event.GetDelayMilliseconds() == m_UDPClient->GetKeepAliveDelay())
		{
			KG_TRACE("Send Keep Alive From Client");
			Kargono::Network::Message msg;
			msg.Header.ID = CustomMsgTypes::KeepAlive;
			SendUDP(msg);
			return true;
		}
		return false;
	}

	bool Client::OnLeaveCurrentSession(Events::LeaveCurrentSession event)
	{
		Kargono::Network::Message msg;
		msg.Header.ID = CustomMsgTypes::LeaveCurrentSession;
		Send(msg);
		m_SessionSlot = std::numeric_limits<uint16_t>::max();
		return true;
	}

	void Client::RunClient()
	{
		auto currentProject = Projects::Project::GetActive();

		bool remoteConnection = false;

		if (currentProject->GetServerLocation() != "LocalMachine")
		{
			remoteConnection = true;
		}

		if (!Connect(currentProject->GetServerIP(), currentProject->GetServerPort(), remoteConnection)) { m_Quit = true; }

		if (m_UDPClient && IsConnected())
		{
			// Start timer for keep alive packets
			Application::GetCurrentApp().SubmitToEventQueue(CreateRef<Events::AddTickGeneratorUsage>(m_UDPClient->GetKeepAliveDelay()));
		}

		while (!m_Quit)
		{
			if (IsConnected())
			{
				ProcessEventQueue();
				ExecuteFunctionQueue();
				Update(5);
			}
			else
			{
				// Client is disconnected
				KG_WARN("Server Down");
				m_Quit = true;
			}
		}
		Disconnect();

		if (m_UDPClient)
		{
			Application::GetCurrentApp().SubmitToEventQueue(CreateRef<Events::RemoveTickGeneratorUsage>(m_UDPClient->GetKeepAliveDelay()));
		}
	}

	void Client::OnMessage(Kargono::Network::Message& msg)
	{
		switch (msg.Header.ID)
		{
			case CustomMsgTypes::AcceptConnection:
			{
				KG_INFO("[SERVER]: Connection has been accepted!");
				uint32_t userCount{};
				msg >> userCount;
				Application::GetCurrentApp().SubmitToEventQueue(CreateRef<Events::UpdateOnlineUsers>(userCount));
				break;
			}

			case CustomMsgTypes::UpdateUserCount:
			{
				uint32_t userCount{};
				msg >> userCount;
				Application::GetCurrentApp().SubmitToEventQueue(CreateRef<Events::UpdateOnlineUsers>(userCount));
				break;
			}

			case CustomMsgTypes::ServerChat:
			{
				uint32_t clientID{};
				std::vector<uint8_t> data{};
				uint64_t dataSize{};
				msg >> clientID;
				msg >> dataSize;
				data = msg.GetBuffer(dataSize);
				std::string text((char*)(data.data()), dataSize);
				KG_INFO("[{}]: {}", clientID, text);
				break;
			}

			case CustomMsgTypes::ApproveJoinSession:
			{
				KG_INFO("[SERVER]: Approved joining session");
				uint16_t userSlot{};
				msg >> userSlot;
				m_SessionSlot = userSlot;
				Application::GetCurrentApp().SubmitToEventQueue(CreateRef<Events::ApproveJoinSession>(userSlot));
				break;
			}

			case CustomMsgTypes::UpdateSessionUserSlot:
			{
				uint16_t userSlot{};
				msg >> userSlot;
				Application::GetCurrentApp().SubmitToEventQueue(CreateRef<Events::UpdateSessionUserSlot>(userSlot));
				KG_INFO("[SERVER]: Updated User Slot {}", userSlot);
				break;
			}

			case CustomMsgTypes::UserLeftSession:
			{
				KG_INFO("[SERVER]: A User Left the Current Session");
				uint16_t userSlot{};
				msg >> userSlot;
				Application::GetCurrentApp().SubmitToEventQueue(CreateRef<Events::UserLeftSession>(userSlot));
				break;
			}

			case CustomMsgTypes::DenyJoinSession:
			{
				KG_INFO("[SERVER]: Denied joining session");
				break;
			}

			case CustomMsgTypes::CurrentSessionInit:
			{
				KG_INFO("[SERVER]: Active Session is initializing...");
				Application::GetCurrentApp().SubmitToEventQueue(CreateRef<Events::CurrentSessionInit>());
				break;
			}

			case CustomMsgTypes::InitSyncPing:
			{
				Kargono::Network::Message newMessage;
				newMessage.Header.ID = CustomMsgTypes::InitSyncPing;
				Send(newMessage);
				break;
			}

			case CustomMsgTypes::StartSession:
			{
				float waitTime{};
				msg >> waitTime;
				Timers::AsyncBusyTimer::CreateTimer(waitTime, [&]()
				{
					// Note Starting Update
					Network::Client::GetActiveClient()->SubmitToEventQueue(CreateRef<Events::StartSession>());

					// Open Gameplay
					Application::GetCurrentApp().Application::SubmitToEventQueue(CreateRef<Events::StartSession>());
				});
				
				break;
			}

			case CustomMsgTypes::SessionReadyCheckConfirm:
			{
				float waitTime{};
				msg >> waitTime;
				Timers::AsyncBusyTimer::CreateTimer(waitTime, [&]()
				{
					// Open Gameplay
					Application::GetCurrentApp().Application::SubmitToEventQueue(CreateRef<Events::SessionReadyCheckConfirm>());
				});

				break;
			}

			case CustomMsgTypes::UpdateEntityLocation:
			{
				uint64_t id;
				float x, y, z;
				msg >> z;
				msg >> y;
				msg >> x;
				msg >> id;
				Math::vec3 trans{x, y, z};
				Application::GetCurrentApp().Application::SubmitToEventQueue(CreateRef<Events::UpdateEntityLocation>(id, trans));

				break;
			}

			case CustomMsgTypes::UpdateEntityPhysics:
			{
				uint64_t id;
				float x, y, z, linx, liny;

				msg >> liny;
				msg >> linx;
				msg >> z;
				msg >> y;
				msg >> x;
				msg >> id;
				Math::vec3 trans{x, y, z};
				Math::vec2 linearV{linx, liny};
				Application::GetCurrentApp().Application::SubmitToEventQueue(CreateRef<Events::UpdateEntityPhysics>(id, trans, linearV));

				break;
			}

			case CustomMsgTypes::ReceiveSignal:
			{
				uint16_t signal{};
				msg >> signal;
				Application::GetCurrentApp().Application::SubmitToEventQueue(CreateRef<Events::ReceiveSignal>(signal));
				break;
			}

			case CustomMsgTypes::KeepAlive:
			{
				break;
			}

			case CustomMsgTypes::UDPInit:
			{
				KG_TRACE("Received UDP Init Packet");
				m_UDPConnectionSuccessful = true;
				break;
			}

			default:
			{
				KG_ERROR("Invalid message type sent to client!");
				break;
			}
		}
	}

	void Client::StopClient()
	{
		m_Quit = true;
		WakeUpNetworkThread();
	}

	void Client::SubmitToFunctionQueue(const std::function<void()>& function)
	{
		std::scoped_lock<std::mutex> lock(m_FunctionQueueMutex);

		m_FunctionQueue.emplace_back(function);
	}

	void Client::ExecuteFunctionQueue()
	{
		KG_PROFILE_FUNCTION();
		std::scoped_lock<std::mutex> lock(m_FunctionQueueMutex);

		for (auto& func : m_FunctionQueue) { func(); }
		m_FunctionQueue.clear();
	}

	void Client::SubmitToEventQueue(Ref<Events::Event> e)
	{
		std::scoped_lock<std::mutex> lock(m_EventQueueMutex);

		m_EventQueue.emplace_back(e);

		// Alert thread to wake up and process event
		WakeUpNetworkThread();
	}

	void Client::ProcessEventQueue()
	{
		KG_PROFILE_FUNCTION();

		std::scoped_lock<std::mutex> lock(m_EventQueueMutex);

		for (auto& event : m_EventQueue)
		{
			OnEvent(*event);
		}
		m_EventQueue.clear();
	}
}



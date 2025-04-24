#pragma once

#include "EventsPlugin/Event.h"
#include "Kargono/Core/UUID.h"
#include "Kargono/Math/Math.h"

#include <sstream>
#include <limits>

namespace Kargono::Events
{
	//============================================================
	// Update Online Users Class
	//============================================================
	
	class ReceiveOnlineUsers : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		
		ReceiveOnlineUsers(uint32_t newUserCount)
			: m_UserCount(newUserCount) {}

		//==============================
		// Getters/Setters
		//==============================

		uint32_t GetUserCount() const { return m_UserCount; }

		virtual EventType GetEventType() const override { return EventType::ReceiveOnlineUsers; }
		virtual int GetCategoryFlags() const override { return EventCategory::Network; }
	private:
		uint32_t m_UserCount;
	};

	//============================================================
	// Signal All Event Class
	//============================================================

	class SignalAll : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================

		SignalAll(uint16_t signal)
			: m_Signal(signal) {}

		//==============================
		// Getters/Setters
		//==============================

		uint16_t GetSignal() const { return m_Signal; }

		virtual EventType GetEventType() const override { return EventType::SignalAll; }
		virtual int GetCategoryFlags() const override { return EventCategory::Network; }
	private:

		uint16_t m_Signal;
	};

	//============================================================
	// Receive Signal Event Class
	//============================================================

	class ReceiveSignal : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================

		ReceiveSignal(uint16_t signal)
			: m_Signal(signal) {}

		//==============================
		// Getters/Setters
		//==============================

		uint16_t GetSignal() const { return m_Signal; }

		virtual EventType GetEventType() const override { return EventType::ReceiveSignal; }
		virtual int GetCategoryFlags() const override { return EventCategory::Network; }
	private:
		uint16_t m_Signal;
	};

	//============================================================
	// Request Join Session Event Class
	//============================================================

	class RequestJoinSession : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================

		RequestJoinSession() = default;

		//==============================
		// Getters/Setters
		//==============================

		virtual EventType GetEventType() const override { return EventType::RequestJoinSession; }
		virtual int GetCategoryFlags() const override { return EventCategory::Network; }
	};

	//============================================================
	// Start Session Event Class
	//============================================================

	class StartSession : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================

		StartSession() = default;

		//==============================
		// Getters/Setters
		//==============================

		virtual EventType GetEventType() const override { return EventType::StartSession; }
		virtual int GetCategoryFlags() const override { return EventCategory::Network; }
	};

	//============================================================
	// Enable Ready Check Class
	//============================================================

	class EnableReadyCheck : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================

		EnableReadyCheck() = default;

		//==============================
		// Getters/Setters
		//==============================

		virtual EventType GetEventType() const override { return EventType::EnableReadyCheck; }
		virtual int GetCategoryFlags() const override { return EventCategory::Network; }
	};

	//============================================================
	// Send Ready Check Class
	//============================================================

	class SessionReadyCheck : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================

		SessionReadyCheck() = default;

		//==============================
		// Getters/Setters
		//==============================

		virtual EventType GetEventType() const override { return EventType::SendReadyCheck; }
		virtual int GetCategoryFlags() const override { return EventCategory::Network; }
	};

	//============================================================
	// Send Ready Check Class
	//============================================================

	class SessionReadyCheckConfirm : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================

		SessionReadyCheckConfirm() = default;

		//==============================
		// Getters/Setters
		//==============================

		virtual EventType GetEventType() const override { return EventType::SendReadyCheckConfirm; }
		virtual int GetCategoryFlags() const override { return EventCategory::Network; }
	};

	//============================================================
	// Connection Terminated Class
	//============================================================

	class ConnectionTerminated : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================

		ConnectionTerminated() = default;

		//==============================
		// Getters/Setters
		//==============================

		virtual EventType GetEventType() const override { return EventType::ConnectionTerminated; }
		virtual int GetCategoryFlags() const override { return EventCategory::Network; }
	};

	//============================================================
	// Leave Current Session Class
	//============================================================

	class LeaveCurrentSession : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================

		LeaveCurrentSession() = default;

		//==============================
		// Getters/Setters
		//==============================

		virtual EventType GetEventType() const override { return EventType::LeaveCurrentSession; }
		virtual int GetCategoryFlags() const override { return EventCategory::Network; }
	};

	//============================================================
	// User Left Session Class
	//============================================================

	class UserLeftSession : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================

		UserLeftSession(uint16_t newSlot)
			: m_UserSlot(newSlot) {}

		//==============================
		// Getters/Setters
		//==============================

		uint16_t GetUserSlot() const { return m_UserSlot; }

		virtual EventType GetEventType() const override { return EventType::UserLeftSession; }
		virtual int GetCategoryFlags() const override { return EventCategory::Network; }

	private:
		uint16_t m_UserSlot;
	};

	//============================================================
	// Request User Count Class
	//============================================================

	class RequestUserCount : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================

		RequestUserCount() = default;

		//==============================
		// Getters/Setters
		//==============================

		virtual EventType GetEventType() const override { return EventType::RequestUserCount; }
		virtual int GetCategoryFlags() const override { return EventCategory::Network; }
	};

	//============================================================
	// Current Session Initialization Class
	//============================================================

	class CurrentSessionInit : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================

		CurrentSessionInit() = default;

		//==============================
		// Getters/Setters
		//==============================

		virtual EventType GetEventType() const override { return EventType::CurrentSessionInit; }
		virtual int GetCategoryFlags() const override { return EventCategory::Network; }
	};

	//============================================================
	// Approve Join Session Class
	//============================================================

	class ApproveJoinSession : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================

		ApproveJoinSession(uint16_t newSlot)
			: m_UserSlot(newSlot) {}

		//==============================
		// Getters/Setters
		//==============================

		uint16_t GetUserSlot() const { return m_UserSlot; }

		virtual EventType GetEventType() const override { return EventType::ApproveJoinSession; }
		virtual int GetCategoryFlags() const override { return EventCategory::Network; }
	private:
		uint16_t m_UserSlot;
	};

	//============================================================
	// Update Session User Slot Class
	//============================================================

	class UpdateSessionUserSlot : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================

		UpdateSessionUserSlot(uint16_t newSlot)
			: m_UserSlot(newSlot) {}

		//==============================
		// Getters/Setters
		//==============================

		uint16_t GetUserSlot() const { return m_UserSlot; }

		virtual EventType GetEventType() const override { return EventType::UpdateSessionUserSlot; }
		virtual int GetCategoryFlags() const override { return EventCategory::Network; }
	private:
		uint16_t m_UserSlot;
	};

	//============================================================
	// Send All Entity Location Event Class
	//============================================================

	class SendAllEntityLocation : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================

		SendAllEntityLocation(uint64_t entityID, Math::vec3 translation)
			: m_EntityID(entityID), m_Translation(translation) {}

		//==============================
		// Getters/Setters
		//==============================

		uint64_t GetEntityID() const { return m_EntityID; }
		Math::vec3 GetTranslation() const { return m_Translation; }

		virtual EventType GetEventType() const override { return EventType::SendAllEntityLocation; }
		virtual int GetCategoryFlags() const override { return EventCategory::Network; }

	private:
		uint64_t m_EntityID;
		Math::vec3 m_Translation;
	};

	//============================================================
	// Update Entity Location Event Class
	//============================================================

	class UpdateEntityLocation : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================

		UpdateEntityLocation(uint64_t entityID, Math::vec3 translation)
			: m_EntityID(entityID), m_Translation(translation) {}

		//==============================
		// Getters/Setters
		//==============================

		uint64_t GetEntityID() const { return m_EntityID; }
		Math::vec3 GetTranslation() const { return m_Translation; }

		virtual EventType GetEventType() const override { return EventType::UpdateEntityLocation; }
		virtual int GetCategoryFlags() const override { return EventCategory::Network; }
	private:
		uint64_t m_EntityID;
		Math::vec3 m_Translation;
	};

	//============================================================
	// Send All Entity Physics Event Class
	//============================================================

	class SendAllEntityPhysics : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================

		SendAllEntityPhysics(uint64_t entityID, Math::vec3 translation, Math::vec2 linearVelocity)
			: m_EntityID(entityID), m_Translation(translation), m_LinearVelocity(linearVelocity) {}

		//==============================
		// Getters/Setters
		//==============================

		uint64_t GetEntityID() const { return m_EntityID; }
		Math::vec3 GetTranslation() const { return m_Translation; }
		Math::vec2 GetLinearVelocity() const { return m_LinearVelocity; }

		virtual EventType GetEventType() const override { return EventType::SendAllEntityPhysics; }
		virtual int GetCategoryFlags() const override { return EventCategory::Network; }
	private:
		uint64_t m_EntityID;
		Math::vec3 m_Translation;
		Math::vec2 m_LinearVelocity;
	};

	//============================================================
	// Update Entity Physics Event Class
	//============================================================

	class UpdateEntityPhysics : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================

		UpdateEntityPhysics(uint64_t entityID, Math::vec3 translation, Math::vec2 linearVelocity)
			: m_EntityID(entityID), m_Translation(translation), m_LinearVelocity(linearVelocity) {}

		//==============================
		// Getters/Setters
		//==============================

		uint64_t GetEntityID() const { return m_EntityID; }
		Math::vec3 GetTranslation() const { return m_Translation; }
		Math::vec2 GetLinearVelocity() const { return m_LinearVelocity; }

		virtual EventType GetEventType() const override { return EventType::UpdateEntityPhysics; }
		virtual int GetCategoryFlags() const override { return EventCategory::Network; }
	private:
		uint64_t m_EntityID;
		Math::vec3 m_Translation;
		Math::vec2 m_LinearVelocity;
	};

	//============================================================
	// Deny Join Session Class
	//============================================================

	class DenyJoinSession : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================

		DenyJoinSession() = default;

		//==============================
		// Getters/Setters
		//==============================

		virtual EventType GetEventType() const override { return EventType::DenyJoinSession; }
		virtual int GetCategoryFlags() const override { return EventCategory::Network; }

	};

}

#pragma once

#include "Kargono/Events/Event.h"
#include "Kargono/Core/UUID.h"
#include "Kargono/Math/Math.h"

#include <sstream>
#include <limits>

namespace Kargono::Events
{
	//============================================================
	// Update Online Users Class
	//============================================================
	
	class UpdateOnlineUsers : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		
		UpdateOnlineUsers(uint32_t newUserCount)
			: m_UserCount(newUserCount) {}

		//==============================
		// Getters/Setters
		//==============================

		uint32_t GetUserCount() const { return m_UserCount; }

		static EventType GetStaticType() { return EventType::UpdateOnlineUsers; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "UpdateOnlineUsers"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Networking; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "UpdateOnlineUsers: " << m_UserCount;
			return ss.str();
		}
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

		static EventType GetStaticType() { return EventType::SignalAll; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "SignalAll"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Networking; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "SignalAll: " << m_Signal;
			return ss.str();
		}
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

		static EventType GetStaticType() { return EventType::ReceiveSignal; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "ReceiveSignal"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Networking; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "ReceiveSignal: " << m_Signal;
			return ss.str();
		}
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

		static EventType GetStaticType() { return EventType::RequestJoinSession; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "RequestJoinSession"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Networking; }
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

		static EventType GetStaticType() { return EventType::StartSession; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "StartSession"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Networking; }
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

		static EventType GetStaticType() { return EventType::EnableReadyCheck; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "EnableReadyCheck"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Networking; }
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

		static EventType GetStaticType() { return EventType::SendReadyCheck; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "SendReadyCheck"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Networking; }
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

		static EventType GetStaticType() { return EventType::SendReadyCheckConfirm; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "SendReadyCheckConfirm"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Networking; }
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

		static EventType GetStaticType() { return EventType::ConnectionTerminated; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "ConnectionTerminated"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Networking; }
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

		static EventType GetStaticType() { return EventType::LeaveCurrentSession; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "LeaveCurrentSession"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Networking; }
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

		static EventType GetStaticType() { return EventType::UserLeftSession; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "UserLeftSession"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Networking; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "User Left Session " << m_UserSlot;
			return ss.str();
		}

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

		static EventType GetStaticType() { return EventType::RequestUserCount; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "RequestUserCount"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Networking; }
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

		static EventType GetStaticType() { return EventType::CurrentSessionInit; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "CurrentSessionInit"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Networking; }
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

		static EventType GetStaticType() { return EventType::ApproveJoinSession; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "ApproveJoinSession"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Networking; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "ApproveJoinSession " << m_UserSlot;
			return ss.str();
		}
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

		static EventType GetStaticType() { return EventType::UpdateSessionUserSlot; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "UpdateSessionUserSlot"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Networking; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "UpdateSessionUserSlot " << m_UserSlot;
			return ss.str();
		}
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

		static EventType GetStaticType() { return EventType::SendAllEntityLocation; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "SendAllEntityLocation"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Networking; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "SendAllEntityLocation " << m_EntityID;
			return ss.str();
		}
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

		static EventType GetStaticType() { return EventType::UpdateEntityLocation; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "UpdateEntityLocation"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Networking; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "UpdateEntityLocation " << m_EntityID;
			return ss.str();
		}
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

		static EventType GetStaticType() { return EventType::SendAllEntityPhysics; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "SendAllEntityPhysics"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Networking; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "SendAllEntityPhysics " << m_EntityID;
			return ss.str();
		}
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

		static EventType GetStaticType() { return EventType::UpdateEntityPhysics; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "UpdateEntityPhysics"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Networking; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "UpdateEntityPhysics " << m_EntityID;
			return ss.str();
		}
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

		static EventType GetStaticType() { return EventType::DenyJoinSession; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "DenyJoinSession"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Networking; }

	};

}

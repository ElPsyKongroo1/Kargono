#pragma once

#include "Kargono/Events/Event.h"
#include "Kargono/Core/UUID.h"

#include <sstream>

namespace Kargono::Events
{
	//============================================================
	// Window Resize Event Class
	//============================================================
	// This event class represents a Window Resize event that is thrown by
	//		the current windowing system. This event simply contains the new
	//		Width and Height which can be used to resize the viewport among
	//		other use-cases.
	class WindowResizeEvent : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This event is initialized with the new width and height of the window.
		WindowResizeEvent(unsigned int width, unsigned int height)
			: m_Width(width), m_Height(height) {}

		//==============================
		// Getters/Setters
		//==============================

		unsigned int GetWidth() const { return m_Width; }
		unsigned int GetHeight() const { return m_Height; }

		static EventType GetStaticType() { return EventType::WindowResize; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "WindowResizeEvent"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Application; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}
	private:
		// m_Width and m_Height represent the new width and height of the window
		//		after the resize has occurred on the Window.
		unsigned int m_Width, m_Height;
	};
	//============================================================
	// Window Close Event Class
	//============================================================
	// This event represents the situation where the current window is closed.
	//		This event is used to completely close both the application and
	//		the window.
	class WindowCloseEvent : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This event does not include any data. The constructor
		//		is simple.
		WindowCloseEvent() = default;

		//==============================
		// Getters/Setters
		//==============================
		static EventType GetStaticType() { return EventType::WindowClose; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "WindowCloseEvent"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Application; }
	};

	//============================================================
	// Application Close Event Class
	//============================================================
	// This event represents closing the runtime application. In the editor
	//		this simply calls OnStop(), however, in the runtime application
	//		it calls a WindowCloseEvent().
	class ApplicationCloseEvent : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This event does not include any data. The constructor
		//		is simple.
		ApplicationCloseEvent() = default;

		//==============================
		// Getters/Setters
		//==============================
		static EventType GetStaticType() { return EventType::AppClose; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "ApplicationCloseEvent"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Application; }
	};

	//============================================================
	// Clean Up Timers Event Class
	//============================================================

	class CleanUpTimersEvent : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		CleanUpTimersEvent() = default;

		//==============================
		// Getters/Setters
		//==============================
		static EventType GetStaticType() { return EventType::CleanUpTimers; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "CleanUpTimersEvent"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Application; }
	};

	//============================================================
	// Physics Collision Event Class
	//============================================================
	// This event represents a collision that occurs inside the Physics system.
	//		A collision occurs between two entities and the UUID of each entity
	//		is recorded. This information is used later in the event pipeline
	//		to handle concepts such as audio effects.
	class PhysicsCollisionEvent : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This event is initialized with the UUID of both entities involved
		//		in the collision.
		PhysicsCollisionEvent(UUID entityOne, UUID entityTwo)
			: m_EntityOne(entityOne), m_EntityTwo(entityTwo){}

		//==============================
		// Getters/Setters
		//==============================

		UUID GetEntityOne() const { return m_EntityOne; }
		UUID GetEntityTwo() const { return m_EntityTwo; }

		static EventType GetStaticType() { return EventType::PhysicsCollision; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "PhysicsCollisionEvent"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Application; }
	private:
		// These are the UUID's of both entities involved in the collision event.
		UUID m_EntityOne;
		UUID m_EntityTwo;
	};

	//============================================================
	// Physics Collision End Class
	//============================================================
	class PhysicsCollisionEnd : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This event is initialized with the UUID of both entities involved
		//		in the collision.
		PhysicsCollisionEnd(UUID entityOne, UUID entityTwo)
			: m_EntityOne(entityOne), m_EntityTwo(entityTwo) {}

		//==============================
		// Getters/Setters
		//==============================

		UUID GetEntityOne() const { return m_EntityOne; }
		UUID GetEntityTwo() const { return m_EntityTwo; }

		static EventType GetStaticType() { return EventType::PhysicsCollisionEnd; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "PhysicsCollisionEnd"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Application; }
	private:
		// These are the UUID's of both entities involved in the collision event.
		UUID m_EntityOne;
		UUID m_EntityTwo;
	};

	//============================================================
	// App Tick Event Class
	//============================================================
	class AppTickEvent : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		AppTickEvent(uint64_t delayMilliseconds)
			: m_DelayMilliseconds(delayMilliseconds) {}

		//==============================
		// Getters/Setters
		//==============================

		uint64_t GetDelayMilliseconds() const { return m_DelayMilliseconds; }

		static EventType GetStaticType() { return EventType::AppTick; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "AppTickEvent"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Application; }
	private:
		uint64_t m_DelayMilliseconds;
	};

	//============================================================
	// Add Tick Generator Usage Event Class
	//============================================================
	class AddTickGeneratorUsage : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		AddTickGeneratorUsage(uint64_t delayMilliseconds)
			: m_DelayMilliseconds(delayMilliseconds) {}

		//==============================
		// Getters/Setters
		//==============================

		uint64_t GetDelayMilliseconds() const { return m_DelayMilliseconds; }

		static EventType GetStaticType() { return EventType::AddTickGeneratorUsage; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "AddTickGeneratorUsage"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Application; }
	private:
		uint64_t m_DelayMilliseconds;
	};

	//============================================================
	// Remove Tick Generator Usage Event Class
	//============================================================
	class RemoveTickGeneratorUsage : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		RemoveTickGeneratorUsage(uint64_t delayMilliseconds)
			: m_DelayMilliseconds(delayMilliseconds) {}

		//==============================
		// Getters/Setters
		//==============================

		uint64_t GetDelayMilliseconds() const { return m_DelayMilliseconds; }

		static EventType GetStaticType() { return EventType::RemoveTickGeneratorUsage; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "RemoveTickGeneratorUsage"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Application; }
	private:
		uint64_t m_DelayMilliseconds;
	};

	//============================================================
	// Skip Update Event Class
	//============================================================
	class SkipUpdateEvent : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		SkipUpdateEvent(uint32_t count)
			: m_Count(count) {}

		//==============================
		// Getters/Setters
		//==============================

		uint64_t GetSkipCount() const { return m_Count; }

		static EventType GetStaticType() { return EventType::SkipUpdate; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "SkipUpdateEvent"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Application; }
	private:
		uint32_t m_Count;
	};

	//============================================================
	// Add Extra Update Event Class
	//============================================================
	class AddExtraUpdateEvent : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		AddExtraUpdateEvent(uint32_t count)
			: m_Count(count) {}

		//==============================
		// Getters/Setters
		//==============================

		uint64_t GetExtraUpdateCount() const { return m_Count; }

		static EventType GetStaticType() { return EventType::AddExtraUpdate; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "AddExtraUpdateEvent"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Application; }
	private:
		uint32_t m_Count;
	};




}

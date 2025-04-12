#pragma once

#include "Kargono/Events/Event.h"
#include "Kargono/Core/UUID.h"

#include <sstream>
#include <string>

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

		virtual EventType GetEventType() const override { return EventType::WindowResize; }
		virtual int GetCategoryFlags() const override { return EventCategory::Application; }
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
		virtual EventType GetEventType() const override { return EventType::WindowClose; }
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
		virtual EventType GetEventType() const override { return EventType::AppClose; }
		virtual int GetCategoryFlags() const override { return EventCategory::Application; }
	};

#if 0
	class AppUpdateEvent : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		AppUpdateEvent(float deltaTime)
			: m_DeltaTime(deltaTime) {}

		AppUpdateEvent(size_t deltaTimeMs) : m_DeltaTime()

		//==============================
		// Getters/Setters
		//==============================

		float GetDeltaTime() const { return m_DeltaTime; }

		virtual EventType GetEventType() const override { return EventType::AppUpdate; }
		virtual int GetCategoryFlags() const override { return EventCategory::Application; }
	private:
		float m_DeltaTime;
	};
#endif

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
		virtual EventType GetEventType() const override { return EventType::CleanUpTimers; }
		virtual int GetCategoryFlags() const override { return EventCategory::Application; }
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

		virtual EventType GetEventType() const override { return EventType::AppTick; }
		virtual int GetCategoryFlags() const override { return EventCategory::Application; }
	private:
		uint64_t m_DelayMilliseconds;
	};

	class ApplicationResizeEvent : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This event is initialized with the new width and height of the window.
		ApplicationResizeEvent(unsigned int width, unsigned int height)
			: m_Width(width), m_Height(height) {}

		//==============================
		// Getters/Setters
		//==============================
		unsigned int GetWidth() const { return m_Width; }
		unsigned int GetHeight() const { return m_Height; }

		virtual EventType GetEventType() const override { return EventType::AppResize; }
		virtual int GetCategoryFlags() const override { return EventCategory::Application; }
	private:
		// m_Width and m_Height represent the new width and height of the window
		//		after the resize has occurred on the Window.
		unsigned int m_Width, m_Height;
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

		virtual EventType GetEventType() const override { return EventType::AddTickGeneratorUsage; }
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

		virtual EventType GetEventType() const override { return EventType::RemoveTickGeneratorUsage; }
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

		virtual EventType GetEventType() const override { return EventType::SkipUpdate; }
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

		virtual EventType GetEventType() const override { return EventType::AddExtraUpdate; }
		virtual int GetCategoryFlags() const override { return EventCategory::Application; }
	private:
		uint32_t m_Count;
	};


	enum class LogEventLevel
	{
		None = 0,
		Info,
		Warning,
		Critical
	};

	//============================================================
	// Log Event Class
	//============================================================
	class LogEvent : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		LogEvent(const char* eventText, LogEventLevel eventLevel)
			: m_EventText(eventText), m_EventLevel(eventLevel) {}

		//==============================
		// Getters/Setters
		//==============================

		std::string GetEventText() const { return m_EventText; }
		LogEventLevel GetEventLevel() const { return m_EventLevel; }

		virtual EventType GetEventType() const override { return EventType::LogEvent; }
		virtual int GetCategoryFlags() const override { return EventCategory::Application; }
	private:
		std::string m_EventText;
		LogEventLevel m_EventLevel;
	};


}

#pragma once

#include "Event.h"


namespace Kargono
{
/// @class WindowResizeEvent : public Event
	class KG_API WindowResizeEvent : public Event
	{
	public:
/// @brief Constructor for WindowResizeEvent class
		WindowResizeEvent(unsigned int width, unsigned int height)
			: m_Width(width), m_Height(height) {}

/// @brief Get the width of the window
		inline unsigned int GetWidth() const { return m_Width; }
/// @brief Get the height of the window
		inline unsigned int GetHeight() const { return m_Height; }

/// @brief Convert the event to a string representation
		std::string ToString() const override 
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		unsigned int m_Width, m_Height;
	};

/// @class WindowCloseEvent : public Event
	class KG_API WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() {}

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

/// @class AppTickEvent : public Event
	class KG_API AppTickEvent : public Event
	{
	public:
		AppTickEvent() {}

		EVENT_CLASS_TYPE(AppTick)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};
/// @class AppUpdateEvent : public Event
	class KG_API AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent() {}

		EVENT_CLASS_TYPE(AppUpdate)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};
/// @class AppRenderEvent : public Event
	class KG_API AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() {}

		EVENT_CLASS_TYPE(AppRender)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};


}
/// @brief Ensures that this header is included only once in the compilation process
#pragma once

/// @brief Including the Event header for inheritance and use of the Event base class
#include "Event.h"


/// @brief Namespace for all classes and components of the Kargono game engine
namespace Kargono
{
/// @class WindowResizeEvent
/// @brief Event class for window resize events
/// @details This event is triggered when the game window is resized
	class KG_API WindowResizeEvent : public Event
	{
	public:
/// @brief Constructor for the WindowResizeEvent class
/// @param width The new width of the game window
/// @param height The new height of the game window
		WindowResizeEvent(unsigned int width, unsigned int height)
			: m_Width(width), m_Height(height) {}

/// @brief Getter function for the width of the game window
/// @return The width of the game window
		inline unsigned int GetWidth() const { return m_Width; }
/// @brief Getter function for the height of the game window
/// @return The height of the game window
		inline unsigned int GetHeight() const { return m_Height; }

/// @brief Converts the WindowResizeEvent to a string representation
/// @details This function returns a string representation of the WindowResizeEvent
/// @return A string representation of the WindowResizeEvent
		std::string ToString() const override 
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

/// @brief Macro for defining the event class category as WindowResize
		EVENT_CLASS_TYPE(WindowResize)
/// @brief Macro for defining the event category as Application
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		unsigned int m_Width, m_Height;
	};

/// @class WindowCloseEvent
/// @brief Event class for window close events
/// @details This event is triggered when the game window is closed
	class KG_API WindowCloseEvent : public Event
	{
	public:
/// @brief Constructor for the WindowCloseEvent class
		WindowCloseEvent() {}

/// @brief Macro for defining the event class category as WindowClose
		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

/// @class AppTickEvent
/// @brief Event class for application tick events
/// @details This event is triggered on every tick of the game engine
	class KG_API AppTickEvent : public Event
	{
	public:
/// @brief Constructor for the AppTickEvent class
		AppTickEvent() {}

/// @brief Macro for defining the event class category as AppTick
		EVENT_CLASS_TYPE(AppTick)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};
/// @class AppUpdateEvent
/// @brief Event class for application update events
/// @details This event is triggered on every update of the game engine
	class KG_API AppUpdateEvent : public Event
	{
	public:
/// @brief Constructor for the AppUpdateEvent class
		AppUpdateEvent() {}

/// @brief Macro for defining the event class category as AppUpdate
		EVENT_CLASS_TYPE(AppUpdate)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};
/// @class AppRenderEvent
/// @brief Event class for application render events
/// @details This event is triggered on every render of the game engine
	class KG_API AppRenderEvent : public Event
	{
	public:
/// @brief Constructor for the AppRenderEvent class
		AppRenderEvent() {}

/// @brief Macro for defining the event class category as AppRender
		EVENT_CLASS_TYPE(AppRender)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};


}
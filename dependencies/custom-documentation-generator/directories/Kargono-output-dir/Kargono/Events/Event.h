#pragma once
#include <Kargono/Core.h>

namespace Kargono 
{
/// @brief Enumeration defining different event types in the game engine
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

/// @brief Enumeration defining different event categories in the game engine
	enum EventCategory 
	{
		None = 0,
		EventCategoryApplication	= BIT(0),
		EventCategoryInput			= BIT(1),
		EventCategoryKeyboard		= BIT(2),
		EventCategoryMouse			= BIT(3),
		EventCategoryMouseButton	= BIT(4)

	};

/// @brief Macro for defining the event class category and its members
#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::##type;} \
							    virtual EventType GetEventType() const override { return GetStaticType();}\
								virtual const char* GetName() const override {return #type;}

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category;}

/// @class Event
	class KG_API Event 
	{
	public:
		
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

/// @return The category flags of the event
		inline bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}
/// @brief Boolean indicating if the event has been handled or not
		bool Handled = false;
	};

/// @class EventDispatcher
	class EventDispatcher
	{
/// @tparam K The type of event handler function
		template<typename K>
/// @typedef EventFn
		using EventFn = std::function<bool(K&)>;
	public:
/// @brief Constructor for the EventDispatcher class
		EventDispatcher(Event& event) : m_Event(event)
		{
		}

/// @tparam T The type of event handler function
		template<typename T>
/// @brief Dispatches the event to the appropriate event handler
		bool Dispatch(EventFn<T> func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.Handled = func(*(T*)&m_Event);
				return true;
			}
			return false;
		}
	private:
		Event& m_Event;
	};

/// @brief Overloaded output stream operator
	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
	
}
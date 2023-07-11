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

/// @brief Macro for defining the event class category
#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category;}

/// @class Event
	class KG_API Event 
	{
/// @class EventDispatcher
		friend class EventDispatcher;
	public:
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

/// @brief Checks if the event is in a specific category
		inline bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}
	protected:
		bool m_Handled = false;
	};

	class EventDispatcher
	{
/// @tparam K - type of the event
		template<typename K>
		using EventFn = std::function<bool(K&)>;
	public:
		EventDispatcher(Event& event) : m_Event(event)
		{
		}

		template<typename T>
		bool Dispatch(EventFn<T> func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.m_Handled = func(*(T*)&m_Event);
				return true;
			}
			return false;
		}
	private:
		Event& m_Event;
	};

/// @brief Overrides the << operator for outputting the event as a string
	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
	
}
/// @brief Include guard to ensure this file is only included once
#pragma once
/// @brief Include the core header file from the Kargono namespace
#include <Kargono/Core.h>

namespace Kargono 
{
/// @brief An enumeration defining the different types of events in the Kargono engine
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

/// @brief An enumeration defining the different categories of events in the Kargono engine
	enum EventCategory 
	{
		None = 0,
		EventCategoryApplication	= BIT(0),
		EventCategoryInput			= BIT(1),
		EventCategoryKeyboard		= BIT(2),
		EventCategoryMouse			= BIT(3),
		EventCategoryMouseButton	= BIT(4)

	};

/// @brief Macro for defining the event class type and its members
#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::##type;} \
							    virtual EventType GetEventType() const override { return GetStaticType();}\
								virtual const char* GetName() const override {return #type;}

/// @brief Macro for defining the event class category and its members
#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category;}

/// @brief The base class for all events in the Kargono engine
	class KG_API Event 
	{
	public:
		
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}
		bool Handled = false;
	};

/// @brief Class for dispatching events in the Kargono engine
	class EventDispatcher
	{
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
				m_Event.Handled = func(*(T*)&m_Event);
				return true;
			}
			return false;
		}
	private:
		Event& m_Event;
	};

/// @brief Overloaded stream insertion operator for printing events
	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
	
}
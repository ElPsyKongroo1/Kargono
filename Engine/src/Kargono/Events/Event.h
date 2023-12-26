#pragma once

#include <Kargono/Core/Base.h>

#include <string>
#include <functional>

//============================================================
// Events Namespace
//============================================================
// This namespace provides the main classes that allow the Event Pipeline
//		to work correctly. The intended use of classes in this namespace
//		is:
//		1. Create an OnEvent(Event) function in the main application layer that
//		links to other OnEvent(Event) functions depending on the particular scenerio.
//		(These linkages constitute the event pipeline)
//		2. Provide this main OnEvent(Event) function to an event generating section
//		of the engine such as the Windowing Library (GLFW currently) that can receive
//		input events such as KeyPressed events.
//		3. Add Dispatchers throughout the OnEvent(Event) function pipeline that link
//		to local functions that handle events. The dispatchers check if the event type
//		of the currently running event matches the dispatcher's type.
//		4. Implement logic to handle events in different scenerios using the m_Handled
//		member of the event class.
namespace Kargono::Events
{
	//==============================
	// Event Type Enum
	//==============================
	// This enum represents all of the different event types present in
	//		the application that are mutually exclusive. This list will
	//		expand as more event types are required.
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender, AppClose,
		PhysicsCollision,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	//==============================
	// Event Category Enum
	//==============================
	// This enum provides an easy method to determine if a particular event
	//		falls into one or more non-mutually exclusive event categories.
	//		The enum is structured as a bitfield to allow a particular event
	//		to have multiple categories. Ex: A MouseButtonEvent could be labeled
	//		as a Mouse, MouseButton, and Input event.
	enum EventCategory 
	{
		None				= 0,
		Application			= BIT(0),
		Input				= BIT(1),
		Keyboard			= BIT(2),
		Mouse				= BIT(3),
		MouseButton			= BIT(4)

	};

	//============================================================
	// Event Class
	//============================================================
	// This Event class provides a uniform interface for different event
	//		types to be routed through the same event pipeline. This is
	//		an abstract class that holds multiple getter functions that
	//		allow for easy identification of the event type among other
	//		properties. This class also has a Handled member which
	//		informs the event pipeline that the event has been used
	//		to run a function call.
	class Event 
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This constructor is simply set to prevent object slicing
		//		during the destruction of the parent class.
		virtual ~Event() = default;

		//==============================
		// Getters/Setters
		//==============================
		// These getter functions provide a method to identify event types
		//		and manage the events externally. The ToString function is
		//		for debugging.
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }
		// This function provides an easy method for determining if an event
		//		is part of a broader category since EventCategory's are bit
		//		fields.
		bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}
		// This variable determines if an event has been handled.
		//		A handled event can be managed by the event pipeline.
		//		Ex: An event that has ran its functionality can be
		//		set to handled. Later in the event pipeline, there
		//		can be an early-out if the event is labeled handled.
		bool Handled = false;
	};


	//============================================================
	// Event Dispatcher Class
	//============================================================
	// This class is a supporting functionality class for the event class. This
	//		class is meant to be instantiated inside of the event pipeline with
	//		the currently managed event. Once this class is instantiated,
	//		it can be used to dispatch an event (determine if the event type
	//		is correct for a particular function and run the function if correct).
	class EventDispatcher
	{
		// Templated alias for a function pointer is used in the
		//		Dispatch function.
		template<typename K>
		using EventFn = std::function<bool(K&)>;
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This constructor simply initializes the dispatcher with an event
		//		to handle. This construction method allows for one easy
		//		connection between the event and the dispatcher to be made.
		//		Multiple dispatches can be run with different function pointers
		//		with the same dispatch object and event.
		EventDispatcher(Event& event) : m_Event(event)
		{
		}
		//==============================
		// Execute and Handle Event
		//==============================
		// This function provides a method for ensuring the event initialized with
		//		m_Event has the same EventType as the template type T. Once the
		//		event types match, the provided function is called and the event is
		//		set to handled. This allows for an easy API to externally handle
		//		different scenerios involving event management inside the event
		//		pipeline.
		template<typename T>
		bool Dispatch(EventFn<T> func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.Handled |= func(*(T*)&m_Event);
				return true;
			}
			return false;
		}
	private:
		// m_Event holds a reference to the currently managed event inside the
		//		event pipeline. This event is stored internally so that it does
		//		not need to be provided to the Dispatch call repeatadly.
		Event& m_Event;
	};

	//==============================
	// Operator Overload
	//==============================
	// This overload allows for easy logging of the event type when debugging.
	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
	
}

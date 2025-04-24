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
		// Application
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		CleanUpTimers,
		SkipUpdate, AddExtraUpdate, LogEvent,
		AppTick, AppUpdate, AppRender, AppClose, AppResize,
		AddTickGeneratorUsage, RemoveTickGeneratorUsage,
		// Assets
		ManageAsset,
		// Physics
		PhysicsCollisionStart, PhysicsCollisionEnd,
		// Keyboard
		KeyPressed, KeyReleased, KeyTyped,
		// Mouse Button
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
		// Network
		ReceiveOnlineUsers, RequestJoinSession, ApproveJoinSession, DenyJoinSession, RequestUserCount,
		LeaveCurrentSession, UserLeftSession, CurrentSessionInit, ConnectionTerminated, StartSession,
		UpdateSessionUserSlot, EnableReadyCheck, SendReadyCheck, SendReadyCheckConfirm, SendAllEntityLocation,
		UpdateEntityLocation, SendAllEntityPhysics, UpdateEntityPhysics, SignalAll, ReceiveSignal,
		// Scene
		ManageEntity, ManageScene,
		// Editor
		ManageEditor
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
		Scene				= BIT(2),
		Network				= BIT(3),
		Keyboard			= BIT(4),
		Mouse				= BIT(5),
		MouseButton			= BIT(6),
		Physics				= BIT(7),
		Asset				= BIT(8),
		Editor				= BIT(9)
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
		virtual int GetCategoryFlags() const = 0;
		// This function provides an easy method for determining if an event
		//		is part of a broader category since EventCategory's are bit
		//		fields.
		bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}
	};

	using EventCallbackFn = std::function<void(Events::Event*)>;
	
}

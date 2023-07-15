#pragma once

#include "Event.h"

namespace Kargono
{
/// @class KeyEvent
/// @brief Base class for keyboard events in the game engine
/// @details This class is used as a base class for handling keyboard events
/// @inherits Event
	class KG_API KeyEvent : public Event
	{
	public:
		inline int GetKeyCode() const { return m_KeyCode; }

/// @brief Macro for defining the event class category for keyboard events
/// @note This macro allows for categorizing keyboard events as both keyboard and input events
		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
/// @brief Constructor for KeyEvent class
/// @param keycode The key code associated with the keyboard event
		KeyEvent(int keycode) : m_KeyCode(keycode) {}
		int m_KeyCode;
 	};

/// @class KeyPressedEvent
/// @brief Class for handling key pressed events in the game engine
/// @inherits KeyEvent
	class KG_API KeyPressedEvent : public KeyEvent 
	{
	public:
/// @brief Constructor for KeyPressedEvent class
/// @param keycode The key code associated with the key pressed event
/// @param repeatCount The number of times the key was repeatedly pressed
		KeyPressedEvent(int keycode, int repeatCount) : KeyEvent(keycode), m_RepeatCount(repeatCount) {}

/// @brief Getter method for the repeat count of the key
		inline int GetRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override 
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";
			return ss.str();
		}

/// @brief Macro for defining the event type for key pressed events
		EVENT_CLASS_TYPE(KeyPressed)
	private:
		int m_RepeatCount;
	};

/// @class KeyReleasedEvent
/// @brief Class for handling key released events in the game engine
/// @inherits KeyEvent
	class KG_API KeyReleasedEvent : public KeyEvent
	{
	public:
/// @brief Constructor for KeyReleasedEvent class
/// @param keycode The key code associated with the key released event
		KeyReleasedEvent(int keycode) : KeyEvent(keycode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}

/// @brief Macro for defining the event type for key released events
		EVENT_CLASS_TYPE(KeyReleased)
	};

/// @class KeyTypedEvent
/// @brief Class for handling key typed events in the game engine
/// @inherits KeyEvent
	class KG_API KeyTypedEvent : public KeyEvent
	{
	public:
/// @brief Constructor for KeyTypedEvent class
/// @param keycode The key code associated with the key typed event
		KeyTypedEvent(int keycode) : KeyEvent(keycode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_KeyCode;
			return ss.str();
		}

/// @brief Macro for defining the event type for key typed events
		EVENT_CLASS_TYPE(KeyTyped)
	};
}
#pragma once

#include "Event.h"

// @namespace Kargono - Namespace for Kargono game engine
namespace Kargono
{
// @class KeyEvent - Event for key events in the game engine.
	class KG_API KeyEvent : public Event
	{
	public:
		inline int GetKeyCode() const { return m_KeyCode; }

// @brief Define the event category as Keyboard and Input.
		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
// @brief Construct a KeyEvent object with the given key code.
		KeyEvent(int keycode) : m_KeyCode(keycode) {}
		int m_KeyCode;
 	};

// @class KeyPressedEvent - Event for key press events in the game engine.
	class KG_API KeyPressedEvent : public KeyEvent 
	{
	public:
		KeyPressedEvent(int keycode, int repeatCount) : KeyEvent(keycode), m_RepeatCount(repeatCount) {}

		inline int GetRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override 
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";
			return ss.str();
		}

// @brief Define the event type as KeyPressed.
		EVENT_CLASS_TYPE(KeyPressed)
	private:
// @param m_RepeatCount - The repeat count of the event.
		int m_RepeatCount;
	};

// @class KeyReleasedEvent - Event for key release events in the game engine.
	class KG_API KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(int keycode) : KeyEvent(keycode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}

// @brief Define the event type as KeyReleased.
		EVENT_CLASS_TYPE(KeyReleased)
	};
}
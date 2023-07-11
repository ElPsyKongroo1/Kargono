#pragma once

#include "Event.h"

namespace Kargono
{
	class KG_API KeyEvent : public Event
	{
	public:
/// @brief Get the key code of the keyboard event
/// @return The key code as an integer value
		inline int GetKeyCode() const { return m_KeyCode; }

/// @brief Define the category of the KeyEvent as a combination of EventCategoryKeyboard and EventCategoryInput flags
		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
/// @brief Construct a KeyEvent object with the specified key code
/// @param keycode The key code as an integer value
		KeyEvent(int keycode) : m_KeyCode(keycode) {}
		int m_KeyCode;
 	};

/// @class KeyPressedEvent
/// @brief Class representing a key press event in the game engine
	class KG_API KeyPressedEvent : public KeyEvent 
	{
	public:
/// @brief Construct a KeyPressedEvent object with the specified key code and repeat count
/// @param keycode The key code as an integer value
/// @param repeatCount The count of key repeats as an integer value
		KeyPressedEvent(int keycode, int repeatCount) : KeyEvent(keycode), m_RepeatCount(repeatCount) {}

/// @brief Get the count of key repeats in the KeyPressedEvent
/// @return The repeat count as an integer value
		inline int GetRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override 
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";
			return ss.str();
		}

/// @brief Set the event type of the KeyPressedEvent to KeyPressed
		EVENT_CLASS_TYPE(KeyPressed)
	private:
/// @brief The count of key repeats in the KeyPressedEvent
		int m_RepeatCount;
	};

	class KG_API KeyReleasedEvent : public KeyEvent
	{
	public:
/// @brief Construct a KeyReleasedEvent object with the specified key code
/// @param keycode The key code as an integer value
		KeyReleasedEvent(int keycode) : KeyEvent(keycode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}

/// @brief Set the event type of the KeyReleasedEvent to KeyReleased
		EVENT_CLASS_TYPE(KeyReleased)
	};
}
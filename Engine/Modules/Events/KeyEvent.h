#pragma once

#include "Modules/Events/Event.h"
#include "Kargono/Core/KeyCodes.h"

namespace Kargono::Events
{
	//============================================================
	// Key Pressed Event Class
	//============================================================
	// This event represents the moment a key is pressed on a keyboard.
	//		This event has a KeyCode which is present in the parent class.
	//		This event is thrown by the current Windowing system. The
	//		m_IsRepeat simply describes whether the key is flagged as
	//		repeating by the windowing system.
	class KeyPressedEvent : public Event 
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This constructor initializes both the m_KeyCode from the parent class
		//		and the m_IsRepeat member.
		KeyPressedEvent(const KeyCode keycode, bool isRepeat = false)
			: m_KeyCode(keycode), m_IsRepeat(isRepeat) {}

		//==============================
		// Getters/Setters
		//==============================
		bool IsRepeat() const { return m_IsRepeat; }
		KeyCode GetKeyCode() const { return m_KeyCode; }

		virtual EventType GetEventType() const override { return EventType::KeyPressed; }
		virtual int GetCategoryFlags() const override { return EventCategory::Keyboard | EventCategory::Input; }
	private:
		// m_IsRepeat represents whether the Key has been held down long enough
		//		for the windowing system to label the key as repeating.
		bool m_IsRepeat;
		KeyCode m_KeyCode;
	};
	//============================================================
	// Key Released Event Class
	//============================================================
	// This event represents the moment a particular key is released on a
	//		keyboard. This event is thrown by the current windowing system.
	class KeyReleasedEvent : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This constructor simply initializes the parent class member m_KeyCode.
		KeyReleasedEvent(const KeyCode keycode) : m_KeyCode(keycode) {}

		//==============================
		// Getters/Setters
		//==============================
		KeyCode GetKeyCode() const { return m_KeyCode; }

		virtual EventType GetEventType() const override { return EventType::KeyReleased; }
		virtual int GetCategoryFlags() const override { return EventCategory::Keyboard | EventCategory::Input; }
	private:
		KeyCode m_KeyCode;
	};
	//============================================================
	// Key Typed Event Class
	//============================================================
	// This event represents the moment a key is pressed on a key
	//		that correctly translates into a character for user
	//		input. This conceptually has a lot of overlap with the
	//		KeyPressedEvent but should be treated separately for
	//		organizational sake. This event is thrown by the windowing system.
	class KeyTypedEvent : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This constructor simply initializes the parent class member m_KeyCode.
		KeyTypedEvent(const KeyCode keycode) : m_KeyCode(keycode) {}

		//==============================
		// Getters/Setters
		//==============================
		KeyCode GetKeyCode() const { return m_KeyCode; }

		virtual EventType GetEventType() const override { return EventType::KeyTyped; }
		virtual int GetCategoryFlags() const override { return EventCategory::Keyboard | EventCategory::Input; }
	private:
		KeyCode m_KeyCode;
	};
}

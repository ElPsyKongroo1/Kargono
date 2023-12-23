#pragma once

#include "Kargono/Events/Event.h"
#include "Kargono/Core/KeyCodes.h"

namespace Kargono::Events
{
	//============================================================
	// Key Event Class
	//============================================================
	// This is an abstract class that provides access to the m_KeyCode for
	//		subclasses such as the KeyPressedEvent, KeyReleasedEvent, etc...
	//		All of these sublcass events involve a keycode, just in different
	//		scenerios.
	class KeyEvent : public Event
	{
	protected:
		//==============================
		// Constructors and Destructors
		//==============================
		// This abstract class is simply initialized with the m_KeyCode
		//		it uses after being created in a subclass.
		KeyEvent(const KeyCode keycode) : m_KeyCode(keycode) {}
	public:
		//==============================
		// Getters/Setters
		//==============================
		KeyCode GetKeyCode() const { return m_KeyCode; }
		virtual int GetCategoryFlags() const override { return EventCategory::Keyboard | EventCategory::Input; }
	protected:
		// m_KeyCode represents the key that was involved in one of the
		//		subclass events such as KeyPressed, KeyReleased, or KeyTyped etc...
		KeyCode m_KeyCode;
 	};
	//============================================================
	// Key Pressed Event Class
	//============================================================
	// This event represents the moment a key is pressed on a keyboard.
	//		This event has a KeyCode which is present in the parent class.
	//		This event is thrown by the current Windowing system. The
	//		m_IsRepeat simply describes whether the key is flagged as
	//		repeating by the windowing system.
	class KeyPressedEvent : public KeyEvent 
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This constructor initializes both the m_KeyCode from the parent class
		//		and the m_IsRepeat member.
		KeyPressedEvent(const KeyCode keycode, bool isRepeat = false)
			: KeyEvent(keycode), m_IsRepeat(isRepeat) {}

		//==============================
		// Getters/Setters
		//==============================
		bool IsRepeat() const { return m_IsRepeat; }

		static EventType GetStaticType() { return EventType::KeyPressed; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "KeyPressedEvent"; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " (repeat = " << m_IsRepeat << ")";
			return ss.str();
		}
	private:
		// m_IsRepeat represents whether the Key has been held down long enough
		//		for the windowing system to label the key as repeating.
		bool m_IsRepeat;
	};
	//============================================================
	// Key Released Event Class
	//============================================================
	// This event represents the moment a particular key is released on a
	//		keyboard. This event is thrown by the current windowing system.
	class KeyReleasedEvent : public KeyEvent
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This constructor simply initializes the parent class member m_KeyCode.
		KeyReleasedEvent(const KeyCode keycode) : KeyEvent(keycode) {}

		//==============================
		// Getters/Setters
		//==============================
		static EventType GetStaticType() { return EventType::KeyReleased; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "KeyReleasedEvent"; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}
	};
	//============================================================
	// Key Typed Event Class
	//============================================================
	// This event represents the moment a key is pressed on a key
	//		that correctly translates into a character for user
	//		input. This conceptually has a lot of overlap with the
	//		KeyPressedEvent but should be treated separately for
	//		organizational sake. This event is thrown by the windowing system.
	class KeyTypedEvent : public KeyEvent
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This constructor simply initializes the parent class member m_KeyCode.
		KeyTypedEvent(const KeyCode keycode) : KeyEvent(keycode) {}

		//==============================
		// Getters/Setters
		//==============================

		static EventType GetStaticType() { return EventType::KeyTyped; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "KeyTypedEvent"; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_KeyCode;
			return ss.str();
		}
	};
}

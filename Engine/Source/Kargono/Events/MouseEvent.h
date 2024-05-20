#pragma once

#include "Kargono/Events/Event.h"
#include "Kargono/Core/MouseCodes.h"

namespace Kargono::Events
{
	//============================================================
	// Mouse Moved Event Class
	//============================================================
	// This event represents the moment where a new location for the mouse
	//		is detected by the windowing system. This mouse movement can
	//		be used for concepts such as an FPS camera that changes its
	//		pitch/yaw with the mouse. The m_MouseX and m_MouseY variables
	//		represent the new mouse location on the window.
	class MouseMovedEvent : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		MouseMovedEvent(const float x, const float y)
			: m_MouseX(x), m_MouseY(y) {}

		//==============================
		// Getters/Setters
		//==============================
		float GetX() const { return m_MouseX; }
		float GetY() const { return m_MouseY; }

		static EventType GetStaticType() { return EventType::MouseMoved; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "MouseMovedEvent"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Mouse | EventCategory::Input; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

	private:
		// These variables represent the new mouse location on the window.
		float m_MouseX, m_MouseY;
	};
	//============================================================
	// Mouse Scrolled Event Class
	//============================================================
	// This event represents the moment when the windowing system detects
	//		a change in the mouse's scroll wheel's position. The m_YOffset
	//		represents the up and down directions of the standard scroll wheel.
	//		Some mice have horizontal movement which can be represented by
	//		the m_XOffset.
	class MouseScrolledEvent : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		MouseScrolledEvent(const float xOffset, const float yOffset)
			: m_XOffset(xOffset), m_YOffset(yOffset) {}

		//==============================
		// Getters/Setters
		//==============================
		float GetXOffset() const { return m_XOffset; }
		float GetYOffset() const { return m_YOffset; }

		static EventType GetStaticType() { return EventType::MouseScrolled; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "MouseScrolledEvent"; }
		virtual int GetCategoryFlags() const override { return EventCategory::Mouse | EventCategory::Input; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << m_XOffset << ", " << m_YOffset;
			return ss.str();
		}

	private:
		// These two variables represent the relative amount of scrolling done
		//		but the mouse's scroll wheel. The m_YOffset variable represents
		//		the standard up and down scroll, but the m_XOffset allows for
		//		support for different mouse setups.
		float m_XOffset, m_YOffset;
	};
	//============================================================
	// Mouse Button Event Class
	//============================================================
	// This is an abstract class that serves to provide the common variable
	//		of m_Button to subclasses such as MouseButtonPressed and Mouse
	//		ButtonReleased events. The button represents a mouse button on
	//		the mouse.
	class MouseButtonEvent : public Event
	{
	protected:
		//==============================
		// Constructors and Destructors
		//==============================
		MouseButtonEvent(const MouseCode button)
			: m_Button(button) {}
	public:
		//==============================
		// Getters/Setters
		//==============================
		MouseCode GetMouseButton() const { return m_Button; }
		virtual int GetCategoryFlags() const override { return EventCategory::Mouse | EventCategory::Input | EventCategory::MouseButton; }

	protected:
		// m_Button represents a mousebutton on the mouse through a button code.
		MouseCode m_Button;
	};
	//============================================================
	// Mouse Button Pressed Event Class
	//============================================================
	// This event represents the moment when a mouse button is pressed
	//		and detected by the current windowing system. The button
	//		is available in the parent class.
	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		MouseButtonPressedEvent(const MouseCode button)
			: MouseButtonEvent(button) {}

		//==============================
		// Getters/Setters
		//==============================

		static EventType GetStaticType() { return EventType::MouseButtonPressed; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "MouseButtonPressedEvent"; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_Button;
			return ss.str();
		}

	};
	//============================================================
	// Mouse Button Released Event Class
	//============================================================
	// This event represents the moment when a mouse button is released
	//		and detected by the current windowing system. The button
	//		is available in the parent class.
	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		MouseButtonReleasedEvent(const MouseCode button)
			: MouseButtonEvent(button) {}

		//==============================
		// Getters/Setters
		//==============================

		static EventType GetStaticType() { return EventType::MouseButtonReleased; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "MouseButtonReleasedEvent"; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button;
			return ss.str();
		}

	};
	

}

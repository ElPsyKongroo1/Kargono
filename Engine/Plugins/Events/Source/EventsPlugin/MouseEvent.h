#pragma once

#include "EventsPlugin/Event.h"
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

		virtual EventType GetEventType() const override { return EventType::MouseMoved; }
		virtual int GetCategoryFlags() const override { return EventCategory::Mouse | EventCategory::Input; }

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

		virtual EventType GetEventType() const override { return EventType::MouseScrolled; }
		virtual int GetCategoryFlags() const override { return EventCategory::Mouse | EventCategory::Input; }

	private:
		// These two variables represent the relative amount of scrolling done
		//		but the mouse's scroll wheel. The m_YOffset variable represents
		//		the standard up and down scroll, but the m_XOffset allows for
		//		support for different mouse setups.
		float m_XOffset, m_YOffset;
	};
	//============================================================
	// Mouse Button Pressed Event Class
	//============================================================
	// This event represents the moment when a mouse button is pressed
	//		and detected by the current windowing system. The button
	//		is available in the parent class.
	class MouseButtonPressedEvent : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		MouseButtonPressedEvent(const MouseCode button)
			: m_Button(button) {}

		//==============================
		// Getters/Setters
		//==============================
		MouseCode GetMouseButton() const { return m_Button; }

		virtual EventType GetEventType() const override { return EventType::MouseButtonPressed; }
		virtual int GetCategoryFlags() const override { return EventCategory::Mouse | EventCategory::Input | EventCategory::MouseButton; }

	private:
		MouseCode m_Button;

	};
	//============================================================
	// Mouse Button Released Event Class
	//============================================================
	// This event represents the moment when a mouse button is released
	//		and detected by the current windowing system. The button
	//		is available in the parent class.
	class MouseButtonReleasedEvent : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		MouseButtonReleasedEvent(const MouseCode button)
			: m_Button(button) {}

		//==============================
		// Getters/Setters
		//==============================
		MouseCode GetMouseButton() const { return m_Button; }

		virtual EventType GetEventType() const override { return EventType::MouseButtonReleased; }
		virtual int GetCategoryFlags() const override { return EventCategory::Mouse | EventCategory::Input | EventCategory::MouseButton; }

	private:
		MouseCode m_Button;

	};
	

}

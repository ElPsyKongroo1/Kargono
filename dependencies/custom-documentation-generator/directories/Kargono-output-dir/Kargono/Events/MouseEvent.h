#pragma once

#include "Event.h"

namespace Kargono 
{
/// @brief Represents a mouse moved event
	class KG_API MouseMovedEvent : public Event 
	{
	public:
/// @brief Construct a mouse moved event
/// @param x The new x-coordinate of the mouse
/// @param y The new y-coordinate of the mouse
		MouseMovedEvent(float x, float y)
			: m_MouseX(x), m_MouseY(y) {}

/// @brief Get the x-coordinate of the mouse
/// @return The x-coordinate of the mouse
		inline float GetX() const { return m_MouseX; }
/// @brief Get the y-coordinate of the mouse
/// @return The y-coordinate of the mouse
		inline float GetY() const { return m_MouseY; }

/// @brief Convert the event to a string representation
/// @return The string representation of the event
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_MouseX, m_MouseY;
	};

	class KG_API MouseScrolledEvent : public Event
	{
	public:
/// @brief Construct a mouse scrolled event
/// @param xOffset The offset value to scroll horizontally
/// @param yOffset The offset value to scroll vertically
		MouseScrolledEvent(float xOffset, float yOffset)
			: m_XOffset(xOffset), m_YOffset(yOffset) {}

/// @brief Get the horizontal offset value of the scroll
/// @return The horizontal offset value of the scroll
		inline float GetXOffset() const { return m_XOffset; }
/// @brief Get the vertical offset value of the scroll
/// @return The vertical offset value of the scroll
		inline float GetYOffset() const { return m_YOffset; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << m_XOffset << ", " << m_YOffset;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_XOffset;
		float m_YOffset;
	};

	class KG_API MouseButtonEvent : public Event
	{
	public:
/// @brief Get the ID of the mouse button
/// @return The ID of the mouse button
		inline float GetMouseButton() const { return m_Button; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	protected:
		MouseButtonEvent(int button)
			: m_Button(button) {}

		int m_Button;
	};

	class KG_API MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:

/// @brief Construct a mouse button pressed event
/// @param button The ID of the mouse button
		MouseButtonPressedEvent(int button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)

	};

	class KG_API MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:

/// @brief Construct a mouse button released event
/// @param button The ID of the mouse button
		MouseButtonReleasedEvent(int button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)

	};
	

}
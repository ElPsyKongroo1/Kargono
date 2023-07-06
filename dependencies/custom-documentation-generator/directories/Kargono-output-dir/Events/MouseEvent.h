#pragma once

#include "Event.h"

namespace Kargono 
{
/// @class MouseMovedEvent
	class KG_API MouseMovedEvent : public Event 
	{
	public:
/// @brief Constructs a mouse moved event object with the given x and y coordinates
		MouseMovedEvent(float x, float y)
			: m_MouseX(x), m_MouseY(y) {}

/// @brief Gets the x coordinate of the mouse position
		inline float GetX() const { return m_MouseX; }
/// @brief Gets the y coordinate of the mouse position
		inline float GetY() const { return m_MouseY; }

/// @brief Converts the mouse moved event object to a string representation
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

/// @brief Specifies the event class type as MouseMoved
		EVENT_CLASS_TYPE(MouseMoved)
/// @brief Specifies the event category as EventCategoryMouse and EventCategoryInput
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_MouseX, m_MouseY;
	};

	class KG_API MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float xOffset, float yOffset)
			: m_XOffset(xOffset), m_YOffset(yOffset) {}

		inline float GetXOffset() const { return m_XOffset; }
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
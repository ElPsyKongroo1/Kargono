#pragma once

#include "Event.h"

/// @brief Namespace that contains all classes and functions related to the Kargono game engine.
namespace Kargono 
{
/// @class MouseMovedEvent
/// @brief Event class representing the mouse moved event.
	class KG_API MouseMovedEvent : public Event 
	{
	public:
/// @brief Construct a new MouseMovedEvent object.
/// @param x The x-coordinate of the mouse position.
/// @param y The y-coordinate of the mouse position.
		MouseMovedEvent(float x, float y)
			: m_MouseX(x), m_MouseY(y) {}

/// @brief Get the x-coordinate of the mouse position.
/// @return The x-coordinate of the mouse position.
		inline float GetX() const { return m_MouseX; }
/// @brief Get the y-coordinate of the mouse position.
/// @return The y-coordinate of the mouse position.
		inline float GetY() const { return m_MouseY; }

/// @brief Convert the mouse moved event to a string representation.
/// @return A string representation of the mouse moved event.
/// @brief Convert the mouse scrolled event to a string representation.
/// @return A string representation of the mouse scrolled event.
/// @brief Convert the mouse button pressed event to a string representation.
/// @return A string representation of the mouse button pressed event.
/// @brief Convert the mouse button released event to a string representation.
/// @return A string representation of the mouse button released event.
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

/// @brief Define the event class type as MouseMoved.
		EVENT_CLASS_TYPE(MouseMoved)
/// @brief Define the event class category as EventCategoryMouse and EventCategoryInput.
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_MouseX, m_MouseY;
	};

/// @class MouseScrolledEvent
/// @brief Event class representing the mouse scrolled event.
	class KG_API MouseScrolledEvent : public Event
	{
	public:
/// @brief Construct a new MouseScrolledEvent object.
/// @param xOffset The x-offset of the mouse scroll event.
/// @param yOffset The y-offset of the mouse scroll event.
		MouseScrolledEvent(float xOffset, float yOffset)
			: m_XOffset(xOffset), m_YOffset(yOffset) {}

/// @brief Get the x-offset of the mouse scroll event.
/// @return The x-offset of the mouse scroll event.
		inline float GetXOffset() const { return m_XOffset; }
/// @brief Get the y-offset of the mouse scroll event.
/// @return The y-offset of the mouse scroll event.
		inline float GetYOffset() const { return m_YOffset; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << m_XOffset << ", " << m_YOffset;
			return ss.str();
		}

/// @brief Define the event class type as MouseScrolled.
		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_XOffset;
		float m_YOffset;
	};

/// @class MouseButtonEvent
/// @brief Base event class for mouse button events.
	class KG_API MouseButtonEvent : public Event
	{
	public:
/// @brief Get the mouse button.
/// @return The mouse button.
		inline int GetMouseButton() const { return m_Button; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	protected:
		MouseButtonEvent(int button)
			: m_Button(button) {}

		int m_Button;
	};

/// @class MouseButtonPressedEvent
/// @brief Event class representing the mouse button pressed event.
	class KG_API MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:

/// @brief Construct a new MouseButtonPressedEvent object.
/// @param button The mouse button that was pressed.
		MouseButtonPressedEvent(int button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_Button;
			return ss.str();
		}

/// @brief Define the event class type as MouseButtonPressed.
		EVENT_CLASS_TYPE(MouseButtonPressed)

	};

/// @class MouseButtonReleasedEvent
/// @brief Event class representing the mouse button released event.
	class KG_API MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:

/// @brief Construct a new MouseButtonReleasedEvent object.
/// @param button The mouse button that was released.
		MouseButtonReleasedEvent(int button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button;
			return ss.str();
		}

/// @brief Define the event class type as MouseButtonReleased.
		EVENT_CLASS_TYPE(MouseButtonReleased)

	};
	

}
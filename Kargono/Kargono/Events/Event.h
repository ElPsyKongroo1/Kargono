#pragma once
#include <Kargono/Core.h>

#include <string>
#include <functional>

namespace Kargono 
{
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory 
	{
		None = 0,
		EventCategoryApplication	= Bit(0),
		EventCategoryInput			= Bit(1),
		EventCategoryKeyboard		= Bit(2),
		EventCategoryMouse			= Bit(3),
		EventCategoryMouseButton	= Bit(4)

	};

	class KG_API Event 
	{
		friend class EventDispatcher;
	public:
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}
	protected:
		bool m_Handled = false;
	};
	
}
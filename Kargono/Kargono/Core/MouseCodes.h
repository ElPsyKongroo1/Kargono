#pragma once

namespace Kargono
{
	typedef enum class MouseCode : uint16_t
	{
		// From glfw3.h
		Button0 = 0,
		Button1 = 1,
		Button2 = 2,
		Button3 = 3,
		Button4 = 4,
		Button5 = 5,
		Button6 = 6,
		Button7 = 7,

		ButtonLast = Button7,
		ButtonLeft = Button0,
		ButtonRight = Button1,
		ButtonMiddle = Button2
	} Mouse;

	inline std::ostream& operator<<(std::ostream& os, MouseCode mouseCode)
	{
		os << static_cast<int32_t>(mouseCode);
		return os;
	}
}

#define KG_MOUSE_BUTTON_0      ::Kargono::Mouse::Button0
#define KG_MOUSE_BUTTON_1      ::Kargono::Mouse::Button1
#define KG_MOUSE_BUTTON_2      ::Kargono::Mouse::Button2
#define KG_MOUSE_BUTTON_3      ::Kargono::Mouse::Button3
#define KG_MOUSE_BUTTON_4      ::Kargono::Mouse::Button4
#define KG_MOUSE_BUTTON_5      ::Kargono::Mouse::Button5
#define KG_MOUSE_BUTTON_6      ::Kargono::Mouse::Button6
#define KG_MOUSE_BUTTON_7      ::Kargono::Mouse::Button7
#define KG_MOUSE_BUTTON_LAST   ::Kargono::Mouse::ButtonLast
#define KG_MOUSE_BUTTON_LEFT   ::Kargono::Mouse::ButtonLeft
#define KG_MOUSE_BUTTON_RIGHT  ::Kargono::Mouse::ButtonRight
#define KG_MOUSE_BUTTON_MIDDLE ::Kargono::Mouse::ButtonMiddle
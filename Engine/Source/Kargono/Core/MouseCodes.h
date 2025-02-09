#pragma once

namespace Kargono
{
	using MouseCode = uint16_t;
}

namespace Kargono::Mouse
{
	enum : MouseCode
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
		ButtonMiddle = Button2,
		ButtonForward = Button4,
		ButtonBack = Button3
	};

}

namespace Kargono::Utility
{
	inline const char* MouseCodeToString(MouseCode mouseCode)
	{
		switch (mouseCode)
		{
		case Mouse::Button0: return "Button0";
		case Mouse::Button1: return "Button1";
		case Mouse::Button2: return "Button2";
		case Mouse::Button3: return "Button3";
		case Mouse::Button4: return "Button4";
		case Mouse::Button5: return "Button5";
		case Mouse::Button6: return "Button6";
		case Mouse::Button7: return "Button7";
		}

		return "";
	}
}

#pragma once

namespace Kargono
{
	typedef enum class KeyCode : uint16_t
	{
		// From glfw3.h
		Space = 32,
		Apostrophe = 39, /* ' */
		Comma = 44, /* , */
		Minus = 45, /* - */
		Period = 46, /* . */
		Slash = 47, /* / */

		D0 = 48, /* 0 */
		D1 = 49, /* 1 */
		D2 = 50, /* 2 */
		D3 = 51, /* 3 */
		D4 = 52, /* 4 */
		D5 = 53, /* 5 */
		D6 = 54, /* 6 */
		D7 = 55, /* 7 */
		D8 = 56, /* 8 */
		D9 = 57, /* 9 */

		Semicolon = 59, /* ; */
		Equal = 61, /* = */

		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,

		LeftBracket = 91,  /* [ */
		Backslash = 92,  /* \ */
		RightBracket = 93,  /* ] */
		GraveAccent = 96,  /* ` */

		World1 = 161, /* non-US #1 */
		World2 = 162, /* non-US #2 */

		/* Function keys */
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,

		/* Keypad */
		KP0 = 320,
		KP1 = 321,
		KP2 = 322,
		KP3 = 323,
		KP4 = 324,
		KP5 = 325,
		KP6 = 326,
		KP7 = 327,
		KP8 = 328,
		KP9 = 329,
		KPDecimal = 330,
		KPDivide = 331,
		KPMultiply = 332,
		KPSubtract = 333,
		KPAdd = 334,
		KPEnter = 335,
		KPEqual = 336,

		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348
	} Key;

	inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
	{
		os << static_cast<int32_t>(keyCode);
		return os;
	}
}

// From glfw3.h
#define KG_KEY_SPACE           ::Kargono::Key::Space
#define KG_KEY_APOSTROPHE      ::Kargono::Key::Apostrophe    /* ' */
#define KG_KEY_COMMA           ::Kargono::Key::Comma         /* , */
#define KG_KEY_MINUS           ::Kargono::Key::Minus         /* - */
#define KG_KEY_PERIOD          ::Kargono::Key::Period        /* . */
#define KG_KEY_SLASH           ::Kargono::Key::Slash         /* / */
#define KG_KEY_0               ::Kargono::Key::D0
#define KG_KEY_1               ::Kargono::Key::D1
#define KG_KEY_2               ::Kargono::Key::D2
#define KG_KEY_3               ::Kargono::Key::D3
#define KG_KEY_4               ::Kargono::Key::D4
#define KG_KEY_5               ::Kargono::Key::D5
#define KG_KEY_6               ::Kargono::Key::D6
#define KG_KEY_7               ::Kargono::Key::D7
#define KG_KEY_8               ::Kargono::Key::D8
#define KG_KEY_9               ::Kargono::Key::D9
#define KG_KEY_SEMICOLON       ::Kargono::Key::Semicolon     /* ; */
#define KG_KEY_EQUAL           ::Kargono::Key::Equal         /* = */
#define KG_KEY_A               ::Kargono::Key::A
#define KG_KEY_B               ::Kargono::Key::B
#define KG_KEY_C               ::Kargono::Key::C
#define KG_KEY_D               ::Kargono::Key::D
#define KG_KEY_E               ::Kargono::Key::E
#define KG_KEY_F               ::Kargono::Key::F
#define KG_KEY_G               ::Kargono::Key::G
#define KG_KEY_H               ::Kargono::Key::H
#define KG_KEY_I               ::Kargono::Key::I
#define KG_KEY_J               ::Kargono::Key::J
#define KG_KEY_K               ::Kargono::Key::K
#define KG_KEY_L               ::Kargono::Key::L
#define KG_KEY_M               ::Kargono::Key::M
#define KG_KEY_N               ::Kargono::Key::N
#define KG_KEY_O               ::Kargono::Key::O
#define KG_KEY_P               ::Kargono::Key::P
#define KG_KEY_Q               ::Kargono::Key::Q
#define KG_KEY_R               ::Kargono::Key::R
#define KG_KEY_S               ::Kargono::Key::S
#define KG_KEY_T               ::Kargono::Key::T
#define KG_KEY_U               ::Kargono::Key::U
#define KG_KEY_V               ::Kargono::Key::V
#define KG_KEY_W               ::Kargono::Key::W
#define KG_KEY_X               ::Kargono::Key::X
#define KG_KEY_Y               ::Kargono::Key::Y
#define KG_KEY_Z               ::Kargono::Key::Z
#define KG_KEY_LEFT_BRACKET    ::Kargono::Key::LeftBracket   /* [ */
#define KG_KEY_BACKSLASH       ::Kargono::Key::Backslash     /* \ */
#define KG_KEY_RIGHT_BRACKET   ::Kargono::Key::RightBracket  /* ] */
#define KG_KEY_GRAVE_ACCENT    ::Kargono::Key::GraveAccent   /* ` */
#define KG_KEY_WORLD_1         ::Kargono::Key::World1        /* non-US #1 */
#define KG_KEY_WORLD_2         ::Kargono::Key::World2        /* non-US #2 */

/* Function keys */
#define KG_KEY_ESCAPE          ::Kargono::Key::Escape
#define KG_KEY_ENTER           ::Kargono::Key::Enter
#define KG_KEY_TAB             ::Kargono::Key::Tab
#define KG_KEY_BACKSPACE       ::Kargono::Key::Backspace
#define KG_KEY_INSERT          ::Kargono::Key::Insert
#define KG_KEY_DELETE          ::Kargono::Key::Delete
#define KG_KEY_RIGHT           ::Kargono::Key::Right
#define KG_KEY_LEFT            ::Kargono::Key::Left
#define KG_KEY_DOWN            ::Kargono::Key::Down
#define KG_KEY_UP              ::Kargono::Key::Up
#define KG_KEY_PAGE_UP         ::Kargono::Key::PageUp
#define KG_KEY_PAGE_DOWN       ::Kargono::Key::PageDown
#define KG_KEY_HOME            ::Kargono::Key::Home
#define KG_KEY_END             ::Kargono::Key::End
#define KG_KEY_CAPS_LOCK       ::Kargono::Key::CapsLock
#define KG_KEY_SCROLL_LOCK     ::Kargono::Key::ScrollLock
#define KG_KEY_NUM_LOCK        ::Kargono::Key::NumLock
#define KG_KEY_PRINT_SCREEN    ::Kargono::Key::PrintScreen
#define KG_KEY_PAUSE           ::Kargono::Key::Pause
#define KG_KEY_F1              ::Kargono::Key::F1
#define KG_KEY_F2              ::Kargono::Key::F2
#define KG_KEY_F3              ::Kargono::Key::F3
#define KG_KEY_F4              ::Kargono::Key::F4
#define KG_KEY_F5              ::Kargono::Key::F5
#define KG_KEY_F6              ::Kargono::Key::F6
#define KG_KEY_F7              ::Kargono::Key::F7
#define KG_KEY_F8              ::Kargono::Key::F8
#define KG_KEY_F9              ::Kargono::Key::F9
#define KG_KEY_F10             ::Kargono::Key::F10
#define KG_KEY_F11             ::Kargono::Key::F11
#define KG_KEY_F12             ::Kargono::Key::F12
#define KG_KEY_F13             ::Kargono::Key::F13
#define KG_KEY_F14             ::Kargono::Key::F14
#define KG_KEY_F15             ::Kargono::Key::F15
#define KG_KEY_F16             ::Kargono::Key::F16
#define KG_KEY_F17             ::Kargono::Key::F17
#define KG_KEY_F18             ::Kargono::Key::F18
#define KG_KEY_F19             ::Kargono::Key::F19
#define KG_KEY_F20             ::Kargono::Key::F20
#define KG_KEY_F21             ::Kargono::Key::F21
#define KG_KEY_F22             ::Kargono::Key::F22
#define KG_KEY_F23             ::Kargono::Key::F23
#define KG_KEY_F24             ::Kargono::Key::F24
#define KG_KEY_F25             ::Kargono::Key::F25

/* Keypad */
#define KG_KEY_KP_0            ::Kargono::Key::KP0
#define KG_KEY_KP_1            ::Kargono::Key::KP1
#define KG_KEY_KP_2            ::Kargono::Key::KP2
#define KG_KEY_KP_3            ::Kargono::Key::KP3
#define KG_KEY_KP_4            ::Kargono::Key::KP4
#define KG_KEY_KP_5            ::Kargono::Key::KP5
#define KG_KEY_KP_6            ::Kargono::Key::KP6
#define KG_KEY_KP_7            ::Kargono::Key::KP7
#define KG_KEY_KP_8            ::Kargono::Key::KP8
#define KG_KEY_KP_9            ::Kargono::Key::KP9
#define KG_KEY_KP_DECIMAL      ::Kargono::Key::KPDecimal
#define KG_KEY_KP_DIVIDE       ::Kargono::Key::KPDivide
#define KG_KEY_KP_MULTIPLY     ::Kargono::Key::KPMultiply
#define KG_KEY_KP_SUBTRACT     ::Kargono::Key::KPSubtract
#define KG_KEY_KP_ADD          ::Kargono::Key::KPAdd
#define KG_KEY_KP_ENTER        ::Kargono::Key::KPEnter
#define KG_KEY_KP_EQUAL        ::Kargono::Key::KPEqual

#define KG_KEY_LEFT_SHIFT      ::Kargono::Key::LeftShift
#define KG_KEY_LEFT_CONTROL    ::Kargono::Key::LeftControl
#define KG_KEY_LEFT_ALT        ::Kargono::Key::LeftAlt
#define KG_KEY_LEFT_SUPER      ::Kargono::Key::LeftSuper
#define KG_KEY_RIGHT_SHIFT     ::Kargono::Key::RightShift
#define KG_KEY_RIGHT_CONTROL   ::Kargono::Key::RightControl
#define KG_KEY_RIGHT_ALT       ::Kargono::Key::RightAlt
#define KG_KEY_RIGHT_SUPER     ::Kargono::Key::RightSuper
#define KG_KEY_MENU            ::Kargono::Key::Menu
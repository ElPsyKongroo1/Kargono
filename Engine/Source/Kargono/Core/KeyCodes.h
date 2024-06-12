#pragma once

#include <string>
#include <array>

namespace Kargono
{
	using KeyCode = uint16_t;
}

namespace Kargono::Key
{
	enum : KeyCode
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
		Escape = 256, /* Function keys */
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
		KP0 = 320, /* Keypad */
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
	};

	inline std::array<uint16_t, 120> s_AllKeyCodes
	{
		32, 39, 44, 45, 46, 47, 48,
			49, 50, 51, 52, 53, 54, 55,
			56, 57, 59, 61, 65, 66, 67,
			68, 69, 70, 71, 72, 73, 74, 75,
			76, 77, 78, 79, 80, 81, 82, 83,
			84, 85, 86, 87, 88, 89, 90, 91,
			92, 93, 96, 161, 162, 256, 257, 258,
			259, 260, 261, 262, 263, 264, 265,
			266, 267, 268, 269, 280, 281, 282,
			283, 284, 290, 291, 292, 293, 294,
			295, 296, 297, 298, 299, 300, 301,
			302, 303, 304, 305, 306, 307, 308,
			309, 310, 311, 312, 313, 314, 320,
			321, 322, 323, 324, 325, 326, 327,
			328, 329, 330, 331, 332, 333, 334,
			335, 336, 340, 341, 342, 343, 344,
			345, 346, 347, 348
	};
}

namespace Kargono::Utility
{

	inline std::string KeyCodeToString(KeyCode key)
	{
		switch (key)
		{
				case Key::Space:				{return "Space"; }
				case Key::Apostrophe:			{return "Apostrophe"; }
				case Key::Comma:				{return "Comma"; }
				case Key::Minus:				{return "Minus"; }
				case Key::Period:				{return "Period"; }
				case Key::Slash:				{return "Slash"; }
				case Key::D0:					{return "D0"; }
				case Key::D1:					{return "D1"; }
				case Key::D2:					{return "D2"; }
				case Key::D3:					{return "D3"; }
				case Key::D4:					{return "D4"; }
				case Key::D5:					{return "D5"; }
				case Key::D6:					{return "D6"; }
				case Key::D7:					{return "D7"; }
				case Key::D8:					{return "D8"; }
				case Key::D9:					{return "D9"; }
				case Key::Semicolon:			{return "Semicolon"; }
				case Key::Equal:				{return "Equal"; }
				case Key::A:					{return "A"; }
				case Key::B:					{return "B"; }
				case Key::C:					{return "C"; }
				case Key::D:					{return "D"; }
				case Key::E:					{return "E"; }
				case Key::F:					{return "F"; }
				case Key::G:					{return "G"; }
				case Key::H:					{return "H"; }
				case Key::I:					{return "I"; }
				case Key::J:					{return "J"; }
				case Key::K:					{return "K"; }
				case Key::L:					{return "L"; }
				case Key::M:					{return "M"; }
				case Key::N:					{return "N"; }
				case Key::O:					{return "O"; }
				case Key::P:					{return "P"; }
				case Key::Q:					{return "Q"; }
				case Key::R:					{return "R"; }
				case Key::S:					{return "S"; }
				case Key::T:					{return "T"; }
				case Key::U:					{return "U"; }
				case Key::V:					{return "V"; }
				case Key::W:					{return "W"; }
				case Key::X:					{return "X"; }
				case Key::Y:					{return "Y"; }
				case Key::Z:					{return "Z"; }
				case Key::LeftBracket:			{return "LeftBracket"; }
				case Key::Backslash:			{return "Backslash"; }
				case Key::RightBracket:			{return "RightBracket"; }
				case Key::GraveAccent:			{return "GraveAccent"; }
				case Key::World1:				{return "World1"; }
				case Key::World2:				{return "World2"; }
				case Key::Escape:				{return "Escape"; }
				case Key::Enter:				{return "Enter"; }
				case Key::Tab:					{return "Tab"; }
				case Key::Backspace:			{return "Backspace"; }
				case Key::Insert:				{return "Insert"; }
				case Key::Delete:				{return "Delete"; }
				case Key::Right:				{return "Right"; }
				case Key::Left:					{return "Left"; }
				case Key::Down:					{return "Down"; }
				case Key::Up:					{return "Up"; }
				case Key::PageUp:				{return "PageUp"; }
				case Key::PageDown:				{return "PageDown"; }
				case Key::Home:					{return "Home"; }
				case Key::End:					{return "End"; }
				case Key::CapsLock:				{return "CapsLock"; }
				case Key::ScrollLock:			{return "ScrollLock"; }
				case Key::NumLock:				{return "NumLock"; }
				case Key::PrintScreen:			{return "PrintScreen"; }
				case Key::Pause:				{return "Pause"; }
				case Key::F1:					{return "F1"; }
				case Key::F2:					{return "F2"; }
				case Key::F3:					{return "F3"; }
				case Key::F4:					{return "F4"; }
				case Key::F5:					{return "F5"; }
				case Key::F6:					{return "F6"; }
				case Key::F7:					{return "F7"; }
				case Key::F8:					{return "F8"; }
				case Key::F9:					{return "F9"; }
				case Key::F10:					{return "F10"; }
				case Key::F11:					{return "F11"; }
				case Key::F12:					{return "F12"; }
				case Key::F13:					{return "F13"; }
				case Key::F14:					{return "F14"; }
				case Key::F15:					{return "F15"; }
				case Key::F16:					{return "F16"; }
				case Key::F17:					{return "F17"; }
				case Key::F18:					{return "F18"; }
				case Key::F19:					{return "F19"; }
				case Key::F20:					{return "F20"; }
				case Key::F21:					{return "F21"; }
				case Key::F22:					{return "F22"; }
				case Key::F23:					{return "F23"; }
				case Key::F24:					{return "F24"; }
				case Key::F25:					{return "F25"; }
				case Key::KP0:					{return "KP0"; }
				case Key::KP1:					{return "KP1"; }
				case Key::KP2:					{return "KP2"; }
				case Key::KP3:					{return "KP3"; }
				case Key::KP4:					{return "KP4"; }
				case Key::KP5:					{return "KP5"; }
				case Key::KP6:					{return "KP6"; }
				case Key::KP7:					{return "KP7"; }
				case Key::KP8:					{return "KP8"; }
				case Key::KP9:					{return "KP9"; }
				case Key::KPDecimal:			{return "KPDecimal"; }
				case Key::KPDivide:				{return "KPDivide"; }
				case Key::KPMultiply:			{return "KPMultiply"; }
				case Key::KPSubtract:			{return "KPSubtract"; }
				case Key::KPAdd:				{return "KPAdd"; }
				case Key::KPEnter:				{return "KPEnter"; }
				case Key::KPEqual:				{return "KPEqual"; }
				case Key::LeftShift:			{return "LeftShift"; }
				case Key::LeftControl:			{return "LeftControl"; }
				case Key::LeftAlt:				{return "LeftAlt"; }
				case Key::LeftSuper:			{return "LeftSuper"; }
				case Key::RightShift:			{return "RightShift"; }
				case Key::RightControl:			{return "RightControl"; }
				case Key::RightAlt:				{return "RightAlt"; }
				case Key::RightSuper:			{return "RightSuper"; }
				case Key::Menu:					{return "Menu";}
				default:						{return ""; }
		}
	}

	inline KeyCode StringToKeyCode(const std::string& codeString)
	{
		
		if (codeString == "Space")				{ return Key::Space; }
		if (codeString == "Apostrophe")			{ return Key::Apostrophe; }
		if (codeString == "Comma")				{ return Key::Comma; }
		if (codeString == "Minus")				{ return Key::Minus; }
		if (codeString == "Period")				{ return Key::Period; }
		if (codeString == "Slash")				{ return Key::Slash; }
		if (codeString == "D0")					{ return Key::D0; }
		if (codeString == "D1")					{ return Key::D1; }
		if (codeString == "D2")					{ return Key::D2; }
		if (codeString == "D3")					{ return Key::D3; }
		if (codeString == "D4")					{ return Key::D4; }
		if (codeString == "D5")					{ return Key::D5; }
		if (codeString == "D6")					{ return Key::D6; }
		if (codeString == "D7")					{ return Key::D7; }
		if (codeString == "D8")					{ return Key::D8; }
		if (codeString == "D9")					{ return Key::D9; }
		if (codeString == "Semicolon")			{ return Key::Semicolon; }
		if (codeString == "Equal")				{ return Key::Equal; }
		if (codeString == "A")					{ return Key::A; }
		if (codeString == "B")					{ return Key::B; }
		if (codeString == "C")					{ return Key::C; }
		if (codeString == "D")					{ return Key::D; }
		if (codeString == "E")					{ return Key::E; }
		if (codeString == "F")					{ return Key::F; }
		if (codeString == "G")					{ return Key::G; }
		if (codeString == "H")					{ return Key::H; }
		if (codeString == "I")					{ return Key::I; }
		if (codeString == "J")					{ return Key::J; }
		if (codeString == "K")					{ return Key::K; }
		if (codeString == "L")					{ return Key::L; }
		if (codeString == "M")					{ return Key::M; }
		if (codeString == "N")					{ return Key::N; }
		if (codeString == "O")					{ return Key::O; }
		if (codeString == "P")					{ return Key::P; }
		if (codeString == "Q")					{ return Key::Q; }
		if (codeString == "R")					{ return Key::R; }
		if (codeString == "S")					{ return Key::S; }
		if (codeString == "T")					{ return Key::T; }
		if (codeString == "U")					{ return Key::U; }
		if (codeString == "V")					{ return Key::V; }
		if (codeString == "W")					{ return Key::W; }
		if (codeString == "X")					{ return Key::X; }
		if (codeString == "Y")					{ return Key::Y; }
		if (codeString == "Z")					{ return Key::Z; }
		if (codeString == "LeftBracket")		{ return Key::LeftBracket; }
		if (codeString == "Backslash")			{ return Key::Backslash; }
		if (codeString == "RightBracket")		{ return Key::RightBracket; }
		if (codeString == "GraveAccent")		{ return Key::GraveAccent; }
		if (codeString == "World1")				{ return Key::World1; }
		if (codeString == "World2")				{ return Key::World2; }
		if (codeString == "Escape")				{ return Key::Escape; }
		if (codeString == "Enter")				{ return Key::Enter; }
		if (codeString == "Tab")				{ return Key::Tab; }
		if (codeString == "Backspace")			{ return Key::Backspace; }
		if (codeString == "Insert")				{ return Key::Insert; }
		if (codeString == "Delete")				{ return Key::Delete; }
		if (codeString == "Right")				{ return Key::Right; }
		if (codeString == "Left")				{ return Key::Left; }
		if (codeString == "Down")				{ return Key::Down; }
		if (codeString == "Up")					{ return Key::Up; }
		if (codeString == "PageUp")				{ return Key::PageUp; }
		if (codeString == "PageDown")			{ return Key::PageDown; }
		if (codeString == "Home")				{ return Key::Home; }
		if (codeString == "End")				{ return Key::End; }
		if (codeString == "CapsLock")			{ return Key::CapsLock; }
		if (codeString == "ScrollLock")			{ return Key::ScrollLock; }
		if (codeString == "NumLock")			{ return Key::NumLock; }
		if (codeString == "PrintScreen")		{ return Key::PrintScreen; }
		if (codeString == "Pause")				{ return Key::Pause; }
		if (codeString == "F1")					{ return Key::F1; }
		if (codeString == "F2")					{ return Key::F2; }
		if (codeString == "F3")					{ return Key::F3; }
		if (codeString == "F4")					{ return Key::F4; }
		if (codeString == "F5")					{ return Key::F5; }
		if (codeString == "F6")					{ return Key::F6; }
		if (codeString == "F7")					{ return Key::F7; }
		if (codeString == "F8")					{ return Key::F8; }
		if (codeString == "F9")					{ return Key::F9; }
		if (codeString == "F10")				{ return Key::F10; }
		if (codeString == "F11")				{ return Key::F11; }
		if (codeString == "F12")				{ return Key::F12; }
		if (codeString == "F13")				{ return Key::F13; }
		if (codeString == "F14")				{ return Key::F14; }
		if (codeString == "F15")				{ return Key::F15; }
		if (codeString == "F16")				{ return Key::F16; }
		if (codeString == "F17")				{ return Key::F17; }
		if (codeString == "F18")				{ return Key::F18; }
		if (codeString == "F19")				{ return Key::F19; }
		if (codeString == "F20")				{ return Key::F20; }
		if (codeString == "F21")				{ return Key::F21; }
		if (codeString == "F22")				{ return Key::F22; }
		if (codeString == "F23")				{ return Key::F23; }
		if (codeString == "F24")				{ return Key::F24; }
		if (codeString == "F25")				{ return Key::F25; }
		if (codeString == "KP0")				{ return Key::KP0; }
		if (codeString == "KP1")				{ return Key::KP1; }
		if (codeString == "KP2")				{ return Key::KP2; }
		if (codeString == "KP3")				{ return Key::KP3; }
		if (codeString == "KP4")				{ return Key::KP4; }
		if (codeString == "KP5")				{ return Key::KP5; }
		if (codeString == "KP6")				{ return Key::KP6; }
		if (codeString == "KP7")				{ return Key::KP7; }
		if (codeString == "KP8")				{ return Key::KP8; }
		if (codeString == "KP9")				{ return Key::KP9; }
		if (codeString == "KPDecimal")			{ return Key::KPDecimal; }
		if (codeString == "KPDivide")			{ return Key::KPDivide; }
		if (codeString == "KPMultiply")			{ return Key::KPMultiply; }
		if (codeString == "KPSubtract")			{ return Key::KPSubtract; }
		if (codeString == "KPAdd")				{ return Key::KPAdd; }
		if (codeString == "KPEnter")			{ return Key::KPEnter; }
		if (codeString == "KPEqual")			{ return Key::KPEqual; }
		if (codeString == "LeftShift")			{ return Key::LeftShift; }
		if (codeString == "LeftControl")		{ return Key::LeftControl; }
		if (codeString == "LeftAlt")			{ return Key::LeftAlt; }
		if (codeString == "LeftSuper")			{ return Key::LeftSuper; }
		if (codeString == "RightShift")			{ return Key::RightShift; }
		if (codeString == "RightControl")		{ return Key::RightControl; }
		if (codeString == "RightAlt")			{ return Key::RightAlt; }
		if (codeString == "RightSuper")			{ return Key::RightSuper; }
		if (codeString == "Menu")				{ return Key::Menu;}
	}
	
}

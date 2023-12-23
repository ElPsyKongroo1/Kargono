#pragma once

#include <string>
#include <array>

namespace Kargono
{
	using KeyCode = uint16_t;

	namespace Key
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

		inline std::string KeyCodeToString(KeyCode key)
		{
			switch (key)
			{
					case Space:				{return "Space"; }
					case Apostrophe:		{return "Apostrophe"; }
					case Comma:				{return "Comma"; }
					case Minus:				{return "Minus"; }
					case Period:			{return "Period"; }
					case Slash:				{return "Slash"; }
					case D0:				{return "D0"; }
					case D1:				{return "D1"; }
					case D2:				{return "D2"; }
					case D3:				{return "D3"; }
					case D4:				{return "D4"; }
					case D5:				{return "D5"; }
					case D6:				{return "D6"; }
					case D7:				{return "D7"; }
					case D8:				{return "D8"; }
					case D9:				{return "D9"; }
					case Semicolon:			{return "Semicolon"; }
					case Equal:				{return "Equal"; }
					case A:					{return "A"; }
					case B:					{return "B"; }
					case C:					{return "C"; }
					case D:					{return "D"; }
					case E:					{return "E"; }
					case F:					{return "F"; }
					case G:					{return "G"; }
					case H:					{return "H"; }
					case I:					{return "I"; }
					case J:					{return "J"; }
					case K:					{return "K"; }
					case L:					{return "L"; }
					case M:					{return "M"; }
					case N:					{return "N"; }
					case O:					{return "O"; }
					case P:					{return "P"; }
					case Q:					{return "Q"; }
					case R:					{return "R"; }
					case S:					{return "S"; }
					case T:					{return "T"; }
					case U:					{return "U"; }
					case V:					{return "V"; }
					case W:					{return "W"; }
					case X:					{return "X"; }
					case Y:					{return "Y"; }
					case Z:					{return "Z"; }
					case LeftBracket:		{return "LeftBracket"; }
					case Backslash:			{return "Backslash"; }
					case RightBracket:		{return "RightBracket"; }
					case GraveAccent:		{return "GraveAccent"; }
					case World1:			{return "World1"; }
					case World2:			{return "World2"; }
					case Escape:			{return "Escape"; }
					case Enter:				{return "Enter"; }
					case Tab:				{return "Tab"; }
					case Backspace:			{return "Backspace"; }
					case Insert:			{return "Insert"; }
					case Delete:			{return "Delete"; }
					case Right:				{return "Right"; }
					case Left:				{return "Left"; }
					case Down:				{return "Down"; }
					case Up:				{return "Up"; }
					case PageUp:			{return "PageUp"; }
					case PageDown:			{return "PageDown"; }
					case Home:				{return "Home"; }
					case End:				{return "End"; }
					case CapsLock:			{return "CapsLock"; }
					case ScrollLock:		{return "ScrollLock"; }
					case NumLock:			{return "NumLock"; }
					case PrintScreen:		{return "PrintScreen"; }
					case Pause:				{return "Pause"; }
					case F1:				{return "F1"; }
					case F2:				{return "F2"; }
					case F3:				{return "F3"; }
					case F4:				{return "F4"; }
					case F5:				{return "F5"; }
					case F6:				{return "F6"; }
					case F7:				{return "F7"; }
					case F8:				{return "F8"; }
					case F9:				{return "F9"; }
					case F10:				{return "F10"; }
					case F11:				{return "F11"; }
					case F12:				{return "F12"; }
					case F13:				{return "F13"; }
					case F14:				{return "F14"; }
					case F15:				{return "F15"; }
					case F16:				{return "F16"; }
					case F17:				{return "F17"; }
					case F18:				{return "F18"; }
					case F19:				{return "F19"; }
					case F20:				{return "F20"; }
					case F21:				{return "F21"; }
					case F22:				{return "F22"; }
					case F23:				{return "F23"; }
					case F24:				{return "F24"; }
					case F25:				{return "F25"; }
					case KP0:				{return "KP0"; }
					case KP1:				{return "KP1"; }
					case KP2:				{return "KP2"; }
					case KP3:				{return "KP3"; }
					case KP4:				{return "KP4"; }
					case KP5:				{return "KP5"; }
					case KP6:				{return "KP6"; }
					case KP7:				{return "KP7"; }
					case KP8:				{return "KP8"; }
					case KP9:				{return "KP9"; }
					case KPDecimal:			{return "KPDecimal"; }
					case KPDivide:			{return "KPDivide"; }
					case KPMultiply:		{return "KPMultiply"; }
					case KPSubtract:		{return "KPSubtract"; }
					case KPAdd:				{return "KPAdd"; }
					case KPEnter:			{return "KPEnter"; }
					case KPEqual:			{return "KPEqual"; }
					case LeftShift:			{return "LeftShift"; }
					case LeftControl:		{return "LeftControl"; }
					case LeftAlt:			{return "LeftAlt"; }
					case LeftSuper:			{return "LeftSuper"; }
					case RightShift:		{return "RightShift"; }
					case RightControl:		{return "RightControl"; }
					case RightAlt:			{return "RightAlt"; }
					case RightSuper:		{return "RightSuper"; }
					case Menu:				{return "Menu";}
					default:				{return ""; }
			}
		}

		inline KeyCode StringToKeyCode(const std::string& codeString)
		{
			
			if (codeString == "Space")				{ return Space; }
			if (codeString == "Apostrophe")			{ return Apostrophe; }
			if (codeString == "Comma")				{ return Comma; }
			if (codeString == "Minus")				{ return Minus; }
			if (codeString == "Period")				{ return Period; }
			if (codeString == "Slash")				{ return Slash; }
			if (codeString == "D0")					{ return D0; }
			if (codeString == "D1")					{ return D1; }
			if (codeString == "D2")					{ return D2; }
			if (codeString == "D3")					{ return D3; }
			if (codeString == "D4")					{ return D4; }
			if (codeString == "D5")					{ return D5; }
			if (codeString == "D6")					{ return D6; }
			if (codeString == "D7")					{ return D7; }
			if (codeString == "D8")					{ return D8; }
			if (codeString == "D9")					{ return D9; }
			if (codeString == "Semicolon")			{ return Semicolon; }
			if (codeString == "Equal")				{ return Equal; }
			if (codeString == "A")					{ return A; }
			if (codeString == "B")					{ return B; }
			if (codeString == "C")					{ return C; }
			if (codeString == "D")					{ return D; }
			if (codeString == "E")					{ return E; }
			if (codeString == "F")					{ return F; }
			if (codeString == "G")					{ return G; }
			if (codeString == "H")					{ return H; }
			if (codeString == "I")					{ return I; }
			if (codeString == "J")					{ return J; }
			if (codeString == "K")					{ return K; }
			if (codeString == "L")					{ return L; }
			if (codeString == "M")					{ return M; }
			if (codeString == "N")					{ return N; }
			if (codeString == "O")					{ return O; }
			if (codeString == "P")					{ return P; }
			if (codeString == "Q")					{ return Q; }
			if (codeString == "R")					{ return R; }
			if (codeString == "S")					{ return S; }
			if (codeString == "T")					{ return T; }
			if (codeString == "U")					{ return U; }
			if (codeString == "V")					{ return V; }
			if (codeString == "W")					{ return W; }
			if (codeString == "X")					{ return X; }
			if (codeString == "Y")					{ return Y; }
			if (codeString == "Z")					{ return Z; }
			if (codeString == "LeftBracket")		{ return LeftBracket; }
			if (codeString == "Backslash")			{ return Backslash; }
			if (codeString == "RightBracket")		{ return RightBracket; }
			if (codeString == "GraveAccent")		{ return GraveAccent; }
			if (codeString == "World1")				{ return World1; }
			if (codeString == "World2")				{ return World2; }
			if (codeString == "Escape")				{ return Escape; }
			if (codeString == "Enter")				{ return Enter; }
			if (codeString == "Tab")				{ return Tab; }
			if (codeString == "Backspace")			{ return Backspace; }
			if (codeString == "Insert")				{ return Insert; }
			if (codeString == "Delete")				{ return Delete; }
			if (codeString == "Right")				{ return Right; }
			if (codeString == "Left")				{ return Left; }
			if (codeString == "Down")				{ return Down; }
			if (codeString == "Up")					{ return Up; }
			if (codeString == "PageUp")				{ return PageUp; }
			if (codeString == "PageDown")			{ return PageDown; }
			if (codeString == "Home")				{ return Home; }
			if (codeString == "End")				{ return End; }
			if (codeString == "CapsLock")			{ return CapsLock; }
			if (codeString == "ScrollLock")			{ return ScrollLock; }
			if (codeString == "NumLock")			{ return NumLock; }
			if (codeString == "PrintScreen")		{ return PrintScreen; }
			if (codeString == "Pause")				{ return Pause; }
			if (codeString == "F1")					{ return F1; }
			if (codeString == "F2")					{ return F2; }
			if (codeString == "F3")					{ return F3; }
			if (codeString == "F4")					{ return F4; }
			if (codeString == "F5")					{ return F5; }
			if (codeString == "F6")					{ return F6; }
			if (codeString == "F7")					{ return F7; }
			if (codeString == "F8")					{ return F8; }
			if (codeString == "F9")					{ return F9; }
			if (codeString == "F10")				{ return F10; }
			if (codeString == "F11")				{ return F11; }
			if (codeString == "F12")				{ return F12; }
			if (codeString == "F13")				{ return F13; }
			if (codeString == "F14")				{ return F14; }
			if (codeString == "F15")				{ return F15; }
			if (codeString == "F16")				{ return F16; }
			if (codeString == "F17")				{ return F17; }
			if (codeString == "F18")				{ return F18; }
			if (codeString == "F19")				{ return F19; }
			if (codeString == "F20")				{ return F20; }
			if (codeString == "F21")				{ return F21; }
			if (codeString == "F22")				{ return F22; }
			if (codeString == "F23")				{ return F23; }
			if (codeString == "F24")				{ return F24; }
			if (codeString == "F25")				{ return F25; }
			if (codeString == "KP0")				{ return KP0; }
			if (codeString == "KP1")				{ return KP1; }
			if (codeString == "KP2")				{ return KP2; }
			if (codeString == "KP3")				{ return KP3; }
			if (codeString == "KP4")				{ return KP4; }
			if (codeString == "KP5")				{ return KP5; }
			if (codeString == "KP6")				{ return KP6; }
			if (codeString == "KP7")				{ return KP7; }
			if (codeString == "KP8")				{ return KP8; }
			if (codeString == "KP9")				{ return KP9; }
			if (codeString == "KPDecimal")			{ return KPDecimal; }
			if (codeString == "KPDivide")			{ return KPDivide; }
			if (codeString == "KPMultiply")			{ return KPMultiply; }
			if (codeString == "KPSubtract")			{ return KPSubtract; }
			if (codeString == "KPAdd")				{ return KPAdd; }
			if (codeString == "KPEnter")			{ return KPEnter; }
			if (codeString == "KPEqual")			{ return KPEqual; }
			if (codeString == "LeftShift")			{ return LeftShift; }
			if (codeString == "LeftControl")		{ return LeftControl; }
			if (codeString == "LeftAlt")			{ return LeftAlt; }
			if (codeString == "LeftSuper")			{ return LeftSuper; }
			if (codeString == "RightShift")			{ return RightShift; }
			if (codeString == "RightControl")		{ return RightControl; }
			if (codeString == "RightAlt")			{ return RightAlt; }
			if (codeString == "RightSuper")			{ return RightSuper; }
			if (codeString == "Menu")				{ return Menu;}
		}
	}
}

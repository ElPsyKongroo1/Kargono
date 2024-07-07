#pragma once
#include <string>
#include <algorithm>

namespace Kargono::Utility
{
	//==============================
	// General Operations Class
	//==============================
	class Operations
	{
	public:
		//==============================
		// Boolean Operations
		//==============================
		static void ToggleBoolean(bool& boolToToggle)
		{
			boolToToggle = !boolToToggle;
		}

		static void RemoveCharacterFromString(std::string& string, char character)
		{
			std::erase(string, character);
		}
	};
}

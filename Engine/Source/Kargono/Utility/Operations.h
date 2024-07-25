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
		template <typename T>
		static T RoundDown(T valueToRound, T multipleOf)
		{
			// Avoid division by zero
			if (multipleOf == 0)
			{
				return valueToRound; 
			}
			return valueToRound - (valueToRound % multipleOf);
		}
		template <typename T>
		static T RoundUp(T valueToRound, T multipleOf) {
			// Avoid division by zero
			if (multipleOf == 0)
			{
				return valueToRound;
			}
			T remainder = valueToRound % multipleOf;

			// If valueToRound is already a multiple of multipleOf
			if (remainder == 0)
			{
				return valueToRound; 
			}
			return valueToRound + (multipleOf - remainder);
		}
	};
}

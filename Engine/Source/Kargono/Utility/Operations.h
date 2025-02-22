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

		//==============================
		// String Operations
		//==============================
		static void RemoveCharacterFromString(std::string& string, char character)
		{
			std::erase(string, character);
		}

		static void RemoveWhitespaceFromString(std::string& string) 
		{
			std::erase_if(string, [](unsigned char character)
			{
				return std::isspace(character); 
			});
		}

		static std::string_view GetSubstringToCharacter(const std::string& inputString, char queryCharacter)
		{
			// Find the position of the first occurrence of queryCharacter
			size_t position = inputString.find(queryCharacter);

			// If the character is not found, return the entire string
			if (position == std::string::npos) 
			{
				return std::string_view(inputString);
			}

			// Otherwise, return a substring view up to the found position
			return std::string_view(inputString.data(), position);
		}

		//==============================
		// Integer Operations
		//==============================
		template <typename T>
		static T RoundDown(T valueToRound, T multipleOf)
		{
			// Avoid division by zero
			if (multipleOf == 0)
			{
				return valueToRound;
			}

			// Calculate the remainder
			T remainder = valueToRound % multipleOf;

			// If the remainder is not zero and the value is negative, adjust the value
			if (remainder != 0 && valueToRound < 0)
			{
				remainder += multipleOf;
			}

			return valueToRound - remainder;
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
